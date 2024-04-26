#ifndef _MFG_M154_H_
#define _MFG_M154_H_


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


#define MFG_ERR_CMD_LEN                     -1
#define MFG_ERR_CMD_FORMAT                  -2
#define MFG_ERR_CMD_PARAM                   -3
#define MFG_ERR_CMD_UNALLOWED               -4
#define MFG_ERR_CMD_FAILED                  -5


// CCI Data
typedef struct __cci_paras_ram_t {
    int32_t rx_frm_cnt;
    int32_t rssi_avg;
    int32_t freq_offset_avg;
    int32_t lqi_avg;
    int32_t rssi_sum;
    int32_t freq_offset_sum;
    int32_t lqi_sum;
}cci_paras_ram_t;


// Public Function
void m154_init(void);
void m154_exit(void);

int32_t m154_cmd_channel_switch(uint8_t *data, uint16_t len);
int32_t m154_cmd_frequency_ctrl(uint8_t *data, uint16_t len);
int32_t m154_cmd_set_seq_num(uint8_t *data, uint16_t len);
int32_t m154_cmd_tx_toggle(uint8_t *data, uint16_t len);
int32_t m154_cmd_rx_ctrl(uint8_t *data, uint16_t len);


#endif
