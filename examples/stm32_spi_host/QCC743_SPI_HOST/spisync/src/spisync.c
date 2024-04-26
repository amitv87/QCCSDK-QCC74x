
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include "message_buffer.h"
#include <event_groups.h>

#include <utils_crc.h>
#include <spisync.h>
#include <ramsync.h>
#include <spisync_port.h>

#define UINT32_DIFF(a, b) ((uint32_t)((int32_t)(a) - (int32_t)(b)))

int start_dbg = 0;

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
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (xPortIsInsideInterrupt()) {
        if (xTimerStartFromISR(spisync->timer, &xHigherPriorityTaskWoken) != pdPASS) {
            printf("Failed to start timer!\r\n");
        }
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        if (xTimerStart(spisync->timer, 0) != pdPASS) {
            printf("Failed to start timer!\r\n");
        }
    }
}

static void _reset_spisync(spisync_t *spisync)
{
    /* reset low */
	if (!spisync->master)
		lramsync_reset(&spisync->hw);

    /* slot */
    memset(spisync->p_tx, 0, sizeof(spisync_txbuf_t));
    memset(spisync->p_rx, 0, sizeof(spisync_rxbuf_t));
    for (int i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
        spisync->p_tx->slot[i].tag.magic = SPISYNC_SLOT_MAGIC;
    }

    /* local sequence */
    spisync->tx_seq             = 0;
    spisync->txtag_errtimes     = 0;
    spisync->txwrite_continue   = 0;
    spisync->rxpattern_checked  = 0;
    spisync->rxread_continue    = 0;
    spisync->printed_err_reason = 0;
    spisync->reset_pending = 0;

    /* for rx notify */
    xEventGroupClearBits(spisync->event, EVT_SPISYNC_ALL_BIT);
    //xTimerReset(spisync->timer, 0);
    //xTimerStop(spisync->timer, 0);
    xStreamBufferReset(spisync->tx_hmsgbuffer);
    xStreamBufferReset(spisync->tx_lmsgbuffer);
    xStreamBufferReset(spisync->rx_msgbuffer);

    /* calulate crc for rx */
    memset(spisync->p_rxcache, 0, sizeof(spisync_slot_t));

    /* start low */
    if (!spisync->master)
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

static int slave_process_slot_check(spisync_t *spisync)
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
            spisync_log("check pattern times:%ld, delay to %d ms reset\r\n",
                spisync->rxpattern_checked, SPISYNC_PATTERN_DELAYMS);
            _spisync_pattern_process(spisync);
        }

        return 1;
    } else {
        if (spisync->rxpattern_checked) {
            printf("rxpattern_checked:%ld, throhold:%d\r\n", spisync->rxpattern_checked, SPISYNC_PATTERN_TIMES);
            spisync->rxpattern_checked = 0;
        }
    }

    return res;
}

static int master_slot_check(spisync_t *spisync)
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

static int process_slot_check(spisync_t *spisync)
{
	if (spisync->master)
		return master_slot_check(spisync);

	return slave_process_slot_check(spisync);
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

int event_cnt = 0;

static void __ramsync_low_rx_cb(spisync_t *spisync)
{
    if (process_slot_check(spisync)) {
    	spisync_start_reset(spisync);
        return;
    }
#if 1
    // Trigger a read event every time
    _spisync_setevent(spisync, EVT_SPISYNC_READ_BIT);
#else
    // Trigger a read event by (continue || (rseq == seq) )

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
    int slot_count = 0;
#if 0
    if (spisync->txwrite_continue) {
        /* Check if there is an empty slot */
        for (i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
            slot_count = (SPISYNC_TXSLOT_COUNT -
                    (UINT32_DIFF(spisync->p_tx->slot[i].payload.seq,
                                spisync->p_rx->slot[i].tag.rseq)));
            if (slot_count > 0) {
                break;
            }
        }
        // if (slot have empty)
        if (slot_count > 0) {
            _spisync_setevent(spisync, EVT_SPISYNC_WRITE_BIT);
        }
    }
#else
    _spisync_setevent(spisync, EVT_SPISYNC_WRITE_BIT);
#endif
}

static void __ramsync_low_reset_cb(void *spisync)
{
    // todo:
}

void timer_callback(TimerHandle_t xTimer)
{
    spisync_t *spisync = (spisync_t *)pvTimerGetTimerID(xTimer);

    spisync_log("timer callback sets reset bit\r\n");
    _spisync_setevent(spisync, EVT_SPISYNC_RESET_BIT);
}

int spisync_init(spisync_t *spisync)
{
    int i;
    BaseType_t ret;

    if (NULL == spisync) {
        return -1;
    }

    node_mem_t node_txbuf[SPISYNC_TXSLOT_COUNT];
    node_mem_t node_rxbuf[SPISYNC_RXSLOT_COUNT];

    /* slot */
    memset(spisync, 0, sizeof(spisync_t));
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
    spisync->state = SPISYNC_STATE_NORMAL;
    spisync->master = 1;
    spisync->reset_pending = 0;

    /* sys hdl */
    spisync->event         = xEventGroupCreate();
    spisync->timer         = xTimerCreate("delay_rst",
                                        pdMS_TO_TICKS(SPISYNC_PATTERN_DELAYMS),
                                        pdFALSE, (void *)spisync, timer_callback);
    spisync->tx_hmsgbuffer = xMessageBufferCreate(SPISYNC_TX_HMSGBUFFER_SIZE);
    spisync->tx_lmsgbuffer = xMessageBufferCreate(SPISYNC_TX_LMSGBUFFER_SIZE);
    spisync->rx_msgbuffer  = xMessageBufferCreate(SPISYNC_RX_MSGBUFFER_SIZE);

    ret = xTaskCreate(spisync_task,
                (char*)"spisync",
                SPISYNC_STACK_SIZE/sizeof(StackType_t), spisync,
                SPISYNC_TASK_PRI,
                &spisync->taskhdr);
    if (ret != pdPASS)
    	printf("failed to create spisync task, %ld\r\n", ret);

    /* set type , and set name */
    lramsync_init(
        &spisync->hw,
        node_txbuf, SPISYNC_TXSLOT_COUNT,
        __ramsync_low_tx_cb, spisync,
        node_rxbuf, SPISYNC_RXSLOT_COUNT,
        __ramsync_low_rx_cb, spisync,
        __ramsync_low_reset_cb, spisync
        );

    /* master needs to reset for re-synchronization */
    if (spisync->master) {
    	spisync->state = SPISYNC_STATE_INIT;
    	_spisync_setevent(spisync, EVT_SPISYNC_RESET_BIT);
    }
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
        spisync_log("arg err, spisync:%p\r\n", spisync);
        return -1;
    }

    return xMessageBufferReceive(spisync->rx_msgbuffer, buf, len, timeout_ms);
}

int spisync_write_hpri(spisync_t *spisync, void *buf, uint32_t len, uint32_t timeout_ms)
{
    int res;
    if (len > SPISYNC_PAYLOADBUF_LEN) {
        spisync_err("write len too long!\r\n");
        return -1;
    }
    if (spisync->state != SPISYNC_STATE_NORMAL) {
    	//spisync_err("spisync write failed due to state %x\r\n", spisync->state);
    	return -2;
    }

    res = xMessageBufferSend(spisync->tx_hmsgbuffer, buf, len, timeout_ms);
    spisync_trace("spisync_write_hpri res:%d\r\n", res);
    _spisync_setevent(spisync, EVT_SPISYNC_WRITE_BIT);
    return res;
}

int spisync_write_lpri(spisync_t *spisync, void *buf, uint32_t len, uint32_t timeout_ms)
{
    int res;
    if (len > SPISYNC_PAYLOADBUF_LEN) {
        spisync_err("write len too long!\r\n");
        return -1;
    }

    if (spisync->state != SPISYNC_STATE_NORMAL) {
    	//spisync_err("spisync write failed due to state %x\r\n", spisync->state);
    	return -2;
    }

    res = xMessageBufferSend(spisync->tx_lmsgbuffer, buf, len, timeout_ms);
    spisync_trace("spisync_write_lpri res:%d\r\n", res);
    _spisync_setevent(spisync, EVT_SPISYNC_WRITE_BIT);
    return res;
}

int spisync_write(spisync_t *spisync, void *buf, uint32_t len, uint32_t timeout_ms)
{
    return spisync_write_lpri(spisync, buf, len, timeout_ms);
}

#define SPISYNC_RESET_PATTERN	0x55

static void spisync_master_reset(spisync_t *spisync)
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

static void spisync_reset(spisync_t *spisync)
{
	if (spisync->master)
		spisync_master_reset(spisync);
	else
		_reset_spisync(spisync);
}

/*---------------------- moudule ------------------------*/
static void _reset_process(spisync_t *spisync)
{
	spisync_reset(spisync);
    //_reset_spisync(spisync);
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

void _write_process(spisync_t *spisync)
{
    int empty_slot_totalcount = 0;
    int empty_slot;
    uint32_t current;

    int i, low_available_bytes, high_available_bytes;
    uint32_t data_len;
    StreamBufferHandle_t current_stream_buffer;
    struct crc32_stream_ctx crc_ctx;

again:
    /* Check if there is an empty slot */
    empty_slot_totalcount = 0;
    empty_slot       = 0;
    for (i = 0; i < SPISYNC_TXSLOT_COUNT; i++) {
    	uint32_t una = get_latest_rseq(spisync);
    	//printf("slot %d, seq %u, una %u\r\n", i, spisync->p_tx->slot[i].payload.seq, una);
        current = UINT32_DIFF(spisync->p_tx->slot[i].payload.seq,
        		una);
        // ( current > SPISYNC_TXSLOT_COUNT) ===> w_seq too small and old
        // (0 == current)                    ===> w_seq==r_seq
        if (( current > SPISYNC_TXSLOT_COUNT) || (0 == current)) {
            empty_slot_totalcount++;
            empty_slot = i;
            break;
        }
    }
    if (empty_slot_totalcount <= 0) {
        // no empty so set txwrite_continue flag.
        spisync->txwrite_continue = 1;// todo: check path txwrite_continue
        return;
    }

    /* check if there is valid bytes */
    high_available_bytes =
        SPISYNC_TX_HMSGBUFFER_SIZE - xMessageBufferSpaceAvailable(spisync->tx_hmsgbuffer);
    low_available_bytes =
        SPISYNC_TX_LMSGBUFFER_SIZE - xMessageBufferSpaceAvailable(spisync->tx_lmsgbuffer);
    if ((low_available_bytes <= 0) && (high_available_bytes <= 0)) {// todo: maybe < is ok
        spisync->txwrite_continue = 0;
        return;
    }

    if (high_available_bytes > 0) {
        current_stream_buffer = spisync->tx_hmsgbuffer;
    } else {
        current_stream_buffer = spisync->tx_lmsgbuffer;
    }

    /* pop from lmsgbuffer or lmsgbuffer to slot */
    data_len = xMessageBufferReceive(
            current_stream_buffer,
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

    utils_crc32_stream_init(&crc_ctx);
    utils_crc32_stream_feed_block(&crc_ctx,
            (uint8_t *)&spisync->p_tx->slot[empty_slot].payload,
            spisync->p_tx->slot[empty_slot].payload.len);

    spisync->p_tx->slot[empty_slot].payload.crc = utils_crc32_stream_results(&crc_ctx);
#if 0
    if (xMessageBufferSpaceAvailable(spisync->tx_hmsgbuffer) ||
    		xMessageBufferSpaceAvailable(spisync->tx_lmsgbuffer))
    	goto again;
#endif
    spisync_trace("write slot:%d, len:%d\r\n", empty_slot, data_len);
}

static void _read_process(spisync_t *spisync)
{
    uint32_t lentmp;
    uint32_t crctmp;
    uint32_t seqtmp;
    uint32_t rseqtmp;
    struct crc32_stream_ctx crc_ctx;
    int space_valid;

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
        // Check available spaces in the buffer in for rx
        space_valid = xMessageBufferSpacesAvailable(spisync->rx_msgbuffer);
        spisync_trace("space_valid:%d,  rseq:%d, seq:%d, len:%d\r\n", space_valid, rseqtmp, seqtmp, lentmp);
        if ((space_valid <= 4) || ((space_valid - 4) < (lentmp - SPISYNC_PAYLOADLENSEQ_LEN))) {
            spisync->rxread_continue = 1;
            break;
        }
        // copy data and crc
        memcpy(spisync->p_rxcache, &spisync->p_rx->slot[i].payload, lentmp);
        spisync->p_rxcache->crc = spisync->p_rx->slot[i].payload.crc;
        // Check if the relevant parameters are within the expected range
        if ((spisync->p_rxcache->seq != seqtmp) ||
            (spisync->p_rxcache->len != lentmp)) {
            continue;
        }

        // check crc
        utils_crc32_stream_init(&crc_ctx);
        utils_crc32_stream_feed_block(&crc_ctx,
                (uint8_t *)&spisync->p_rxcache->len,
                spisync->p_rxcache->len);
        crctmp = utils_crc32_stream_results(&crc_ctx);
        if (spisync->p_rxcache->crc != crctmp) {
            //spisync_log("crc err, 0x%08X != 0x%08X\r\n", crctmp, spisync->p_rxcache->crc);
            continue;
        }

        // push
        if ((lentmp - SPISYNC_PAYLOADLENSEQ_LEN) != xMessageBufferSend(spisync->rx_msgbuffer,
                                                       spisync->p_rxcache->buf,
                                                       (lentmp - SPISYNC_PAYLOADLENSEQ_LEN),
                                                       0)) {
            spisync_log("push err, never here.\r\n");
        } else {
            spisync_trace("rx->streambuffer:%d\r\n", (lentmp - SPISYNC_PAYLOADLENSEQ_LEN));
        }

        // update tx st rseq
        for (int m = 0; m < SPISYNC_TXSLOT_COUNT; m++) {
            spisync->p_tx->slot[m].tag.rseq = spisync->p_rxcache->seq;
        }
    }
}

static void spisync_task(void *arg)
{
     spisync_t *spisync = (spisync_t *)arg;
     EventBits_t eventBits;

     while (1) {
         eventBits = xEventGroupWaitBits(spisync->event,
             EVT_SPISYNC_RESET_BIT|EVT_SPISYNC_CTRL_BIT|EVT_SPISYNC_WRITE_BIT|EVT_SPISYNC_READ_BIT,
             pdTRUE, pdFALSE,
             portMAX_DELAY);

         if (eventBits  & EVT_SPISYNC_RESET_BIT) {
             spisync_log("recv bit EVT_SPISYNC_RESET_BIT\r\n");
             _reset_process(spisync);
         }
         if (eventBits  & EVT_SPISYNC_CTRL_BIT) {
             spisync_log("recv bit EVT_SPISYNC_CTRL_BIT\r\n");
             _ctrl_process(spisync);
         }
         if (spisync->state == SPISYNC_STATE_NORMAL) {
			 if (eventBits  & EVT_SPISYNC_WRITE_BIT) {
				 spisync_trace("recv bit EVT_SPISYNC_WRITE_BIT\r\n");
				 _write_process(spisync);
			 }
			 if (eventBits  & EVT_SPISYNC_READ_BIT) {
				 spisync_trace("recv bit EVT_SPISYNC_READ_BIT\r\n");
					 _read_process(spisync);
			 }
         }
     }
}

/*--------------------- debug moudule ----------------------*/
int spisync_dump(spisync_t *spisync)
{
    int i;

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

    spisync_dbg("spisync->tx_hmsgbuffer  total:%ld, free_space:%ld\r\n",
            SPISYNC_TX_HMSGBUFFER_SIZE,
            xMessageBufferSpaceAvailable(spisync->tx_hmsgbuffer)
            );
    spisync_dbg("spisync->tx_lmsgbuffer  total:%ld, free_space:%ld\r\n",
            SPISYNC_TX_LMSGBUFFER_SIZE,
            xMessageBufferSpaceAvailable(spisync->tx_lmsgbuffer)
            );
    spisync_dbg("spisync->rx_msgbuffer   total:%ld, free_space:%ld\r\n",
            SPISYNC_RX_MSGBUFFER_SIZE,
            xMessageBufferSpaceAvailable(spisync->rx_msgbuffer)
            );

    return 0;
}

