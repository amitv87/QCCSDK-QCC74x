#ifndef _MFG_MAIN_H_
#define _MFG_MAIN_H_

#include <stdio.h>
#include <FreeRTOS.h>
#include <queue.h>
#include "wl_cli_api.h"

void mfg_main(void *pvParameters);
void mfg_temp_trim(void *pvParameters);


#if defined(MFG_RAM_QCC743) || defined(MFG_CCI_QCC743) || defined(MFG_GU_QCC743)
#define MFG_QCC743
#endif

#if defined(MFG_RAM_QCC74x_undef) || defined(MFG_GU_QCC74x_undef)
#define MFG_QCC74x_undef
#endif

#if defined(MFG_GU_QCC74x_undef) || defined(MFG_GU_QCC743)
#define MFG_GU
#endif

#if defined(MFG_RAM_QCC743) || defined(MFG_RAM_QCC74x_undef) || defined(MFG_CCI_QCC743)
#define MFG_RAM
#endif


#define MFG_Q_NUM                           8
#define MFG_CMD_MAX_LEN                     128
#define mfg_print                           printf

#if defined(CONFIG_BLE_MFG_HCI_CMD) || defined(CONFIG_BT_MFG_HCI_CMD)
#define BT_LE_HCI_CMD         1
#define BT_LE_HCI_CMD_HDR_LEN 4
#define BT_HCI_CMD            0
#endif

void cmd_le_rx_test(char *buf, int len, int argc, char **argv);
void cmd_le_tx_test(char *buf, int len, int argc, char **argv);
void cmd_le_test_stop(char *buf, int len, int argc, char **argv);
void cmd_bt_rx_test(char *buf, int len, int argc, char **argv);
void cmd_bt_tx_test(char *buf, int len, int argc, char **argv);
void cmd_bt_test_stop(char *buf, int len, int argc, char **argv);

#define MFG_TX_MODE_B                       0
#define MFG_TX_MODE_G                       1
#define MFG_TX_MODE_N                       2
#define MFG_TX_MODE_AX                       3

#define MFG_BOOT_REQ_ADDR              4 * 1024

typedef struct mfg_tx_para{
    int8_t tx_pwr_dbm;
    uint8_t tx_duty;
    uint8_t mfgmode;
    uint8_t tx_mode;
    uint8_t tx_mcs;
    uint8_t cont_mode;
    uint8_t sg_chantype;
    uint8_t ax_coding_type;
    uint8_t ax_gi;
    uint16_t frequency;
    uint16_t tx_len;
    uint16_t sg_channel;
    uint16_t sg_channel_index;
    uint16_t ble_is_sending;
    uint16_t ble_sending_power;
    uint16_t zb_sending_power;
    uint16_t tx_on;
    uint16_t bw;
}mfg_tx_para_t;

typedef enum mfg_thread_state
{
    MFG_THREAD_NOT_CREATE,
    MFG_THREAD_RUNNING,
    MFG_THREAD_SUSPEND,
}mfg_thread_state_t;


/*mfg command varials*/
typedef int32_t (*pfun_cmd_process)(uint8_t *data, uint16_t len);
typedef struct mfg_cmd_tag{
    uint8_t cmd;
    pfun_cmd_process cmd_process;
}mfg_cmd_t;

extern QueueHandle_t Message_Queue;
extern uint8_t *phy_cli_list[20];
extern uint8_t phy_cli_cmd[20][32];

#if defined(MFG_QCC743)
#define UART_TX_PIN 21
#define UART_RX_PIN 22

#define MFG_XTAL_TYPE GLB_XTAL_40M
#define MFG_XTAL_VALUE_STR "40000000"
#define MFG_XTAL_VALUE 40 * 1000 * 1000
#define MFG_RAM_RFTLV_BASE_ADDR 0x62fc0400
#define MFG_GU_RFTLV_BASE_ADDR 0xa0000400
#endif

#if defined(MFG_QCC74x_undef)
#define UART_TX_PIN 28
#define UART_RX_PIN 29

#define MFG_XTAL_TYPE GLB_XTAL_26M
#define MFG_XTAL_VALUE_STR "26000000"
#define MFG_XTAL_VALUE 26 * 1000 * 1000
#define MFG_RAM_RFTLV_BASE_ADDR 0x20fc0400
#define MFG_GU_RFTLV_BASE_ADDR 0x80000400
#endif

#define MFG_PWR_OFFSET_MAX (15)
#define MFG_PWR_OFFSET_MIN (-16)

#ifdef MFG_HTOL
/**
* WiFi RX/TX Configuration
*/
#define QCC74x_HTOL_CH_NUM       (14)
#define QCC74x_HTOL_TX_DUR_MS    (200)
#define QCC74x_HTOL_RX_DUR_MS    (80)
#define QCC74x_HTOL_TX_FRAME_LEN (800) //tx frame len, Unit is Byte
#define QCC74x_HTOL_TX_11AX_POWER (18)  //dbm
#define QCC74x_HTOL_TX_11N_POWER (18)  //dbm
#define QCC74x_HTOL_TX_11B_POWER (23)  //dbm


/**
 *  RF Calibration Configuration
 **/
#define QCC74x_HTOL_RFCAL_LOOP_CNT (10000) //100ms * 10000 = 16min

#endif

extern void cmd_bandwidth_register(char *buf, int len, int argc, char **argv);
#ifdef MFG_GU//defined(MFG_LPGU)
#define LP_FW_START_ADDR    0x23022000
extern unsigned char __lpfw_start[];
typedef void (*pentry_t)(void);
#define LP_FW_PRE_JUMP_ADDR 0x20010000
#define IOT2LP_PARA_ADDR    0x20010400

typedef struct {
    uint8_t tim_wakeup_en; /* 1: enable tim wakeup */
    uint8_t channel;
    uint8_t bssid[6] ;
    uint8_t mac[6];
    uint8_t dtim_num;
    uint8_t mtimer_timeout_ms;

    uint32_t aid ;
    uint32_t tpre ;

    uint8_t timeout_cnt_th;
    uint8_t ap_loss_cnt_th;

    uint32_t rtc_timeout_ms; /* 0: disable rtc wakeup */

    int32_t  wakeup_reason; /* Cause of wakeup */
    uint32_t lpfw_recv_cnt; /* count of loss packet during rtc_timeout_ms */
    uint32_t lpfw_loss_cnt; /* count of wakeup during rtc_timeout_ms */
}qcc74x_lp_fw_cfg_t;

typedef struct __attribute__((packed)) {
    uint32_t pattern;/*0xAA5555AA*/
    /* wifi para */
    uint8_t ap_channel;
    uint8_t rsvd_1;
    uint16_t aid;
    uint8_t bssid[6];
    uint8_t local_mac[6];
    /* system para */
    uint32_t wakeup_flag;
    uint32_t flash_offset;
    uint32_t app_entry;
    uint32_t args[4];
    uint32_t cpu_regs[32];
    uint32_t tim_wake_enable;
    uint32_t rtc_wakeup_ms;     /* Timeout duration of the rtc */
    uint64_t rtc_wakeup_cnt;    /* The value of the next rtc wake up */
    int32_t wakeup_reason;      /* reason of wake up */
    /* check */
    int32_t check_data_valid;
    uint32_t rtc_timeout_ms;
    uint32_t pds_sleep_ms;
    /* beacon  */
    int32_t tpre;
    uint32_t next_beacon_stamp_rtc_valid;
    uint64_t next_beacon_stamp_rtc;
    uint8_t continuous_loss_cnt;
    uint8_t timeout_cnt_th;     /*  */
    uint8_t ap_loss_cnt_th;     /* Must be greater than timeout_cnt_th */
    uint8_t mtimer_timeout_ms;
    int32_t lpfw_wakeup_cnt;
    uint8_t dtim_num;
    uint8_t rc32k_err_rate;  /* rc32k err rate,10 for 1‰, 20 for 2‰ */
    /*rc32k_trim */
    uint8_t rc32k_auto_cal;
    uint8_t rsvd_rtc;
    uint16_t recal_cnt;
    uint32_t last_rtc_cnt;
    uint32_t last_time_stamp;
    uint32_t lpfw_recv_cnt;
    uint32_t lpfw_loss_cnt;
    uint32_t rc32k_fr_ext;
    uint32_t tim_offset;
}iot2lp_para_t;

/* beacon stamp valid type */
#define BEACON_STAMP_LPFW   1
#define BEACON_STAMP_APP    2

/* Conversion of units */
#define QCC74x_US_TO_PDS_CNT(us)     ((us) * 512 / 15625)   /* us * 32768 / 1000 /1000 */
#define QCC74x_MS_TO_PDS_CNT(ms)     ((ms) * 4096 / 125)    /* ms * 32768 / 1000 */
#define QCC74x_PDS_CNT_TO_US(cnt)    ((cnt) * 15625 / 512)  /* cnt / 32768 * 1000 /1000 */
#define QCC74x_PDS_CNT_TO_MS(cnt)    ((cnt) * 125 / 4096)   /* cnt / 32768 * 1000 */

/*  */
#define PROTECT_AF_MS      (7)
#define PROTECT_BF_MS      (4)
#endif
#endif

