/**
  ******************************************************************************
  * @file    at_ble_main.h
  * @version V1.0
  * @date
  * @brief   This file is part of AT command framework
  ******************************************************************************
  */

#ifndef AT_BLE_MAIN_H
#define AT_BLE_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ble_scan_callback_t)(uint8_t addr_type, uint8_t *addr, int8_t rssi, uint8_t evtype, uint8_t *data, int len);

int at_ble_start(void);

int at_ble_stop(void);

int at_ble_init(int role);

int at_ble_set_public_addr(uint8_t *addr);

int at_ble_get_public_addr(uint8_t *addr);

int at_ble_set_random_addr(uint8_t *addr);

int at_ble_scan_start(ble_scan_callback_t cb);

int at_ble_scan_stop(void);

int at_ble_adv_start(void);

int at_ble_adv_stop(void);

int at_ble_is_connected(int idx);

int at_ble_conn(int idx, uint8_t *addr, int addr_type, int timeout);

int at_ble_conn_get_addr(int idx, uint8_t *addr);

int at_ble_conn_get_param(int idx, int *min_interval, int *max_interval, int *cur_interval, int *latency, int *timeout);

int at_ble_conn_update_param(int idx, int min_interval, int max_interval, int latency, int timeout);

int at_ble_disconn(int idx);

int at_ble_conn_update_datalen(int idx, int data_len);

int at_ble_conn_get_mtu(int idx, int *mtu_size);

int at_ble_conn_update_mtu(int idx, int mtu_size);

int at_ble_gatts_service_get(int srv_idx, uint8_t *srv_uuid, uint8_t *srv_type);

int at_ble_gatts_service_set(int srv_idx, uint8_t *srv_uuid, uint8_t srv_type);

int at_ble_gatts_service_del(int srv_idx);

int at_ble_gatts_service_char_get(int srv_idx, int char_idx, uint8_t *char_uuid, uint32_t *char_prop);

int at_ble_gatts_service_char_set(int srv_idx, int char_idx, uint8_t *char_uuid, uint32_t char_prop);

int at_ble_gatts_service_notify(int srv_idx, int char_idx, void * buffer, int length);

int at_ble_gatts_service_indicate(int srv_idx, int char_idx, void * buffer, int length);

#ifdef __cplusplus
}
#endif

#endif/* AT_BLE_MAIN_H */

