
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

#if SPISYNC_MASTER_ENABLE
#include "main.h"

static void spisync_start_announce_reset(spisync_t *s);

const char *spisync_state_str[] = {
	[SPISYNC_STATE_INIT] = "init",
	[SPISYNC_STATE_RESET_ANNOUNCE] = "rst_ann",
	[SPISYNC_STATE_WAITING] = "waiting",
	[SPISYNC_STATE_ACTIVE] = "active",
	[SPISYNC_STATE_IDLE] = "idle",
};
#endif

#define UINT32_DIFF(a, b) ((uint32_t)((int32_t)(a) - (int32_t)(b)))
#define INT32_DIFF(a, b)  (( int32_t)((int32_t)(a) - (int32_t)(b)))

spisync_t *g_spisync_current = NULL;// for dump

/*--------------------- internal process module ----------------------*/
static void spisync_task(void *arg);
static uint32_t get_latest_rseq(spisync_t *spisync);

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
        spisync_log("RX(tsk:%f bps, usage:%ld, %ld pak, %ld ms)    TX(tsk:%f bps, usage:%ld, %ld pak, %ld ms)\r\n",
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
#if !SPISYNC_MASTER_ENABLE
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
#endif

#if SPISYNC_MASTER_ENABLE
static int check_slot_tag(spisync_t *spisync)
{
    int i, error = 0;
    char *reason = "unknown";

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

	if (error) {
		spisync_log("spisync needs reset, %s, slot %d magic 0x%lx, flag 0x%lx, err time %ld\r\n",
				reason, i, spisync->p_rx->slot[i].tag.magic,
				spisync->p_rx->slot[i].tag.flag, spisync->txtag_errtimes);
	}
    return error;
}

static void spisync_idle_timer_callback(TimerHandle_t xTimer)
{
    spisync_t *spisync = (spisync_t *)pvTimerGetTimerID(xTimer);

    spisync_log("idle timer callback, sets GOTO_IDLE event\r\n");
    _spisync_setevent(spisync, EVT_SPISYNC_GOTO_IDLE);
}

static inline int spisync_slave_data_ready(void)
{
    return HAL_GPIO_ReadPin(SPI_SLAVE_DATA_RDY_GPIO_Port, SPI_SLAVE_DATA_RDY_Pin) == GPIO_PIN_SET;
}

static inline int spisync_sndbuf_empty(spisync_t *s)
{
    int i;
    unsigned long avail;

    for (i = 0; i < 3; i++) {
        avail = xStreamBufferBytesAvailable(s->ptx_streambuffer[i]);
        if (avail > 0)
            return 0;
    }
    return 1;
}

static inline int spisync_has_unacked_data(spisync_t *s)
{
    uint32_t acked_seq;

    acked_seq = get_latest_rseq(s);
    return INT32_DIFF(s->tx_seq, acked_seq) > 0;
}

static inline int spisync_active(spisync_t *s)
{
    return !spisync_sndbuf_empty(s) ||
            spisync_has_unacked_data(s) ||
            spisync_slave_data_ready();
}

#else

static void _spisync_pattern_process(spisync_t *spisync)
{
    int res;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (xPortIsInsideInterrupt()) {
        if (xTimerStartFromISR(spisync->rst_tmr, &xHigherPriorityTaskWoken) != pdPASS) {
            printf("Failed to start timer!\r\n");
        }
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        res = xTimerStart(spisync->rst_tmr, 1000);
        if (res != pdPASS) {
            printf("Failed to start timer!\r\n");
        }
    }
}

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

static void __ramsync_low_rx_cb(void *arg)
{
	spisync_t *spisync = (spisync_t *)arg;

    /* debug */
    if (spisync) {
        spisync->isr_rxcnt++;
    }

    _spisync_setevent(spisync, EVT_SPISYNC_POLL);
}

static void __ramsync_low_tx_cb(void *arg)
{
    spisync_t *spisync = (spisync_t *)arg;

    /* debug */
    if (spisync) {
        spisync->isr_txcnt++;
    }
    /* there is no need to generate POLL event on both TX/RX completion interrupt */
    //_spisync_setevent(spisync, EVT_SPISYNC_POLL);
}

static void __ramsync_low_reset_cb(void *arg)
{
    spisync_t *spisync = (spisync_t *)arg;

    if (spisync->config && spisync->config->reset_cb) {
        spisync->config->reset_cb(spisync->config->reset_arg);
    }
}

void spisync_reset_timer_callback(TimerHandle_t xTimer)
{
    spisync_t *spisync = (spisync_t *)pvTimerGetTimerID(xTimer);

#if SPISYNC_MASTER_ENABLE
    spisync_log("timer callback sets reset bit\r\n");
#endif
    _spisync_setevent(spisync, EVT_SPISYNC_RESET_BIT);
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
    spisync->state = SPISYNC_STATE_INIT;
    spisync->idle_tmr = xTimerCreate("spisync_idle",
                                pdMS_TO_TICKS(SPISYNC_IDLE_TIMER_PERIOD_MS),
                                pdFALSE, (void *)spisync,
                                spisync_idle_timer_callback);
    if (!spisync->idle_tmr) {
        spisync_err("failed to create spisync idle timer\r\n");
        return -1;
    }
#endif
    /* debug */
    debug_init_reset(spisync, 1);// 1 init, 0 reset

    /* sys hdl */
    spisync->event         = xEventGroupCreate();
    spisync->rst_tmr       = xTimerCreate("delay_rst",
                                        pdMS_TO_TICKS(SPISYNC_PATTERN_DELAYMS),
                                        pdFALSE,
                                        (void *)spisync,
                                        spisync_reset_timer_callback);
    spisync->w_mutex       = xSemaphoreCreateMutex();
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
    	spisync_err("failed to create spisync task, %ld\r\n", ret);
    	return -1;
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
    _spisync_setevent(spisync, EVT_SPISYNC_RESET_BIT);
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
        spisync_log("arg err, spisync:%p\r\n", spisync);
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
        spisync_log("arg err, spisync:%p, len:%ld, max_len:%d\r\n",
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

    if ((NULL == spisync) || (type > 3)) {
        spisync_err("arg error. spisync:%p, len:%ld, type:%d\r\n", spisync, len, type);
        return -1;
    }

    if (xSemaphoreTake(spisync->w_mutex, timeout_ms) == pdTRUE) {
        ret = xStreamBufferSend(spisync->ptx_streambuffer[type], buf, len, timeout_ms);
        if (ret != len) {
            spisync_trace("spisync_write ret:%d\r\n", ret);
        }
        _spisync_setevent(spisync, EVT_SPISYNC_WRITE_BIT | EVT_SPISYNC_POLL);
        xSemaphoreGive(spisync->w_mutex);
    } else {
        spisync_log("take mutex error, so write NULL.");
        ret = -1;
    }
    return ret;
}
#if SPISYNC_MASTER_ENABLE
static void spisync_start_announce_reset(spisync_t *s)
{
	int state;
	BaseType_t ret;

	memset(s->p_tx, SPISYNC_RESET_PATTERN, sizeof(spisync_txbuf_t));
	ret = xTimerStart(s->rst_tmr, 1000);
	if (ret != pdPASS) {
		spisync_err("failed to start timer in normal state, %ld\r\n", ret);
		return;
	}

	state = s->state;
	s->state = SPISYNC_STATE_RESET_ANNOUNCE;
	spisync_log("spisync state %s --> reset announce\r\n",
				 spisync_state_str[state]);
}

static void spisync_start_wait_resync(spisync_t *s)
{
	int state;
	BaseType_t ret;

	lramsync_reset(&s->hw);
	ret = xTimerStart(s->rst_tmr, 1000);
	if (ret != pdPASS) {
		spisync_err("failed to start timer when announce reset, %ld\r\n", ret);
		return;
	}

	state = s->state;
	s->state = SPISYNC_STATE_WAITING;
	spisync_log("spisync state %s --> waiting\r\n", spisync_state_str[state]);
}

static void spisync_goto_idle(spisync_t *s)
{
	int state = s->state;

	lramsync_suspend(&s->hw);
	s->state = SPISYNC_STATE_IDLE;
	spisync_log("spisync state %s --> idle\r\n", spisync_state_str[state]);
}

static void spisync_goto_active(spisync_t *s, int reset)
{
	int state;
	BaseType_t ret;

	state = s->state;
	s->state = SPISYNC_STATE_ACTIVE;
	spisync_log("spisync state %s --> active\r\n", spisync_state_str[state]);
	if (reset) {
		_reset_spisync(s);
		lramsync_start(&s->hw);
	} else {
		lramsync_resume(&s->hw);
	}
	ret = xTimerReset(s->idle_tmr, 1000);
	if (ret != pdPASS)
		spisync_err("failed to reset timer in %s, err %ld\r\n", __func__, ret);
}

#endif
/*---------------------- moudule ------------------------*/
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
    uint32_t rclamp;
    int32_t diff;
    struct crc32_stream_ctx crc_ctx;

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
    for (i = 0; i < 3; i++) {
        available_bytes = xStreamBufferBytesAvailable(spisync->ptx_streambuffer[i]);
        if (available_bytes <= 0) {
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
        spisync_err("write process never here. except spi reset?\r\n");
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
                                  (uint8_t *)&spisync->p_tx->slot[empty_slot].payload, 
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
    uint32_t seqtmp;
    uint32_t rseqtmp;
    int space_valid;
    int read_times = SPISYNC_RX_READTIMES;
    uint8_t pack_type;
    struct crc32_stream_ctx crc_ctx;

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
                                  (uint8_t *)&spisync->p_rxcache->len, 
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
        read_times--;
    }
}

static void spisync_active_process(spisync_t *s, EventBits_t events)
{
	BaseType_t ret;

	/*
	 * 1. someone has some data to send.
	 * 2. spi dma interrupt requests to poll.
	 * 3. slave requests to send data.
	 */
	if (events & EVT_SPISYNC_POLL) {
		/* start to reset if corrupted data is found */
		if (check_slot_tag(s)) {
			xTimerStop(s->idle_tmr, 1000);
			spisync_start_announce_reset(s);
			return;
		}

		/* read data if any */
		_read_process(s);
		/* send data is any */
		_write_process(s);

		/* reset idle monitor and stay active */
		if (spisync_active(s)) {
			ret = xTimerReset(s->idle_tmr, 1000);
			if (ret != pdPASS)
				spisync_err("failed to reset timer in %s, err %ld\r\n", __func__, ret);
		}
	}

	/* idle timer is one-shot and already in dormant state */
	if (events & EVT_SPISYNC_GOTO_IDLE) {
		if (!spisync_active(s))
			spisync_goto_idle(s);
		else
			spisync_log("still has some work to do, ignore GOTO_IDLE\r\n");
	}
}

static void spisync_task(void *arg)
{
    spisync_t *spisync = (spisync_t *)arg;
    EventBits_t eventBits;

    while (1) {
        eventBits = xEventGroupWaitBits(spisync->event, EVT_SPISYNC_ALL_BIT,
                pdTRUE, pdFALSE, portMAX_DELAY);

        if (eventBits & EVT_SPISYNC_CTRL_BIT) {
            spisync_log("recv bit EVT_SPISYNC_CTRL_BIT\r\n");
            spisync->tsk_ctrlcnt++;
            _ctrl_process(spisync);
        }

        if (eventBits & EVT_SPISYNC_DUMP_BIT)
            spisync_dump(spisync);

        switch (spisync->state) {
        case SPISYNC_STATE_INIT:
        	if (eventBits & EVT_SPISYNC_RESET_BIT)
        		spisync_start_announce_reset(spisync);
            break;

        case SPISYNC_STATE_RESET_ANNOUNCE:
        	if (eventBits & EVT_SPISYNC_RESET_BIT)
        		spisync_start_wait_resync(spisync);
            break;

        case SPISYNC_STATE_WAITING:
        	if (eventBits & EVT_SPISYNC_RESET_BIT)
        		spisync_goto_active(spisync, 1);
            break;

        case SPISYNC_STATE_ACTIVE:
        	spisync_active_process(spisync, eventBits);
            break;

        case SPISYNC_STATE_IDLE:
        	if (eventBits & EVT_SPISYNC_POLL) {
				if (spisync_active(spisync))
					spisync_goto_active(spisync, 0);
        	}
            break;

        default:
        	spisync_err("spisync state 0x%x is invalid\r\n", spisync->state);
        	break;
        }
    }
}


/*--------------------- debug moudule ----------------------*/
int spisync_iperf(spisync_t *spisync, int enable)
{
    spisync->iperf = enable;

    spisync_log("iperf:%ld\r\n", spisync->iperf);
    return 0;
}

int spisync_iperftx(spisync_t *spisync, uint32_t time_sec)
{
    uint8_t *buf;
    uint32_t start_cnt, current_cnt;
    uint32_t tx_cnt = 0;
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

    spisync_log("Iperf %ld - %ld spisync_tx [%f] bps, [%ld] Bytes/S, [%ld] packet/S\r\n",
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
        spisync_dbg("tx[%d] 0x%08lX,rseq:%ld,clamp:%ld-%ld-%ld,flag:%ld    len:%d,seq:%d,type:%08lx,crc:0x%08lX\r\n",
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
#if SPISYNC_MASTER_ENABLE
    spisync_dbg("Master: send buffer empty = %d\r\n", spisync_sndbuf_empty(spisync));
    spisync_dbg("Master: has outstanding data = %d\r\n", spisync_has_unacked_data(spisync));
    spisync_dbg("Master: slave data ready = %d\r\n", spisync_slave_data_ready());
    spisync_dbg("Master: state %s\r\n", spisync_state_str[spisync->state]);
#endif
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
    const uint8_t *name;
    uint8_t type;
    uint32_t per;
    uint32_t t;
} spisync_iperf_t;

typedef int (*iperf_fun_cb_t)(spisync_t *spisync, uint8_t type, void *buf, uint32_t len, uint32_t timeout_ms);

static void task_spisync_iperf(void *par)
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
        return;
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

static void task_spisync_type(void *par)
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
        return;
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
    return 0;
}

#if SPISYNC_MASTER_ENABLE
void HAL_GPIO_EXTI_Rising_Callback(uint16_t pin)
{
    BaseType_t task_woken = pdFALSE;

	spisync_log("%s pin %d\r\n", __func__, pin);
    _spisync_setevent(g_spisync_current, EVT_SPISYNC_POLL);
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t pin)
{
    BaseType_t task_woken = pdFALSE;

	spisync_log("%s pin %d\r\n", __func__, pin);
    _spisync_setevent(g_spisync_current, EVT_SPISYNC_POLL);
}
#endif
