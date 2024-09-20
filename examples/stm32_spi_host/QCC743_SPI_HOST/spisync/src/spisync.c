
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include "stream_buffer.h"
#include <event_groups.h>

#include <utils_crc.h>
#include <spisync.h>
#include <ramsync.h>
#include <spisync_port.h>
#include "main.h"

/* Enable this when debugging or profiling. */
#define SPISYNC_DEBUG_PRINT 0

/* state machine. */
enum spisync_state {
	SPISYNC_STATE_INIT,
	/* Announcing reset pattern on the bus. */
	SPISYNC_STATE_RESET_ANNOUNCE,
	/* Waiting for some time after announcement. */
	SPISYNC_STATE_WAITING,
	SPISYNC_STATE_ACTIVE,
	SPISYNC_STATE_IDLE,
};

const char *spisync_state_str[] = {
	[SPISYNC_STATE_INIT] = "init",
	[SPISYNC_STATE_RESET_ANNOUNCE] = "rst_ann",
	[SPISYNC_STATE_WAITING] = "waiting",
	[SPISYNC_STATE_ACTIVE] = "active",
	[SPISYNC_STATE_IDLE] = "idle",
};

static void spisync_start_announce_reset(spisync_t *s);

spisync_t *g_spisync_current = NULL;// for dump

/* a<b ? */
#define SEQ_LT(a,b)     ((int16_t)((uint16_t)(a) - (uint16_t)(b)) < 0)
/* a<=b ? */
#define SEQ_LEQ(a,b)    ((int16_t)((uint16_t)(a) - (uint16_t)(b)) <= 0)
/* a>b ? */
#define SEQ_GT(a,b)     ((int16_t)((uint16_t)(a) - (uint16_t)(b)) > 0)
/* a>=b ? */
#define SEQ_GEQ(a,b)    ((int16_t)((uint16_t)(a) - (uint16_t)(b)) >= 0)

/* a<b ? */
#define CLAMP_LT(a,b)     ((int32_t)((uint32_t)(a) - (uint32_t)(b)) < 0)
/* a<=b ? */
#define CLAMP_LEQ(a,b)    ((int32_t)((uint32_t)(a) - (uint32_t)(b)) <= 0)
/* a>b ? */
#define CLAMP_GT(a,b)     ((int32_t)((uint32_t)(a) - (uint32_t)(b)) > 0)
/* a>=b ? */
#define CLAMP_GEQ(a,b)    ((int32_t)((uint32_t)(a) - (uint32_t)(b)) >= 0)

#define UINT32_MIN(a, b)  ((a>b)?b:a)

#define CALC_CLAMP_THRESHOLD  (((((1U << 32) - 1) / (6)) - 6)) // 32bit, clamp_tot_cnt=6
#define CALC_ACKSEQ_THRESHOLD (((((1U << 16) - 1) / (SPISYNC_RXSLOT_COUNT)) - SPISYNC_RXSLOT_COUNT)) // uint16, rx_slat_cnt=SPISYNC_RXSLOT_COUNT

/* spisync events */
#define EVT_SPISYNC_RESET_BIT       (1<<1)
#define EVT_SPISYNC_CTRL_BIT        (1<<2)
#define EVT_SPISYNC_POLL_BIT        (1<<3)
#define EVT_SPISYNC_DUMP_BIT        (1<<4)
#define EVT_SPISYNC_ALL_BIT         (EVT_SPISYNC_RESET_BIT | \
                                     EVT_SPISYNC_CTRL_BIT  | \
                                     EVT_SPISYNC_POLL_BIT | \
                                     EVT_SPISYNC_DUMP_BIT)

/* hardcode */
#define D_STREAM_OFFSET             (3)
#define D_QUEUE_OFFSET              (0)
#define PAYLOADBUF0_SEG_OFFSET      (4)

static int _is_streamtype(uint8_t type)
{
    if ((SPISYNC_TYPESTREAM_AT == type) ||
    	(SPISYNC_TYPESTREAM_USER1 == type) ||
		(SPISYNC_TYPESTREAM_USER2 == type)) {
        return 1;
    }

    return 0;
}
static int _is_msgtype(uint8_t type)
{
    if ((SPISYNC_TYPEMSG_PBUF == type) ||
    	(SPISYNC_TYPEMSG_SYSCTRL == type) ||
		(SPISYNC_TYPEMSG_USER1 == type)) {
        return 1;
    }

    return 0;
}

/*--------------------- internal process module ----------------------*/
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

int _spisync_reset_clamp(spisync_t *spisync)
{
    for (int i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        spisync->p_tx->slot[i].tag.clamp[0] = SPISYNC_RXMSG_PBUF_ITEMS;
        spisync->p_tx->slot[i].tag.clamp[1] = SPISYNC_RXMSG_SYSCTRL_ITEMS;
        spisync->p_tx->slot[i].tag.clamp[2] = SPISYNC_RXMSG_USER1_ITEMS;
        spisync->p_tx->slot[i].tag.clamp[3] = SPISYNC_RXSTREAM_AT_BYTES;
        spisync->p_tx->slot[i].tag.clamp[4] = SPISYNC_RXSTREAM_USER1_BYTES;
        spisync->p_tx->slot[i].tag.clamp[5] = SPISYNC_RXSTREAM_USER2_BYTES;
    }

    return 0;
}

int _spisync_update_clamp(spisync_t *spisync, uint8_t type, uint16_t cnt)
{
    spisync_trace("-------- update clamp type:%d, cnt:%d\r\n", type, cnt);

    for (int i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        if (_is_streamtype(type)) {
            spisync->p_tx->slot[i].tag.clamp[type] += cnt;
        } else if (_is_msgtype(type)) {
            spisync->p_tx->slot[i].tag.clamp[type] += cnt;
        } else {
            return -1;
        }
    }

    return 0;
}

int spisync_set_clamp(spisync_t *spisync, uint8_t type, uint32_t cnt)
{
    spisync_trace("-------- update clamp type:%d, cnt:%d\r\n", type, cnt);

    for (int i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        if (_is_streamtype(type)) {
            spisync->p_tx->slot[i].tag.clamp[type] = cnt;
        } else if (_is_msgtype(type)) {
            spisync->p_tx->slot[i].tag.clamp[type] = cnt;
        } else {
            return -1;
        }
    }

    return 0;
}

int _spisync_reset_localvar(spisync_t *spisync, char init)
{
    /* global val */
    spisync->tx_seq             = 0;
    spisync->txtag_errtimes     = 0;
    spisync->rxpattern_checked  = 0;
    if (init) {
        spisync->ps_status      = 0;
        spisync->enablerxtx     = 0;
    } else {
        spisync->enablerxtx     = 1;
    }
    for (int i = 0; i < SPISYNC_TYPEMAX; i++) {
        spisync->clamp[i] = 0;
    }

    /* global debug */
    spisync->isr_rxcnt        = 0;
    spisync->isr_txcnt        = 0;
    spisync->rxcrcerr_cnt     = 0;
    if (init) {
        spisync->tsk_rstcnt   = 0;
    }
    spisync->tsk_writecnt     = 0;
    spisync->tsk_readcnt      = 0;

    return 0;
}

static int _spisync_reset_queuestream(spisync_t *spisync)
{
    printf("_spisync_reset_queuestream start\r\n");
    /* Reset Queue */
    for (int i = 0; i < 3; i++) {
        while (uxQueueMessagesWaiting(spisync->prx_queue[i]) > 0) {
            spisync_msg_t msg;
            int res;
            res = xQueueReceive(spisync->prx_queue[i], &msg, 0);
            if ((res == pdTRUE) && (msg.cb)) {
                // free all queue msg.
                msg.cb(msg.cb_arg);
            } else {
                // This point should not be reached under normal circumstances
                spisync_err("This point should not be reached under normal circumstances.\r\n");
            }
        }
    }
    for (int i = 0; i < 3; i++) {
        while (uxQueueMessagesWaiting(spisync->ptx_queue[i]) > 0) {
            spisync_msg_t msg;
            int res;
            res = xQueueReceive(spisync->ptx_queue[i], &msg, 0);
            if ((res == pdTRUE) && (msg.cb)) {
                // free all queue msg.
                msg.cb(msg.cb_arg);
            } else {
                // This point should not be reached under normal circumstances
                spisync_err("This point should not be reached under normal circumstances.\r\n");
            }
        }
    }

    /* Reset Stream */
    xStreamBufferReset(spisync->prx_streambuffer[0]);
    xStreamBufferReset(spisync->prx_streambuffer[1]);
    xStreamBufferReset(spisync->prx_streambuffer[2]);
    xStreamBufferReset(spisync->ptx_streambuffer[0]);
    xStreamBufferReset(spisync->ptx_streambuffer[1]);
    xStreamBufferReset(spisync->ptx_streambuffer[2]);

    printf("_spisync_reset_queuestream end\r\n");

    return 0;
}

static int _spisync_reset_tag(spisync_t *spisync, char init)
{
    /* set version */
    for (int i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        spisync->p_tx->slot[i].tag.version = SPISYNC_SLOT_VERSION;
        if (init) {
            spisync->p_tx->slot[i].tag.flag |= SPISYNC_TAGFLAGBIT_RXERR;
        }
    }

    return 0;
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

    spisync->dbg_tick_old     = xTaskGetTickCount();
    spisync->isr_txcnt_old    = 0;
    spisync->isr_rxcnt_old    = 0;
    spisync->tsk_writecnt_old = 0;
    spisync->tsk_readcnt_old  = 0;
}

static int _reset_spisync(spisync_t *spisync)
{
	spisync_log("start resetting\r\n");
    /* clear slot */
    memset(spisync->p_tx, 0, sizeof(spisync_txbuf_t));
    memset(spisync->p_rx, 0, sizeof(spisync_rxbuf_t));
    /* set tag */
    _spisync_reset_tag(spisync, 0);
    /* set clamp */
    _spisync_reset_clamp(spisync);
    /* for rx notify */
    xEventGroupClearBits(spisync->event, EVT_SPISYNC_ALL_BIT);
    /* Reset StreamBuffer */
    _spisync_reset_queuestream(spisync);
    /* local val sequence+debug */
    _spisync_reset_localvar(spisync, 0);// 1 init, 0 reset
    debug_init_reset(spisync, 0);
    spisync_log("reset done\r\n");
    return 0;
}

static int check_slot_tag(spisync_t *spisync)
{
    int i, error = 0;
    char *reason = "unknown";

    for (i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        if (SPISYNC_SLOT_VERSION != spisync->p_rx->slot[i].tag.version) {
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
		spisync_err("spisync needs reset, %s, slot %d magic 0x%lx, flag 0x%x, err time %d\r\n",
				reason, i, spisync->p_rx->slot[i].tag.version,
				spisync->p_rx->slot[i].tag.flag, spisync->txtag_errtimes);
	}
    return error;
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

        avail = uxQueueMessagesWaiting(s->ptx_queue[i]);
        if (avail > 0)
        	return 0;
    }
    return 1;
}

static uint16_t get_latest_ackseq(spisync_t *spisync);

static inline int spisync_has_unacked_data(spisync_t *s)
{
    uint16_t acked_seq;

    acked_seq = get_latest_ackseq(s);
    return SEQ_GT(s->tx_seq, acked_seq) > 0;
}

static inline int spisync_active(spisync_t *s)
{
    return !spisync_sndbuf_empty(s) ||
            spisync_has_unacked_data(s) ||
            spisync_slave_data_ready();
}

static void __ramsync_low_rx_cb(void *arg)
{
	spisync_t *spisync = (spisync_t *)arg;

    /* debug */
    if (spisync) {
        spisync->isr_rxcnt++;
    }

    _spisync_setevent(spisync, EVT_SPISYNC_POLL_BIT);
}

static void __ramsync_low_tx_cb(void *arg)
{
    spisync_t *spisync = (spisync_t *)arg;

    /* debug */
    if (spisync) {
        spisync->isr_txcnt++;
    }
    /* there is no need to generate POLL event on both TX/RX completion interrupt */
}

static void __ramsync_low_reset_cb(void *arg)
{
}

void spisync_reset_timer_callback(TimerHandle_t xTimer)
{
    spisync_t *spisync = (spisync_t *)pvTimerGetTimerID(xTimer);

    spisync_trace("timer callback sets reset bit\r\n");
    _spisync_setevent(spisync, EVT_SPISYNC_RESET_BIT);
}

/* Note: Return -1 on error and show no mercy to allocated memory during the process. */
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
    if (config) {
    	spisync->config = config->hw;
    	memcpy(spisync->sync_ops, config->ops, sizeof(config->ops));
    }
    spisync->p_tx = (spisync_txbuf_t *)malloc_aligned_with_padding_nocache(sizeof(spisync_txbuf_t), 32);
    if (!spisync->p_tx) {
    	spisync_err("no mem for tx buffer\r\n");
    	return -1;
    }
    spisync->p_rx = (spisync_rxbuf_t *)malloc_aligned_with_padding_nocache(sizeof(spisync_rxbuf_t), 32);
    if (!spisync->p_rx) {
    	spisync_err("no mem for rx buffer\r\n");
    	return -1;
    }
    spisync->p_rxcache = (spisync_slot_t *)pvPortMalloc(sizeof(spisync_slot_t));
    if (!spisync->p_rxcache) {
    	spisync_err("no mem for rxcache\r\n");
    	return -1;
    }
    memset(spisync->p_tx, 0, sizeof(spisync_txbuf_t));
    memset(spisync->p_rx, 0, sizeof(spisync_rxbuf_t));
    memset(spisync->p_rxcache, 0, sizeof(spisync_slot_t));

    /* dma node list update */
    for (i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        node_txbuf[i].buf = &spisync->p_tx->slot[i];
        node_txbuf[i].len = sizeof(spisync_slot_t);
    }
    for (i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        node_rxbuf[i].buf = &spisync->p_rx->slot[i];
        node_rxbuf[i].len = sizeof(spisync_slot_t);
    }
    // update version
    _spisync_reset_tag(spisync, 0);
    /* global init*/
    _spisync_reset_localvar(spisync, 0);

    spisync->event = xEventGroupCreate();
    if (!spisync->event) {
    	spisync_err("failed to create event group\r\n");
    	return -1;
    }
    spisync->timer = xTimerCreate("delay_rst",
                                  pdMS_TO_TICKS(SPISYNC_PATTERN_DURATION_MS),
                                  pdFALSE, (void *)spisync, spisync_reset_timer_callback);
    if (!spisync->timer) {
    	spisync_err("failed to create timer\r\n");
    	return -1;
    }
    spisync->write_lock = xSemaphoreCreateMutex();
    if (!spisync->write_lock) {
    	spisync_err("failed to create write lock\r\n");
    	return -1;
    }
    spisync->opspin_log = xSemaphoreCreateMutex();
    if (!spisync->opspin_log) {
    	spisync_err("failed to create opspin_log");
    	return -1;
    }

    spisync->prx_queue[0] = xQueueCreate(SPISYNC_RXMSG_PBUF_ITEMS, sizeof(spisync_msg_t));
    if (!spisync->prx_queue[0]) {
    	spisync_err("failed to create rx queue0\r\n");
    	return -1;
    }
    spisync->prx_queue[1] = xQueueCreate(SPISYNC_RXMSG_SYSCTRL_ITEMS, sizeof(spisync_msg_t));
    if (!spisync->prx_queue[1]) {
        spisync_err("failed to create rx queue1\r\n");
        return -1;
    }
    spisync->prx_queue[2] = xQueueCreate(SPISYNC_RXMSG_USER1_ITEMS, sizeof(spisync_msg_t));
    if (!spisync->prx_queue[2]) {
        spisync_err("failed to create rx queue2\r\n");
        return -1;
    }
    spisync->ptx_queue[0] = xQueueCreate(SPISYNC_TXMSG_PBUF_ITEMS, sizeof(spisync_msg_t));
    if (!spisync->ptx_queue[0]) {
    	spisync_err("failed to create tx queue0\r\n");
        return -1;
    }
    spisync->ptx_queue[1] = xQueueCreate(SPISYNC_TXMSG_SYSCTRL_ITEMS, sizeof(spisync_msg_t));
    if (!spisync->ptx_queue[1]) {
    	spisync_err("failed to create tx queue1\r\n");
        return -1;
    }
    spisync->ptx_queue[2] = xQueueCreate(SPISYNC_TXMSG_USER1_ITEMS, sizeof(spisync_msg_t));
    if (!spisync->ptx_queue[2]) {
        spisync_err("failed to create tx queue2\r\n");
        return -1;
    }
    spisync->prx_streambuffer[0] = xStreamBufferCreate(SPISYNC_RXSTREAM_AT_BYTES, 1);
    if (!spisync->prx_streambuffer[0]) {
    	spisync_err("failed to create rx streambuffer0\r\n");
    	return -1;
    }
    spisync->prx_streambuffer[1] = xStreamBufferCreate(SPISYNC_RXSTREAM_USER1_BYTES, 1);
    if (!spisync->prx_streambuffer[1]) {
        spisync_err("failed to create rx streambuffer1\r\n");
        return -1;
    }
    spisync->prx_streambuffer[2] = xStreamBufferCreate(SPISYNC_RXSTREAM_USER2_BYTES, 1);
    if (!spisync->prx_streambuffer[2]) {
        spisync_err("failed to create rx streambuffer2\r\n");
        return -1;
    }
    spisync->ptx_streambuffer[0] = xStreamBufferCreate(SPISYNC_TXSTREAM_AT_BYTES, 1);
    if (!spisync->ptx_streambuffer[0]) {
        spisync_err("failed to create tx streambuffer0\r\n");
        return -1;
    }
    spisync->ptx_streambuffer[1] = xStreamBufferCreate(SPISYNC_TXSTREAM_USER1_BYTES, 1);
    if (!spisync->ptx_streambuffer[1]) {
        spisync_err("failed to create tx streambuffer1\r\n");
        return -1;
    }
    spisync->ptx_streambuffer[2] = xStreamBufferCreate(SPISYNC_TXSTREAM_USER2_BYTES, 1);
    if (!spisync->ptx_streambuffer[2]) {
        spisync_err("failed to create tx streambuffer2\r\n");
        return -1;
    }

    spisync->state = SPISYNC_STATE_INIT;
    ret = xTaskCreate(spisync_task,
                (char*)"spisync",
                SPISYNC_STACK_SIZE/sizeof(StackType_t), spisync,
                SPISYNC_TASK_PRI,
                &spisync->task_handle);
    if (ret != pdPASS) {
        printf("failed to create spisync task, %ld\r\n", ret);
        return -1;
    }

    /* set type , and set name */
    lramsync_init(
        config,
        &spisync->hw,
        node_txbuf, SPISYNC_TXSLOT_COUNT,
        __ramsync_low_tx_cb, spisync,
        node_rxbuf, SPISYNC_RXSLOT_COUNT,
        __ramsync_low_rx_cb, spisync,
        __ramsync_low_reset_cb, spisync
        );

    /* master needs to reset for re-synchronization */
    _spisync_setevent(spisync, EVT_SPISYNC_RESET_BIT);
    return 0;
}

int spisync_deinit(spisync_t *spisync)
{
    /* NOT SUPPORT */
    return 0;
}

int spisync_read(spisync_t *spisync, spisync_msg_t *msg, uint32_t flags)
{
    int ret = 0;
    uint32_t clamp_offset;

    if ((NULL == spisync) || (NULL == msg)) {
        spisync_log("arg err, spisync:%p\r\n", spisync);
        return -1;
    }

    if (_is_streamtype(msg->type)) {
        ret = xStreamBufferReceive(spisync->prx_streambuffer[msg->type-D_STREAM_OFFSET], msg->buf, msg->buf_len, msg->timeout);

        // update clamp_offset arg
        if ((ret > 0) && (ret <= msg->buf_len)) {
            clamp_offset = ret;
        } else {
            clamp_offset = 0;
        }
    } else if (_is_msgtype(msg->type)) {
        spisync_msg_t tmpmsg;
        memcpy(&tmpmsg, msg, sizeof(tmpmsg));
        ret = xQueueReceive(spisync->prx_queue[msg->type-D_QUEUE_OFFSET], &tmpmsg, msg->timeout);
        spisync_trace("spisync1:%p, ret:%d, buf_len:%d\r\n", spisync, ret, tmpmsg.buf_len);
        if ((ret == pdTRUE) && (msg->buf) && (msg->buf_len >= tmpmsg.buf_len)) {
            msg->buf_len = tmpmsg.buf_len;
            memcpy(msg->buf, tmpmsg.buf, msg->buf_len);

            if (tmpmsg.cb) {
                spisync_trace("tmpmsg.cb_arg:%p\r\n", tmpmsg.cb_arg);
                tmpmsg.cb(tmpmsg.cb_arg);
            }

            clamp_offset = 1;
        } else {
            spisync_trace("spisync2:%p, ret:%d, buf_len:%d\r\n", spisync, ret, tmpmsg.buf_len);

            clamp_offset = 0;
        }
    }

    /* update clamp */
    _spisync_update_clamp(spisync, msg->type, clamp_offset);
    return ret;
}

int spisync_fakewrite_forread(spisync_t *spisync, spisync_msg_t *msg, uint32_t flags)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int res;

    if ((NULL == spisync) || (NULL == msg) || (msg->buf_len > SPISYNC_PAYLOADBUF_LEN)) {
        spisync_log("arg err, spisync:%p, buf_len:%d, max_len:%d\r\n",
                spisync, msg->buf_len, SPISYNC_PAYLOADBUF_LEN);
        return -1;
    }

    // FIXME: only support AT
    if (xPortIsInsideInterrupt()) {
        res = xStreamBufferSendFromISR(spisync->prx_streambuffer[3-D_STREAM_OFFSET], msg->buf, msg->buf_len, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        res = xStreamBufferSend(spisync->prx_streambuffer[3-D_STREAM_OFFSET], msg->buf, msg->buf_len, msg->timeout);
    }

    return res;
}

static void _msg_free(void *p)
{
    spisync_trace("----------> free:%p\r\n", p);
    vPortFree(p);
}

static void *_msg_malloc(int len)
{
    void *p = pvPortMalloc(len);
    spisync_trace("++++++++++> malloc:%p, len:%d\r\n", p, len);

    return p;
}

int spisync_write(spisync_t *spisync, spisync_msg_t *msg, uint32_t flags)
{
	if (!spisync || !msg)
		return -1;

	if (spisync->state != SPISYNC_STATE_ACTIVE && spisync->state != SPISYNC_STATE_IDLE)
		return -2;

    uint8_t type = msg->type;
    void *buf    = msg->buf;
    uint32_t len = msg->buf_len;
    uint32_t timeout_ms = msg->timeout;

    spisync_trace("spisync_write1 msg type:%d buf:%p buf_len:%d cb:%p cb_arg:%p\r\n",
        msg->type, msg->buf, msg->buf_len, msg->cb, msg->cb_arg);

    int ret = 0;
    if (type >= 6) {
        spisync_err("arg error. spisync:%p, len:%d, type:%u\r\n", spisync, len, type);
        return -1;
    }

    if (xSemaphoreTake(spisync->write_lock, timeout_ms) == pdTRUE) {
        if (_is_streamtype(type)) {
            ret = xStreamBufferSend(spisync->ptx_streambuffer[type-D_STREAM_OFFSET], buf, len, timeout_ms);
            if (ret != len) {
                spisync_trace("spisync_write ret:%d\r\n", ret);
            }
        } else if (_is_msgtype(type)) {
            ret = xQueueSendToBack(spisync->ptx_queue[type-D_QUEUE_OFFSET], msg, timeout_ms);
            spisync_trace("xQueueSendToBack MSG type:%d timeout:%d buf:%p buf_len:%d cb:%p cb_arg:%p, ret:%d\r\n",
                msg->type, msg->timeout, msg->buf, msg->buf_len, msg->cb, msg->cb_arg, ret);
            if (ret ==  pdPASS) {
                ret = 1;
            }
        } else {
            spisync_trace("never here.\r\n");
        }
        xSemaphoreGive(spisync->write_lock);
        _spisync_setevent(spisync, EVT_SPISYNC_POLL_BIT);
    } else {
        spisync_log("take mutex error, so write NULL.");
        ret = -1;
    }
    return ret;
}

int spisync_build_typecb(spisync_ops_t *msg, uint8_t type, spisync_opscb_cb_t cb, void *cb_pri)
{
    if ((!msg) || (type >= 6)) {
        return -1;
    }

    memset(msg, 0, sizeof(spisync_ops_t));
    msg->type   = type;
    msg->cb     = cb;
    msg->cb_pri = cb_pri;

    return 0;
}

int spisync_reg_typecb(spisync_t *spisync, spisync_ops_t *msg)
{
    if ((NULL == spisync) || (msg->type >= 6)) {
        return -1;
    }

    spisync->sync_ops[msg->type].cb = msg->cb;
    spisync->sync_ops[msg->type].cb_pri = msg->cb_pri;

    return 0;
}

static uint32_t get_latest_clamp(spisync_t *spisync, uint8_t type)
{
    uint32_t latest = spisync->p_rx->slot[0].tag.clamp[type];

    for (uint32_t i = 1; i < SPISYNC_RXSLOT_COUNT; i++) {
        if ((spisync->p_rx->slot[i].tag.clamp[type] > latest && (spisync->p_rx->slot[i].tag.clamp[type] - latest) <= CALC_CLAMP_THRESHOLD) ||
            (spisync->p_rx->slot[i].tag.clamp[type] < latest && (latest - spisync->p_rx->slot[i].tag.clamp[type]) > CALC_CLAMP_THRESHOLD)) {
            latest = spisync->p_rx->slot[i].tag.clamp[type];
        }
    }

    return latest;
}

static uint16_t get_latest_ackseq(spisync_t *spisync)
{
    uint16_t latest = spisync->p_rx->slot[0].tag.ackseq;

    for (uint32_t i = 1; i < SPISYNC_RXSLOT_COUNT; i++) {
        if ((spisync->p_rx->slot[i].tag.ackseq > latest && (spisync->p_rx->slot[i].tag.ackseq - latest) <= CALC_ACKSEQ_THRESHOLD) ||
            (spisync->p_rx->slot[i].tag.ackseq < latest && (latest - spisync->p_rx->slot[i].tag.ackseq) > CALC_ACKSEQ_THRESHOLD)) {
            latest = spisync->p_rx->slot[i].tag.ackseq;
        }
    }

    return latest;
}

int _build_msg(spisync_msg_t *msg,
                        uint32_t type,
                        void *buf,
                        uint32_t len,
                        uint32_t timeout)
{
    memset(msg, 0, sizeof(spisync_msg_t));
    msg->type       = type;
    msg->buf_len    = len;
    msg->timeout    = timeout;

    if (_is_streamtype(msg->type)) {
        // No need to malloc in the stream case
        msg->buf    = buf;
    } else {
        // need to malloc in the stream case
        /* malloc and copy */
        msg->buf = _msg_malloc(msg->buf_len);
        if (!msg->buf) {
            return -1;
        }
        memcpy(msg->buf, buf, msg->buf_len);
        /* for cb free */
        msg->cb     = _msg_free;
        msg->cb_arg = msg->buf;
    }

    return 0;
}

int spisync_build_msg(spisync_msg_t *msg,
                        uint32_t type,
                        void *buf,
                        uint32_t len,
                        uint32_t timeout)
{
    memset(msg, 0, sizeof(spisync_msg_t));
    msg->type       = type;
    msg->buf        = buf;
    msg->buf_len    = len;
    msg->timeout    = timeout;

    return 0;
}

static void spisync_start_announce_reset(spisync_t *s)
{
	int state;
	BaseType_t ret;

	memset(s->p_tx, SPISYNC_RESET_PATTERN, sizeof(spisync_txbuf_t));
	ret = xTimerChangePeriod(s->timer, SPISYNC_PATTERN_DURATION_MS, 1000);
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

	lramsync_stop(&s->hw);
	ret = xTimerChangePeriod(s->timer, SPISYNC_RESYNC_WAIT_DURATION_MS, 1000);
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

#define SPISYNC_DEFER_TXN	15

static void spisync_goto_active(spisync_t *s, int reset)
{
	int state;

	state = s->state;
	s->state = SPISYNC_STATE_ACTIVE;
	s->countdown = SPISYNC_DEFER_TXN;
	spisync_log("spisync state %s --> active\r\n", spisync_state_str[state]);
	if (reset) {
		_reset_spisync(s);
		lramsync_start(&s->hw);
	} else {
		lramsync_resume(&s->hw);
	}
}

/*---------------------- moudule ------------------------*/
static void _ctrl_process(spisync_t *spisync)
{
}

void _write_process(spisync_t *spisync)
{
    int empty_slot, empty_flag;
    uint16_t latest_rseq, s_txseq;

    int available_bytes;
    uint32_t data_len;
    StreamBufferHandle_t current_stream_buffer;
    uint32_t current_stream_len;
    uint8_t current_pack_type;
    struct crc32_stream_ctx crc_ctx;
    uint32_t rclamp;
    int32_t diff;
    uint8_t type;
    spisync_msg_t msg;
    int res;
    int slot_start = 0;

    /* Check if there is an empty slot */
    empty_slot = 0;
    empty_flag = 0;

    slot_start++;
    for (int m = slot_start; m < (slot_start+SPISYNC_TXSLOT_COUNT); m++) {
        latest_rseq = get_latest_ackseq(spisync);
        s_txseq = spisync->p_tx->slot[m%SPISYNC_TXSLOT_COUNT].tag.seq;

        if (SEQ_GEQ(latest_rseq, s_txseq)) {
            empty_slot = m%SPISYNC_TXSLOT_COUNT;
            empty_flag = 1;
            spisync_trace("latest_rseq:%d, s_txseq:%d, empty_slot:%d\r\n",
                        latest_rseq, s_txseq, empty_slot);
            break;
        }
    }
    if (!empty_flag) {
        return;
    }

    /* check if there is valid bytes */
    if (xSemaphoreTake(spisync->opspin_log, portMAX_DELAY) == pdTRUE) {
        for (type = 0; type < 6; type++) {
            if (_is_streamtype(type)) {
                available_bytes = xStreamBufferBytesAvailable(spisync->ptx_streambuffer[type-D_STREAM_OFFSET]);
                if (available_bytes <= 0) {
                    continue;
                }

                rclamp = get_latest_clamp(spisync, type);
                spisync_trace("rclamp:%d, local_clamp:%d\r\n", rclamp, spisync->clamp[type]);
                if (CLAMP_LEQ(rclamp, spisync->clamp[type])) {
                	spisync->tx_throttled[type]++;
                    continue;
                }

                diff = rclamp - spisync->clamp[type];
                current_pack_type = type;
                current_stream_buffer = spisync->ptx_streambuffer[type-D_STREAM_OFFSET];
                current_stream_len = UINT32_MIN((UINT32_MIN(diff, available_bytes)), SPISYNC_PAYLOADBUF_LEN);
                spisync_trace("type:%d, len:%ld, recv_clamp:%d, local_clamp:%d, available:%d\r\n",
                        current_pack_type, current_stream_len, rclamp, spisync->clamp[type], available_bytes);
            } else if (_is_msgtype(type)) {
                if (uxQueueMessagesWaiting(spisync->ptx_queue[type-D_QUEUE_OFFSET]) == 0) {
                    continue;
                }

                rclamp = get_latest_clamp(spisync, type);
                spisync_trace("rclamp:%d, local_clamp:%d\r\n", rclamp, spisync->clamp[type]);
                if (CLAMP_LEQ(rclamp, spisync->clamp[type])) {
                	//printf("throttled, %lu - %lu\r\n", spisync->clamp[type], rclamp);
                	spisync->tx_throttled[type]++;
                    continue;
                }

                current_pack_type = type;
            } else {
                spisync_err("never here type:%d.\r\n", type);
                continue;
            }
            break;
        }
    }
    xSemaphoreGive(spisync->opspin_log);

    if (type == 6) {// can't write to slot
        return;
    }

    /* pop from msgbuffer to slot */
    if (_is_streamtype(current_pack_type)) {
        data_len = xStreamBufferReceive(
                current_stream_buffer,
                spisync->p_tx->slot[empty_slot].payload.seg[0].buf,
                current_stream_len,
                0);
        spisync_trace("------------- write-> ackseq:%d, seq:%d, slot:%d, len:%d, avail:%d, current:%d, oldseq:%d\r\n",
                latest_rseq, (spisync->tx_seq + 1), empty_slot, data_len, available_bytes, current, s_txseq);
        if ((data_len <= 0) || (data_len > SPISYNC_PAYLOADBUF_LEN)) {
            spisync_err("write process never here. except spi reset?, data_len:%d\r\n", data_len);
            return;
        }
        // update clamp
        spisync->clamp[current_pack_type] += data_len;
    } else if (_is_msgtype(current_pack_type)) {
        res = xQueueReceive(spisync->ptx_queue[type-D_QUEUE_OFFSET], &msg, 0);
        if (pdTRUE != res) {
            spisync_err("never here type:%d.\r\n", type);
            return;
        }

        spisync_trace("xQueueReceive res:%d, msg type:%d buf:%p buf_len:%d cb:%p cb_arg:%p\r\n",
                res, msg.type, msg.buf, msg.buf_len, msg.cb, msg.cb_arg);
        //return;
#if 1
        if ((msg.buf_len > 0) && (msg.buf_len <= SPISYNC_PAYLOADBUF_LEN)) {
            memcpy(spisync->p_tx->slot[empty_slot].payload.seg[0].buf, msg.buf, msg.buf_len);
        } else {
            spisync_err("write process never here. pbuf too long?\r\n");
            if (msg.cb) {
                msg.cb(msg.cb_arg);
            }
            return;
        }
        if (msg.cb) {
            msg.cb(msg.cb_arg);
        }
        data_len = msg.buf_len;
        // update clamp
        spisync->clamp[current_pack_type] += 1;
#endif
    } else {
        spisync_err("never here type:%d.\r\n", type);
        return;
    }
    // update clamp
    spisync->tx_seq++;

    spisync->p_tx->slot[empty_slot].tag.seq = spisync->tx_seq;
    spisync->p_tx->slot[empty_slot].payload.tot_len = data_len + 4;// seg_len_type_res
    spisync->p_tx->slot[empty_slot].payload.seg[0].len = data_len;
    spisync->p_tx->slot[empty_slot].payload.seg[0].type = current_pack_type;

    // CRC check
    utils_crc32_stream_init(&crc_ctx);
    spisync_trace("utils_crc32_stream_feed_block slot:%d, buf:%p, len:%d\r\n",
            empty_slot,
            (const uint8_t *)&spisync->p_tx->slot[empty_slot].payload,
            spisync->p_tx->slot[empty_slot].payload.seg[0].len);
    utils_crc32_stream_feed_block(&crc_ctx,
                                  (const uint8_t *)&spisync->p_tx->slot[empty_slot].payload.tot_len,
                                  spisync->p_tx->slot[empty_slot].payload.tot_len + 4);
    spisync->p_tx->slot[empty_slot].payload.crc = utils_crc32_stream_results(&crc_ctx);
    spisync->p_tx->slot[empty_slot].tail.seq_mirror = spisync->tx_seq;

    // debug
    spisync->tsk_writecnt += 1;

    spisync_trace("write slot:%d, latest_rseq:%d, Txseq:%d, len:%d,  %d-%d-%d-%d  %d-%d-%d-%d\r\n",
            empty_slot, latest_rseq, spisync->tx_seq, data_len,
            spisync->p_tx->slot[0].tag.seq,
            spisync->p_tx->slot[1].tag.seq,
            spisync->p_tx->slot[2].tag.seq,
            spisync->p_tx->slot[3].tag.seq,
            spisync->p_rx->slot[0].tag.ackseq,
            spisync->p_rx->slot[1].tag.ackseq,
            spisync->p_rx->slot[2].tag.ackseq,
            spisync->p_rx->slot[3].tag.ackseq
            );
}

static void _read_process(spisync_t *spisync)
{
    uint32_t lentmp;
    uint32_t crctmp;
    uint16_t seqtmp, rseqtmp;
    struct crc32_stream_ctx crc_ctx;
    int read_times = SPISYNC_RX_READTIMES;
    uint8_t pack_type;

    while (read_times) {
        for (int i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
            // check version
            if (SPISYNC_SLOT_VERSION != spisync->p_rx->slot[i].tag.version) {
                continue;
            }

            // check seq
            if (spisync->p_rx->slot[i].tag.seq != spisync->p_rx->slot[i].tail.seq_mirror) {
                continue;
            }

            // update tmp
            rseqtmp = spisync->p_tx->slot[i].tag.ackseq;
            seqtmp  = spisync->p_rx->slot[i].tag.seq;
            lentmp  = spisync->p_rx->slot[i].payload.tot_len;
            pack_type = (spisync->p_rx->slot[i].payload.seg[0].type)&0xFF;

            // check seq len valid
            if ((lentmp > SPISYNC_PAYLOADLEN_MAX) || ((uint16_t)seqtmp != (uint16_t)(rseqtmp + 1)) || (pack_type >= 6)) {
                continue;
            }

            // Check available spaces in the buffer in for rx

            // copy data and crc
            memcpy(spisync->p_rxcache, &spisync->p_rx->slot[i], lentmp+sizeof(slot_tag_t)+4);
            spisync->p_rxcache->payload.crc = spisync->p_rx->slot[i].payload.crc;
            // Check if the relevant parameters are within the expected range
            if ((spisync->p_rxcache->tag.seq != seqtmp) ||
                (spisync->p_rxcache->payload.tot_len != lentmp) ||
                ((spisync->p_rxcache->payload.seg[0].type & 0xFF) != pack_type)) {
                continue;
            }

            // check crc 
            utils_crc32_stream_init(&crc_ctx);
            utils_crc32_stream_feed_block(&crc_ctx,
                                  (const uint8_t *)&spisync->p_rxcache->payload.tot_len,
                                  spisync->p_rxcache->payload.tot_len + 4);
            crctmp = utils_crc32_stream_results(&crc_ctx);
            if (spisync->p_rxcache->payload.crc != crctmp) {
                spisync->rxcrcerr_cnt++;
                spisync_trace("crc err, 0x%08X != 0x%08X\r\n", crctmp, spisync->p_rxcache->payload.crc);
                continue;
            }

            // push
            spisync->tsk_readcnt++;

            spisync_trace("recv slot %d rx seq:  %d,%d,%d,%d,%d,%d  %d,%d,%d,%d,%d,%d\r\n",
                    i,
                    spisync->p_rx->slot[0].tag.seq,
                    spisync->p_rx->slot[1].tag.seq,
                    spisync->p_rx->slot[2].tag.seq,
                    spisync->p_rx->slot[3].tag.seq,
                    spisync->p_rx->slot[4].tag.seq,
                    spisync->p_rx->slot[5].tag.seq,
                    spisync->p_tx->slot[0].tag.ackseq,
                    spisync->p_tx->slot[1].tag.ackseq,
                    spisync->p_tx->slot[2].tag.ackseq,
                    spisync->p_tx->slot[3].tag.ackseq,
                    spisync->p_tx->slot[4].tag.ackseq,
                    spisync->p_tx->slot[5].tag.ackseq
                    );

            /* handle by buf buf_len */
            if (spisync->sync_ops[pack_type].cb && spisync->sync_ops[pack_type].cb) {
                // zero copy
                spisync_opscb_arg_t msg;
                msg.buf     = spisync->p_rxcache->payload.seg[0].buf;
                msg.buf_len = (lentmp > PAYLOADBUF0_SEG_OFFSET) ? (lentmp - PAYLOADBUF0_SEG_OFFSET) : 0;
                spisync->sync_ops[pack_type].cb(spisync->sync_ops[pack_type].cb_pri, &msg);
                // _spisync_update_clamp(spisync, pack_type, 1);// clamp++
            } else {
                // normal to stream or queue
                if (_is_streamtype(pack_type)) {
                    if ((lentmp - PAYLOADBUF0_SEG_OFFSET) != xStreamBufferSend(spisync->prx_streambuffer[pack_type-D_STREAM_OFFSET],
                            spisync->p_rxcache->payload.seg[0].buf, (lentmp - PAYLOADBUF0_SEG_OFFSET), 0)) {
                        spisync_err("flowctrl warning. except spi reset?\r\n");
                    }
                } else {
                    spisync_msg_t msg;
                    if (_build_msg(&msg, pack_type, spisync->p_rxcache->payload.seg[0].buf,
                        (lentmp - PAYLOADBUF0_SEG_OFFSET), 0) == 0) {
                        if (xQueueSendToBack(spisync->prx_queue[pack_type-D_QUEUE_OFFSET], &msg, 0)) {
                            // if (msg.cb) {
                            //     msg.cb(msg.buf);
                            // }
                        }
                    } else {
                        printf("mem error --> drop msg %d\r\n", (lentmp - PAYLOADBUF0_SEG_OFFSET));
                    }
                }
            }

            /* update tx st ackseq */
            for (int m = 0; m < SPISYNC_TXSLOT_COUNT; m++) {
                spisync->p_tx->slot[m].tag.ackseq = spisync->p_rxcache->tag.seq;
            }
        }
        read_times--;
    }
}

static void spisync_active_process(spisync_t *s, EventBits_t events)
{
	/*
	 * 1. someone has some data to send.
	 * 2. spi dma interrupt requests to poll.
	 * 3. slave requests to send data.
	 */
	if (events & EVT_SPISYNC_POLL_BIT) {
		/* start to reset if corrupted data is found */
		if (check_slot_tag(s)) {
			spisync_start_announce_reset(s);
			return;
		}

		/* read data if any */
		_read_process(s);
		/* send data is any */
		_write_process(s);

		if (!spisync_active(s)) {
			s->countdown--;
			if (s->countdown <= 0)
				spisync_goto_idle(s);
		} else {
			/* Reset the countdown */
			s->countdown = SPISYNC_DEFER_TXN;
		}

	}
}

#if SPISYNC_DEBUG_PRINT
static void print_debug_stats(spisync_t *spisync)
{
    uint32_t current_tick = xTaskGetTickCount();

    uint32_t tick_diff = current_tick - spisync->dbg_tick_old;

    if (tick_diff >= 1000) {
        uint32_t isr_txcnt_diff = spisync->isr_txcnt - spisync->isr_txcnt_old;
        uint32_t isr_rxcnt_diff = spisync->isr_rxcnt - spisync->isr_rxcnt_old;
        uint32_t tsk_writecnt_diff = spisync->tsk_writecnt - spisync->tsk_writecnt_old;
        uint32_t tsk_readcnt_diff = spisync->tsk_readcnt - spisync->tsk_readcnt_old;

        uint32_t bps = isr_txcnt_diff * sizeof(spisync_slot_t) * 8;

        spisync_dbg("Tick delta: %"PRIu32"\r\n", tick_diff);
        spisync_dbg("ISR TX delta: %"PRIu32"\r\n", isr_txcnt_diff);
        spisync_dbg("ISR RX delta: %"PRIu32"\r\n", isr_rxcnt_diff);
        spisync_dbg("Task Write Count delta: %"PRIu32"\r\n", tsk_writecnt_diff);
        spisync_dbg("Task Read Count delta: %"PRIu32"\r\n", tsk_readcnt_diff);
        spisync_dbg("Bits Per Second (bps): %"PRIu32"\r\n", bps);

        spisync->dbg_tick_old = current_tick;
        spisync->isr_txcnt_old = spisync->isr_txcnt;
        spisync->isr_rxcnt_old = spisync->isr_rxcnt;
        spisync->tsk_writecnt_old = spisync->tsk_writecnt;
        spisync->tsk_readcnt_old = spisync->tsk_readcnt;
    }
}
#endif

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
            spisync_status(spisync);

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
        	if (eventBits & EVT_SPISYNC_POLL_BIT) {
				if (spisync_active(spisync))
					spisync_goto_active(spisync, 0);
        	}
            break;

        default:
        	spisync_err("spisync state 0x%x is invalid\r\n", spisync->state);
        	break;
        }
#if SPISYNC_DEBUG_PRINT
        print_debug_stats(spisync);
#endif
    }
}

/*--------------------- debug module ----------------------*/
/* debug */
int spisync_status(spisync_t *spisync)
{
    int i;

    if ((NULL == spisync) && (NULL == g_spisync_current)) {
        return -1;
    } else if (NULL == spisync) {
        spisync = g_spisync_current;
    }

    spisync_dbg("sizeof(slot_tag_t)    =%d\r\n", sizeof(slot_tag_t));
    spisync_dbg("sizeof(slot_payload_t)=%d\r\n", sizeof(slot_payload_t));
    spisync_dbg("sizeof(slot_tail_t)   =%d\r\n", sizeof(slot_tail_t));
    spisync_dbg("sizeof(spisync_slot_t) =%d\r\n", sizeof(spisync_slot_t));
    spisync_dbg("CLAMP_THRESHOLD:%d, ACKSEQ_THRESHOLD:%d\r\n", CALC_CLAMP_THRESHOLD, CALC_ACKSEQ_THRESHOLD);
    spisync_dbg("TxSlot--------------------------------------------------\r\n");
    for (i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        spisync_dbg("tx[%ld] 0x%04lX,seq:%d,ackseq:%ld,flag:%ld,clamp:%ld-%ld-%ld-%ld-%ld-%ld, len:%d,type:%02X,crc:0x%08lX\r\n",
                i,
                spisync->p_tx->slot[i].tag.version,
                spisync->p_tx->slot[i].tag.seq,
                spisync->p_tx->slot[i].tag.ackseq,
                spisync->p_tx->slot[i].tag.flag,
                spisync->p_tx->slot[i].tag.clamp[0], spisync->p_tx->slot[i].tag.clamp[1], spisync->p_tx->slot[i].tag.clamp[2],
                spisync->p_tx->slot[i].tag.clamp[3], spisync->p_tx->slot[i].tag.clamp[4], spisync->p_tx->slot[i].tag.clamp[5],
                spisync->p_tx->slot[i].payload.seg[0].len,
                spisync->p_tx->slot[i].payload.seg[0].type,
                spisync->p_tx->slot[i].payload.crc);
        spisync_buf("     ", &spisync->p_tx->slot[i].payload, 32);
    }
    spisync_dbg("RxSlot--------------------------------------------------\r\n");
    for (i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        spisync_dbg("rx[%ld] 0x%04lX,seq:%d,ackseq:%ld,flag:%ld,clamp:%ld-%ld-%ld-%ld-%ld-%ld, len:%d,type:%02X,crc:0x%08lX\r\n",
                i,
                spisync->p_rx->slot[i].tag.version,
                spisync->p_rx->slot[i].tag.seq,
                spisync->p_rx->slot[i].tag.ackseq,
                spisync->p_rx->slot[i].tag.flag,
                spisync->p_rx->slot[i].tag.clamp[0], spisync->p_rx->slot[i].tag.clamp[1], spisync->p_rx->slot[i].tag.clamp[2],
                spisync->p_rx->slot[i].tag.clamp[3], spisync->p_rx->slot[i].tag.clamp[4], spisync->p_rx->slot[i].tag.clamp[5],
                spisync->p_rx->slot[i].payload.seg[0].len,
                spisync->p_rx->slot[i].payload.seg[0].type,
                spisync->p_rx->slot[i].payload.crc);
        spisync_buf("     ", &spisync->p_rx->slot[i].payload, 32);
    }

    spisync_dbg("Rx------------------------------------------------------\r\n");
    spisync_dbg("RxQueue[0] tot:%ld, free_space:%ld, used:%ld\r\n", SPISYNC_TYPEMSG_PBUF,
                uxQueueSpacesAvailable(spisync->prx_queue[0]), uxQueueMessagesWaiting(spisync->prx_queue[0]));
    spisync_dbg("RxQueue[1] tot:%ld, free_space:%ld, used:%ld\r\n", SPISYNC_TYPEMSG_SYSCTRL,
                uxQueueSpacesAvailable(spisync->prx_queue[1]), uxQueueMessagesWaiting(spisync->prx_queue[1]));
    spisync_dbg("RxQueue[2] tot:%ld, free_space:%ld, used:%ld\r\n", SPISYNC_TYPEMSG_USER1,
                uxQueueSpacesAvailable(spisync->prx_queue[2]), uxQueueMessagesWaiting(spisync->prx_queue[2]));
    spisync_dbg("Rxstream[0] tot:%ld, free_space:%ld, used:%ld\r\n", SPISYNC_TYPESTREAM_AT,
                xStreamBufferSpacesAvailable(spisync->prx_streambuffer[0]),
                (SPISYNC_RXSTREAM_AT_BYTES-xStreamBufferSpacesAvailable(spisync->prx_streambuffer[0])));
    spisync_dbg("Rxstream[1] tot:%ld, free_space:%ld, used:%ld\r\n", SPISYNC_TYPESTREAM_USER1,
                xStreamBufferSpacesAvailable(spisync->prx_streambuffer[1]),
                (SPISYNC_RXSTREAM_USER1_BYTES-xStreamBufferSpacesAvailable(spisync->prx_streambuffer[1])));
    spisync_dbg("Rxstream[2] tot:%ld, free_space:%ld, used:%ld\r\n", SPISYNC_TYPESTREAM_USER2,
                xStreamBufferSpacesAvailable(spisync->prx_streambuffer[2]),
                (SPISYNC_RXSTREAM_USER2_BYTES-xStreamBufferSpacesAvailable(spisync->prx_streambuffer[2])));
    spisync_dbg("Tx------------------------------------------------------\r\n");
    spisync_dbg("TxQueue[0] tot:%ld, free_space:%ld, used:%ld\r\n", SPISYNC_TYPEMSG_PBUF,
                uxQueueSpacesAvailable(spisync->ptx_queue[0]), uxQueueMessagesWaiting(spisync->ptx_queue[0]));
    spisync_dbg("TxQueue[1] tot:%ld, free_space:%ld, used:%ld\r\n", SPISYNC_TYPEMSG_SYSCTRL,
                uxQueueSpacesAvailable(spisync->ptx_queue[1]), uxQueueMessagesWaiting(spisync->ptx_queue[1]));
    spisync_dbg("TxQueue[2] tot:%ld, free_space:%ld, used:%ld\r\n", SPISYNC_TYPEMSG_USER1,
                uxQueueSpacesAvailable(spisync->ptx_queue[2]), uxQueueMessagesWaiting(spisync->ptx_queue[2]));
    spisync_dbg("Txstream[0] tot:%ld, free_space:%ld, used:%ld\r\n", SPISYNC_TYPESTREAM_AT,
                xStreamBufferSpacesAvailable(spisync->ptx_streambuffer[0]),
                (SPISYNC_TXSTREAM_AT_BYTES-xStreamBufferSpacesAvailable(spisync->ptx_streambuffer[0])));
    spisync_dbg("Txstream[1] tot:%ld, free_space:%ld, used:%ld\r\n", SPISYNC_TYPESTREAM_USER1,
                xStreamBufferSpacesAvailable(spisync->ptx_streambuffer[1]),
                (SPISYNC_TXSTREAM_USER1_BYTES-xStreamBufferSpacesAvailable(spisync->ptx_streambuffer[1])));
    spisync_dbg("Txstream[2] tot:%ld, free_space:%ld, used:%ld\r\n", SPISYNC_TYPESTREAM_USER2,
                xStreamBufferSpacesAvailable(spisync->ptx_streambuffer[2]),
                (SPISYNC_TXSTREAM_USER2_BYTES-xStreamBufferSpacesAvailable(spisync->ptx_streambuffer[2])));
    spisync_dbg("------------------------------------------------------\r\n");

    spisync_dbg("Txseq:%ld, isrRx:%d, isrTx:%d, %d, Rst:%d, CrcE:%d, tskW:%d, tskR:%d\r\n",
            spisync->tx_seq,
            spisync->isr_rxcnt,
            spisync->isr_txcnt,
            spisync->tsk_rstcnt,
            spisync->rxcrcerr_cnt,
            spisync->tsk_writecnt, spisync->tsk_readcnt);
    spisync_dbg("Clamp:%ld-%ld-%ld-%ld-%ld-%ld\r\n",
            spisync->clamp[0], spisync->clamp[1], spisync->clamp[2],
            spisync->clamp[3], spisync->clamp[4], spisync->clamp[5]);
    spisync_dbg("TagE:%ld, RxPattern:%ld\r\n",
            spisync->txtag_errtimes, spisync->rxpattern_checked);

    for (uint32_t i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        spisync_dbg(" %d", spisync->p_rx->slot[i].tag.ackseq);
    }
    spisync_dbg(")\r\n");
    spisync_dbg("flow control tx throttled: ");
    for (int i = 0; i < SPISYNC_TYPEMAX; i++) {
    	spisync_dbg("%lu ", spisync->tx_throttled[i]);
	}
	spisync_dbg("\r\n");
	spisync_dbg("Master: send buffer empty = %d\r\n", spisync_sndbuf_empty(spisync));
    spisync_dbg("Master: has outstanding data = %d\r\n", spisync_has_unacked_data(spisync));
    spisync_dbg("Master: slave data ready = %d\r\n", spisync_slave_data_ready());
    spisync_dbg("Master: state %s\r\n", spisync_state_str[spisync->state]);
    return 0;
}

int spisync_status_internal(spisync_t *spisync)
{
    _spisync_setevent(spisync, EVT_SPISYNC_DUMP_BIT);
    return 0;
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t pin)
{
	spisync_trace("%s pin %d\r\n", __func__, pin);
    _spisync_setevent(g_spisync_current, EVT_SPISYNC_POLL_BIT);
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t pin)
{
	spisync_trace("%s pin %d\r\n", __func__, pin);
    _spisync_setevent(g_spisync_current, EVT_SPISYNC_POLL_BIT);
}
