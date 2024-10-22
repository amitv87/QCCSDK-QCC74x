#ifndef QCC74x_SDIO_ETH_H_D3XYUMHC
#define QCC74x_SDIO_ETH_H_D3XYUMHC

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include "config.h"
#include "sdio_msgs.h"
#include "wifi.h"

#define SDIO_VENDOR_ID      0x424c
#define SD_DEVICE_ID        0x0606

#define QCC74x_ETH_TX_TIMEOUT       (30 * HZ)

#define QCC74x_NDEV_FLOW_CTRL_STOP      32
#define QCC74x_NDEV_FLOW_CTRL_RESTART   16

#define QCC74x_TXQ_NDEV_FLOW_CTRL 1

#define QCC74x_FUNC_SCRATCH_BASE           0x60
#define FREE_TX_PBUF_CNT_OFFSET        (QCC74x_FUNC_SCRATCH_BASE + 0)
#define WIFI_MAC_ADDR_SCRATCH_OFFSET   1
#define QCC74x_WIFI_MAC_ADDR_OFFSET        (QCC74x_FUNC_SCRATCH_BASE + WIFI_MAC_ADDR_SCRATCH_OFFSET)
#define TTY_RCV_BUF_REM_OFFSET(tty_id) (QCC74x_FUNC_SCRATCH_BASE + 1 + (tty_id))

#define TTY_RCV_BUF_REM_TO_BYTES(val)  (val * 128)

#define SDIO_TX_TIMEOUT 300

#define SDIOCMD_BUF_LEN 2048

#define USE_POLLING_RX 0

#define SD_RX_BUF_LEN 2048

/* SDIO_DNLD_MAX_PORTS should be equal with SDIO_RX_MAX_PORT_NUM
in components/network/sdiowifi_mgmr/trcver_sdio.h */

/* Note: The value of SDIO_DNLD_MAX_PORTS for ezviz must be 4 */
#define SDIO_DNLD_MAX_PORTS 4

#define TX_CREDIT_THRESHOLD 3

#define TX_ETH_FRAME_BULK 4

typedef void (*qcc74x_sdio_read_cb_t)(void *cb_arg, const void *data_ptr, uint8_t num_of_pdu, const uint16_t data_len);

struct xfr_ctx {
    u8 *prx_data;
    u8 *ptx_data;
    qcc74x_sdio_read_cb_t read_process;
    void *read_process_cb_arg;
};

struct qcc74x_eth_device {
    qcc74x_wifi_mode_t mode;
    struct sdio_func *func;
    struct net_device *net;
    struct rtnl_link_stats64 stats;
    u8 sta_mac[ETH_ALEN];
    u8 ap_mac[ETH_ALEN];
    const struct qcc74x_sdio_card_reg *reg;
    u8 *mp_regs;
    u8 max_ports;
    u8 status;
    u32 mp_rd_bitmap;
    u32 mp_wr_bitmap;

    u8 curr_rd_port;
    u8 curr_wr_port;

    u8 my_curr_rd_port;

    struct xfr_ctx xfr;

    void *sdiocmd_buf;

    bool has_control_mask;
    u32 io_port;
    u32 int_status;
    u8 qcc74x_processing;
    u8 qcc74x_tx_processing;
    u8 more_task_flag;
    u8 tx_more_task_flag;

    spinlock_t int_lock;
    spinlock_t main_proc_lock;

    spinlock_t tx_proc_lock;

    /* firmware version for compatibility */
    uint32_t fw_version;
    struct completion req_done;
    /* local sequence of sending packet */
    u8 snd_nxt;
    /*
     * the sequence clamp of sending packet, and snd_nxt never
     * exceeds seq_clamp, which is how flow control works.
     */
    u8 seq_clamp;

    struct sk_buff_head sk_list;

    struct work_struct main_work;
    struct workqueue_struct *workqueue;

    struct work_struct tx_work;
    struct workqueue_struct *txworkqueue;

    struct sk_buff_head cmd_list;

    bool sta_connected;

    atomic_t tty_credit;
    struct sk_buff_head tty_msg_list;
#if CARD_DEAD_CHECK
    unsigned long last_ping_recv_time;
    struct timer_list check_card_timer;
    bool check_card_flag;
#endif
    /* statistics */
    unsigned long max_tx_loop_elapsed_ms;
    unsigned long max_netif_stopped_ms;
    unsigned long sdio_write_timeo_cnt;
    unsigned long sdio_write_retries;
    unsigned long sdio_io_fail_cnt;
    /* jiffies in which netif is stopped */
    unsigned long netif_stopped_ts;
};

/* extern struct qcc74x_eth_device *gl_dev; */

int qcc74x_sdio_write(struct qcc74x_eth_device *dev, const void *data_ptr, const uint16_t data_len);
int qcc74x_sdio_read(struct qcc74x_eth_device *dev, void **data);
int qcc74x_eth_get_mac_addr(struct qcc74x_eth_device *dev);

static inline u16 get_tx_eth_frame_subtype(struct qcc74x_eth_device *dev)
{
    u16 subtype = QC_MSG_ETH_WIFI_FRAME_SUBTYPE_STA_TO_WIFI_TX;

    if (dev->mode == QCC74x_MODE_AP) {
        subtype = QC_MSG_ETH_WIFI_FRAME_SUBTYPE_AP_TO_WIFI_TX;
    }
    return subtype;
}

static inline u16 get_rx_eth_frame_subtype(struct qcc74x_eth_device *dev)
{
    u16 subtype = QC_MSG_ETH_WIFI_FRAME_SUBTYPE_STA_FROM_WIFI_RX;

    if (dev->mode == QCC74x_MODE_AP) {
        subtype = QC_MSG_ETH_WIFI_FRAME_SUBTYPE_AP_FROM_WIFI_RX;
    }
    return subtype;
}

#endif /* end of include guard: QCC74x_SDIO_ETH_H_D3XYUMHC */
