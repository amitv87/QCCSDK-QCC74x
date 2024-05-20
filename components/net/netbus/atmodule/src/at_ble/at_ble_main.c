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

#define CHECK_BLE_SRV_IDX_VALID(idx) \
    if (!at_ble_srv_idx_is_valid(idx)) { \
        AT_BLE_PRINTF("service idx is invalid\r\n"); \
        return 0; \
    }

#define CHECK_BLE_CHAR_IDX_VALID(idx) \
    if (!at_ble_char_idx_is_valid(idx)) { \
        AT_BLE_PRINTF("characteristic idx is invalid\r\n"); \
        return 0; \
    }


static int g_ble_is_inited = 0;
static uint8_t g_ble_public_addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static ble_scan_callback_t g_ble_scan_callback = NULL;
static uint8_t g_ble_selected_id = BT_ID_DEFAULT;
#if defined(CONFIG_BLE_MULTI_ADV)
static int g_ble_adv_id;
#endif

#define MAX_BLE_CONN 3
#define BLE_CONN_STATE_CONNCTING 1
#define BLE_CONN_STATE_CONNECTED 2
#define BLE_CONN_STATE_DISCONNECTED 3

struct ble_conn_data
{
    uint8_t valid;
    uint8_t idx;
    uint8_t addr[6];
    uint8_t addr_type;
    struct bt_conn *conn;
    uint8_t state;
    uint16_t min_interval;
    uint16_t max_interval;
    uint16_t cur_interval;
    uint16_t latency;
    uint16_t timeout;
};

static struct ble_conn_data g_ble_conn_data[MAX_BLE_CONN];

struct ble_char_data
{
    uint8_t valid;
    uint8_t char_uuid[16];
    uint32_t char_prop;
};

struct ble_srv_data
{
    uint8_t valid;
    uint8_t srv_uuid[16];
    uint8_t srv_type;

    struct ble_char_data srv_char[BLE_CHAR_MAX_NUM];
};

static struct ble_srv_data g_ble_srv_data[BLE_SRV_MAX_NUM];


static struct ble_conn_data *ble_conn_data_get_by_idx(int idx)
{
    if (idx < 0 || idx >= MAX_BLE_CONN)
        return NULL;

    if (!g_ble_conn_data[idx].valid)
        return NULL;

    return &g_ble_conn_data[idx];
}

static struct ble_conn_data *ble_conn_data_get_by_conn(struct bt_conn *conn)
{
    int i;

    if (conn == NULL)
        return NULL;

    for (i = 0; i < MAX_BLE_CONN; i++) {
        if (g_ble_conn_data[i].valid && g_ble_conn_data[i].conn == conn) {
            return &g_ble_conn_data[i];
        }
    }

    return NULL;
}

static void ble_conn_data_set(int idx, uint8_t *addr, uint8_t addr_type, struct bt_conn *conn, uint16_t min_interval, uint16_t max_interval, uint8_t state)
{
    if (idx < 0 || idx >= MAX_BLE_CONN)
        return NULL;

    memcpy(g_ble_conn_data[idx].addr, addr, 6);
    g_ble_conn_data[idx].idx = idx;
    g_ble_conn_data[idx].addr_type = addr_type;
    g_ble_conn_data[idx].conn = conn;
    g_ble_conn_data[idx].state = state;
    g_ble_conn_data[idx].min_interval = min_interval;
    g_ble_conn_data[idx].max_interval = max_interval;
    g_ble_conn_data[idx].cur_interval = 0;
    g_ble_conn_data[idx].latency = 0;
    g_ble_conn_data[idx].timeout = 0;
    g_ble_conn_data[idx].valid = 1;
}

static void ble_addr_trans(uint8_t *src, uint8_t *dst)
{
    int i;

    for (i = 0; i < 6; i++) {
        dst[i] = src[5 - i];
    }
}

//07af27a5-9c22-11ea-9afe-02fcdc4e7412
#define BT_UUID_SVC_BLE_TP              BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x07af27a5, 0x9c22, 0x11ea, 0x9afe, 0x02fcdc4e7412))
//07af27a6-9c22-11ea-9afe-02fcdc4e7412
#define BT_UUID_CHAR_BLE_TP_RD      BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x07af27a6, 0x9c22, 0x11ea, 0x9afe, 0x02fcdc4e7412))
//07af27a7-9c22-11ea-9afe-02fcdc4e7412
#define BT_UUID_CHAR_BLE_TP_WR      BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x07af27a7, 0x9c22, 0x11ea, 0x9afe, 0x02fcdc4e7412))
//07af27a8-9c22-11ea-9afe-02fcdc4e7412
#define BT_UUID_CHAR_BLE_TP_IND      BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x07af27a8, 0x9c22, 0x11ea, 0x9afe, 0x02fcdc4e7412))
//07af27a9-9c22-11ea-9afe-02fcdc4e7412
#define BT_UUID_CHAR_BLE_TP_NOT      BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x07af27a9, 0x9c22, 0x11ea, 0x9afe, 0x02fcdc4e7412))

static int ble_tp_recv_rd(struct bt_conn *conn,	const struct bt_gatt_attr *attr,
                                        void *buf, u16_t len, u16_t offset)
{
    return 0;
}

static int ble_tp_recv_wr(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                        const void *buf, u16_t len, u16_t offset, u8_t flags)
{
    return 0;
}

static void ble_tp_ind_ccc_changed(const struct bt_gatt_attr *attr, u16_t value)
{

}

static void ble_tp_not_ccc_changed(const struct bt_gatt_attr *attr, u16_t value)
{
  
}

static struct bt_gatt_service *ble_tp_service[BLE_SRV_MAX_NUM] = {NULL};
static struct bt_gatt_attr *ble_tp_attrs = NULL;

static const struct bt_gatt_attr ble_tp_read[] = {
	BT_GATT_CHARACTERISTIC(BT_UUID_CHAR_BLE_TP_RD,
							BT_GATT_CHRC_READ,
							BT_GATT_PERM_READ,
							ble_tp_recv_rd,
							NULL,
							NULL)
};

static const struct bt_gatt_attr ble_tp_write[] = {
	BT_GATT_CHARACTERISTIC(BT_UUID_CHAR_BLE_TP_WR,
							BT_GATT_CHRC_WRITE,
							BT_GATT_PERM_WRITE,
							NULL,
							ble_tp_recv_wr,
							NULL)
};

static const struct bt_gatt_attr ble_tp_write_no_rsp[] = {
	BT_GATT_CHARACTERISTIC(BT_UUID_CHAR_BLE_TP_WR,
							BT_GATT_CHRC_WRITE_WITHOUT_RESP,
							BT_GATT_PERM_PREPARE_WRITE,
							NULL,
							ble_tp_recv_wr,
							NULL)
};

static const struct bt_gatt_attr ble_tp_notify[] = {
	BT_GATT_CHARACTERISTIC(BT_UUID_CHAR_BLE_TP_NOT,
							BT_GATT_CHRC_NOTIFY,
							0,
							NULL,
							NULL,
							NULL),
							
	BT_GATT_CCC(ble_tp_not_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
};

static const struct bt_gatt_attr ble_tp_indicate[] = {
	BT_GATT_CHARACTERISTIC(BT_UUID_CHAR_BLE_TP_IND,
							BT_GATT_CHRC_INDICATE,
							0,
							NULL,
							NULL,
							NULL),

	BT_GATT_CCC(ble_tp_ind_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
};

static int ble_get_service_attr_num(struct ble_srv_data *data)
{
    int attr_num = 1; //service
    int i;

    for (i = 0; i < BLE_CHAR_MAX_NUM; i++) {
        if (data->srv_char[i].valid == 1) {
            attr_num += 2;

            if ((data->srv_char[i].char_prop & BLE_GATT_CHAR_PROP_NOTIFY) || (data->srv_char[i].char_prop & BLE_GATT_CHAR_PROP_INDICATE)) {
                attr_num += 1; //notify and indicate use 1 more attrs
            }
        }
    }

    return attr_num;
}

static int ble_create_service_main(struct bt_gatt_attr *attr, uint8_t srv_uuid[16], uint8_t srv_type)
{
    struct bt_gatt_attr srv_attr;

    if (srv_type == BLE_GATT_SRV_PRIMARY) {
        srv_attr.uuid = BT_UUID_GATT_PRIMARY;
    } else {
        srv_attr.uuid = BT_UUID_GATT_SECONDARY;
    }
    srv_attr.read = bt_gatt_attr_read_service;
    srv_attr.write = NULL;
    srv_attr.user_data = BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x07af27a5, 0x9c22, 0x11ea, 0x9afe, 0x02fcdc4e7412));
    srv_attr.perm = BT_GATT_PERM_READ;      

    memcpy(attr, &srv_attr , sizeof(struct bt_gatt_attr));
    return 1;
}

static int ble_create_service_char(struct bt_gatt_attr *attr, uint8_t char_uuid[16], uint32_t char_prop)
{
    if (char_prop & BLE_GATT_CHAR_PROP_READ) {
         memcpy(attr, &ble_tp_read, sizeof(ble_tp_read));
         return (sizeof(ble_tp_read)/sizeof(struct bt_gatt_attr));
    }
    if (char_prop & BLE_GATT_CHAR_PROP_WRITE_WITHOUT_RESP) {
         memcpy(attr, &ble_tp_write_no_rsp, sizeof(ble_tp_write_no_rsp));
         return (sizeof(ble_tp_write_no_rsp)/sizeof(struct bt_gatt_attr));
    }
    if (char_prop & BLE_GATT_CHAR_PROP_WRITE) {
         memcpy(attr, &ble_tp_write, sizeof(ble_tp_write));
         return (sizeof(ble_tp_write)/sizeof(struct bt_gatt_attr));
    }
    if (char_prop & BLE_GATT_CHAR_PROP_NOTIFY) {
         memcpy(attr, &ble_tp_notify, sizeof(ble_tp_notify));
         return (sizeof(ble_tp_notify)/sizeof(struct bt_gatt_attr));
    }
    if (char_prop & BLE_GATT_CHAR_PROP_INDICATE) {
         memcpy(attr, &ble_tp_indicate, sizeof(ble_tp_indicate));
         return (sizeof(ble_tp_indicate)/sizeof(struct bt_gatt_attr));
    }

    return 0;
}

static struct bt_gatt_service *ble_create_service(struct ble_srv_data *data)
{
    struct bt_gatt_service *service = NULL;
    struct bt_gatt_attr *attrs = NULL;
    int attrs_num = 0;
    int attrs_create_num = 0;
    int i;
    struct ble_char_data *srv_char = NULL;
    int ret = 0;

    attrs_num = ble_get_service_attr_num(data);
    if (attrs_num <= 1) { //not support no characteristic
        return NULL;
    }

    service = (struct bt_gatt_service *)pvPortMalloc(sizeof(struct bt_gatt_service));
    if (!service) {
        AT_BLE_PRINTF("error, malloc service failed\r\n");
        return NULL;
    }
    memset(service, 0, sizeof(struct bt_gatt_service));

    attrs = (struct bt_gatt_attr *)pvPortMalloc(sizeof(struct bt_gatt_attr) * attrs_num);
    if (!attrs) {
        AT_BLE_PRINTF("error, malloc attrs failed\r\n");
        vPortFree(service);
        return NULL;
    }
    memset(attrs, 0, sizeof(struct bt_gatt_attr) * attrs_num);

    if ((ret = ble_create_service_main(attrs + attrs_create_num, data->srv_uuid, data->srv_type)) > 0) {
        attrs_create_num += ret;
    }
    for (i = 0; i < BLE_CHAR_MAX_NUM; i++) {
        srv_char = &data->srv_char[i];
        if ((ret = ble_create_service_char(attrs + attrs_create_num, srv_char->char_uuid, srv_char->char_prop)) > 0) {
            attrs_create_num += ret;
        }
    }
    AT_BLE_PRINTF("attrs_create_num = %d\r\n", attrs_create_num);
    service->attrs = attrs;
    service->attr_count = attrs_create_num;
    return service;
}
    
static void ble_add_service(void)
{
    struct bt_gatt_service *service = NULL;
    int i;

    for (i = 0; i < BLE_SRV_MAX_NUM; i++) {
        if (g_ble_srv_data[i].valid) {
            service = ble_create_service(&g_ble_srv_data[i]);

            bt_gatt_service_register(service);
            ble_tp_service[i] = service;
        }
    }
}

static void ble_del_service(void)
{
    struct bt_gatt_attr *attrs = NULL;
    int i;

    for (i = 0; i < BLE_SRV_MAX_NUM; i++) {
        if (ble_tp_service[i]) {
            bt_gatt_service_unregister(ble_tp_service[i]);

            attrs = ble_tp_service[i]->attrs;
            if (attrs) {
                vPortFree(attrs);
            }
            vPortFree(ble_tp_service[i]);
            ble_tp_service[i] = NULL;
        }
    }
}

static void ble_connected(struct bt_conn *conn, u8_t err)
{
    if(err || conn->type != BT_CONN_TYPE_LE)
    {
        return;
    }
    AT_BLE_PRINTF("%s conn: 0x%x\r\n",__func__, conn);

    struct ble_conn_data *conn_data = ble_conn_data_get_by_conn(conn);
    if (!conn_data)
        return;
    conn_data->state = BLE_CONN_STATE_CONNECTED;
}

static void ble_disconnected(struct bt_conn *conn, u8_t reason)
{ 
    int ret;

    if(conn->type != BT_CONN_TYPE_LE)
    {
        return;
    }

    AT_BLE_PRINTF("%s conn: 0x%x, reason: %d\r\n",__func__, conn, reason);

    // enable adv
    ret = set_adv_enable(true);
    if(ret) {
        AT_BLE_PRINTF("Restart adv fail. \r\n");
    }

    struct ble_conn_data *conn_data = ble_conn_data_get_by_conn(conn);
    if (!conn_data)
        return;

    conn_data->state = BLE_CONN_STATE_DISCONNECTED;
    at_response_string("+BLEDISCONN:%d,\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n",
            conn_data->idx,
            conn_data->addr[0],
            conn_data->addr[1],
            conn_data->addr[2],
            conn_data->addr[3],
            conn_data->addr[4],
            conn_data->addr[5]);
}

static void ble_le_param_updated(struct bt_conn *conn, u16_t interval, u16_t latency, u16_t timeout)
{
    AT_BLE_PRINTF("LE conn param updated: int 0x%04x lat %d to %d \r\n", interval, latency, timeout);

    struct ble_conn_data *conn_data = ble_conn_data_get_by_conn(conn);
    if (!conn_data)
        return;

    conn_data->cur_interval = interval;
    conn_data->latency = latency;
    conn_data->timeout = timeout;
    at_response_string("+BLECONNPARAM:%d,%d,%d,%d,%d,%d\r\n",
            conn_data->idx,
            conn_data->min_interval,
            conn_data->max_interval,
            conn_data->cur_interval,
            conn_data->latency,
            conn_data->timeout);
}

#if defined(CONFIG_BT_SMP)
static void ble_identity_resolved(struct bt_conn *conn, const bt_addr_le_t *rpa,
			      const bt_addr_le_t *identity)
{
	char addr_identity[BT_ADDR_LE_STR_LEN];
	char addr_rpa[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(identity, addr_identity, sizeof(addr_identity));
	bt_addr_le_to_str(rpa, addr_rpa, sizeof(addr_rpa));

	AT_BLE_PRINTF("Identity resolved %s -> %s \r\n", addr_rpa, addr_identity);
}

static void ble_security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	AT_BLE_PRINTF("Security changed: %s level %u \r\n", addr, level);
}
#endif

static struct bt_conn_cb ble_conn_callbacks = {
	.connected	=   ble_connected,
	.disconnected	=   ble_disconnected,
	.le_param_updated = ble_le_param_updated,
#if defined(CONFIG_BT_SMP)
	.identity_resolved = ble_identity_resolved,
	.security_changed = ble_security_changed,
#endif
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
    uint8_t ble_addr[6];

    if (g_ble_scan_callback)
    {
        ble_addr_trans((uint8_t *)addr->a.val, ble_addr);
        g_ble_scan_callback(addr->type, ble_addr, rssi, evtype, buf->data, buf->len);
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

        ble_add_service();
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
    struct ble_conn_data *conn_data = ble_conn_data_get_by_idx(idx);
    if (!conn_data)
        return 0;

    if (conn_data->state == BLE_CONN_STATE_CONNECTED)
        return 1;
    else
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
    int ret = 0;
    uint64_t start_time = at_current_ms_get();

    ble_addr.type = addr_type; /*Get addr type, 0:ADDR_PUBLIC, 1:ADDR_RAND, 2:ADDR_RPA_OR_PUBLIC, 3:ADDR_RPA_OR_RAND*/
    ble_addr_trans(addr, ble_addr.a.val);

    param.timeout = (timeout * 1000) / 10;

    conn = bt_conn_create_le(&ble_addr, &param);
    if (!conn) {
        AT_BLE_PRINTF("Connection failed\r\n");
    } else {
        ble_conn_data_set(idx, addr, addr_type, conn, param.interval_min, param.interval_max, BLE_CONN_STATE_CONNCTING);

        if(conn->state == BT_CONN_CONNECTED) {
            AT_BLE_PRINTF("Le link with this peer device has existed\r\n");
            return 1;
        } else {
            AT_BLE_PRINTF("Connection pending\r\n");
            while(at_current_ms_get() - start_time < timeout*1000) {
                if (at_ble_is_connected(idx)) {
                     at_response_string("+BLECONN:%d,\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n",
                            idx,
                            addr[0],
                            addr[1],
                            addr[2],
                            addr[3],
                            addr[4],
                            addr[5]);
                    ret = 1;
                    break;
                }

                vTaskDelay(50);
            }
        }
    }

    return ret;
}

int at_ble_conn_get_addr(int idx, uint8_t *addr)
{
    struct ble_conn_data *conn_data = ble_conn_data_get_by_idx(idx);
    if (!conn_data)
        return 0;

     if (conn_data->state != BLE_CONN_STATE_CONNECTED)
        return 0;

     memcpy(addr, conn_data->addr, 6);
     return 1;
}

int at_ble_conn_get_param(int idx, int *min_interval, int *max_interval, int *cur_interval, int *latency, int *timeout)
{
    struct ble_conn_data *conn_data = ble_conn_data_get_by_idx(idx);
    if (!conn_data)
        return 0;

     if (conn_data->state != BLE_CONN_STATE_CONNECTED)
        return 0;

     *min_interval = conn_data->min_interval;
     *max_interval = conn_data->max_interval;
     *cur_interval = conn_data->cur_interval;
     *latency = conn_data->latency;
     *timeout = conn_data->timeout;
     return 1;
}

int at_ble_conn_update_param(int idx, int min_interval, int max_interval, int latency, int timeout)
{
    struct bt_le_conn_param param;
    int err = 0;
    struct ble_conn_data *conn_data = ble_conn_data_get_by_idx(idx);
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

    struct ble_conn_data *conn_data = ble_conn_data_get_by_idx(idx);
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
    struct ble_conn_data *conn_data = ble_conn_data_get_by_idx(idx);
    if (!conn_data)
        return 0;

    *mtu_size = bt_gatt_get_mtu(conn_data->conn);
    return 1;
}

int at_ble_conn_update_mtu(int idx, int mtu_size)
{
    struct ble_conn_data *conn_data = ble_conn_data_get_by_idx(idx);
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

static int at_ble_srv_idx_is_valid(int idx)
{
    if (idx < 0 || idx >= BLE_SRV_MAX_NUM)
        return 0;
    else
        return 1;
}

static int at_ble_char_idx_is_valid(int idx)
{
    if (idx < 0 || idx >= BLE_CHAR_MAX_NUM)
        return 0;
    else
        return 1;
}

int at_ble_gatts_service_get(int srv_idx, uint8_t *srv_uuid, uint8_t *srv_type)
{
    CHECK_BLE_SRV_IDX_VALID(srv_idx);

    if (g_ble_srv_data[srv_idx].valid == 0)
        return 0;

    memcpy(srv_uuid, g_ble_srv_data[srv_idx].srv_uuid, 16);
    *srv_type = g_ble_srv_data[srv_idx].srv_type;
    return 1;
}

int at_ble_gatts_service_set(int srv_idx, uint8_t *srv_uuid, uint8_t srv_type)
{
    CHECK_BLE_SRV_IDX_VALID(srv_idx);

    if (g_ble_srv_data[srv_idx].valid == 1)
        return 0;

    g_ble_srv_data[srv_idx].valid = 1;
    memcpy(g_ble_srv_data[srv_idx].srv_uuid, srv_uuid, 16);
    g_ble_srv_data[srv_idx].srv_type = srv_type;
    return 1;
}

int at_ble_gatts_service_del(int srv_idx)
{
    int i;

    CHECK_BLE_SRV_IDX_VALID(srv_idx);

    if (g_ble_srv_data[srv_idx].valid == 0)
        return 0;

    g_ble_srv_data[srv_idx].valid = 0;
    memset(g_ble_srv_data[srv_idx].srv_uuid, 0, sizeof(g_ble_srv_data[srv_idx].srv_uuid));
    g_ble_srv_data[srv_idx].srv_type = 0;
    for (i = 0; i < BLE_CHAR_MAX_NUM; i++) {
        g_ble_srv_data[srv_idx].srv_char[i].valid = 0;
        memset(g_ble_srv_data[srv_idx].srv_char[i].char_uuid, 0, sizeof(g_ble_srv_data[srv_idx].srv_char[i].char_uuid));
        g_ble_srv_data[srv_idx].srv_char[i].char_prop = 0;
    }

    return 1;
}

int at_ble_gatts_service_char_get(int srv_idx, int char_idx, uint8_t *char_uuid, uint32_t *char_prop)
{
    CHECK_BLE_SRV_IDX_VALID(srv_idx);
    CHECK_BLE_CHAR_IDX_VALID(char_idx);

    if (g_ble_srv_data[srv_idx].valid == 0)
        return 0;
    if (g_ble_srv_data[srv_idx].srv_char[char_idx].valid == 0)
        return 0;

    memcpy(char_uuid, g_ble_srv_data[srv_idx].srv_char[char_idx].char_uuid, 16);
    *char_prop = g_ble_srv_data[srv_idx].srv_char[char_idx].char_prop;
    return 1;
}

int at_ble_gatts_service_char_set(int srv_idx, int char_idx, uint8_t *char_uuid, uint32_t char_prop)
{
    CHECK_BLE_SRV_IDX_VALID(srv_idx);
    CHECK_BLE_CHAR_IDX_VALID(char_idx);

    if (g_ble_srv_data[srv_idx].valid == 0)
        return 0;
    if (g_ble_srv_data[srv_idx].srv_char[char_idx].valid == 1)
        return 0;

    g_ble_srv_data[srv_idx].srv_char[char_idx].valid = 1;
    memcpy(g_ble_srv_data[srv_idx].srv_char[char_idx].char_uuid, char_uuid, 16);
    g_ble_srv_data[srv_idx].srv_char[char_idx].char_prop = char_prop;
    return 1;
}

int at_ble_gatts_service_notify(int srv_idx, int char_idx, void * buffer, int length)
{
    CHECK_BLE_SRV_IDX_VALID(srv_idx);
    CHECK_BLE_CHAR_IDX_VALID(char_idx);

    if (g_ble_srv_data[srv_idx].valid == 0)
        return 0;
    if (g_ble_srv_data[srv_idx].srv_char[char_idx].valid == 0)
        return 0;

    if (g_ble_srv_data[srv_idx].srv_char[char_idx].char_prop & BLE_GATT_CHAR_PROP_NOTIFY)
    {
        //TODO
        return 1;
    }
    else
        return 0;
}

int at_ble_gatts_service_indicate(int srv_idx, int char_idx, void * buffer, int length)
{
    CHECK_BLE_SRV_IDX_VALID(srv_idx);
    CHECK_BLE_CHAR_IDX_VALID(char_idx);

    if (g_ble_srv_data[srv_idx].valid == 0)
        return 0;
    if (g_ble_srv_data[srv_idx].srv_char[char_idx].valid == 0)
        return 0;

    if (g_ble_srv_data[srv_idx].srv_char[char_idx].char_prop & BLE_GATT_CHAR_PROP_INDICATE)
    {
        //TODO
        return 1;
    }
    else
        return 0;
}

int at_ble_start(void)
{
    return 0;
}

int at_ble_stop(void)
{
    return 0;
}

