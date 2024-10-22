#ifndef TRCVER_SDIO_H_7YSRGJ3T
#define TRCVER_SDIO_H_7YSRGJ3T

#include <stdbool.h>
#include <stdint.h>
#include <lwip/pbuf.h>
#include "sdiowifi_utils.h"

#define LP_FW_START_ADDR    0x23026800 // XXX duplicated
#define LP_FW_MAX_SIZE      (37 * 1024)

#ifndef QCC74x_SDIO_RX_MAX_PORT_NUM
#define SDIO_RX_MAX_PORT_NUM   8
#else
#define SDIO_RX_MAX_PORT_NUM QCC74x_SDIO_RX_MAX_PORT_NUM
#endif

#define SDIO_RX_BUF_SIZE     2048
#define SDIO_TX_BUF_SIZE     2048
#define SDIO_RX_BUF_START    SDM_ALIGN_HI(LP_FW_START_ADDR, 32)
#define SDIO_RX_BUF_SIZE_TOT (SDIO_RX_MAX_PORT_NUM * SDIO_RX_BUF_SIZE)
#define SDIO_RX_BUF_END      (SDIO_RX_BUF_START + SDIO_RX_BUF_SIZE_TOT)
#define SW_TX_BUF_SIZE       (LP_FW_MAX_SIZE - SDIO_RX_BUF_SIZE_TOT - 128)
#define SW_TX_BUF_START      SDM_ALIGN_HI(SDIO_RX_BUF_END, 32)

typedef void (*qcc74x_sdio_read_cb_t)(void *cb_arg, const void *data_ptr, uint16_t data_len);

// init
int qcc74x_sdu_init(void);

// read register
int qcc74x_sdio_read_cb_register(void *env, qcc74x_sdio_read_cb_t cb, void *cb_arg);

// ctrl
int32_t qcc74x_sdio_handshake(void);
void qcc74x_sdio_tx_timer_stop(void);

// write
/* data_len is len(V), excluding len(TL) */
int qcc74x_sdio_write_pbuf(void *env, uint16_t type, uint16_t subtype, struct pbuf *p, bool is_amsdu, void *cb, void *cb_arg);
int qcc74x_sdio_write_cmd(void *env, uint16_t type, uint16_t subtype, const void *headroom, uint16_t headroom_len, const void *tailroom, uint16_t tailroom_len);

#endif /* end of include guard: TRCVER_SDIO_H_7YSRGJ3T */
