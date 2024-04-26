/**
  ******************************************************************************
  * @file    at_ble_main.c
  * @version V1.0
  * @date
  * @brief   This file is part of AT command framework
  ******************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "conn.h"
#include "conn_internal.h"
#include "gatt.h"
#include "hci_core.h"
#if defined(QCC74x_undef) || defined(QCC74x_undef)
#include "ble_lib_api.h"
#else
#include "btble_lib_api.h"
#endif
#include "l2cap_internal.h"
#if defined(CONFIG_BLE_MULTI_ADV)
#include "multi_adv.h"
#endif
#include "bluetooth.h"
#include "hci_driver.h"

#include "at_main.h"
#include "at_core.h"
#include "at_port.h"
#include "at_base_config.h"
#include "at_ble_config.h"
#include "at_ble_main.h"

#define AT_BLE_PRINTF              printf


static int g_ble_is_inited = 0;
static uint8_t g_ble_public_addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static ble_scan_callback_t g_ble_scan_callback = NULL;
static uint8_t g_ble_selected_id = BT_ID_DEFAULT;
#if defined(CONFIG_BLE_MULTI_ADV)
static int g_ble_adv_id;
#endif

#define MAX_BLE_CONN 3

struct ble_conn_data
{
    uint8_t valid;
    uint8_t addr[6];
    uint8_t addr_type;
    struct bt_conn *conn;
};

static struct ble_conn_data g_ble_conn_data[MAX_BLE_CONN];

static struct ble_conn_data *get_ble_conn_data(int idx)
{
    if (idx < 0 || idx >= MAX_BLE_CONN)
        return NULL;

    if (!g_ble_conn_data[idx].valid)
        return NULL;

    return &g_ble_conn_data[idx];
}


static void ble_connected(struct bt_conn *conn, u8_t err)
{
    if(err || conn->type != BT_CONN_TYPE_LE)
    {
        return;
    }
    AT_BLE_PRINTF("%s",__func__);
}

static void ble_disconnected(struct bt_conn *conn, u8_t reason)
{ 
    int ret;

    if(conn->type != BT_CONN_TYPE_LE)
    {
        return;
    }

    AT_BLE_PRINTF("%s",__func__);

    // enable adv
    ret = set_adv_enable(true);
    if(ret) {
        AT_BLE_PRINTF("Restart adv fail. \r\n");
    }
}

static struct bt_conn_cb ble_conn_callbacks = {
	.connected	=   ble_connected,
	.disconnected	=   ble_disconnected,
};

void bt_enable_cb(int err)
{
    if (!err) {
        bt_addr_le_t bt_addr;
        bt_get_local_public_address(&bt_addr);
        AT_BLE_PRINTF("BD_ADDR:(MSB)%02x:%02x:%02x:%02x:%02x:%02x(LSB) \r\n",
               bt_addr.a.val[5], bt_addr.a.val[4], bt_addr.a.val[3], bt_addr.a.val[2], bt_addr.a.val[1], bt_addr.a.val[0]);
        bt_conn_cb_register(&ble_conn_callbacks);
    }
}

int at_ble_init(int role)
{
    (void)role;

    if (g_ble_is_inited == 0) {
        // Initialize BLE controller
#if defined(QCC74x_undef) || defined(QCC74x_undef)
        ble_controller_init(configMAX_PRIORITIES - 1);
#else
        btble_controller_init(configMAX_PRIORITIES - 1);
#endif
        // Initialize BLE Host stack
        hci_driver_init();
        bt_enable(bt_enable_cb);

        g_ble_is_inited = 1;
    } else {
        return -1;
    }

    return 0;
}

int at_ble_set_public_addr(uint8_t *addr)
{
    memcpy(g_ble_public_addr, addr, 6);
    return 0;
}

int at_ble_get_public_addr(uint8_t *addr)
{
    memcpy(addr, g_ble_public_addr, 6);
    return 0;
}

int at_ble_set_random_addr(uint8_t *addr)
{
    //do nothing, default is random address
    return 0;
}

static void ble_device_found(const bt_addr_le_t *addr, s8_t rssi, u8_t evtype, struct net_buf_simple *buf)
{
    if (g_ble_scan_callback)
    {
        g_ble_scan_callback(addr->type, (uint8_t *)addr->a.val, rssi, buf->data, buf->len);
    }
}

int  at_ble_scan_start(ble_scan_callback_t cb)
{
    struct bt_le_scan_param scan_param;
    int err;

    scan_param.type = at_ble_config->scan_param.scan_type;
    scan_param.filter_dup = 0;
    scan_param.interval = at_ble_config->scan_param.scan_interval;
    scan_param.window = at_ble_config->scan_param.scan_window;

    g_ble_scan_callback = cb;
    err = bt_le_scan_start(&scan_param, ble_device_found);
    if(err){
        AT_BLE_PRINTF("ble start scan failed, err = %d\r\n", err);
        return -1;
    }else{
        AT_BLE_PRINTF("ble start scan success\r\n");
        return 0;
    }
}

int at_ble_scan_stop(void)
{
    int err;

    g_ble_scan_callback = NULL;
    err = bt_le_scan_stop();
    if (err) {
        AT_BLE_PRINTF("ble stop scan failed, err = %d\r\n", err);
        return -1;
    } else {
        AT_BLE_PRINTF("ble stop scan success\r\n");
        return 0;
    }
}

int at_ble_adv_start(void)
{
    struct bt_le_adv_param param;
    const struct bt_data *ad;
    size_t ad_len;
    int err = -1;
    /*Get adv type, 0:adv_ind,  1:adv_scan_ind, 2:adv_nonconn_ind 3: adv_direct_ind*/
    uint8_t adv_type;
    /*Get mode, 0:General discoverable,  1:limit discoverable, 2:non discoverable*/
    uint8_t mode;
    char *adv_name = (char*)at_ble_config->ble_name;

    uint8_t non_disc = BT_LE_AD_NO_BREDR;
    uint8_t gen_disc = BT_LE_AD_NO_BREDR | BT_LE_AD_GENERAL;
    uint8_t lim_disc = BT_LE_AD_NO_BREDR | BT_LE_AD_LIMITED;
    struct bt_data ad_discov[2] = {
        BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR | BT_LE_AD_GENERAL),
        BT_DATA(BT_DATA_NAME_COMPLETE, adv_name, strlen(adv_name)),
    };

    adv_type = at_ble_config->adv_param.adv_type;
    mode = 0;
 
    param.id = g_ble_selected_id;
    if(adv_type == 0){
        param.options = (BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME | BT_LE_ADV_OPT_ONE_TIME);
    }else if(adv_type == 1){
        param.options = BT_LE_ADV_OPT_USE_NAME;
    }else if(adv_type == 2){
        param.options = 0;
    }else{
        AT_BLE_PRINTF("Arg1 is invalid\r\n");
        return -1;
    }
    param.interval_min = at_ble_config->adv_param.adv_int_min;
    param.interval_max = at_ble_config->adv_param.adv_int_max;
    
    if (mode == 0) {
        ad_discov[0].data = &gen_disc;
    } else if (mode == 1) {
        ad_discov[0].data = &lim_disc;
    } else if (mode == 2) {
        ad_discov[0].data = &non_disc;
    } else {
        AT_BLE_PRINTF("Invalied AD Mode 0x%x\r\n",mode);
        return -1;
    }

    ad = ad_discov;
    ad_len = ARRAY_SIZE(ad_discov);
	
    if(adv_type == 1 || adv_type == 0){
#if defined(CONFIG_BLE_MULTI_ADV)
        if(ble_adv_id == 0){
            err = bt_le_multi_adv_start(&param, ad, ad_len, &ad_discov[0], 1, &g_ble_adv_id);
        }        
#else
        err = bt_le_adv_start(&param, ad, ad_len, &ad_discov[0], 1);
        #endif/*CONFIG_BLE_MULTI_ADV*/
    }else{
#if defined(CONFIG_BLE_MULTI_ADV)
        if(ble_adv_id == 0){
            err = bt_le_multi_adv_start(&param, ad, ad_len, NULL, 0, &g_ble_adv_id);
        }
#else
        err = bt_le_adv_start(&param, ad, ad_len, NULL, 0);
#endif
    }

    if (err) {
        AT_BLE_PRINTF("ble start advertise failed, err = %d\r\n", err);
        return -1;
    } else {
        AT_BLE_PRINTF("ble start advertise success\r\n");
        return 0;
    }
}

int at_ble_adv_stop(void)
{
#if defined(CONFIG_BLE_MULTI_ADV)
    bool err = -1;
    if(g_ble_adv_id && !bt_le_multi_adv_stop(g_ble_adv_id)){
        ble_adv_id = 0;
        err = 0;
    }
    if(err){
#else
    if (bt_le_adv_stop()) {
#endif
        AT_BLE_PRINTF("ble stop advertise failed\r\n");
        return -1;	
    } else { 
        AT_BLE_PRINTF("ble stop advertise success\r\n");
        return 0;
    }
}

int at_ble_is_connected(int idx)
{
    return 0;
}

int at_ble_conn(int idx, uint8_t *addr, int addr_type, int timeout)
{
    bt_addr_le_t ble_addr;
    struct bt_conn *conn;
    struct bt_le_conn_param param = {
        .interval_min =  BT_GAP_INIT_CONN_INT_MIN,
        .interval_max =  BT_GAP_INIT_CONN_INT_MAX,
        .latency = 0,
        .timeout = 400,
    };

    ble_addr.type = addr_type; /*Get addr type, 0:ADDR_PUBLIC, 1:ADDR_RAND, 2:ADDR_RPA_OR_PUBLIC, 3:ADDR_RPA_OR_RAND*/
    memcpy(ble_addr.a.val, addr, 6);

    param.timeout = (timeout * 1000) / 10;

    conn = bt_conn_create_le(&ble_addr, &param);
    if (!conn) {
        AT_BLE_PRINTF("Connection failed\r\n");
    } else {
        if(conn->state == BT_CONN_CONNECTED) {
            AT_BLE_PRINTF("Le link with this peer device has existed\r\n");
            return 1;
        } else {
            AT_BLE_PRINTF("Connection pending\r\n");
        }
    }

    return 0;
}

int at_ble_conn_get_param(int idx, int *min_interval, int *max_interval, int *cur_interval, int *latency, int *timeout)
{
    return 0;
}

int at_ble_conn_update_param(int idx, int min_interval, int max_interval, int latency, int timeout)
{
    struct bt_le_conn_param param;
    int err = 0;
    struct ble_conn_data *conn_data = get_ble_conn_data(idx);
    if (!conn_data)
        return 0;

    param.interval_min = min_interval;
    param.interval_max = max_interval;
    param.latency = latency;
    param.timeout = timeout;
    
    err = bt_conn_le_param_update(conn_data->conn, &param);
    if (err) {
        AT_BLE_PRINTF("conn update failed (err %d)\r\n", err);
        return 0;
    } else {
        AT_BLE_PRINTF("conn update initiated\r\n");
        return 1;
    }
}

int at_ble_disconn(int idx)
{
    bt_addr_le_t ble_addr;
    struct bt_conn *conn;

    struct ble_conn_data *conn_data = get_ble_conn_data(idx);
    if (!conn_data)
        return 0;
    
    ble_addr.type = conn_data->addr_type;
    memcpy(ble_addr.a.val, conn_data->addr, 6);

    conn = bt_conn_lookup_addr_le(idx, &ble_addr);

    if(!conn){
        AT_BLE_PRINTF("Not connected\r\n");
        return 0;
    }

    if (bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN)) {
        AT_BLE_PRINTF("Disconnection failed\r\n");
    } else {
        AT_BLE_PRINTF("Disconnect successfully\r\n");
    }

    /*Notice:Because conn is got via bt_conn_lookup_addr_le in which bt_conn_ref(increase conn_ref by 1)
      this conn, we need to bt_conn_unref(decrease conn_ref by 1) this conn.*/
    bt_conn_unref(conn);

    return 1;
}

int at_ble_conn_update_datalen(int idx, int data_len)
{
    return 0;
}

static void exchange_func(struct bt_conn *conn, u8_t err,
			  struct bt_gatt_exchange_params *params)
{
	AT_BLE_PRINTF("Exchange %s MTU Size =%d \r\n", err == 0U ? "successful" : "failed", bt_gatt_get_mtu(conn));
}

static struct bt_gatt_exchange_params exchange_params;

int at_ble_conn_get_mtu(int idx, int *mtu_size)
{
    struct ble_conn_data *conn_data = get_ble_conn_data(idx);
    if (!conn_data)
        return 0;

    *mtu_size = bt_gatt_get_mtu(conn_data->conn);
    return 1;
}

int at_ble_conn_update_mtu(int idx, int mtu_size)
{
    struct ble_conn_data *conn_data = get_ble_conn_data(idx);
    if (!conn_data)
        return 0;
    int err;

    exchange_params.func = exchange_func;
    err = bt_gatt_exchange_mtu(conn_data->conn, &exchange_params);
    if (err) {
        AT_BLE_PRINTF("Exchange failed (err %d)\r\n", err);
        return 0;
    } else {
        AT_BLE_PRINTF("Exchange pending\r\n");
        return 1;
    }
}

int at_ble_start(void)
{
    return 0;
}

int at_ble_stop(void)
{
    return 0;
}

