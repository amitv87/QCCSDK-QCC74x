
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include "message_buffer.h"
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
#if SPISYNC_MSGBUFFER_TO_STREAM
    spisync->tsk_rsynccnt     = 0;
#endif
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
    lramsync_reset(&spisync->hw);

    /* slot */
    memset(spisync->p_tx, 0, sizeof(spisync_txbuf_t));
    memset(spisync->p_rx, 0, sizeof(spisync_rxbuf_t));
    for (int i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        spisync->p_tx->slot[i].tag.magic = SPISYNC_SLOT_MAGIC;
    }

    /* local sequnce */
    spisync->tx_seq             = 0;
    spisync->txtag_errtimes     = 0;
    spisync->txwrite_continue   = 0;
    spisync->rxpattern_checked  = 0;
    spisync->rxread_continue    = 0;

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

    xStreamBufferReset(spisync->tx_msgbuffer);
#if SPISYNC_MSGBUFFER_TO_STREAM
    xStreamBufferReset(spisync->rx_msgbuffer);
#endif
    xStreamBufferReset(spisync->rx_streambuffer);

    /* calulate crc for rx */
    memset(spisync->p_rxcache, 0, sizeof(spisync_slot_t));

    /* debug */
    debug_init_reset(spisync, 0);// 1 init, 0 reset

    /* start low */
    lramsync_start(&spisync->hw);
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

static void __ramsync_low_rx_cb(spisync_t *spisync)
{
    /* debug */
    if (spisync) {
        spisync->isr_rxcnt++;
    }

    if (process_slot_check(spisync)) {
        return;// todo : need?
    }
#if 0
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
    _spisync_setevent(spisync, EVT_SPISYNC_READ_BIT);
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
}

static void __ramsync_low_reset_cb(void *spisync)
{
    // todo:
}

void timer_callback(TimerHandle_t xTimer)
{
    spisync_t *spisync = (spisync_t *)pvTimerGetTimerID(xTimer);

    _spisync_setevent(spisync, EVT_SPISYNC_RESET_BIT);
}

int spisync_init(spisync_t *spisync, const spisync_config_t *config)
{
    int i;

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
    }

    /* global init*/
    spisync->tx_seq             = 0;
    spisync->txtag_errtimes     = 0;
    spisync->txwrite_continue   = 0;
    spisync->rxpattern_checked  = 0;
    spisync->rxread_continue    = 0;

    debug_init_reset(spisync, 1);// 1 init, 0 reset

    /* sys hdl */
    spisync->event         = xEventGroupCreate();
    spisync->timer         = xTimerCreate("delay_rst",                          // 定时器的名称
                                        pdMS_TO_TICKS(SPISYNC_PATTERN_DELAYMS), // 周期为 10s
                                        pdFALSE,                                // 自动重载模式
                                        (void *)spisync,                        // 定时器ID，可以为 NULL
                                        timer_callback);                        // 定时器到期时的回调函数
    spisync->tx_msgbuffer = xMessageBufferCreate(SPISYNC_TX_MSGBUFFER_SIZE);
#if SPISYNC_MSGBUFFER_TO_STREAM
    spisync->rx_msgbuffer  = xMessageBufferCreate(SPISYNC_RX_MSGBUFFER_SIZE);
#endif
    spisync->rx_streambuffer = xStreamBufferCreate(SPISYNC_RX_STREAMBUFFER_SIZE, 1);

    xTaskCreate(spisync_task,
                (char*)"spisync",
                SPISYNC_STACK_SIZE/sizeof(StackType_t), spisync,
                SPISYNC_TASK_PRI,
                &spisync->taskhdr);

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

int spisync_read(spisync_t *spisync, void *buf, uint32_t len, uint32_t timeout_ms)
{
    if (NULL == spisync) {
        spisync_log("arg err, spisync:%p\r\n",
                spisync, len, SPISYNC_PAYLOADBUF_LEN);
        return -1;
    }
#if 0
    return xMessageBufferReceive(spisync->rx_msgbuffer, buf, len, timeout_ms);
#else
    return xStreamBufferReceive(spisync->rx_streambuffer, buf, len, timeout_ms);
#endif
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

#if 0
    return xMessageBufferSend(spisync->rx_msgbuffer, buf, len, timeout_ms);
#else
#endif
    if (xPortIsInsideInterrupt()) {
        res = xStreamBufferSendFromISR(spisync->rx_streambuffer,buf, len, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        res = xStreamBufferSend(spisync->rx_streambuffer, buf, len, timeout_ms);
    }

    return res;
}

int spisync_write(spisync_t *spisync, void *buf, uint32_t len, uint32_t timeout_ms)
{
    int res;
    if (len > SPISYNC_PAYLOADBUF_LEN) {
        spisync_err("write len too long!\r\n");
        return -1;
    }

    res = xMessageBufferSend(spisync->tx_msgbuffer, buf, len, timeout_ms);
    spisync_trace("spisync_write_lpri res:%d\r\n", res);
    _spisync_setevent(spisync, EVT_SPISYNC_WRITE_BIT);
    return res;
}

/*---------------------- moudule ------------------------*/
static void _reset_process(spisync_t *spisync)
{
    _reset_spisync(spisync);
}

static void _ctrl_process(spisync_t *spisync)
{
}

static uint32_t get_laster_rseq(spisync_t *spisync)
{
    uint32_t max_val = 0;

    for (uint32_t i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        if (spisync->p_rx->slot[i].tag.rseq > max_val) {
            max_val = spisync->p_rx->slot[i].tag.rseq;
        }
    }
    return max_val;
}

void _write_process(spisync_t *spisync)
{
    int empty_slot_totalcount = 0;
    int empty_slot;
    uint32_t current;
    uint32_t laster_rseq;

    int i, available_bytes;
    uint32_t data_len;
    StreamBufferHandle_t current_stream_buffer;
    struct crc32_stream_ctx crc_ctx;

    /* Check if there is an empty slot */
    empty_slot_totalcount = 0;
    empty_slot       = 0;

    for (i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
#if 1
        laster_rseq = get_laster_rseq(spisync);
        current = UINT32_DIFF(spisync->p_tx->slot[i].payload.seq, laster_rseq);
        // ( current > SPISYNC_TXSLOT_COUNT) ===> w_seq too small and old
        // (0 == current)                    ===> w_seq==r_seq
        if (( current > SPISYNC_TXSLOT_COUNT) || (0 == current)) {
            empty_slot_totalcount++;
            empty_slot = i;
            break;
        }
#else    
        if (0 < (SPISYNC_TXSLOT_COUNT -
                UINT32_DIFF(spisync->p_tx->slot[i].payload.seq,
                            spisync->p_rx->slot[i].tag.rseq))) {
            empty_slot_totalcount++;
            empty_slot = i;
        }
#endif
    }
    if (empty_slot_totalcount <= 0) {
        // no empty so set txwrite_continue flag.
        spisync->txwrite_continue = 1;// todo: check path txwrite_continue
        return;
    }

    /* check if there is valid bytes */
    available_bytes = SPISYNC_TX_MSGBUFFER_SIZE - xMessageBufferSpaceAvailable(spisync->tx_msgbuffer);
    if (available_bytes <= 0) {// todo: maybe < is ok
        spisync->txwrite_continue = 0;
        return;
    }

    /* pop from msgbuffer to slot */
    data_len = xMessageBufferReceive(
            spisync->tx_msgbuffer,
            spisync->p_tx->slot[empty_slot].payload.buf,
            SPISYNC_PAYLOADBUF_LEN,
            0);
    if ((data_len <= 0) || (data_len > SPISYNC_PAYLOADBUF_LEN)) {
        spisync_err("write process never here.\r\n", data_len);
        return;
    }
    spisync->tx_seq++;
    spisync->p_tx->slot[empty_slot].payload.len = SPISYNC_PAYLOADLENSEQ_LEN + data_len;
    spisync->p_tx->slot[empty_slot].payload.seq = spisync->tx_seq;
#if 0
    utils_crc32_stream_init(&crc_ctx);
    utils_crc32_stream_feed_block(&crc_ctx,
            (uint8_t *)&spisync->p_tx->slot[empty_slot].payload,
            spisync->p_tx->slot[empty_slot].payload.len);
    spisync->p_tx->slot[empty_slot].payload.crc = utils_crc32_stream_results(&crc_ctx);
#else 
    spisync->p_tx->slot[empty_slot].payload.crc = utils_crc8((uint8_t *)&spisync->p_tx->slot[empty_slot].payload, 
                                                                    spisync->p_tx->slot[empty_slot].payload.len);
#endif 

    //spisync_log("write slot:%d, len:%d\r\n", empty_slot, data_len);
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

    debug_iperf_print(spisync);
    while (read_times) {
        for (int i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
            // check magic
            if (SPISYNC_SLOT_MAGIC != spisync->p_rx->slot[i].tag.magic) {
                continue;
            }
            // update tmp
            rseqtmp = spisync->p_tx->slot[i].tag.rseq;
            seqtmp  = spisync->p_rx->slot[i].payload.seq;
            lentmp  = spisync->p_rx->slot[i].payload.len;

            // check seq len valid
            if ((lentmp > SPISYNC_PAYLOADLEN_MAX) || (seqtmp != (rseqtmp + 1))) {
                if (i == (SPISYNC_RXSLOT_COUNT - 1)) {
                    spisync->rxread_continue = 0;
                }
                continue;
            }
#if 0// msgbuffer
            // Check available spaces in the buffer in for rx
            space_valid = xMessageBufferSpacesAvailable(spisync->rx_msgbuffer);
            spisync_trace("space_valid:%d,  rseq:%d, seq:%d, len:%d\r\n", space_valid, rseqtmp, seqtmp, lentmp);
            if ((space_valid <= 4) || ((space_valid - 4) < (lentmp - SPISYNC_PAYLOADLENSEQ_LEN))) {
                spisync->rxread_continue = 1;
                spisync_trace("The stream buffer does not have enough space.\r\n");
                break;
            }
#else
            // Check available spaces in the buffer in for rx
            space_valid = xStreamBufferSpacesAvailable(spisync->rx_streambuffer);
            if (space_valid < (lentmp - SPISYNC_PAYLOADLENSEQ_LEN)) {
                spisync->rxread_continue = 1;
                spisync_trace("The stream buffer does not have enough space.\r\n");
                break;
            }
#endif
            // copy data and crc
            memcpy(spisync->p_rxcache, &spisync->p_rx->slot[i].payload, lentmp);
            spisync->p_rxcache->crc = spisync->p_rx->slot[i].payload.crc;
            // Check if the relevant parameters are within the expected range
            if ((spisync->p_rxcache->seq != seqtmp) ||
                (spisync->p_rxcache->len != lentmp)) {
                continue;
            }

#if SPISYNC_MSGBUFFER_TO_STREAM
            // check crc
            utils_crc32_stream_init(&crc_ctx);
            utils_crc32_stream_feed_block(&crc_ctx,
                    (uint8_t *)&spisync->p_rxcache->len,
                    spisync->p_rxcache->len);
            crctmp = utils_crc32_stream_results(&crc_ctx);
            if (spisync->p_rxcache->crc != crctmp) {
                spisync_trace("crc err, 0x%08X != 0x%08X\r\n", crctmp, spisync->p_rxcache->crc);
                continue;
            }
#else 
            if (spisync->p_rxcache->crc != utils_crc8((uint8_t *)&spisync->p_rxcache->len, spisync->p_rxcache->len)) {
                spisync_trace("crc err, 0x%08X != 0x%08X\r\n", crctmp, spisync->p_rxcache->crc);
                continue;
            }
#endif

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

#if SPISYNC_MSGBUFFER_TO_STREAM
            if ((lentmp - SPISYNC_PAYLOADLENSEQ_LEN) != xMessageBufferSend(spisync->rx_msgbuffer,
                                                           spisync->p_rxcache->buf,
                                                           (lentmp - SPISYNC_PAYLOADLENSEQ_LEN),
                                                           0)) {
                spisync_log("push err, never here.\r\n");
            } else {
                spisync_trace("rx->msgbuffer:%d\r\n", (lentmp - SPISYNC_PAYLOADLENSEQ_LEN));
            }

            _spisync_setevent(spisync, EVT_SPISYNC_RSYNC_BIT);
#else
            if ((lentmp - SPISYNC_PAYLOADLENSEQ_LEN) != xStreamBufferSend(spisync->rx_streambuffer,
                        spisync->p_rxcache->buf, (lentmp - SPISYNC_PAYLOADLENSEQ_LEN), 0)) {
                printf("never here.\r\n");
            }
#endif

            // update tx st rseq
            for (int m = 0; m < SPISYNC_TXSLOT_COUNT; m++) {
                spisync->p_tx->slot[m].tag.rseq = spisync->p_rxcache->seq;
            }
        }

        if (xStreamBufferSpacesAvailable(spisync->rx_streambuffer) < SPISYNC_RX_STREAMBUFFER_LEVEL) {
            break; // while (read_times)
        }
        read_times--;
    }
}

#if SPISYNC_MSGBUFFER_TO_STREAM
static void _rsync_process(spisync_t *spisync)
{
    int space;
    int res;
    int available;

    space = xStreamBufferSpacesAvailable(spisync->rx_streambuffer);

    if (space >= SPISYNC_RX_STREAMBUFFER_LEVEL) {
        res = xMessageBufferReceive(spisync->rx_msgbuffer, spisync->p_rxcache->buf, SPISYNC_PAYLOADBUF_LEN, 0);
        if (res > 0) {
            if (res != xStreamBufferSend(spisync->rx_streambuffer, spisync->p_rxcache->buf, res, 0)) {
                printf("never here.\r\n");
            }
        }
    }

    available = SPISYNC_RX_MSGBUFFER_SIZE - xMessageBufferSpaceAvailable(spisync->rx_msgbuffer);
    if (available) {
        _spisync_setevent(spisync, EVT_SPISYNC_RSYNC_BIT);
    }
}
#endif

static void spisync_task(void *arg)
{
     spisync_t *spisync = (spisync_t *)arg;
     EventBits_t eventBits;

     while (1) {
         eventBits = xEventGroupWaitBits(spisync->event,
             EVT_SPISYNC_RESET_BIT|EVT_SPISYNC_CTRL_BIT|EVT_SPISYNC_WRITE_BIT|EVT_SPISYNC_READ_BIT|EVT_SPISYNC_DUMP_BIT,
             pdTRUE, pdFALSE,
             portMAX_DELAY);
         if (eventBits  & EVT_SPISYNC_RESET_BIT) {
             spisync_log("recv bit EVT_SPISYNC_RESET_BIT\r\n");
             spisync->tsk_rstcnt++;
             _reset_process(spisync);
         }
         if (eventBits  & EVT_SPISYNC_CTRL_BIT) {
             spisync_log("recv bit EVT_SPISYNC_CTRL_BIT\r\n");
             spisync->tsk_ctrlcnt++;
             _ctrl_process(spisync);
         }
         if (eventBits  & EVT_SPISYNC_WRITE_BIT) {
             spisync_trace("recv bit EVT_SPISYNC_WRITE_BIT\r\n");
             _write_process(spisync);
         }
         if (eventBits  & EVT_SPISYNC_READ_BIT) {
             spisync_trace("recv bit EVT_SPISYNC_READ_BIT\r\n");
             _read_process(spisync);
         }
#if SPISYNC_MSGBUFFER_TO_STREAM
         if (eventBits  & EVT_SPISYNC_RSYNC_BIT) {
             spisync_trace("recv bit EVT_SPISYNC_RSYNC_BIT\r\n");
             spisync->tsk_rsynccnt++;
             _rsync_process(spisync);
         }
#endif
         if (eventBits  & EVT_SPISYNC_DUMP_BIT) {
            spisync_dump(spisync);
         }
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
        spisync_write(spisync, buf, SPISYNC_PAYLOADBUF_LEN, portMAX_DELAY);
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

    //spisync_log("*0x2000A200 = 0x%08lX\r\n" , (*(volatile uint32_t *)0x2000A200));
    //spisync_log("*0x200008EC = 0x%08lX\r\n" , (*(volatile uint32_t *)0x200008E8));
    //spisync_log("*0x200008F0 = 0x%08lX\r\n" , (*(volatile uint32_t *)0x200008F0));
    //spisync_log("*0x200008F4 = 0x%08lX\r\n" , (*(volatile uint32_t *)0x200008F4));
    //spisync_log("*0x200008F8 = 0x%08lX\r\n" , (*(volatile uint32_t *)0x200008F8));
    //spisync_log("*0x2000A200 = 0x%08lX\r\n" , (*(volatile uint32_t *)0x2000A200));

    for (i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        spisync_dbg("tx[%ld] 0x%08lX,rseq:%ld,flag:%ld    len:%d,seq:%d,crc:0x%08lX\r\n",
                i,
                spisync->p_tx->slot[i].tag.magic,
                spisync->p_tx->slot[i].tag.rseq,
                spisync->p_tx->slot[i].tag.flag,
                spisync->p_tx->slot[i].payload.len,
                spisync->p_tx->slot[i].payload.seq,
                spisync->p_tx->slot[i].payload.crc);
        spisync_buf("     ", &spisync->p_tx->slot[i].payload, 32);
    }

    for (i = 0; i < SPISYNC_RXSLOT_COUNT; i++) {
        spisync_dbg("rx[%ld] 0x%08lX,rseq:%ld,flag:%ld    len:%d,seq:%d,crc:0x%08lX\r\n",
                i,
                spisync->p_rx->slot[i].tag.magic,
                spisync->p_rx->slot[i].tag.rseq,
                spisync->p_rx->slot[i].tag.flag,
                spisync->p_rx->slot[i].payload.len,
                spisync->p_rx->slot[i].payload.seq,
                spisync->p_rx->slot[i].payload.crc);
        spisync_buf("     ", &spisync->p_rx->slot[i].payload, 32);
    }

    spisync_dbg("spisync->tx_seq             = %ld\r\n", spisync->tx_seq);
    spisync_dbg("spisync->txtag_errtimes     = %ld\r\n", spisync->txtag_errtimes);
    spisync_dbg("spisync->txwrite_continue   = %ld\r\n", spisync->txwrite_continue);
    spisync_dbg("spisync->rxpattern_checked  = %ld\r\n", spisync->rxpattern_checked);
    spisync_dbg("spisync->rxread_continue    = %ld\r\n", spisync->rxread_continue);

    spisync_dbg("tx_msgbuffer     total:%ld, free_space:%ld\r\n",
            SPISYNC_TX_MSGBUFFER_SIZE,
            xMessageBufferSpaceAvailable(spisync->tx_msgbuffer)
            );
#if SPISYNC_MSGBUFFER_TO_STREAM
    spisync_dbg("rx_msgbuffer      total:%ld, free_space:%ld\r\n",
            SPISYNC_RX_MSGBUFFER_SIZE,
            xMessageBufferSpaceAvailable(spisync->rx_msgbuffer)
            );
#endif
    spisync_dbg("rx_streambuffer   total:%ld, free_space:%ld\r\n",
            SPISYNC_RX_STREAMBUFFER_SIZE,
            xStreamBufferSpacesAvailable(spisync->rx_streambuffer)
            );
    spisync_dbg("isr_rxcnt    = %d\r\n", spisync->isr_rxcnt);
    spisync_dbg("isr_txcnt    = %d\r\n", spisync->isr_txcnt);
    spisync_dbg("tsk_rstcnt   = %d\r\n", spisync->tsk_rstcnt);
    spisync_dbg("tsk_ctrlcnt  = %d\r\n", spisync->tsk_ctrlcnt);
    spisync_dbg("tsk_writecnt = %d\r\n", spisync->tsk_writecnt);
    spisync_dbg("tsk_readcnt  = %d\r\n", spisync->tsk_readcnt);
#if SPISYNC_MSGBUFFER_TO_STREAM
    spisync_dbg("tsk_rsynccnt = %d\r\n", spisync->tsk_rsynccnt);
#endif
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

