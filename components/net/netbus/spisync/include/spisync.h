#ifndef __SPISYNC_UPPER_H__
#define __SPISYNC_UPPER_H__

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include "message_buffer.h"
#include "stream_buffer.h"
#include <event_groups.h>

#include <ramsync.h>

// slot config
#define SPISYNC_TXSLOT_COUNT        (6)
#define SPISYNC_RXSLOT_COUNT        (6)
#define SPISYNC_SLOT_MAGIC          (0x11223344)
#define SPISYNC_PAYLOADBUF_LEN      (1024 + 512)             // 1536
#define SPISYNC_PAYLOADLENSEQ_LEN   (8)                  // sizeof(len)+sizeof(seq)
#define SPISYNC_PAYLOADCRC_LEN      (4)                      // sizeof(crc)
#define SPISYNC_PAYLOADOTH_LEN      (12)                     // sizeof(len)+sizeof(seq)+sizeof(crc)
#define SPISYNC_PAYLOADLEN_MAX      (SPISYNC_PAYLOADBUF_LEN + SPISYNC_PAYLOADLENSEQ_LEN)
#define SPISYNC_TAGERR_TIMES        (SPISYNC_RXSLOT_COUNT*4)
#define SPISYNC_PATTERN_TIMES       (SPISYNC_TXSLOT_COUNT*5)
#define SPISYNC_PATTERN_DELAYMS     (100)// clock 10M ---> 1KBytes/ms

// tx buf
#define SPISYNC_TX_MSGBUFFER_SIZE   (3*sizeof(spisync_slot_t))

// rx buf
#define SPISYNC_MSGBUFFER_TO_STREAM (0)
#if SPISYNC_MSGBUFFER_TO_STREAM 
#define SPISYNC_RX_MSGBUFFER_SIZE   (3*sizeof(spisync_slot_t))
#endif
#define SPISYNC_RX_STREAMBUFFER_SIZE    (6*sizeof(spisync_slot_t))
#define SPISYNC_RX_READTIMES            (2)

#define SPISYNC_RX_STREAMBUFFER_LEVEL   (SPISYNC_PAYLOADBUF_LEN)

// task config
#define SPISYNC_STACK_SIZE          (2048)
#define SPISYNC_TASK_PRI            (30)

/* enum */
#define EVT_SPISYNC_RESET_BIT       (1<<1)
#define EVT_SPISYNC_CTRL_BIT        (1<<2)
#define EVT_SPISYNC_WRITE_BIT       (1<<3)
#define EVT_SPISYNC_READ_BIT        (1<<4)
#define EVT_SPISYNC_RSYNC_BIT       (1<<5)
#define EVT_SPISYNC_DUMP_BIT        (1<<6)
#define EVT_SPISYNC_ALL_BIT         (EVT_SPISYNC_RESET_BIT | \
                                     EVT_SPISYNC_CTRL_BIT  | \
                                     EVT_SPISYNC_WRITE_BIT | \
                                     EVT_SPISYNC_READ_BIT  | \
                                     EVT_SPISYNC_RSYNC_BIT)

// enum
// #define SPIMSYNC_DEV_TYPE_MASTER    (1)
// #define SPIMSYNC_DEV_TYPE_SLAVE     (2)

/* slot */
typedef struct __slot_tag {
    uint32_t magic;
    uint32_t rseq;
    uint32_t flag;// bit0: 0-noerr, 1-err
} slot_tag_t;

typedef struct __slot_payload {
    uint32_t    len;
    uint32_t    seq;
    uint8_t     buf[SPISYNC_PAYLOADBUF_LEN];
    uint32_t    crc;
} slot_payload_t;

typedef struct __spisync_slot {
    slot_tag_t      tag;
    slot_payload_t  payload;
} spisync_slot_t;

/* tx/rx buf */
typedef struct __spisync_txbuf {
    spisync_slot_t slot[SPISYNC_TXSLOT_COUNT];
} spisync_txbuf_t;

typedef struct __spisync_rxbuf {
    spisync_slot_t slot[SPISYNC_RXSLOT_COUNT];
} spisync_rxbuf_t;

/* hdr */
typedef struct __spisync {
    /* sys hdl */
    TaskHandle_t            taskhdr;
    EventGroupHandle_t      event;
    TimerHandle_t           timer;//only for delay reset
    MessageBufferHandle_t   tx_msgbuffer;
#if SPISYNC_MSGBUFFER_TO_STREAM
    MessageBufferHandle_t   rx_msgbuffer;
#endif
    StreamBufferHandle_t    rx_streambuffer;

    /* local global arg */
    uint32_t                tx_seq;
    uint32_t                txtag_errtimes;
    uint32_t                txwrite_continue;
    uint32_t                rxpattern_checked;
    uint32_t                rxread_continue;

    /* slot */
    /* Because the cache is aligned to 32 bytes, both the beginning and end
    need to consider cache alignment issues. */
    spisync_txbuf_t*        p_tx;
    spisync_rxbuf_t*        p_rx;
    /* calulate crc for rx */
    slot_payload_t*         p_rxcache;

    /* ramsync */
    lramsync_ctx_t          hw;
    // The config is only cleared to 0 during init, unchanged during reset
    const spisync_config_t *config;

    /* debug */
    uint32_t                isr_rxcnt;
    uint32_t                isr_txcnt;
    // The tsk_rstcnt is only cleared to 0 during init, unchanged during reset
    uint32_t                tsk_rstcnt;
    uint32_t                tsk_ctrlcnt;
    uint32_t                tsk_writecnt;
    uint32_t                tsk_readcnt;
#if SPISYNC_MSGBUFFER_TO_STREAM
    uint32_t                tsk_rsynccnt;
#endif
    uint32_t                tsk_rsttick;

    uint32_t                iperf;
    uint32_t                dbg_tick_old;
    uint32_t                isr_rxcnt_old;
    uint32_t                isr_txcnt_old;
    uint32_t                tsk_writecnt_old;
    uint32_t                tsk_readcnt_old;

} spisync_t;

int spisync_init        (spisync_t *spisync, const spisync_config_t *config);
int spisync_deinit      (spisync_t *spisync);
int spisync_read        (spisync_t *spisync, void *buf, uint32_t len, uint32_t timeout_ms);
int spisync_write       (spisync_t *spisync, void *buf, uint32_t len, uint32_t timeout_ms);

int spisync_fakewrite_forread(spisync_t *spisync, void *buf, uint32_t len, uint32_t timeout_ms);

int spisync_dump          (spisync_t *spisync);
int spisync_dump_internal (spisync_t *spisync);

int spisync_iperf         (spisync_t *spisync, int enable);
int spisync_iperftx       (spisync_t *spisync, uint32_t time_sec);

#endif
