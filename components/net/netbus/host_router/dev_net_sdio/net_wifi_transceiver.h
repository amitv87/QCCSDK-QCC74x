#ifndef __NET_WIFI_TRANSCEIVER_H__
#define __NET_WIFI_TRANSCEIVER_H__
#include <stdint.h>
#include <stdbool.h>
#include <sdiowifi_platform_adapt.h>
#include <lwip/netif.h>
#include <lwip/pbuf.h>

#define QC_MSG_TYPE_ETH_WIFI_FRAME 0xabcd
#define QC_MSG_TYPE_CMD            0xef01

/*
 * 0->tx desc cnt
 * 1..6-> mac
 * 7->boot src info
 * 1->tty0 rcv buf rem(reuse)
 * 2->tty1 rcv buf rem(reuse)
 */
#define WIFI_MAC_ADDR_SCRATCH_OFFSET   1
#define BOOT_SRC_INFO_SCRATCH_OFFSET   (1  + 6)
#define TTY_RCV_BUF_REM_OFFSET(tty_id) (1  + (tty_id))
#define QCC74x_BOOT_SRC_PING_VALUE         0xEE
#define QCC74x_BOOT_SRC_PONG_VALUE         0xEF
#define BOOT_SRC_PONG_TIMEOUT_MS       (10 * 1000)

enum {
    QC_MSG_ETH_WIFI_FRAME_SUBTYPE_STA_TO_WIFI_TX = 0x2000,
    QC_MSG_ETH_WIFI_FRAME_SUBTYPE_STA_FROM_WIFI_RX,
    QC_MSG_ETH_WIFI_FRAME_SUBTYPE_AP_TO_WIFI_TX,
    QC_MSG_ETH_WIFI_FRAME_SUBTYPE_AP_FROM_WIFI_RX,
    QC_MSG_ETH_WIFI_FRAME_SUBTYPE_SNIFFER_TO_WIFI_TX,
    QC_MSG_ETH_WIFI_FRAME_SUBTYPE_SNIFFER_FROM_WIFI_RX,
    QC_MSG_ETH_WIFI_FRAME_SUBTYPE_EXT, // UART, etc
};

typedef struct {
    uint16_t type;
    uint16_t len; // type + len + payload
    uint8_t payload[];
} __attribute__((packed)) qcc74x_undef_sdio_msg_t;

typedef struct {
    void *arg;

    bool host_present;

    sdiowifi_mutex tx_lock;
} net_wifi_trcver_t;

#define QC_MSG_ERR_HOST_NOT_READY 2
#define QC_MSG_ERR_TIMEOUT        3
#define QC_MSG_ERR_DESC_USED      4

int qc_net_wifi_trcver_init(net_wifi_trcver_t *trcver, void *arg);
int qc_net_wifi_trcver_reinit(net_wifi_trcver_t *trcver);
int qcc74x_net_wifi_trcver_set_present(net_wifi_trcver_t *trcver, bool present);
int qc_msg_send(net_wifi_trcver_t *trcver, uint16_t type, const void *payload, uint16_t payload_len);
int qc_msg_send_pbuf(net_wifi_trcver_t *trcver, uint16_t type, uint16_t subtype, struct pbuf *p, bool is_amsdu, void *cb, void *cb_arg);
int qc_msg_send_frame_ext(net_wifi_trcver_t *trcver, uint16_t type, uint16_t subtype, const void *headroom, uint16_t headroom_len, const void *tailroom, uint16_t tailroom_len);
uint8_t qc_read_s_reg(net_wifi_trcver_t *trcver, uint32_t offset);
void qc_write_s_reg(net_wifi_trcver_t *trcver, uint32_t offset, uint8_t val);

#endif
