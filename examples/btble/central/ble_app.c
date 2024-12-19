#include <stdint.h>
#include "btble_lib_api.h"
#include "bluetooth.h"
#include "conn.h"
#include "conn_internal.h"
#include "hci_driver.h"
#include "hci_core.h"
#include "gatt.h"
#include "uuid.h"
#include "bt_log.h"
#if defined(CONFIG_BT_STACK_CLI) 
#include "ble_cli_cmds.h"
#endif
#include "ble_app.h"

typedef struct{
    TaskHandle_t handle;
    uint32_t wr_hdl;
}wr_arg_t;

typedef enum{
    MSG_START_SCAN,
    MSG_CREATE_CONNECTION,
    MSG_SECURITY_START,
    MSG_GATT_DISCOVER_START,
    MSG_GATT_SUBSCRIBE,
}bleapp_msg_type;

typedef struct{
    uint8_t level;
}bleapp_sec_param;

typedef struct{
    struct bt_uuid *uuid;
    uint8_t discover_type;
    uint16_t start_handle;
    uint16_t end_handle;
}bleapp_discover_param;

typedef struct{
    uint16_t value_handle;
    uint16_t ccc_handle;
    uint8_t value;
}bleapp_subscribe_param;

typedef struct{
    bt_addr_le_t peer_addr;
}bleapp_create_conn_param;

typedef struct{
    bleapp_msg_type type;
    union{
        bleapp_create_conn_param create_conn;
        bleapp_sec_param sec;
        bleapp_discover_param discover;
        bleapp_subscribe_param subscribe;
    }param;
}bleapp_msg;

typedef struct{
    uint16_t start_handle;
    uint16_t end_handle;
}bleapp_gatt_service;

typedef struct{
    struct bt_uuid *uuid;
    uint16_t handle;
    uint16_t value_handle;
    uint8_t properties;
    sys_snode_t node;
}bleapp_gatt_characteristic;

#define BT_UUID_SVC_BLE_TP     BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x07af27a5, 0x9c22, 0x11ea, 0x9afe, 0x02fcdc4e7412))
#define BT_UUID_CHAR_BLE_TP_WR      BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x07af27a7, 0x9c22, 0x11ea, 0x9afe, 0x02fcdc4e7412))

#define BLE_GATT_INVALID_HANDLE 0xffff
#define BLE_GATT_SUBSCRIBE_PARAM_NUM 10

//#define BLE_SMP_TEST
#if defined(CONFIG_BT_ECC)
#define BLE_SEC_LEVEL 4
#else
#define BLE_SEC_LEVEL 2
#endif

static wr_arg_t wr_arg;
struct bt_uuid *bt_ccc_uuid = BT_UUID_GATT_CCC;
static struct k_fifo ble_central_queue;
static struct k_thread ble_central_task_hdl;
static struct bt_gatt_discover_params bleapp_gatt_discover_params;
static struct bt_gatt_subscribe_params subscribe_params_array[BLE_GATT_SUBSCRIBE_PARAM_NUM];
bleapp_gatt_characteristic *ongoing_characteristic = NULL;
bleapp_gatt_service ongoing_gatt_service;
sys_slist_t *characteristics_list = NULL;
struct bt_conn *bleapp_default_conn = NULL;
bool continue_discover_descriptor = false;

void ble_write_task_set(bool enable, uint16_t handle);
static void bleapp_scan_cb(const bt_addr_le_t *addr, s8_t rssi, u8_t evtype, struct net_buf_simple *buf);
static void bleapp_gatt_discovery(uint8_t type, struct bt_uuid *uuid, uint16_t start_handle, uint16_t end_handle);
#if (defined(BLE_SMP_TEST) && defined(CONFIG_BT_SMP))
static void bleapp_start_security(uint8_t sec_level);
#endif
static void bleapp_start_scan(void);

static void bleapp_connected(struct bt_conn *conn, u8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err) {
        printf("Failed to connect to %s (%u) \r\n", addr, err);
        return;
    }

    printf("Connected: %s, int 0x%04x lat %d to %d\r\n", addr, conn->le.interval, conn->le.latency, conn->le.timeout);

    bleapp_default_conn = conn;

    #if (defined(BLE_SMP_TEST) && defined(CONFIG_BT_SMP))
    bleapp_start_security(BLE_SEC_LEVEL);
    #else
    bleapp_gatt_discovery(BT_GATT_DISCOVER_PRIMARY, BT_UUID_SVC_BLE_TP, 1, 0xffff);
    #endif
}

static void bleapp_disconnected(struct bt_conn *conn, u8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    printf("Disconnected: %s (reason %u) \r\n", addr, reason);

    if (bleapp_default_conn == conn) {
        bleapp_default_conn = NULL;
    }

    ongoing_characteristic = NULL;
    ongoing_gatt_service.start_handle = BLE_GATT_INVALID_HANDLE;
    ongoing_gatt_service.end_handle = BLE_GATT_INVALID_HANDLE;

    if(!bt_addr_le_is_bonded(conn->id, &conn->le.dst))
        memset(&subscribe_params_array[0], 0, sizeof(subscribe_params_array));
    
    if(characteristics_list){
        sys_snode_t *node = sys_slist_peek_head(characteristics_list);
        while(node){
            bleapp_gatt_characteristic *next_characteristic = CONTAINER_OF(node, bleapp_gatt_characteristic, node);
            if(next_characteristic->uuid)
                k_free(next_characteristic->uuid);
            node = sys_slist_peek_next(node);
            k_free(next_characteristic);
        }

        k_free(characteristics_list);
        characteristics_list = NULL;
    } 
    ble_write_task_set(false, 0);
    bleapp_start_scan();
}

#if (defined(BLE_SMP_TEST) && defined(CONFIG_BT_SMP))
static void bleapp_auth_cancel(struct bt_conn *conn)
{ 
	if (bleapp_default_conn) {
		conn = bleapp_default_conn;
	}else {
		conn = NULL;
	}

	if (!conn) {
        printf("Not connected\r\n");
		return;
	}

	bt_conn_auth_cancel(conn);
}

static void bleapp_auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));    

    printf("passkey_str is: %06u\r\n", passkey);
}

static void bleapp_auth_passkey_entry(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printf("Enter passkey for %s\r\n", addr);
}

static void  bleapp_auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printf("Confirm passkey for %s: %06u\r\n", addr, passkey);

    bt_conn_auth_passkey_confirm(bleapp_default_conn);
}

static void bleapp_auth_pairing_confirm(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printf("Confirm pairing for %s\r\n", addr);
         bt_conn_auth_pairing_confirm(conn);

    bt_conn_auth_pairing_confirm(bleapp_default_conn);
}

static void bleapp_auth_pairing_complete(struct bt_conn *conn, bool bonded)
{
    printf("auth_pairing_complete\r\n");
}

static void bleapp_auth_pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    printf("Pairing failed with %s\r\n", addr);
}

static void bleapp_identity_resolved(struct bt_conn *conn, const bt_addr_le_t *rpa, const bt_addr_le_t *identity)
{
    char addr_identity[BT_ADDR_LE_STR_LEN];
    char addr_rpa[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(identity, addr_identity, sizeof(addr_identity));
    bt_addr_le_to_str(rpa, addr_rpa, sizeof(addr_rpa));

    printf("Identity resolved %s -> %s \r\n", addr_rpa, addr_identity);

}

static void bleapp_security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    printf("Security changed: %s level %u err %u\r\n", addr, level, err);
    if(err == BT_SECURITY_ERR_PIN_OR_KEY_MISSING){
        bleapp_start_security(BLE_SEC_LEVEL);
    }
    else if(err == 0){
        //for the case that peer device is previouse bonded and then reconnected
        bleapp_gatt_discovery(BT_GATT_DISCOVER_PRIMARY, BT_UUID_SVC_BLE_TP, 1, 0xffff);    
    }
}

static struct bt_conn_auth_cb bleapp_auth_cb_display = {
    .passkey_display = bleapp_auth_passkey_display,
    .passkey_entry = bleapp_auth_passkey_entry,
    .passkey_confirm = bleapp_auth_passkey_confirm,
    .cancel = bleapp_auth_cancel,
    .pairing_confirm = bleapp_auth_pairing_confirm,
    .pairing_failed = bleapp_auth_pairing_failed,
    .pairing_complete = bleapp_auth_pairing_complete,
};
#endif

static void bleapp_conn_param_updated(struct bt_conn *conn, u16_t interval, u16_t latency, u16_t timeout)
{
    if(conn == bleapp_default_conn)
    {
        printf("%s: int 0x%04x lat %d to %d \r\n", __func__, interval, latency, timeout);
    }
}

static struct bt_conn_cb bleapp_conn_callbacks = {
    .connected = bleapp_connected,
    .disconnected = bleapp_disconnected,
    .le_param_updated = bleapp_conn_param_updated,
     #if (defined(BLE_SMP_TEST) && defined(CONFIG_BT_SMP))
    .identity_resolved = bleapp_identity_resolved,
    .security_changed = bleapp_security_changed,
     #endif
};

static struct bt_uuid *bleapp_gatt_allocate_uuid(struct bt_uuid *uuid)
{
    struct bt_uuid *allocate_uuid = NULL;
    uint8_t uuid_len = 0;
    
    if(uuid == NULL){
        printf("uuid is null\r\n");
        return NULL;
    }
    
    if(uuid->type == BT_UUID_TYPE_16){
        allocate_uuid = k_malloc(sizeof(struct bt_uuid_16));
        uuid_len = sizeof(struct bt_uuid_16);
    }else if(uuid->type == BT_UUID_TYPE_32){
        allocate_uuid = k_malloc(sizeof(struct bt_uuid_32));
        uuid_len = sizeof(struct bt_uuid_32);
    }else if(uuid->type == BT_UUID_TYPE_128){
        allocate_uuid = k_malloc(sizeof(struct bt_uuid_128));
        uuid_len = sizeof(struct bt_uuid_128);
    }

    if(allocate_uuid){
        memcpy(allocate_uuid, uuid, uuid_len);    
    }

    return allocate_uuid;
}

struct bt_gatt_subscribe_params *bleapp_gatt_allocate_subscribe_params(uint16_t ccc_handle)
{
    for(int i = 0; i < BLE_GATT_SUBSCRIBE_PARAM_NUM; i++){
        if(subscribe_params_array[i].ccc_handle == ccc_handle)
            return &subscribe_params_array[i];
    }

    for(int i = 0; i < BLE_GATT_SUBSCRIBE_PARAM_NUM; i++){
        if(subscribe_params_array[i].ccc_handle == 0)
            return &subscribe_params_array[i];
    }

    return NULL;
}

bool bleapp_gatt_subscribed(uint16_t ccc_handle)
{
    for(int i = 0; i < BLE_GATT_SUBSCRIBE_PARAM_NUM; i++){
        if(subscribe_params_array[i].ccc_handle == ccc_handle)
            return true;
    }

    return false;
}

static void bleapp_gatt_subscribe(uint16_t value_handle, uint16_t ccc_handle, uint16_t val)
{
    printf("%s\r\n", __func__);
    bleapp_msg *msg = k_malloc(sizeof(bleapp_msg));
    if(msg == NULL){
        printf("Allocate msg failed\r\n");
        return;
    }
    msg->type = MSG_GATT_SUBSCRIBE;
    msg->param.subscribe.value_handle = value_handle;
    msg->param.subscribe.ccc_handle = ccc_handle;
    msg->param.subscribe.value = val;

    k_fifo_put(&ble_central_queue, (void *)msg);
}

static void bleapp_gatt_print_chrc_props(u8_t properties)
{
    printf("Properties: ");

    if (properties & BT_GATT_CHRC_BROADCAST) {
        printf("[bcast]\r\n");
    }
    if (properties & BT_GATT_CHRC_READ) {
        printf("[read]\r\n");
    }
    if (properties & BT_GATT_CHRC_WRITE) {
        printf("[write]\r\n");
    }
    if (properties & BT_GATT_CHRC_WRITE_WITHOUT_RESP) {
        printf("[write w/w rsp]\r\n");
    }
    if (properties & BT_GATT_CHRC_NOTIFY) {
        printf("[notify]\r\n");
    }
    if (properties & BT_GATT_CHRC_INDICATE) {
        printf("[indicate]\r\n");
    }
    if (properties & BT_GATT_CHRC_AUTH) {
        printf("[auth]\r\n");
    }
    if (properties & BT_GATT_CHRC_EXT_PROP) {
        printf("[ext prop]\r\n");
    }
}

void bleapp_gatt_discovery_descriptor(void)
{
    sys_snode_t *node;
    uint16_t start_handle = BLE_GATT_INVALID_HANDLE;
    uint16_t end_handle = BLE_GATT_INVALID_HANDLE;
    if(characteristics_list == NULL)
        return;
    if(ongoing_characteristic == NULL){
        node = sys_slist_peek_head(characteristics_list);
        ongoing_characteristic =  CONTAINER_OF(node, bleapp_gatt_characteristic, node);
    }else{
        node = sys_slist_peek_next(&ongoing_characteristic->node);
        if(node == NULL)
        {
            printf("Discovery completed\r\n");
            ongoing_characteristic = NULL;
            return;
        }
        else
            ongoing_characteristic = CONTAINER_OF(node, bleapp_gatt_characteristic, node);    
    }
    node = sys_slist_peek_next(&ongoing_characteristic->node);
    while(node){
        bleapp_gatt_characteristic *next_characteristic = CONTAINER_OF(node, bleapp_gatt_characteristic, node);
        //printf("value_handle=%u, nexthandl=%u\r\n",ongoing_characteristic->value_handle, next_characteristic->handle -1);
        if(ongoing_characteristic->value_handle < next_characteristic->handle -1){
            start_handle = ongoing_characteristic->value_handle+1;
            end_handle = next_characteristic->handle -1;
            bleapp_gatt_discovery(BT_GATT_DISCOVER_DESCRIPTOR, NULL, start_handle, end_handle);
            return;
        }else{
            ongoing_characteristic = next_characteristic;
            node = sys_slist_peek_next(node);
        }
    } 
    if(ongoing_characteristic->value_handle < ongoing_gatt_service.end_handle)
    {
        start_handle = ongoing_characteristic->value_handle+1;
        end_handle = ongoing_gatt_service.end_handle;
        bleapp_gatt_discovery(BT_GATT_DISCOVER_DESCRIPTOR, NULL, start_handle, end_handle);
    }
    else
    {
        ongoing_characteristic = NULL;
        printf("Discovery completed\r\n");
    }
}

static u8_t bleapp_gatt_discover_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr, struct bt_gatt_discover_params *params)
{
    struct bt_gatt_service_val *gatt_service = NULL;
    struct bt_gatt_chrc *gatt_chrc = NULL;
    char str[37];
    
    if(!attr){
        if(params->type == BT_GATT_DISCOVER_PRIMARY){
            if(params->uuid){
                printf("There is no target service found\r\n");
                k_free(params->uuid);
                params->uuid = NULL;
            }
        }
        else if(params->type == BT_GATT_DISCOVER_CHARACTERISTIC){
            bleapp_gatt_discovery_descriptor();   
        }
        return BT_GATT_ITER_STOP;
    }

    switch(params->type){
        case BT_GATT_DISCOVER_PRIMARY:
            gatt_service = attr->user_data;
            ongoing_gatt_service.start_handle = attr->handle;
            ongoing_gatt_service.end_handle = gatt_service->end_handle;
            bt_uuid_to_str(gatt_service->uuid, str, sizeof(str));
            printf("Service %s found: start handle 0x%x, end_handle 0x%x\r\n", str, attr->handle, gatt_service->end_handle);
            if(params->uuid){
                k_free(params->uuid);
                params->uuid = NULL;
                gatt_service->uuid = NULL;
            }
            bleapp_gatt_discovery(BT_GATT_DISCOVER_CHARACTERISTIC, NULL, attr->handle, gatt_service->end_handle);
            return BT_GATT_ITER_STOP;//only discovery the target service not all services, so stop.
            break;

        case BT_GATT_DISCOVER_CHARACTERISTIC:
            gatt_chrc = attr->user_data;
            bt_uuid_to_str(gatt_chrc->uuid, str, sizeof(str));
            printf("Characteristic %s found: attr->handle 0x%x  chrc->value_handle 0x%x \r\n", str, attr->handle,gatt_chrc->value_handle);
            bleapp_gatt_print_chrc_props(gatt_chrc->properties);
            if(!bt_uuid_cmp(gatt_chrc->uuid,bleapp_gatt_allocate_uuid(BT_UUID_CHAR_BLE_TP_WR)))
            {
                if(!wr_arg.handle){
                    ble_write_task_set(true, gatt_chrc->value_handle);
                }
            }


            if(characteristics_list == NULL){
                characteristics_list =  k_malloc(sizeof(sys_slist_t));
                if(characteristics_list == NULL){
                    printf("Allocate memory for characteristics_list failed\r\n");
                    return BT_GATT_ITER_CONTINUE;
                }
                sys_slist_init(characteristics_list);
            }
            
            bleapp_gatt_characteristic *char_tmp = (bleapp_gatt_characteristic *)k_malloc(sizeof(bleapp_gatt_characteristic));
            if(char_tmp == NULL){
                printf("Allocate memory for characteristic  failed\r\n");
                return BT_GATT_ITER_CONTINUE;    
            }
            char_tmp->uuid = bleapp_gatt_allocate_uuid((struct bt_uuid *)gatt_chrc->uuid);
            if(char_tmp->uuid == NULL){
                printf("Failed to store charactristic uuid\r\n");
            }
            char_tmp->handle = attr->handle;
            char_tmp->value_handle = gatt_chrc->value_handle;
            char_tmp->properties = gatt_chrc->properties;
            sys_slist_append(characteristics_list, &char_tmp->node);
            break;

        case BT_GATT_DISCOVER_DESCRIPTOR:
            bt_uuid_to_str(attr->uuid, str, sizeof(str));
            printf("Descriptor %s found: handle %x ", str, attr->handle);
            if(ongoing_characteristic->uuid){
                bt_uuid_to_str(ongoing_characteristic->uuid, str, sizeof(str));
                printf("of characteristic %s\r\n", str);
            }
            if(!bt_uuid_cmp(attr->uuid, bt_ccc_uuid)){
                uint8_t subscrib_value = 0x00;
                if(ongoing_characteristic->properties & BT_GATT_CHRC_NOTIFY)
                    subscrib_value |= BT_GATT_CCC_NOTIFY;
                
                if(ongoing_characteristic->properties & BT_GATT_CHRC_INDICATE)
                    subscrib_value |= BT_GATT_CCC_INDICATE;
                printf("subscrib_value=0x%x\r\n", subscrib_value);
                if(subscrib_value != 0 && !bleapp_gatt_subscribed(attr->handle))
                {
                    bleapp_gatt_subscribe(ongoing_characteristic->value_handle, attr->handle, subscrib_value);
                    if(attr->handle == params->end_handle)
                        continue_discover_descriptor = true;   
                }
                else if(attr->handle == params->end_handle)
                    bleapp_gatt_discovery_descriptor();
            }
            break;
    }

    return BT_GATT_ITER_CONTINUE;
}


static u8_t bleapp_gatt_notify_func(struct bt_conn *conn,
			struct bt_gatt_subscribe_params *params,
			const void *data, u16_t length)
{
    if(data == NULL){
        if (!params->value) {
            printf("Unsubscribed\r\n");
            params->value_handle = 0U;
            return 0;
        }
        printf("subscribed\r\n");
        if(continue_discover_descriptor == true){
            continue_discover_descriptor = false;
            bleapp_gatt_discovery_descriptor();
        }
        return 0;
    }else{
         printf("receive notification/indication data from peer device\r\n");
         return BT_GATT_ITER_CONTINUE;
    }
}

static void bleapp_central_task(void *pvParameters)
{
    int err = 0;
    while(1){
        bleapp_msg *msg = k_fifo_get(&ble_central_queue, K_FOREVER);
        if(msg == NULL)
            continue;

        switch(msg->type){
            case MSG_START_SCAN:
            {
                struct bt_le_scan_param scan_param;
                memset(&scan_param, 0, sizeof(scan_param));
                scan_param.type = BT_HCI_LE_SCAN_ACTIVE;
                scan_param.filter_dup = BT_HCI_LE_SCAN_FILTER_DUP_DISABLE;
                scan_param.interval = BT_GAP_SCAN_FAST_INTERVAL;
                scan_param.window = BT_GAP_SCAN_FAST_WINDOW;
            
                err = bt_le_scan_start(&scan_param, bleapp_scan_cb);
            
                if(err){
                    printf("Failed to start scan (err %d) \r\n", err);
                }else{
                    printf("Start scan successfully \r\n");
                } 
            }
            break;
            
            case MSG_CREATE_CONNECTION:
            {
                struct bt_conn* conn = NULL;
                
                struct bt_le_conn_param conn_param = {
                    .interval_min =  BT_GAP_INIT_CONN_INT_MIN,
                    .interval_max =  BT_GAP_INIT_CONN_INT_MAX,
                    .latency = 0,
                    .timeout = 400,
                };
                conn = bt_conn_create_le((const bt_addr_le_t *)(&msg->param.create_conn.peer_addr), (const struct bt_le_conn_param *)&conn_param);

                if(!conn){
                    printf("Connection failed\r\n");
                }else{
                    printf("Connection starting\r\n");
                }
             }
            break;

            case MSG_SECURITY_START:
            {
                err = bt_conn_set_security(bleapp_default_conn, msg->param.sec.level);
                if(err){
                    printf("Start security failed(%d)\r\n", err);
                }else{
                    printf("Start security pending\r\n");
                }
            }
            break;
            
            case MSG_GATT_DISCOVER_START:
            {
                bleapp_gatt_discover_params.uuid = msg->param.discover.uuid;
                bleapp_gatt_discover_params.type = msg->param.discover.discover_type;
                bleapp_gatt_discover_params.start_handle = msg->param.discover.start_handle;
                bleapp_gatt_discover_params.end_handle = msg->param.discover.end_handle;
                bleapp_gatt_discover_params.func = bleapp_gatt_discover_cb;
                
                err = bt_gatt_discover(bleapp_default_conn, &bleapp_gatt_discover_params);
                if(err){
                    printf("Discover failed(%d)\r\n", err);
                }else{
                    printf("Discover pending\r\n");
                }
            }
            break;

            case MSG_GATT_SUBSCRIBE:
            {
                struct bt_gatt_subscribe_params *subscribe_tmp = bleapp_gatt_allocate_subscribe_params(msg->param.subscribe.ccc_handle);
                if(subscribe_tmp == NULL){
                    printf("Allocate memory for subscribe params failed\r\n");
                    return;    
                }
                memset(subscribe_tmp, 0, sizeof(struct bt_gatt_subscribe_params));
                subscribe_tmp->value_handle = msg->param.subscribe.value_handle;
                subscribe_tmp->ccc_handle = msg->param.subscribe.ccc_handle;
                subscribe_tmp->value = msg->param.subscribe.value;
                subscribe_tmp->notify = bleapp_gatt_notify_func;
                printf("Subscribe ccc handle:%u\r\n",  subscribe_tmp->ccc_handle);
                int err = bt_gatt_subscribe(bleapp_default_conn, subscribe_tmp);
                if (err) {
                    printf("Subscribe failed (err %d)\r\n", err);
                    k_free(subscribe_tmp);
                } else {
                    printf("Subscribed starting\r\n");
                }
            }
            break;
               
            default:
            break;
        }
        k_free(msg);
    }   
}

static void bleapp_gatt_discovery(uint8_t type, struct bt_uuid *uuid, uint16_t start_handle, uint16_t end_handle)
{
    bleapp_msg *msg = k_malloc(sizeof(bleapp_msg));
    if(msg == NULL){
        printf("%s,Allocate msg failed\r\n", __func__);
        return;
    }
    
    memset(msg, 0, sizeof(bleapp_msg));
    msg->type = MSG_GATT_DISCOVER_START;
    if(uuid != NULL)
        msg->param.discover.uuid = bleapp_gatt_allocate_uuid(uuid);
    printf("%s,type=%u\r\n", __func__, type);
    msg->param.discover.discover_type = type;
    msg->param.discover.start_handle = start_handle;
    msg->param.discover.end_handle = end_handle;
    vTaskDelay(1000);
    k_fifo_put(&ble_central_queue, (void *)msg);
}

#if (defined(BLE_SMP_TEST) && defined(CONFIG_BT_SMP))
static void bleapp_start_security(uint8_t sec_level)
{
    bleapp_msg *msg = k_malloc(sizeof(bleapp_msg));
    if(msg == NULL){
        printf("%s,Allocate msg failed\r\n", __func__);
        return;
    }
    
    memset(msg, 0, sizeof(bleapp_msg));
    msg->type = MSG_SECURITY_START;
    msg->param.sec.level = sec_level;

    k_fifo_put(&ble_central_queue, (void *)msg);
}
#endif

static bool data_cb(struct bt_data *data, void *user_data)
{
    char *name = user_data;
    u8_t len;

    switch (data->type) {
    case BT_DATA_NAME_SHORTENED:
    case BT_DATA_NAME_COMPLETE:
        len = (data->data_len > 30 - 1)?(30 - 1):(data->data_len);
        memcpy(name, data->data, len);
        return false;		
    default:
        return true;
    }
}

static void bleapp_scan_cb(const bt_addr_le_t *addr, s8_t rssi, u8_t evtype, struct net_buf_simple *buf)
{
    char le_addr[BT_ADDR_LE_STR_LEN];
    char name[30];
    int ret;
    
    (void)memset(name, 0, sizeof(name));
    bt_data_parse(buf, data_cb, name);

    if(!strcmp(name, CONFIG_BT_DEVICE_NAME)){
        bt_addr_le_to_str(addr, le_addr, sizeof(le_addr));
        printf("[DEVICE FOUND]: %s, AD evt type %u, RSSI %i %s \r\n",le_addr, evtype, rssi, name);
        // Stop scan 
        ret = bt_le_scan_stop();
        if(ret){
            printf("Stop scan failed, err = %d\r\n", ret);
            return;
        }

         bleapp_msg *msg = k_malloc(sizeof(bleapp_msg));
         if(msg == NULL){
             printf("Allocate msg failed\r\n");
             return;
         }
         msg->type = MSG_CREATE_CONNECTION;
         bt_addr_le_copy(&msg->param.create_conn.peer_addr, (const bt_addr_le_t *)addr);
         k_fifo_put(&ble_central_queue, (void *)msg);
    }
}

static void bleapp_start_scan(void)
{
    bleapp_msg *msg = k_malloc(sizeof(bleapp_msg));
    if(msg == NULL){
       printf("%s,Allocate msg failed\r\n", __func__);
       return;
    }
    msg->type = MSG_START_SCAN;
    k_fifo_put(&ble_central_queue, (void *)msg);
}

static void ble_write_task(void *arg)
{
    wr_arg_t* args = (wr_arg_t*)arg;
    uint8_t data[244] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};

    for(int i = 0; i < sizeof(data); ++i){
        data[i] = i & 0xff;
    }

    while(args->handle){
        int err = bt_gatt_write_without_response(bleapp_default_conn, (uint32_t)wr_arg.wr_hdl & 0xffff, data,
            244, false);
        if(err){
            printf("Write without response (err %d)\r\n", err);
        }
        printf("Write success\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    printf("Deleted blewrite task\r\n");
    vTaskDelete(NULL);
}

void ble_write_task_set(bool enable, uint16_t handle)
{
    wr_arg.wr_hdl = handle + 0;
    if(!enable){
        if(wr_arg.handle){
            wr_arg.handle = NULL;
        }
        return;
    }

    if(xTaskCreate(ble_write_task, (char*)"blewrite", 256, (void*)&wr_arg, configMAX_PRIORITIES - 5, &wr_arg.handle) == pdPASS){
        printf("Create ble_write_task success, start write\r\n");
    }
    else{
        printf("Create ble_write_task failed\r\n");
    }
}


void bt_enable_cb(int err)
{
    if (!err) {
        bt_addr_le_t bt_addr;
        bt_get_local_public_address(&bt_addr);
        printf("BD_ADDR:(MSB)%02x:%02x:%02x:%02x:%02x:%02x(LSB) \n",
            bt_addr.a.val[5], bt_addr.a.val[4], bt_addr.a.val[3], bt_addr.a.val[2], bt_addr.a.val[1], bt_addr.a.val[0]);

        bt_conn_cb_register(&bleapp_conn_callbacks);
        #if (defined(BLE_SMP_TEST) && defined(CONFIG_BT_SMP))
        bt_conn_auth_cb_register(&bleapp_auth_cb_display);
        #endif
        #if defined(CONFIG_BT_STACK_CLI) 
        ble_cli_register();
        #endif
        ongoing_gatt_service.start_handle = BLE_GATT_INVALID_HANDLE;
        ongoing_gatt_service.end_handle = BLE_GATT_INVALID_HANDLE;
        memset(&subscribe_params_array[0], 0, sizeof(subscribe_params_array));
        k_fifo_init(&ble_central_queue, 10);
        k_thread_create(&ble_central_task_hdl, "central_task", 1536,(k_thread_entry_t)bleapp_central_task, configMAX_PRIORITIES-5);
        bleapp_start_scan();

    }
}

void ble_stack_start(void)
{
    // Initialize BLE controller
    btble_controller_init(configMAX_PRIORITIES - 1);
    // Initialize BLE Host stack
    hci_driver_init();
    bt_enable(bt_enable_cb);
}
