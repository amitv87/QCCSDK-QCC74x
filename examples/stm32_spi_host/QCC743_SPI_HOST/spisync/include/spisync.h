#ifndef __SPISYNC_UPPER_H__
#define __SPISYNC_UPPER_H__

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include "stream_buffer.h"
#include <event_groups.h>
#include "ramsync.h"

/* slot config */
#define SPISYNC_SLOT_VERSION        (0x5501)
#define SPISYNC_TXSLOT_COUNT        (4)
#define SPISYNC_RXSLOT_COUNT        (4)
#define SPISYNC_PAYLOADBUF_LEN      (1536)
#define SPISYNC_PAYLOADLEN_MAX      (SPISYNC_PAYLOADBUF_LEN + 4)//seg_len_type_res,

/* Check the number of tag errors. If the number of errors
   exceeds the allowed limit, the resetflag will set.
 */
#define SPISYNC_TAGERR_TIMES	((SPISYNC_RXSLOT_COUNT + 1) * (SPISYNC_RXSLOT_COUNT + 1))

/* Config msg/stream size */
#define SPISYNC_RXMSG_PBUF_ITEMS       (3)
#define SPISYNC_RXMSG_SYSCTRL_ITEMS    (2)
#define SPISYNC_RXMSG_USER1_ITEMS      (1)
#define SPISYNC_RXSTREAM_AT_BYTES      (1536*3)
#define SPISYNC_RXSTREAM_USER1_BYTES   (1536*1)
#define SPISYNC_RXSTREAM_USER2_BYTES   (1536*1)

#define SPISYNC_TXMSG_PBUF_ITEMS       (11)
#define SPISYNC_TXMSG_SYSCTRL_ITEMS    (2)
#define SPISYNC_TXMSG_USER1_ITEMS      (2)
#define SPISYNC_TXSTREAM_AT_BYTES      (1536*3)
#define SPISYNC_TXSTREAM_USER1_BYTES   (1536*1)
#define SPISYNC_TXSTREAM_USER2_BYTES   (1536*1)

/* task config */
#define SPISYNC_STACK_SIZE             (12048)
#define SPISYNC_TASK_PRI               (55)

/* ========================================================================= */

/* For system wait */
#define SPISYNC_WAIT_FOREVER           (0xFFFFFFFF)
#define SPISYNC_RX_READTIMES           (SPISYNC_RXSLOT_COUNT-1)

/* For Tag flag */
#define SPISYNC_TAGFLAGBIT_RXERR       (0x0001) // bit0 indicates whether the slave device has an error

/* For MSG flag */
#define SPISYNC_MSGFLAGBIT_COPY        (0x01) // bit0 indicates whether to reallocate memory and copy the incoming msg->buf

/* /Msg/Stream type */
#define SPISYNC_TYPEMSG_PBUF           (0)
#define SPISYNC_TYPEMSG_SYSCTRL        (1)
#define SPISYNC_TYPEMSG_USER1          (2)
#define SPISYNC_TYPESTREAM_AT          (3)
#define SPISYNC_TYPESTREAM_USER1       (4)
#define SPISYNC_TYPESTREAM_USER2       (5)
#define SPISYNC_TYPEMAX                (6)

/*
 * clock_10M--->1KBytes/ms
 * clock_20M--->2KBytes/ms
 * clock_40M--->4KBytes/ms
 */
#define SPISYNC_RESET_PATTERN_TIME_MS		(100)
#define SPISYNC_RESET_PATTERN_JITTER_MS		(20)
#define SPISYNC_PATTERN_DURATION_MS		(SPISYNC_RESET_PATTERN_TIME_MS - SPISYNC_RESET_PATTERN_JITTER_MS)
#define SPISYNC_RESYNC_WAIT_DURATION_MS	(SPISYNC_RESET_PATTERN_TIME_MS + SPISYNC_RESET_PATTERN_JITTER_MS)

/* slot.tag */
typedef struct __slot_tag {
    uint16_t version;
    uint16_t seq;
    uint16_t ackseq;
    /* bit0: 1-bus_error */
    uint16_t flag;
    uint32_t clamp[SPISYNC_TYPEMAX];
    uint8_t  resv[2];
    uint16_t crc;
} __attribute__((packed)) slot_tag_t;

/* slot.payload.seg */
typedef struct __payload {
    uint16_t len;
    uint8_t  type;
    uint8_t  resv[1];
    uint8_t  buf[1];
} __attribute__((packed)) slot_seg_t;

/* slot.payload */
typedef struct __slot_payload {
    uint16_t tot_len;
    uint8_t  res[2];
    union {
        uint8_t     raw[1536 + 4];
        slot_seg_t  seg[1];
    };
    uint32_t crc;
} __attribute__((packed)) slot_payload_t;

/* slot.tail */
typedef struct __slot_tail {
    uint16_t    seq_mirror;
    uint8_t     reserved[2];
} __attribute__((packed)) slot_tail_t;

/* slot */
typedef struct __spisync_slot {
    slot_tag_t      tag;
    slot_payload_t  payload;
    slot_tail_t     tail;
} __attribute__((packed)) spisync_slot_t;

/* tx/rx buf */
typedef struct __spisync_txbuf {
    spisync_slot_t slot[SPISYNC_TXSLOT_COUNT];
} __attribute__((packed)) spisync_txbuf_t;

typedef struct __spisync_rxbuf {
    spisync_slot_t slot[SPISYNC_RXSLOT_COUNT];
} __attribute__((packed)) spisync_rxbuf_t;

/* writ or read arg*/
typedef struct __spisync_msg {
    uint8_t type;      // for spisync
    uint32_t timeout;   // rtos send/recv timeout
    void     *buf;      // real buf addr
    uint32_t buf_len;   // read buf length
    void (*cb)(void *arg);  // pbuf_free or complete used
    void *cb_arg;            // pbuf_free or complete used
} spisync_msg_t;

typedef struct __spisync {
    /* sys hdl */
    TaskHandle_t            task_handle;
    EventGroupHandle_t      event;       // EVT wait
    TimerHandle_t           timer;       // Only for delay to reset
    SemaphoreHandle_t       write_lock;  // spisync_write lock
    SemaphoreHandle_t       opspin_log;
    SemaphoreHandle_t       updateclamp; // updateclamp

    QueueHandle_t           prx_queue[3];
    StreamBufferHandle_t    prx_streambuffer[3];
    QueueHandle_t           ptx_queue[3];
    StreamBufferHandle_t    ptx_streambuffer[3];

    /* local global arg */
    uint8_t                 enablerxtx;
    uint16_t                tx_seq;
    uint32_t                txtag_errtimes;
    uint32_t                rxpattern_checked;
    uint32_t                ps_status;  // 0:active, 1:sleeping
    uint32_t                clamp[SPISYNC_TYPEMAX];   // local for tx // local-reicved  // s->m flowctrl

    /* local cb */
    spisync_ops_t           sync_ops[SPISYNC_TYPEMAX]; // for spisync zerocopy, eg:fhost_tx

    /* slot */
    /* Because the cache is aligned to 32 bytes, both the beginning and end
       need to consider cache alignment issues.
     */
    spisync_txbuf_t*        p_tx;
    spisync_rxbuf_t*        p_rx;
    /* calulate crc for rx */
    spisync_slot_t*         p_rxcache;

    /* ramsync */
    lramsync_ctx_t          hw;
	/* The config is only cleared to 0 during init, unchanged during reset */
    const spisync_hw_t      *config;
    int state;
    /* number of deferred spi transactions before going to IDLE. */
    int countdown;
    /* debug */
    uint32_t                isr_rxcnt;
    uint32_t                isr_txcnt;
    uint32_t                rxcrcerr_cnt;
    uint32_t                tsk_rstcnt;
    uint32_t                tsk_writecnt;
    uint32_t                tsk_readcnt;
    uint32_t                tsk_ctrlcnt;
    uint32_t                tsk_rsttick;
    uint32_t                dbg_tick_old;
    uint32_t                isr_rxcnt_old;
    uint32_t                isr_txcnt_old;
    uint32_t                tsk_writecnt_old;
    uint32_t                tsk_readcnt_old;
    uint32_t 				tx_throttled[SPISYNC_TYPEMAX];
} spisync_t;

/* base api */
int spisync_init        (spisync_t *spisync, const spisync_config_t *config);
int spisync_build_typecb(spisync_ops_t *msg, uint8_t type, spisync_opscb_cb_t cb, void *cb_pri);
int spisync_reg_typecb  (spisync_t *spisync, spisync_ops_t *msg);
int spisync_deinit      (spisync_t *spisync);
int spisync_read        (spisync_t *spisync, spisync_msg_t *msg, uint32_t flags);
int spisync_write       (spisync_t *spisync, spisync_msg_t *msg, uint32_t flags);
int spisync_build_msg   (spisync_msg_t *msg, uint32_t type, void *buf, uint32_t len, uint32_t timeout);

/* debug api */
int spisync_status              (spisync_t *spisync);
int spisync_status_internal     (spisync_t *spisync);
int spisync_fakewrite_forread   (spisync_t *spisync, spisync_msg_t *msg, uint32_t flags);

/* todo */
int spisync_set_clamp(spisync_t *spisync, uint8_t type, uint32_t cnt);

#define SPISYNC_RESET_PATTERN		(0x55)

#define SPISYNC_IDLE_TIMER_PERIOD_MS    50

int spisync_dump          (spisync_t *spisync);
int spisync_dump_internal (spisync_t *spisync);

int spisync_iperf         (spisync_t *spisync, int enable);
int spisync_iperftx       (spisync_t *spisync, uint32_t time_sec);

int spisync_iperf_test(spisync_t *spisync, uint8_t tx_en, uint8_t type, uint32_t per, uint32_t t);
int spisync_type_test(spisync_t *spisync, uint32_t period, uint32_t t);

#endif

