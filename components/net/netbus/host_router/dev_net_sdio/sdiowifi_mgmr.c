#include <sdiowifi_securec_wrap.h>
#include <stdbool.h>
#include <string.h>
#include <sdiowifi_platform_adapt.h>
#include <lwip/netif.h>
//#include <qcc74x_wifi.h>
#include <wifi_mgmr_ext.h>
//#include <vfs.h>

#include <rnm_server.h>

#include <sdiowifi_config.h>
#include <sdiowifi_mgmr.h>
#include <sdiowifi_mgmr_type.h>
#include <sdiowifi_tty.h>

#include "sdio_port.h"

#if 0
#if defined(SDIO_REUSE_LP_RAM) && !(defined(CFG_QCC743) && defined(LP_APP))
#error "SDIO lp ram reuse configuration error"
#endif
#endif

#define SDU_WORKER_EV_RESET (1 << 0)

void notify_sdu_worker(int event, int isr);
void sdu_software_reset_set(int enable);

sdiowifi_mgmr_t g_sdiowifi_mgmr;

int sdiowifi_update_tx_credit(struct sdiowifi_tx_buf *tb)
{
    *(volatile unsigned char *)(0x2000d167) = (unsigned char)tb->una_seq;
    qc_write_s_reg(&g_sdiowifi_mgmr.trcver, 0, tb->una_seq + tb->window_size);

    return 0;
}

static int rnm_tx_data(void *env, rnm_output_data_type_t type, const void *data, uint16_t data_length)
{
    int ret = RNM_OUTPUT_FN_SUCCESS;
    int tx_ret = 0;
    net_wifi_trcver_t *trcver = (net_wifi_trcver_t *)env;

    if (type == RNM_OUT_COMMAND) {
        tx_ret = qc_msg_send(trcver, QC_MSG_TYPE_CMD, data, data_length);
    } else if (type == RNM_OUT_SNIFFER_FRAME) {
        // XXX not implemented
    }
    if (tx_ret == QC_MSG_ERR_DESC_USED) {
        HR_LOGW("cmd QC_MSG_ERR_DESC_USED -> sdiowifi_mgmr_reinit -> SDU_WORKER_EV_RESET\r\n");
        sdiowifi_mgmr_reinit(false);
        hr_coredump();
        ret = RNM_OUTPUT_FN_ERROR_OTHER;
    } else if (tx_ret == QC_MSG_ERR_TIMEOUT) {
        ret = RNM_OUTPUT_FN_TIMEOUT;
    } else if (tx_ret != 0) {
        ret = RNM_OUTPUT_FN_ERROR_OTHER;
    }
    return ret;
}

#ifdef SDIOWIFI_HEARTBEAT
static int heartbeat_failure(void *arg)
{
    sdiowifi_mgmr_t *sdm = (sdiowifi_mgmr_t *)arg;

    (void)sdm;

    HR_LOGW("heartbeat fail\r\n");
    sdiowifi_mgmr_reinit(false);
    return 0;
}
#endif

static void ready_cb(void *arg)
{
    sdiowifi_mgmr_t *sdm = (sdiowifi_mgmr_t *)arg;
    HR_LOGI("device ready\r\n");

    qcc74x_net_wifi_trcver_set_present(&sdm->trcver, true);

    sdiowifi_update_tx_credit(&sdm->tx_desc);
#ifdef DHCP_IN_EMB
    rnms_notify_sta_ip_addr(sdm->rnm);
#endif

#ifdef SDIOWIFI_HEARTBEAT
    rnms_heartbeat_register_failure_cb(sdm->rnm, heartbeat_failure, sdm);
    rnms_heartbeat_start(sdm->rnm, 200, 2);
#endif

#if SDIOWIFI_TTY
    sdiowifi_tty_send_bsr(sdm, 0, true, 0);
#endif
}

static void tx_buf_init(sdiowifi_mgmr_t *sdm)
{
    sdiowifi_tx_buf_init(&sdm->tx_desc);

    sdiowifi_tx_buf_lpmem_register(&sdm->tx_desc);
    
#if CONFIG_SDIO_HIGH_PERFORMANCE
    extern uint8_t _heap_wifi_start;
    extern uint8_t _heap_wifi_size;
    sdiowifi_tx_buf_mem_register(&sdm->tx_desc, &_heap_wifi_start, (size_t)&_heap_wifi_size);
#endif
#if 0
#ifdef CFG_QCC743
#ifdef LP_APP
#ifdef SDIO_REUSE_LP_RAM
    sdiowifi_tx_buf_mem_register(&sdm->tx_desc, (void *)SW_TX_BUF_START, SW_TX_BUF_SIZE);
#else
    extern uint8_t __LD_CONFIG_EM_SEL;
    volatile uint32_t em_size;

    em_size = (uint32_t)&__LD_CONFIG_EM_SEL;
    if (em_size == 0) {
        sdiowifi_tx_buf_mem_register(&sdm->tx_desc, (void *)(0x23010000 + (160 - 32) * 1024), 32 * 1024);
    }
#endif // SDIO_REUSE_LP_RAM
#endif // LP_APP

#ifndef QCC74x_SDIO_BUFF_REDUCTION
    extern uint8_t _heap_wifi_start;
    extern uint8_t _heap_wifi_size;
    sdiowifi_tx_buf_mem_register(&sdm->tx_desc, &_heap_wifi_start, (size_t)&_heap_wifi_size);
#endif

#else
    static uint32_t tx_buf[22 * 1024 / 4] __attribute__((section(".wifi_ram")));
    sdiowifi_tx_buf_mem_register(&sdm->tx_desc, tx_buf, sizeof(tx_buf));
#endif
#endif
    sdiowifi_tx_buf_init_done(&sdm->tx_desc);
}

int sdiowifi_mgmr_start(sdiowifi_mgmr_t *sdm)
{
    if (!sdm || sdm->init) {
        return -1;
    }
    MEMSET_SAFE(sdm,  sizeof(*sdm),  0,  sizeof(*sdm));
    if ((sdm->rnm = rnm_server_init(&sdm->trcver, rnm_tx_data, sdm, ready_cb)) == NULL) {
        return -1;
    }
    qc_net_wifi_trcver_init(&sdm->trcver, sdm);
    tx_buf_init(sdm);

#if SDIOWIFI_TTY
    sdiowifi_tty_init(sdm, 0, SDIOWIFI_TTY_PATH);
#endif

    sdm->init = true;

    return 0;
}

int sdiowifi_mgmr_reinit(bool init_tx_buf)
{
    (void)init_tx_buf;

    sdiowifi_mgmr_t *sdm = &g_sdiowifi_mgmr;
    if (!sdm->init) {
        HR_LOGW("%s(%d): sdm is not initialized\r\n", __func__, __LINE__);
        return -1;
    }
    sdu_software_reset_set(true);
    notify_sdu_worker(SDU_WORKER_EV_RESET, 0);
    return 0;
}

int sdiowifi_tx_buf_init_after_lp(void)
{
    sdiowifi_mgmr_t *sdm = &g_sdiowifi_mgmr;
    if (!sdm->init) {
        HR_LOGW("%s(%d): sdm is not initialized\r\n", __func__, __LINE__);
        return -1;
    }

    tx_buf_init(sdm);
    return 0;
}

int sdiowifi_mgmr_ps_prepare(void)
{
    sdiowifi_mgmr_t *sdm = &g_sdiowifi_mgmr;

    qcc74x_net_wifi_trcver_set_present(&sdm->trcver, false);
    qcc74x_sdio_tx_timer_stop();

#ifdef SDIOWIFI_HEARTBEAT
    rnms_heartbeat_stop(sdm->rnm);
#endif
    return 0;
}
