#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include "mfg_m154.h"
#include "qcc743_common.h"
#include "lmac154.h"
#include "wl_api.h"

#define MFG_STATE_IDLE                      0
#define MFG_STATE_CW_ON                     1
#define MFG_STATE_ZB_TX_ON                  2
#define MFG_STATE_ZB_RX_ON                  3
#define MFG_STATE_BLE_TX_ON                 4
#define MFG_STATE_BLE_RX_ON                 5

#define mfg_print(fmt, ...)                 printf("[mfg fw] "fmt, ##__VA_ARGS__)
#define mfg_zb_print(fmt, ...)              printf("[mfg fw][zb] "fmt, ##__VA_ARGS__)
#define mfg_ble_print(fmt, ...)             printf("[mfg fw][ble] "fmt, ##__VA_ARGS__)
#define mfg_err_print(fmt, ...)             printf("[mfg err] "fmt, ##__VA_ARGS__)


static cci_paras_ram_t cci_paras_env __attribute__((section(".cci_ram")));
static cci_paras_ram_t all_paras_env __attribute__((section(".cci_ram")));
static uint32_t mfg_seq_num_cnt[256];
static uint8_t mfg_zb_pkt[] = {0x41,0x88,0x00,0xAA,0x1A,0xFF,0xFF,0x44,0x33,0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10};
static int mfg_state = MFG_STATE_IDLE;
static int mfg_tx_cnt = 0;
static int mfg_tx_freq = 100;
static TimerHandle_t mfg_timer = NULL;


static int32_t mfg_show_error(int32_t err_code)
{
    switch(err_code)
    {
        case MFG_ERR_CMD_LEN:
        case MFG_ERR_CMD_FORMAT:
        case MFG_ERR_CMD_PARAM:
            mfg_err_print("Invalid command\r\n");
        break;

        case MFG_ERR_CMD_UNALLOWED:
            mfg_err_print("Unallowed command\r\n");
        break;

        case MFG_ERR_CMD_FAILED:
            mfg_err_print("Command execution failed\r\n");
        break;

        default:
            mfg_err_print("Unknown error\r\n");
        break;
    }

    return err_code;
}

static void mfg_process(TimerHandle_t xTimer)
{
    // ZB TX Process
    if(mfg_state == MFG_STATE_ZB_TX_ON){
        lmac154_triggerTx(mfg_zb_pkt, sizeof(mfg_zb_pkt), 0);
        mfg_tx_cnt++;
    }
}


// Callback Function
void lmac154_rxDoneEvent(uint8_t *rx_buf, uint8_t rx_len, uint8_t crc_fail)
{
#if defined(MFG_ATE)
    // Check frame length
    if(rx_len != sizeof(mfg_zb_pkt) + 2){
        return;
    }
#endif

    // Check crc
    if(crc_fail){
        return;
    }

    // Check sequence number
    if(rx_buf[2] == mfg_zb_pkt[2]){
        cci_paras_env.rx_frm_cnt++;
        cci_paras_env.rssi_sum += lmac154_getRSSI();
        cci_paras_env.freq_offset_sum += lmac154_getFrequencyOffset();
        cci_paras_env.lqi_sum += lmac154_getLQI();

#if defined(MFG_ATE)
        mfg_zb_print("%d - rx_len %d, seq_num %d\r\n", (int)cci_paras_env.rx_frm_cnt, rx_len, rx_buf[2]);
#endif
    }

    all_paras_env.rx_frm_cnt++;
    all_paras_env.rssi_sum += lmac154_getRSSI();
    all_paras_env.freq_offset_sum += lmac154_getFrequencyOffset();
    all_paras_env.lqi_sum += lmac154_getLQI();

    mfg_seq_num_cnt[rx_buf[2]]++;
}

void lmac154_ackEvent(uint8_t ack_received, uint8_t frame_pending, uint8_t seq_num)
{
    if(ack_received){
        mfg_zb_print("Ack Received (FP: %d, SN: 0x%02X)\r\n", frame_pending, seq_num);
    }else{
        mfg_zb_print("No Ack\r\n");
    }
}

void qcc743_lmac154_getInterruptHandler(void *arg)
{
    lmac154_getInterruptHandler();
}

// Public Function
void m154_init(void)
{
    // Initialize MAC154
    lmac154_init();
    // bz only on
    uint32_t val = *(volatile uint32_t*)0x20001220;
    val |= (1<<26);
    val |= (1<<27);
    *(volatile uint32_t*)0x20001220 = val;

    // qcc74x_irq_attach(M154_INT_IRQn, qcc743_lmac154_getInterruptHandler, NULL);
    qcc74x_irq_attach(M154_INT_IRQn, lmac154_getInterruptHandler(), NULL);
    qcc74x_irq_enable(M154_INT_IRQn);

    // Set PAN ID and Short Address
    lmac154_setPanId(0x1AAA);
    lmac154_setShortAddr(0x3344);

    // Start Timer
    if(mfg_timer == NULL){
        mfg_timer = xTimerCreate("mfg_timer", 1000/mfg_tx_freq, pdTRUE, NULL, mfg_process);
        xTimerStart(mfg_timer, 0);
    }
}

void m154_exit(void)
{
    wl_154_optimize_restore();
    // bz only off
    uint32_t val = *(volatile uint32_t*)0x20001220;
    val &= (~(1<<26));
    val &= (~(1<<27));
    *(volatile uint32_t*)0x20001220 = val;
}


int32_t m154_cmd_channel_switch(uint8_t *data, uint16_t len)
{
    int channel;

    len = strlen((char *)data);

    if(len == 0){
        return mfg_show_error(MFG_ERR_CMD_LEN);
    }

    if(mfg_state != MFG_STATE_IDLE){
        return mfg_show_error(MFG_ERR_CMD_UNALLOWED);
    }

    channel = atoi((char *)data);

    if(channel >= 11 && channel <= 26){
        //if(mfg_power_offset_en){
        //    bz_phy_optimize_tx_channel(2405 + 5 * (channel - 11));
        //}
        wl_154_optimize(2405 + 5 * (channel - 11));

        lmac154_setChannel(channel - 11);
        mfg_zb_print("Set channel %d (%dMHz)\r\n", channel, 2405 + 5 * (channel - 11));
    }else{
        return mfg_show_error(MFG_ERR_CMD_PARAM);
    }

    return 0;
}

int32_t m154_cmd_frequency_ctrl(uint8_t *data, uint16_t len)
{
    int txFreq;

    len = strlen((char *)data);

    if(len == 0){
        return mfg_show_error(MFG_ERR_CMD_LEN);
    }

    if(mfg_state != MFG_STATE_IDLE){
        return mfg_show_error(MFG_ERR_CMD_UNALLOWED);
    }

    txFreq = atoi((char *)data);

    if(txFreq >= 1 && txFreq <= 1000){
        mfg_tx_freq = txFreq;
        mfg_zb_print("TX %d frames per second\r\n", txFreq);

        xTimerChangePeriod(mfg_timer, 1000/mfg_tx_freq, 0);

    }else{
        return mfg_show_error(MFG_ERR_CMD_PARAM);
    }

    return 0;
}

int32_t m154_cmd_set_seq_num(uint8_t *data, uint16_t len)
{
    int seqNum;

    len = strlen((char *)data);

    if(len == 0){
        return mfg_show_error(MFG_ERR_CMD_LEN);
    }

    seqNum = atoi((char *)data);

    if(seqNum >= 0 && seqNum <= 255){
        mfg_zb_pkt[2] = seqNum;
        mfg_zb_print("Set sequence number %d\r\n", seqNum);
    }else{
        return mfg_show_error(MFG_ERR_CMD_PARAM);
    }

    return 0;
}

int32_t m154_cmd_tx_toggle(uint8_t *data, uint16_t len)
{
    len = strlen((char *)data);

    if(len != 1){
        return mfg_show_error(MFG_ERR_CMD_LEN);
    }

    if(data[0] == '1'){
        if(mfg_state != MFG_STATE_IDLE){
            return mfg_show_error(MFG_ERR_CMD_UNALLOWED);
        }

        mfg_tx_cnt = 0;
        mfg_state = MFG_STATE_ZB_TX_ON;
        mfg_zb_print("TX toggle on\r\n");
    }else if(data[0] == '0'){
        if(mfg_state != MFG_STATE_ZB_TX_ON){
            return mfg_show_error(MFG_ERR_CMD_UNALLOWED);
        }

        mfg_state = MFG_STATE_IDLE;
        mfg_zb_print("TX toggle off\r\n");
        mfg_zb_print("tx_frm_cnt:%d\r\n", mfg_tx_cnt);

        // Stop rx because of HW RX-after-TX
        arch_delay_ms(10);
        lmac154_disableRx();
    }else{
        return mfg_show_error(MFG_ERR_CMD_PARAM);
    }

    return 0;
}

int32_t m154_cmd_rx_ctrl(uint8_t *data, uint16_t len)
{
    uint32_t max = 0;
    int seqNum = 0;
    int i;

    len = strlen((char *)data);

    if(len != 2){
        return mfg_show_error(MFG_ERR_CMD_LEN);
    }

    if(data[0] != ':') {
        return mfg_show_error(MFG_ERR_CMD_FORMAT);
    }

    if(data[1] == 's' || data[1] == 'S'){
        if(mfg_state != MFG_STATE_IDLE){
            return mfg_show_error(MFG_ERR_CMD_UNALLOWED);
        }

        memset(&cci_paras_env, 0, sizeof(cci_paras_env));
        memset(&all_paras_env, 0, sizeof(all_paras_env));
        memset(mfg_seq_num_cnt, 0, sizeof(mfg_seq_num_cnt));

        if(data[1] == 's'){
            lmac154_enableRxPromiscuousMode(0, 0);
        }else{
            lmac154_enableRxPromiscuousMode(1, 0);
        }

        lmac154_enableRx();
        mfg_state = MFG_STATE_ZB_RX_ON;
        mfg_zb_print("RX start\r\n");
    }else if(data[1] == 'p' || data[1] == 'P'){
        if(mfg_state != MFG_STATE_ZB_RX_ON){
            return mfg_show_error(MFG_ERR_CMD_UNALLOWED);
        }

        lmac154_disableRx();
        lmac154_disableRxPromiscuousMode();
        mfg_state = MFG_STATE_IDLE;
        mfg_zb_print("RX stop\r\n");

        if(cci_paras_env.rx_frm_cnt > 0){
            cci_paras_env.rssi_avg = (cci_paras_env.rssi_sum / cci_paras_env.rx_frm_cnt);
            cci_paras_env.freq_offset_avg = (cci_paras_env.freq_offset_sum / cci_paras_env.rx_frm_cnt);
            cci_paras_env.lqi_avg = (cci_paras_env.lqi_sum / cci_paras_env.rx_frm_cnt);
        }

        if(all_paras_env.rx_frm_cnt > 0){
            all_paras_env.rssi_avg = (all_paras_env.rssi_sum / all_paras_env.rx_frm_cnt);
            all_paras_env.freq_offset_avg = (all_paras_env.freq_offset_sum / all_paras_env.rx_frm_cnt);
            all_paras_env.lqi_avg = (all_paras_env.lqi_sum / all_paras_env.rx_frm_cnt);
        }

        mfg_zb_print("seq-num-matched rx_frm_cnt:%ld, rssi_avg:%ld, freq_offset_avg:%ld, lqi_avg:%ld\r\n", cci_paras_env.rx_frm_cnt, cci_paras_env.rssi_avg, cci_paras_env.freq_offset_avg, cci_paras_env.lqi_avg);
        mfg_zb_print("total rx_frm_cnt:%ld, rssi_avg:%ld, freq_offset_avg:%ld, lqi_avg:%ld\r\n", all_paras_env.rx_frm_cnt, all_paras_env.rssi_avg, all_paras_env.freq_offset_avg, all_paras_env.lqi_avg);

        for(i=0; i<256; i++){
            if(mfg_seq_num_cnt[i] > max){
                max = mfg_seq_num_cnt[i];
                seqNum = i;
            }
        }

        if(max > 0){
            mfg_zb_print("most matched seq num:%d, cnt:%lu\r\n", seqNum, max);
        }

#if !defined(MFG_ATE)
    }else if(data[1] == 'd'){
        if(mfg_state != MFG_STATE_IDLE){
            return mfg_show_error(MFG_ERR_CMD_UNALLOWED);
        }

        lmac154_disableAutoRxDataRate();
        mfg_zb_print("P_MODE disabled\r\n");
    }else if(data[1] == 'e'){
        if(mfg_state != MFG_STATE_IDLE){
            return mfg_show_error(MFG_ERR_CMD_UNALLOWED);
        }

        lmac154_enableAutoRxDataRate();
        mfg_zb_print("P_MODE enabled\r\n");
#endif
    }else{
        return mfg_show_error(MFG_ERR_CMD_PARAM);
    }

    return 0;
}
