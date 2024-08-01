
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include "stream_buffer.h"
#include <event_groups.h>

#include <utils_crc.h>
#include <spisync.h>
#include <ramsync.h>
#include <spisync_port.h>

#define UINT32_DIFF(a, b) ((uint32_t)((int32_t)(a) - (int32_t)(b)))
#define INT32_DIFF(a, b)  (( int32_t)((int32_t)(a) - (int32_t)(b)))

spisync_t *g_spisync_current = NULL;// for dump

/*--------------------- internal process module ----------------------*/

#if SPISYNC_MASTER_ENABLE
#else
void spisync_s2m_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    qcc74x_gpio_init(gpio, SPISYNC_CTRL_PIN, GPIO_OUTPUT | GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);

    qcc74x_gpio_set(gpio, SPISYNC_CTRL_PIN);
}

void spisyn_s2m_set()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    qcc74x_gpio_set(gpio, SPISYNC_CTRL_PIN);
}

void spisync_s2m_reset()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    qcc74x_gpio_reset(gpio, SPISYNC_CTRL_PIN);
}

bool spisync_s2m_get()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    return qcc74x_gpio_read(gpio, SPISYNC_CTRL_PIN);
}
#endif

static void spisync_task(void *arg);
static void _spisync_setevent(spisync_t *spisync, uint32_t evt)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (xPortIsInsideInterrupt()) {
        xEventGroupSetBitsFromISR(spisync->event, evt, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        xEventGroupSetBits(spisync->event, evt);
    }
}

static void _spisync_pattern_process(spisync_t *spisync)
{
    int res;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (xPortIsInsideInterrupt()) {
        if (xTimerStartFromISR(spisync->timer, &xHigherPriorityTaskWoken) != pdPASS) {
            printf("Failed to start timer!\r\n");
        }
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        res = xTimerStart(spisync->timer, 1000);
        if (res != pdPASS) {
            printf("Failed to start timer!\r\n");
        }
    }
}

static void debug_init_reset(spisync_t *spisync, char init)// 1 init, 0 reset
{
    /* debug */
    spisync->isr_rxcnt        = 0;
    spisync->isr_txcnt        = 0;
    if (init) {
        spisync->tsk_rstcnt   = 0;
    }
    spisync->tsk_ctrlcnt      = 0;
    spisync->tsk_writecnt     = 0;
    spisync->tsk_readcnt      = 0;
    spisync->tsk_rsttick      = xTaskGetTickCount();

    spisync->iperf            = 0;
    spisync->dbg_tick_old     = xTaskGetTickCount();
    spisync->isr_txcnt_old    = 0;
    spisync->isr_rxcnt_old    = 0;
    spisync->tsk_writecnt_old = 0;
    spisync->tsk_readcnt_old  = 0;
}

static void debug_iperf_print(spisync_t *spisync)
{
    uint32_t diff_tick;
    uint32_t cur_tick     = xTaskGetTickCount();
    uint32_t isr_rxcnt    = spisync->isr_rxcnt;
    uint32_t isr_txcnt    = spisync->isr_txcnt;
    uint32_t tsk_writecnt = spisync->tsk_writecnt;
    uint32_t tsk_readcnt  = spisync->tsk_readcnt;

    if (!spisync->iperf) {
        return;
    }

    diff_tick = UINT32_DIFF(cur_tick, spisync->dbg_tick_old);
    if (diff_tick >= 1000) {
        spisync_log("RX(tsk:%f bps, usage:%d, %d pak, %d ms)    TX(tsk:%f bps, usage:%d, %d pak, %d ms)\r\n",
                ((float)((tsk_readcnt - spisync->tsk_readcnt_old)*SPISYNC_PAYLOADBUF_LEN*8))/(((float)diff_tick)/1000),
                ((tsk_readcnt - spisync->tsk_readcnt_old) * 100)/(isr_rxcnt - spisync->isr_rxcnt_old),
                (tsk_readcnt - spisync->tsk_readcnt_old), diff_tick,
                ((float)((tsk_writecnt - spisync->tsk_writecnt_old)*SPISYNC_PAYLOADBUF_LEN*8))/(((float)diff_tick)/1000),
                ((tsk_writecnt - spisync->tsk_writecnt_old) * 100)/(isr_txcnt - spisync->isr_txcnt_old),
                (tsk_writecnt - spisync->tsk_writecnt_old), diff_tick
              );
        spisync->dbg_tick_old     = cur_tick;
        spisync->isr_rxcnt_old    = isr_rxcnt;
        spisync->isr_txcnt_old    = isr_txcnt;
        spisync->tsk_writecnt_old = tsk_writecnt;
        spisync->tsk_readcnt_old  = tsk_readcnt;
    }
}

static void _reset_spisync(spisync_t *spisync)
{
    /* reset low */
#if SPISYNC_MASTER_ENABLE
#else
	lramsync_reset(&spisync->hw);
#endif

    /* slot */
    memset(spisync->p_tx, 0, sizeof(spisync_txbuf_t));
    memset(spisync->p_rx, 0, sizeof(spisync_rxbuf_t));
    for (int i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        spisync->p_tx->slot[i].tag.magic = SPISYNC_SLOT_MAGIC;
        spisync->p_tx->slot[i].tag.clamp[0] = SPISYNC_RX0_STREAMBUFFER_SIZE;
        spisync->p_tx->slot[i].tag.clamp[1] = SPISYNC_RX1_STREAMBUFFER_SIZE;
        spisync->p_tx->slot[i].tag.clamp[2] = SPISYNC_RX2_STREAMBUFFER_SIZE;
    }

    /* local sequence */
    spisync->tx_seq             = 0;
    spisync->txtag_errtimes     = 0;
    spisync->txwrite_continue   = 0;
    spisync->rxpattern_checked  = 0;
    spisync->rxread_continue    = 0;
    spisync->clamp[0]           = 0;
    spisync->clamp[1]           = 0;
    spisync->clamp[2]           = 0;
#if SPISYNC_MASTER_ENABLE
    spisync->printed_err_reason = 0;
    spisync->reset_pending = 0;
#endif

    /* for rx notify */
    xEventGroupClearBits(spisync->event, EVT_SPISYNC_ALL_BIT);

    /* for timer stop */
    /* Starting a timer ensures the timer is in the active state.  If the timer
     * is not stopped, deleted, or reset in the mean time, the callback function
     * associated with the timer will get called 'n' ticks after xTimerStart() was
     * called, where 'n' is the timers defined period.
     *
     * xTimerStart(...)
     */
    //xTimerReset(spisync->timer, 0);
    //xTimerStop(spisync->timer, 0);

    xStreamBufferReset(spisync->ptx_streambuffer[0]);
    xStreamBufferReset(spisync->ptx_streambuffer[1]);
    xStreamBufferReset(spisync->ptx_streambuffer[2]);
    xStreamBufferReset(spisync->prx_streambuffer[0]);
    xStreamBufferReset(spisync->prx_streambuffer[1]);
    xStreamBufferReset(spisync->prx_streambuffer[2]);

    /* calulate crc for rx */
    memset(spisync->p_rxcache, 0, sizeof(spisync_slot_t));

    /* debug */
    debug_init_reset(spisync, 0);// 1 init, 0 reset

    /* start low */
#if SPISYNC_MASTER_ENABLE
#else
    lramsync_start(&spisync->hw);
#endif
}

static int __check_tag(spisync_t *spisync)
{
    for (int i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        if (SPISYNC_SLOT_MAGIC != spisync->p_rx->slot[i].tag.magic) {
            return 1;
        }
    }
    return 0;
}
static int __check_pattern(uint32_t *data, uint32_t data_len)
{
    if (((data[0] == 0x55555555) && (data[1] == 0x55555555) && (data[2] == 0x55555555) &&
         (data[3] == 0x55555555) && (data[4] == 0x55555555) && (data[5] == 0x55555555)) ||
        ((data[0] == 0xAAAAAAAA) && (data[1] == 0xAAAAAAAA) && (data[2] == 0xAAAAAAAA) &&
         (data[3] == 0xAAAAAAAA) && (data[4] == 0xAAAAAAAA) && (data[5] == 0xAAAAAAAA))
        ) {
        return 1;
    }
    return 0;
}
#if SPISYNC_MASTER_ENABLE
static int process_slot_check(spisync_t *spisync)
{
    int i, error = 0;
    char *reason = "unknown";

    if (spisync->state != SPISYNC_STATE_NORMAL)
    	return 0;

    for (i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        if (SPISYNC_SLOT_MAGIC != spisync->p_rx->slot[i].tag.magic) {
        	spisync->txtag_errtimes++;
            if (spisync->txtag_errtimes >= SPISYNC_TAGERR_TIMES) {
            	error = 1;
            	reason = "error magic";
            	break;
            }
        } else if (spisync->p_rx->slot[i].tag.flag) {
        	error = 1;
        	reason = "slave set reset flag";
        	break;
        } else {
        	//spisync->txtag_errtimes = 0;
        }
    }

	if (error && !spisync->printed_err_reason) {
		spisync->printed_err_reason = 1;
		spisync_log("spisync needs reset, %s, slot %d magic 0x%x, flag 0x%lx, err time %d\r\n",
				reason, i, spisync->p_rx->slot[i].tag.magic,
				spisync->p_rx->slot[i].tag.flag, spisync->txtag_errtimes);
	}
    return error;
}
static void spisync_start_reset(spisync_t *spisync)
{
	if (!spisync->master)
		return;

	/* test_and_set_bit is preferred to prevent repeated trigger */
	if (spisync->reset_pending)
		return;

	spisync->reset_pending = 1;;
	_spisync_setevent(spisync, EVT_SPISYNC_RESET_BIT);
}
#else
static int process_slot_check(spisync_t *spisync)
{
    int res = 0;

    // tag check
    if (__check_tag(spisync)) {
        spisync->txtag_errtimes++;
        if (spisync->txtag_errtimes > SPISYNC_TAGERR_TIMES) {
            for (int i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
                spisync->p_tx->slot[i].tag.flag |= 0x1;// set rx flag err by tx
            }
            res = 1;
        }
    } else {
        spisync->txtag_errtimes = 0;
    }

    // pattern check
    if (__check_pattern((uint32_t *)(&spisync->p_rx->slot[0]), sizeof(spisync_slot_t))) {
        spisync->rxpattern_checked++;
        spisync_trace("pattern_checked:%d\r\n", spisync->rxpattern_checked);
        if (spisync->rxpattern_checked == SPISYNC_PATTERN_TIMES) {
            spisync_log("check pattern times:%ld, delay to %ld ms reset\r\n",
                spisync->rxpattern_checked, SPISYNC_PATTERN_DELAYMS);
            _spisync_pattern_process(spisync);
        }

        return 1;
    } else {
        if (spisync->rxpattern_checked) {
            printf("rxpattern_checked:%ld, throhold:%ld\r\n", spisync->rxpattern_checked, SPISYNC_PATTERN_TIMES);
            spisync->rxpattern_checked = 0;
        }
    }

    return res;
}
#endif

static void __ramsync_low_rx_cb(spisync_t *spisync)
{
    /* debug */
    if (spisync) {
        spisync->isr_rxcnt++;
    }

    if (process_slot_check(spisync)) {
#if SPISYNC_MASTER_ENABLE
		spisync_start_reset(spisync);
#endif
        return;// todo : need?
    }
#if 1
    // Trigger a read event every time
    _spisync_setevent(spisync, EVT_SPISYNC_READ_BIT);
#else
    // Trigger a read event by (continue || (rseq == seq) )
    if ((spisync->rxread_continue)) {// todo: add ||read vaild ?
        _spisync_setevent(spisync, EVT_SPISYNC_READ_BIT);
        return;
    }
    for (int i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        if ((spisync->p_tx->slot[i].tag.rseq + 1) == spisync->p_rx->slot[i].payload.seq) {
            _spisync_setevent(spisync, EVT_SPISYNC_READ_BIT);
            return;
        }
    }
#endif
}

static void __ramsync_low_tx_cb(void *arg)
{
    spisync_t *spisync = (spisync_t *)arg;
    int i;
    int have_empty_slot = 0;

    /* debug */
    if (spisync) {
        spisync->isr_txcnt++;
    }
#if 1
    _spisync_setevent(spisync, EVT_SPISYNC_WRITE_BIT);
#else
    if (spisync->txwrite_continue) {
        /* Check if there is an empty slot */
        for (i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
            if (0 < (SPISYNC_TXSLOT_COUNT -
                    (UINT32_DIFF(spisync->p_tx->slot[i].payload.seq,
                                spisync->p_rx->slot[i].tag.rseq)))) {
                have_empty_slot = 1;
                break;
            }
        }

        if (have_empty_slot > 0) {
            //_spisync_setevent(spisync, EVT_SPISYNC_WRITE_BIT);
        }
    }
    _spisync_setevent(spisync, EVT_SPISYNC_WRITE_BIT);
#endif
}

static void __ramsync_low_reset_cb(void *arg)
{
    spisync_t *spisync = (spisync_t *)arg;

    if (spisync->config && spisync->config->reset_cb) {
        spisync->config->reset_cb(spisync->config->reset_arg);
    }
}

void timer_callback(TimerHandle_t xTimer)
{
    spisync_t *spisync = (spisync_t *)pvTimerGetTimerID(xTimer);

#if SPISYNC_MASTER_ENABLE
    spisync_log("timer callback sets reset bit\r\n");
#endif
    _spisync_setevent(spisync, EVT_SPISYNC_RESET_BIT);
}

int spisync_wakeup(spisync_t *spisync)
{
    if ((NULL == spisync) && (NULL == g_spisync_current)) {
        return -1;
    } else if (NULL == spisync) {
        spisync = g_spisync_current;
    }

#ifdef LP_APP
    extern int enable_tickless;
    enable_tickless = 0;
#endif

    spisync->ps_status = 0;

    _spisync_setevent(spisync, EVT_SPISYNC_WAKEUP_BIT);
    return 0;
}

int spisync_status_get(spisync_t *spisync)
{
    if ((NULL == spisync) && (NULL == g_spisync_current)) {
        return -1;
    } else if (NULL == spisync) {
        spisync = g_spisync_current;
    }

    //SPISYNC_IDLE, // IDLE
    //SPISYNC_BUSY, // BUSY
 
    return 0;
}

int spisync_ps_enter(spisync_t *spisync)
{
    if ((NULL == spisync) && (NULL == g_spisync_current)) {
        return -1;
    } else if (NULL == spisync) {
        spisync = g_spisync_current;
    }

    spisync->ps_status = 1;
    return 0;
}

int spisync_ps_exit(spisync_t *spisync)
{
    if ((NULL == spisync) && (NULL == g_spisync_current)) {
        return -1;
    } else if (NULL == spisync) {
        spisync = g_spisync_current;
    }

    spisync->ps_status = 2;
    return 0;
}

int spisync_init(spisync_t *spisync, const spisync_config_t *config)
{
    int i;
    BaseType_t ret;

    if (NULL == spisync) {
        return -1;
    }

    node_mem_t node_txbuf[SPISYNC_TXSLOT_COUNT];
    node_mem_t node_rxbuf[SPISYNC_RXSLOT_COUNT];

    g_spisync_current = spisync;

    /* slot */
    memset(spisync, 0, sizeof(spisync_t));
    spisync->config = config;
    spisync->p_tx = (spisync_txbuf_t *)malloc_aligned_with_padding_nocache(sizeof(spisync_txbuf_t), 32);
    spisync->p_rx = (spisync_rxbuf_t *)malloc_aligned_with_padding_nocache(sizeof(spisync_rxbuf_t), 32);
    spisync->p_rxcache = (slot_payload_t *)pvPortMalloc(sizeof(slot_payload_t));
    memset(spisync->p_tx, 0, sizeof(spisync_txbuf_t));
    memset(spisync->p_rx, 0, sizeof(spisync_rxbuf_t));
    memset(spisync->p_rxcache, 0, sizeof(slot_payload_t));
    // dma node list update
    for (i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        node_txbuf[i].buf = &spisync->p_tx->slot[i];
        node_txbuf[i].len = sizeof(spisync_slot_t);
    }
    for (i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        node_rxbuf[i].buf = &spisync->p_rx->slot[i];
        node_rxbuf[i].len = sizeof(spisync_slot_t);
    }
    // update magic
    for (i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        spisync->p_tx->slot[i].tag.magic = SPISYNC_SLOT_MAGIC;
        spisync->p_tx->slot[i].tag.flag = 0x1;// for poweron reset
        spisync->p_tx->slot[i].tag.clamp[0] = SPISYNC_RX0_STREAMBUFFER_SIZE;
        spisync->p_tx->slot[i].tag.clamp[1] = SPISYNC_RX1_STREAMBUFFER_SIZE;
        spisync->p_tx->slot[i].tag.clamp[2] = SPISYNC_RX2_STREAMBUFFER_SIZE;
    }

    /* global init*/
    spisync->tx_seq             = 0;
    spisync->txtag_errtimes     = 0;
    spisync->txwrite_continue   = 0;
    spisync->rxpattern_checked  = 0;
    spisync->rxread_continue    = 0;
    spisync->clamp[0]           = 0;
    spisync->clamp[1]           = 0;
    spisync->clamp[2]           = 0;
#if SPISYNC_MASTER_ENABLE
    spisync->state = SPISYNC_STATE_NORMAL;
    spisync->master = 1;
    spisync->reset_pending = 0;
#endif
    /* debug */
    debug_init_reset(spisync, 1);// 1 init, 0 reset

    /* sys hdl */
    spisync->event         = xEventGroupCreate();
    spisync->timer         = xTimerCreate("delay_rst",
                                        pdMS_TO_TICKS(SPISYNC_PATTERN_DELAYMS), // per 10s
                                        pdFALSE,                                // auto reload
                                        (void *)spisync,
                                        timer_callback);
    spisync->w_mutex       = xSemaphoreCreateMutex();
#if SPISYNC_MASTER_ENABLE
#else
    spisync->pin_mutex     = xSemaphoreCreateMutex();
#endif
    spisync->ptx_streambuffer[0] = xStreamBufferCreate(SPISYNC_TX0_STREAMBUFFER_SIZE, 1);
    spisync->ptx_streambuffer[1] = xStreamBufferCreate(SPISYNC_TX1_STREAMBUFFER_SIZE, 1);
    spisync->ptx_streambuffer[2] = xStreamBufferCreate(SPISYNC_TX2_STREAMBUFFER_SIZE, 1);

    spisync->prx_streambuffer[0] = xStreamBufferCreate(SPISYNC_RX0_STREAMBUFFER_SIZE, 1);
    spisync->prx_streambuffer[1] = xStreamBufferCreate(SPISYNC_RX1_STREAMBUFFER_SIZE, 1);
    spisync->prx_streambuffer[2] = xStreamBufferCreate(SPISYNC_RX2_STREAMBUFFER_SIZE, 1);

    ret = xTaskCreate(spisync_task,
                (char*)"spisync",
                SPISYNC_STACK_SIZE/sizeof(StackType_t), spisync,
                SPISYNC_TASK_PRI,
                &spisync->taskhdr);
    if (ret != pdPASS) {
    	printf("failed to create spisync task, %ld\r\n", ret);
	}

    /* set type , and set name */
    lramsync_init(
        spisync->config,
        &spisync->hw,
        node_txbuf, SPISYNC_TXSLOT_COUNT,
        __ramsync_low_tx_cb, spisync,
        node_rxbuf, SPISYNC_RXSLOT_COUNT,
        __ramsync_low_rx_cb, spisync,
        __ramsync_low_reset_cb, spisync
        );
#if SPISYNC_MASTER_ENABLE
    /* master needs to reset for re-synchronization */
    spisync->state = SPISYNC_STATE_INIT;
    _spisync_setevent(spisync, EVT_SPISYNC_RESET_BIT);
#endif

#if SPISYNC_MASTER_ENABLE
#else
    spisync_s2m_init();
    spisyn_s2m_set();
    arch_delay_ms(10);
    spisync_s2m_reset();
#endif
    return 0;
}

int spisync_deinit(spisync_t *spisync)
{
    lramsync_deinit(&spisync->hw);
    //free_aligned_with_padding_nocache(spisync->p_tx);
    //free_aligned_with_padding_nocache(spisync->p_rx);
    // todo
    return 0;
}

int spisync_read(spisync_t *spisync, uint8_t type, void *buf, uint32_t len, uint32_t timeout_ms)
{
    int i;
    int ret;
    uint32_t clamp;

    if (NULL == spisync) {
        spisync_log("arg err, spisync:%p\r\n",
                spisync, len, SPISYNC_PAYLOADBUF_LEN);
        return -1;
    }
    ret = xStreamBufferReceive(spisync->prx_streambuffer[type], buf, len, timeout_ms);

    // update to tx clamp
    if ((ret > 0) && (ret <= len)) {
        clamp = spisync->p_tx->slot[0].tag.clamp[type] + ret;
        for (i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
            spisync->p_tx->slot[i].tag.clamp[type] = clamp;
        }
    }

    return ret;
}

int spisync_fakewrite_forread(spisync_t *spisync, void *buf, uint32_t len, uint32_t timeout_ms)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int res;

    if ((NULL == spisync) || (len > SPISYNC_PAYLOADBUF_LEN)) {
        spisync_log("arg err, spisync:%p, len:%d, max_len:%d\r\n",
                spisync, len, SPISYNC_PAYLOADBUF_LEN);
        return -1;
    }

    if (xPortIsInsideInterrupt()) {
        res = xStreamBufferSendFromISR(spisync->prx_streambuffer[0], buf, len, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        res = xStreamBufferSend(spisync->prx_streambuffer[0], buf, len, timeout_ms);
    }

    return res;
}

static uint32_t get_latest_clamp(spisync_t *spisync, uint8_t type);
int spisync_write(spisync_t *spisync, uint8_t type, void *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret;
    uint32_t rclamp, clamp;
    int32_t diff;
    if ((NULL == spisync) || (type > 3)) {
        spisync_err("arg error. spisync:%p, len:%d, type:%d\r\n", spisync, len, type);
        return -1;
    }

    if (spisync->ps_status) {
        spisync_wakeup(spisync);
        vTaskDelay(300);
        spisync->ps_status = 0;
    }
    // todo: add clamp here.
#if 0
    // compare
    rclamp = get_latest_clamp(spisync, type);
    diff = INT32_DIFF(rclamp, spisync->clamp[type]);
    if ((diff <= 0) || (diff <= len)) {
        spisync_log("no more buff???,diff:%d, rclamp:%d + %d = %d\r\n",
            diff, spisync->clamp[type], len, (spisync->clamp[type] + len));
        return 0;
    }
    // update
    spisync_log("spisync write ---> local diff:%d, clamp:%d + %d = %d\r\n",
            diff, spisync->clamp[type], len, (spisync->clamp[type] + len));
    spisync->clamp[type] += len;
#endif

    if (xSemaphoreTake(spisync->w_mutex, timeout_ms) == pdTRUE) {
        ret = xStreamBufferSend(spisync->ptx_streambuffer[type], buf, len, timeout_ms);
        if (ret != len) {
            spisync_trace("spisync_write ret:%d\r\n", ret);
        }
        _spisync_setevent(spisync, EVT_SPISYNC_WRITE_BIT);
        xSemaphoreGive(spisync->w_mutex);
    } else {
        spisync_log("take mutex error, so write NULL.");
        ret = -1;
    }

#if SPISYNC_MASTER_ENABLE
#else
    if (xSemaphoreTake(spisync->pin_mutex, portMAX_DELAY) == pdTRUE) {
        spisyn_s2m_set();
    }
    xSemaphoreGive(spisync->pin_mutex);
#endif

    return ret;
}
#if SPISYNC_MASTER_ENABLE
static void _reset_master_spisync(spisync_t *spisync)
{
	BaseType_t ret;
	int state = spisync->state;

	switch (state) {
	case SPISYNC_STATE_INIT:
	case SPISYNC_STATE_NORMAL:
		memset(spisync->p_tx, SPISYNC_RESET_PATTERN, sizeof(spisync_txbuf_t));
		ret = xTimerStart(spisync->timer, 1000);
		if (ret != pdPASS)
			spisync_err("failed to start timer in normal state, %ld\r\n", ret);
		else
			spisync_log("started timer in state %d\r\n", state);

		spisync->state = SPISYNC_STATE_RESET_ANNOUNCE;
		spisync_log("spisync state %s --> reset announce\r\n",
					state == SPISYNC_STATE_INIT ? "init" : "normal");
		break;

	case SPISYNC_STATE_RESET_ANNOUNCE:
		lramsync_reset(&spisync->hw);
		ret = xTimerStart(spisync->timer, 1000);
		if (ret != pdPASS)
			spisync_err("failed to start timer when announce reset, %ld\r\n", ret);
		else
			spisync_log("started timer in rst ann\r\n");

		spisync->state = SPISYNC_STATE_WAITING;
		spisync_log("spisync state reset announce --> waiting\r\n");
		break;

	case SPISYNC_STATE_WAITING:
		spisync->state = SPISYNC_STATE_NORMAL;
		spisync_log("spisync state waiting --> normal\r\n");
		_reset_spisync(spisync);
		lramsync_start(&spisync->hw);
		break;

	default:
		spisync_err("unknown spisync state 0x%x\r\n", state);
		break;
	}
}
#endif
/*---------------------- moudule ------------------------*/
static void _reset_process(spisync_t *spisync)
{
#if SPISYNC_MASTER_ENABLE
	_reset_master_spisync(spisync);
#else
    _reset_spisync(spisync);
#endif
}

static void _ctrl_process(spisync_t *spisync)
{
}

static uint32_t get_latest_rseq(spisync_t *spisync)
{
    uint32_t max_val = 0;

    for (uint32_t i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        if (spisync->p_rx->slot[i].tag.rseq > max_val) {
            max_val = spisync->p_rx->slot[i].tag.rseq;
        }
    }
    return max_val;
}

const uint32_t c_clamp_table[] = {
SPISYNC_TX0_STREAMBUFFER_SIZE,
SPISYNC_TX1_STREAMBUFFER_SIZE,
SPISYNC_TX2_STREAMBUFFER_SIZE,
};
static uint32_t get_latest_clamp(spisync_t *spisync, uint8_t type)
{
#if 0
    uint32_t max_val = 0;

    for (uint32_t i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        if (spisync->p_rx->slot[i].tag.rseq > max_val) {
            max_val = spisync->p_rx->slot[i].tag.rseq;
        }
    }
    return max_val;
#else
  #if 0// todo
    uint32_t max_val = spisync->p_rx->slot[0].tag.clamp[type];
    int diff;

    for (uint32_t i = 1; i < 3; i++) {
        diff = INT32_DIFF(spisync->p_rx->slot[i].tag.clamp[type], max_val);
        if ((diff > 0) && (diff < c_clamp_table[type])) {
            max_val = spisync->p_rx->slot[i].tag.clamp[type];
        }
    }
    return max_val;
  #else
    return spisync->p_rx->slot[0].tag.clamp[type];
  #endif
#endif
}

void _write_process(spisync_t *spisync)
{
    int empty_slot, empty_flag;
    uint32_t current;
    uint32_t latest_rseq;
    uint32_t s_txseq;

    int i, available_bytes;
    uint32_t data_len;
    StreamBufferHandle_t current_stream_buffer;
    uint32_t current_stream_len;
    uint8_t current_pack_type;
    struct crc32_stream_ctx crc_ctx;
    uint32_t rclamp, clamp;
    int32_t diff;
#if SPISYNC_MASTER_ENABLE
#else
    int stream_invalid = 0;
#endif

    /* Check if there is an empty slot */
    empty_slot = 0;
    empty_flag = 0;
    for (i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        latest_rseq = get_latest_rseq(spisync);
        s_txseq = spisync->p_tx->slot[i].payload.seq;
        current = UINT32_DIFF(s_txseq, latest_rseq);
        // ( current > SPISYNC_TXSLOT_COUNT) ===> w_seq too small and old
        // (0 == current)                    ===> w_seq==r_seq
        if (( current > SPISYNC_TXSLOT_COUNT) || (0 == current)) {
            empty_slot = i;
            empty_flag = 1;
            break;
        }
    }
    if (!empty_flag) {
        return;
    }

    /* check if there is valid bytes */
#if SPISYNC_MASTER_ENABLE
#else
    if (xSemaphoreTake(spisync->pin_mutex, portMAX_DELAY) == pdTRUE) {
#endif
        for (i = 0; i < 3; i++) {
            available_bytes = xStreamBufferBytesAvailable(spisync->ptx_streambuffer[i]);
            if (available_bytes <= 0) {
#if SPISYNC_MASTER_ENABLE
#else
                stream_invalid++;
#endif
                continue;
            }
            rclamp = get_latest_clamp(spisync, i);
            diff = INT32_DIFF(rclamp, spisync->clamp[i]);
            if (diff <= 0) {
                continue;
            }
            current_pack_type = i;
            current_stream_buffer = spisync->ptx_streambuffer[i];
            current_stream_len = (diff < SPISYNC_PAYLOADBUF_LEN)?diff:SPISYNC_PAYLOADBUF_LEN;
            spisync_trace("type:%d, len:%d, diff:%d\r\n", 
                    current_pack_type, current_stream_len, diff);
            break;
        }

#if SPISYNC_MASTER_ENABLE
#else
        if ((3 == stream_invalid) && (spisync->tx_seq == latest_rseq)) {
            spisync_s2m_reset();
        }
    }
    xSemaphoreGive(spisync->pin_mutex);
#endif

    if (i == 3) {// can't write to slot
        return;
    }

    /* pop from msgbuffer to slot */
    data_len = xStreamBufferReceive(
            current_stream_buffer,
            spisync->p_tx->slot[empty_slot].payload.buf,
            current_stream_len,
            0);
    spisync_trace("write-> rseq:%d, seq:%d, slot:%d, len:%d, avail:%d, current:%d, oldseq:%d\r\n",
            latest_rseq, (spisync->tx_seq + 1),
            empty_slot, data_len, available_bytes,
            current, s_txseq
            );
    if ((data_len <= 0) || (data_len > SPISYNC_PAYLOADBUF_LEN)) {
        spisync_err("write process never here. except spi reset?\r\n", data_len);
        return;
    }
    // update clamp
    spisync->clamp[current_pack_type] += data_len;
    spisync->tx_seq++;
    spisync->p_tx->slot[empty_slot].payload.len = SPISYNC_PAYLOADLENSEQ_LEN + data_len;
    spisync->p_tx->slot[empty_slot].payload.seq = spisync->tx_seq;
    spisync->p_tx->slot[empty_slot].payload.type = 
        ((spisync->p_tx->slot[empty_slot].payload.type & 0xFFFFFF00) | current_pack_type);

    // CRC check
    utils_crc32_stream_init(&crc_ctx);
    utils_crc32_stream_feed_block(&crc_ctx,
                                  (const uint8_t *)&spisync->p_tx->slot[empty_slot].payload, 
                                  spisync->p_tx->slot[empty_slot].payload.len);
    spisync->p_tx->slot[empty_slot].payload.crc = utils_crc32_stream_results(&crc_ctx);

    spisync->p_tx->slot[empty_slot].tail.seq_mirror = spisync->tx_seq;

    // debug
    spisync->tsk_writecnt += 1;

    spisync_trace("write slot:%d, len:%d,  %d,%d,%d,%d,%d,%d,  %d,%d,%d,%d,%d,%d,\r\n",
            empty_slot, data_len,
            spisync->p_tx->slot[0].payload.seq,
            spisync->p_tx->slot[1].payload.seq,
            spisync->p_tx->slot[2].payload.seq,
            spisync->p_tx->slot[3].payload.seq,
            spisync->p_tx->slot[4].payload.seq,
            spisync->p_tx->slot[5].payload.seq,
            spisync->p_rx->slot[0].tag.rseq,
            spisync->p_rx->slot[1].tag.rseq,
            spisync->p_rx->slot[2].tag.rseq,
            spisync->p_rx->slot[3].tag.rseq,
            spisync->p_rx->slot[4].tag.rseq,
            spisync->p_rx->slot[5].tag.rseq
            );
}

static void _read_process(spisync_t *spisync)
{
    uint32_t lentmp;
    uint32_t crctmp;
    uint32_t seqtmp;
    uint32_t rseqtmp;
    struct crc32_stream_ctx crc_ctx;
    int space_valid;
    int read_times = SPISYNC_RX_READTIMES;
    uint8_t pack_type;

    debug_iperf_print(spisync);
    while (read_times) {
        for (int i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
            // check magic
            if (SPISYNC_SLOT_MAGIC != spisync->p_rx->slot[i].tag.magic) {
                continue;
            }

            // check seq
            if (spisync->p_rx->slot[i].payload.seq != spisync->p_rx->slot[i].tail.seq_mirror) {
                continue;
            }

            // update tmp
            rseqtmp = spisync->p_tx->slot[i].tag.rseq;
            seqtmp  = spisync->p_rx->slot[i].payload.seq;
            lentmp  = spisync->p_rx->slot[i].payload.len;
            pack_type = (spisync->p_rx->slot[i].payload.type)&0xFF;

            // check seq len valid
            if ((lentmp > SPISYNC_PAYLOADLEN_MAX) || (seqtmp != (rseqtmp + 1)) || (pack_type >= 3)) {
                if (i == (SPISYNC_RXSLOT_COUNT - 1)) {
                    spisync->rxread_continue = 0;
                }
                continue;
            }

            // Check available spaces in the buffer in for rx
            space_valid = xStreamBufferSpacesAvailable(spisync->prx_streambuffer[pack_type]);
            if (space_valid < (lentmp - SPISYNC_PAYLOADLENSEQ_LEN)) {
                spisync->rxread_continue = 1;
                spisync_trace("The stream buffer does not have enough space.\r\n");
                break;
            }

            // copy data and crc
            memcpy(spisync->p_rxcache, &spisync->p_rx->slot[i].payload, lentmp);
            spisync->p_rxcache->crc = spisync->p_rx->slot[i].payload.crc;
            // Check if the relevant parameters are within the expected range
            if ((spisync->p_rxcache->seq != seqtmp) ||
                (spisync->p_rxcache->len != lentmp) ||
                ((spisync->p_rxcache->type & 0xFF) != pack_type)) {
                continue;
            }

            // check crc 
            utils_crc32_stream_init(&crc_ctx);
            utils_crc32_stream_feed_block(&crc_ctx,
                                  (const uint8_t *)&spisync->p_rxcache->len, 
                                  spisync->p_rxcache->len);
            if (spisync->p_rxcache->crc != utils_crc32_stream_results(&crc_ctx)) {
                spisync_trace("crc err, 0x%08X != 0x%08X\r\n", crctmp, spisync->p_rxcache->crc);
                continue;
            }

            // push
            spisync->tsk_readcnt++;

            spisync_trace("recv slot %d rx seq:  %d,%d,%d,%d,%d,%d  %d,%d,%d,%d,%d,%d\r\n",
                    i,
                    spisync->p_rx->slot[0].payload.seq,
                    spisync->p_rx->slot[1].payload.seq,
                    spisync->p_rx->slot[2].payload.seq,
                    spisync->p_rx->slot[3].payload.seq,
                    spisync->p_rx->slot[4].payload.seq,
                    spisync->p_rx->slot[5].payload.seq,
                    spisync->p_tx->slot[0].tag.rseq,
                    spisync->p_tx->slot[1].tag.rseq,
                    spisync->p_tx->slot[2].tag.rseq,
                    spisync->p_tx->slot[3].tag.rseq,
                    spisync->p_tx->slot[4].tag.rseq,
                    spisync->p_tx->slot[5].tag.rseq
                    );

            if ((lentmp - SPISYNC_PAYLOADLENSEQ_LEN) != xStreamBufferSend(spisync->prx_streambuffer[pack_type],
                        spisync->p_rxcache->buf, (lentmp - SPISYNC_PAYLOADLENSEQ_LEN), 0)) {
                spisync_err("never here.except spi reset?\r\n");
            }

            // update tx st rseq
            for (int m = 0; m < SPISYNC_TXSLOT_COUNT; m++) {
                spisync->p_tx->slot[m].tag.rseq = spisync->p_rxcache->seq;
            }
        }

        if (xStreamBufferSpacesAvailable(spisync->prx_streambuffer[pack_type]) < SPISYNC_RX_STREAMBUFFER_LEVEL) {
            break; // while (read_times)
        }
        read_times--;
    }
}

static void _wakeup_process(spisync_t *spisync)
{
#if 1
    _spisync_setevent(spisync, EVT_SPISYNC_RESET_BIT);
#else
    for (int i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        spisync->p_tx->slot[i].tag.flag |= 0x1;// set rx flag err by tx
    }
#endif
}

static void spisync_task(void *arg)
{
     spisync_t *spisync = (spisync_t *)arg;
     EventBits_t eventBits;

     while (1) {
         eventBits = xEventGroupWaitBits(spisync->event,
             EVT_SPISYNC_RESET_BIT|EVT_SPISYNC_CTRL_BIT|EVT_SPISYNC_WRITE_BIT| \
             EVT_SPISYNC_READ_BIT|EVT_SPISYNC_WAKEUP_BIT| \
             EVT_SPISYNC_DUMP_BIT,
             pdTRUE, pdFALSE,
             portMAX_DELAY);
         if (eventBits  & EVT_SPISYNC_RESET_BIT) {
             spisync_log("recv bit EVT_SPISYNC_RESET_BIT\r\n");
             spisync->tsk_rstcnt++;
             _reset_process(spisync);
         }
         if (eventBits  & EVT_SPISYNC_WAKEUP_BIT) {
             spisync_log("recv bit EVT_SPISYNC_WAKEUP_BIT\r\n");
             _wakeup_process(spisync);
         }
         if (eventBits  & EVT_SPISYNC_CTRL_BIT) {
             spisync_log("recv bit EVT_SPISYNC_CTRL_BIT\r\n");
             spisync->tsk_ctrlcnt++;
             _ctrl_process(spisync);
         }
		 if (eventBits  & EVT_SPISYNC_DUMP_BIT) {
            spisync_dump(spisync);
         }
		 
		 // normal start
#if SPISYNC_MASTER_ENABLE
         if (spisync->state != SPISYNC_STATE_NORMAL) {
		 	continue;
		 }
#endif
		 if (eventBits  & EVT_SPISYNC_WRITE_BIT) {
			 spisync_trace("recv bit EVT_SPISYNC_WRITE_BIT\r\n");
			 _write_process(spisync);
		 }
		 if (eventBits  & EVT_SPISYNC_READ_BIT) {
			 spisync_trace("recv bit EVT_SPISYNC_READ_BIT\r\n");
				 _read_process(spisync);
		 }
		// normal end
     }
}

/*--------------------- debug moudule ----------------------*/
int spisync_iperf(spisync_t *spisync, int enable)
{
    spisync->iperf = enable;

    spisync_log("iperf:%d\r\n", spisync->iperf);
}

int spisync_iperftx(spisync_t *spisync, uint32_t time_sec)
{
    uint8_t *buf;
    uint32_t start_cnt, current_cnt;
    uint32_t tx_cnt = 0;
    float    res = 1.00;
    uint32_t iperf_flag = spisync->iperf;

    // enter
    spisync->iperf = 1;

    start_cnt = xTaskGetTickCount();
    buf = pvPortMalloc(SPISYNC_PAYLOADBUF_LEN);
    if (NULL == buf) {
        return -1;
    }

    while (1) {
        current_cnt = xTaskGetTickCount();
        if ((time_sec * 1000) <= UINT32_DIFF(current_cnt, start_cnt)) {
            break;
        }
        spisync_write(spisync, 0, buf, SPISYNC_PAYLOADBUF_LEN, portMAX_DELAY);
        tx_cnt++;
    }

    // resume
    spisync->iperf = iperf_flag;

    spisync_log("Iperf %d - %d spisync_tx [%f] bps, [%d] Bytes/S, [%d] packet/S\r\n",
            start_cnt, current_cnt,
            ((float)((tx_cnt * SPISYNC_PAYLOADBUF_LEN)*8/time_sec)),
            (tx_cnt * SPISYNC_PAYLOADBUF_LEN)/time_sec,
            tx_cnt/time_sec);

    return 0;
}

/* debug */
int spisync_dump(spisync_t *spisync)
{
    int i;

    if ((NULL == spisync) && (NULL == g_spisync_current)) {
        return -1;
    } else if (NULL == spisync) {
        spisync = g_spisync_current;
    }

    for (i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        spisync_dbg("tx[%ld] 0x%08lX,rseq:%ld,clamp:%ld-%ld-%ld,flag:%ld    len:%d,seq:%d,type:%08lx,crc:0x%08lX\r\n",
                i,
                spisync->p_tx->slot[i].tag.magic,
                spisync->p_tx->slot[i].tag.rseq,
                spisync->p_tx->slot[i].tag.clamp[0], spisync->p_tx->slot[i].tag.clamp[1], spisync->p_tx->slot[i].tag.clamp[2],
                spisync->p_tx->slot[i].tag.flag,
                spisync->p_tx->slot[i].payload.len,
                spisync->p_tx->slot[i].payload.seq,
                spisync->p_tx->slot[i].payload.type,
                spisync->p_tx->slot[i].payload.crc);
        spisync_buf("     ", &spisync->p_tx->slot[i].payload, 32);
    }

    for (i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        spisync_dbg("rx[%ld] 0x%08lX,rseq:%ld,clamp:%ld-%ld-%ld,flag:%ld    len:%d,seq:%d,type:%08lx,crc:0x%08lX\r\n",
                i,
                spisync->p_rx->slot[i].tag.magic,
                spisync->p_rx->slot[i].tag.rseq,
                spisync->p_rx->slot[i].tag.clamp[0], spisync->p_rx->slot[i].tag.clamp[1], spisync->p_rx->slot[i].tag.clamp[2],
                spisync->p_rx->slot[i].tag.flag,
                spisync->p_rx->slot[i].payload.len,
                spisync->p_rx->slot[i].payload.seq,
                spisync->p_rx->slot[i].payload.type,
                spisync->p_rx->slot[i].payload.crc);
        spisync_buf("     ", &spisync->p_rx->slot[i].payload, 32);
    }

    spisync_dbg("spisync->tx_seq             = %ld\r\n", spisync->tx_seq);
    spisync_dbg("spisync->txtag_errtimes     = %ld\r\n", spisync->txtag_errtimes);
    spisync_dbg("spisync->txwrite_continue   = %ld\r\n", spisync->txwrite_continue);
    spisync_dbg("spisync->clamp[0,1,2]       = %ld,%ld,%ld\r\n", spisync->clamp[0], spisync->clamp[1], spisync->clamp[2]);
    spisync_dbg("spisync->rxpattern_checked  = %ld\r\n", spisync->rxpattern_checked);
    spisync_dbg("spisync->rxread_continue    = %ld\r\n", spisync->rxread_continue);

    spisync_dbg("ptx_streambuffer[0]   total:%ld, free_space:%ld\r\n",
            SPISYNC_TX0_STREAMBUFFER_SIZE, xStreamBufferSpacesAvailable(spisync->ptx_streambuffer[0])
            );
    spisync_dbg("ptx_streambuffer[1]   total:%ld, free_space:%ld\r\n",
            SPISYNC_TX1_STREAMBUFFER_SIZE, xStreamBufferSpacesAvailable(spisync->ptx_streambuffer[1])
            );
    spisync_dbg("ptx_streambuffer[2]   total:%ld, free_space:%ld\r\n",
            SPISYNC_TX2_STREAMBUFFER_SIZE, xStreamBufferSpacesAvailable(spisync->ptx_streambuffer[2])
            );
    spisync_dbg("prx_streambuffer[0] total:%ld, free_space:%ld\r\n",
            SPISYNC_RX0_STREAMBUFFER_SIZE,
            xStreamBufferSpacesAvailable(spisync->prx_streambuffer[0])
            );
    spisync_dbg("prx_streambuffer[1] total:%ld, free_space:%ld\r\n",
            SPISYNC_RX1_STREAMBUFFER_SIZE,
            xStreamBufferSpacesAvailable(spisync->prx_streambuffer[1])
            );
    spisync_dbg("prx_streambuffer[2] total:%ld, free_space:%ld\r\n",
            SPISYNC_RX2_STREAMBUFFER_SIZE,
            xStreamBufferSpacesAvailable(spisync->prx_streambuffer[2])
            );
    spisync_dbg("isr_rxcnt    = %d\r\n", spisync->isr_rxcnt);
    spisync_dbg("isr_txcnt    = %d\r\n", spisync->isr_txcnt);
    spisync_dbg("tsk_rstcnt   = %d\r\n", spisync->tsk_rstcnt);
    spisync_dbg("tsk_ctrlcnt  = %d\r\n", spisync->tsk_ctrlcnt);
    spisync_dbg("tsk_writecnt = %d\r\n", spisync->tsk_writecnt);
    spisync_dbg("tsk_readcnt  = %d\r\n", spisync->tsk_readcnt);
    spisync_dbg("tsk_rsttick  = %d\r\n", spisync->tsk_rsttick);

    spisync_dbg("iperf            = %d\r\n", spisync->iperf);
    spisync_dbg("dbg_tick_old     = %d\r\n", spisync->dbg_tick_old);
    spisync_dbg("isr_rxcnt_old    = %d\r\n", spisync->isr_rxcnt_old);
    spisync_dbg("isr_txcnt_old    = %d\r\n", spisync->isr_txcnt_old);
    spisync_dbg("tsk_writecnt_old = %d\r\n", spisync->tsk_writecnt_old);
    spisync_dbg("tsk_readcnt_old  = %d\r\n", spisync->tsk_readcnt_old);

    return 0;
}

int spisync_dump_internal(spisync_t *spisync)
{
    _spisync_setevent(spisync, EVT_SPISYNC_DUMP_BIT);

    return 0;
}

/* iperf */
typedef struct _iperf_desc {
    spisync_t *spisync;
    uint8_t *name;
    uint8_t type;
    uint32_t per;
    uint32_t t;
} spisync_iperf_t;

typedef int (*iperf_fun_cb_t)(spisync_t *spisync, uint8_t type, void *buf, uint32_t len, uint32_t timeout_ms);

int task_spisync_iperf(void *par)
{
    spisync_iperf_t *arg = (spisync_iperf_t *)par;
    uint8_t *buf;
    uint32_t start_cnt, first_cnt, current_cnt;
    int32_t diff;
    int ret = 0;
    uint32_t send_bytes = 0;
    uint32_t send_bytes_old = 0;
    iperf_fun_cb_t cb = NULL;

    if (NULL == arg) {
        spisync_err("task arg error.\r\n");
        return;
    }

    buf = pvPortMalloc(SPISYNC_PAYLOADBUF_LEN);
    if (NULL == buf) {
        vPortFree(arg);
        return -1;
    }

    if (arg->name[0] == 't') {
        cb = spisync_write;
        printf("write\r\n");
    } else {
        cb = spisync_read;
        printf("read\r\n");
    }

    start_cnt = xTaskGetTickCount();
    first_cnt = start_cnt;
    while (1) {
        //printf("arg->spisync:%p, buf:%d, type:%d\r\n", arg->spisync, buf, arg->type);
        ret = cb(arg->spisync, arg->type, buf, SPISYNC_PAYLOADBUF_LEN, 10);
        if (ret > 0) {
            send_bytes += ret;
        }

        // per print
        current_cnt = xTaskGetTickCount();
        diff = INT32_DIFF(current_cnt, start_cnt);
        if (diff >= (arg->per*1000)) {
            start_cnt += (arg->per*1000);
            spisync_log("iperf %d-%d: type:%d %s %ld bps\r\n",
                    start_cnt, current_cnt, arg->type,
                    arg->name,
                    ((send_bytes - send_bytes_old)*8));
            send_bytes_old = send_bytes;
        }

        // total t
        diff = INT32_DIFF(current_cnt, first_cnt);
        if (diff > (arg->t *1000)) {
            spisync_log("iperf %d-%d: type:%d %s %ld bps\r\n",
                    first_cnt, current_cnt, arg->type,
                    arg->name,
                    (send_bytes*8)/((current_cnt - first_cnt)/1000));
            break;
        }
    }
    vPortFree(arg);
    vTaskDelete(NULL);
}

int spisync_iperf_test(spisync_t *spisync, uint8_t tx_en, uint8_t type, uint32_t per, uint32_t t)
{
    spisync_iperf_t *arg;
    const uint8_t *tx_str = "tx";
    const uint8_t *rx_str = "rx";
    int ret;

    arg = pvPortMalloc(SPISYNC_PAYLOADBUF_LEN);
    if (NULL == arg) {
        return -1;
    }

    // update arg
    arg->spisync = spisync;
    if (tx_en) {
        arg->name = tx_str;
    } else {
        arg->name = rx_str;
    }
    arg->type = type;
    arg->per  = per;
    arg->t    = t;

    // creat task
    if (tx_en) {
        ret = xTaskCreate(task_spisync_iperf,
                    (char*)"iperf_tx",
                    4096/sizeof(StackType_t), arg,
                    SPISYNC_TASK_PRI,
                    NULL);
    } else {
        ret = xTaskCreate(task_spisync_iperf,
                    (char*)"iperf_rx",
                    4096/sizeof(StackType_t), arg,
                    SPISYNC_TASK_PRI,
                    NULL);
    }
    if (ret != pdPASS) {
    	spisync_err("Failed to create spisync tx task, %ld\r\n", ret);
        vPortFree(arg);
	}

    return 0;
}

// type period send&&recv test
typedef struct _type_desc {
    spisync_t *spisync;
    uint32_t per;
    uint32_t t;
} spisync_type_t;

int task_spisync_type(void *par)
{
    spisync_type_t *arg = (spisync_type_t *)par;
    uint8_t *buf;
    uint32_t start_cnt, first_cnt, current_cnt;
    int32_t diff;
    int ret0, ret1, ret2, recv0, recv1, recv2;

    if (NULL == arg) {
        spisync_err("task arg error.\r\n");
        return;
    }
    spisync_log("task_spisync_type, period:%d, timeout:%d\r\n", arg->per, arg->t);

    buf = pvPortMalloc(SPISYNC_PAYLOADBUF_LEN);
    if (NULL == buf) {
        spisync_err("mem error.\r\n");
        vPortFree(arg);
        return -1;
    }

    start_cnt = xTaskGetTickCount();
    first_cnt = start_cnt;
    while (1) {
        // per print
        current_cnt = xTaskGetTickCount();
        diff = INT32_DIFF(current_cnt, start_cnt);
        if (diff >= (arg->per*1000)) {
            start_cnt += (arg->per*1000);

            ret0 = 0;//spisync_write(arg->spisync, 0, buf, SPISYNC_PAYLOADBUF_LEN/2, 0);
            ret1 = spisync_write(arg->spisync, 1, buf, SPISYNC_PAYLOADBUF_LEN/2, 0);
            ret2 = spisync_write(arg->spisync, 2, buf, SPISYNC_PAYLOADBUF_LEN/2, 0);
            recv0 = 0;//spisync_read(arg->spisync, 0, buf, SPISYNC_PAYLOADBUF_LEN, 0);
            recv1 = spisync_read(arg->spisync, 1, buf, SPISYNC_PAYLOADBUF_LEN, 0);
            recv2 = spisync_read(arg->spisync, 2, buf, SPISYNC_PAYLOADBUF_LEN, 0);
            spisync_log("send:%d-%d-%d, recv:%d-%d-%d\r\n",
                    ret0, ret1, ret2, recv0, recv1, recv2);
        }

        // total t
        diff = INT32_DIFF(current_cnt, first_cnt);
        if (diff > (arg->t *1000)) {
            spisync_log("type test, send&&recv end.\r\n");
            break;
        }

        vTaskDelay(100);
    }
    vPortFree(arg);
    vTaskDelete(NULL);
}

int spisync_type_test(spisync_t *spisync, uint32_t period, uint32_t t)
{
    spisync_type_t *arg;
    int ret;

    arg = pvPortMalloc(SPISYNC_PAYLOADBUF_LEN);
    if (NULL == arg) {
        spisync_err("mem error.\r\n");
        return -1;
    }

    arg->spisync = spisync;
    arg->per = period;
    arg->t = t;
    ret = xTaskCreate(task_spisync_type,
                (char*)"type_tc",
                4096/sizeof(StackType_t), arg,
                SPISYNC_TASK_PRI,
                NULL);
    if (ret != pdPASS) {
    	spisync_err("Failed to create spisync tx task, %ld\r\n", ret);
        vPortFree(arg);
	}
}

