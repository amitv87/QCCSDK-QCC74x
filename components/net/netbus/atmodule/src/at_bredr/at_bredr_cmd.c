#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "at_main.h"
#include "at_core.h"
#include <byteorder.h>
#include <bluetooth.h>
#include <hci_host.h>
#include <conn.h>
#include <conn_internal.h>
#include <hci_core.h>
#include <l2cap.h>
#include <l2cap_internal.h>
#include <utils_hex.h>
#include <btble_lib_api.h>
#include <hci_driver.h>
#include "bt_log.h"
#if CONFIG_BT_A2DP
#include <a2dp.h>
#endif
#if CONFIG_BT_AVRCP
#include <avrcp.h>
#endif
#if CONFIG_BT_AVRCP
//#include <rfcomm.h>
#endif
#if CONFIG_BT_HFP
#include <hfp_hf.h>
#endif

#if CONFIG_BT_SPP
#include <spp.h>
#endif

#define AT_BREDR_PRINTF     printf

static void bredr_connected(struct bt_conn *conn, u8_t err);
static void bredr_disconnected(struct bt_conn *conn, u8_t reason);
static int get_mac_from_string(char *string, uint8_t mac[6]);


static int at_exe_cmd_bredr_init(int argc, const char **argv);
static int at_exe_cmd_bredr_disconnect(int argc, const char **argv);
static int at_exe_cmd_bredr_start_inquiry(int argc, const char **argv);
static int at_exe_cmd_bredr_stop_inquiry(int argc, const char **argv);
static int at_exe_cmd_bredr_write_eir(int argc, const char **argv);
static int at_exe_cmd_bredr_write_local_name(int argc, const char **argv);
static int at_setup_cmd_bredr_connectable(int argc, const char **argv);
static int at_setup_cmd_bredr_discoverable(int argc, const char **argv);
static int at_setup_cmd_bredr_connect(int argc, const char **argv);
static int at_setup_cmd_bredr_remote_name(int argc, const char **argv);
static int at_setup_cmd_bredr_unpair(int argc, const char **argv);
static int at_setup_cmd_bredr_set_tx_pwr(int argc, const char **argv);

#if CONFIG_BT_HFP
static int at_exe_cmd_hfp_connect(int argc, const char **argv);
static int at_exe_cmd_hfp_disconnect(int argc, const char **argv);
static int at_exe_cmd_hfp_answer(int argc, const char **argv);
static int at_exe_cmd_hfp_terminate_call(int argc, const char **argv);
static int at_exe_cmd_hfp_disable_nrec(int argc, const char **argv);
static int at_exe_cmd_hfp_query_list_calls(int argc, const char **argv);
static int at_exe_cmd_hfp_subscriber_number_info(int argc, const char **argv);
static int at_setup_cmd_hfp_voice_recognition(int argc, const char **argv);
static int at_setup_cmd_hfp_voice_req_phone_num(int argc, const char **argv);
static int at_setup_cmd_hfp_accept_incoming_caller(int argc, const char **argv);
static int at_setup_cmd_hfp_set_mic_volume(int argc, const char **argv);
static int at_setup_cmd_hfp_set_speaker_volume(int argc, const char **argv);
static int at_setup_cmd_hfp_response_call(int argc, const char **argv);
static int at_setup_cmd_hfp_hf_send_indicator(int argc, const char **argv);
static int at_setup_cmd_hfp_hf_update_indicator(int argc, const char **argv);
#endif
#if CONFIG_BT_A2DP
static int at_exe_cmd_a2dp_connect(int argc, const char **argv);
#if 0
static int at_exe_cmd_a2dp_disconnect(int argc, const char **argv);
static int at_exe_cmd_a2dp_suspend_stream(int argc, const char **argv);
static int at_exe_cmd_a2dp_close_stream(int argc, const char **argv);
static int at_exe_cmd_a2dp_open_stream(int argc, const char **argv);
static int at_exe_cmd_a2dp_start_stream(int argc, const char **argv);
#endif
#endif
#if CONFIG_BT_SPP
static int at_exe_cmd_bredr_sppdisconnect(int argc, const char **argv);
static int at_exe_cmd_bredr_sppconnect(int argc, const char **argv);
static int at_setup_cmd_bredr_sppsend(int argc, const char **argv);
#endif

#if CONFIG_BT_AVRCP
static int at_exe_cmd_avrcp_connect(int argc, const char **argv);
static int at_exe_cmd_avrcp_get_play_status(int argc, const char **argv);
static int at_setup_cmd_avrcp_pth_key(int argc, const char **argv);
static int at_setup_cmd_avrcp_pth_key_act(int argc, const char **argv);
static int at_setup_cmd_avrcp_change_vol(int argc, const char **argv);
#endif

static bool init = false;
static struct bt_conn_info conn_info;
static struct bt_conn *default_conn = NULL;
static struct bt_br_discovery_result result[10] = { 0 };
static struct bt_conn_cb conn_callbacks = {
    .connected = bredr_connected,
    .disconnected = bredr_disconnected,
};

#if CONFIG_BT_A2DP
struct k_thread media_transport;
static void a2dp_chain(struct bt_conn *conn, uint8_t state);
static void a2dp_stream(uint8_t state);
static void a2dp_start_cfm(void);

static void a2dp_chain(struct bt_conn *conn, uint8_t state)
{
    AT_BREDR_PRINTF("%s, conn: %p \n", __func__, conn);

    if (state == BT_A2DP_CHAIN_CONNECTED) {
        AT_BREDR_PRINTF("a2dp connected. \n");
    } else if (state == BT_A2DP_CHAIN_DISCONNECTED) {
        AT_BREDR_PRINTF("a2dp disconnected. \n");
    }
}

static void a2dp_stream(uint8_t state)
{
    AT_BREDR_PRINTF("%s, state: %d \n", __func__, state);

    if (state == BT_A2DP_STREAM_START) {
        AT_BREDR_PRINTF("a2dp play. \n");
    } else if (state == BT_A2DP_STREAM_SUSPEND) {
        AT_BREDR_PRINTF("a2dp stop. \n");
    }
}
static struct a2dp_callback a2dp_callbacks =
{
    .chain = a2dp_chain,
    .stream = a2dp_stream,
#if CONFIG_BT_A2DP_SOURCE
    .start_cfm = a2dp_start_cfm,
#endif
};
#endif

#if CONFIG_BT_AVRCP
static void avrcp_chain(struct bt_conn *conn, uint8_t state);
static void avrcp_absvol(uint8_t vol);
static void avrcp_play_status(uint32_t song_len, uint32_t song_pos, uint8_t status);

static struct avrcp_callback avrcp_callbacks =
{
    .chain = avrcp_chain,
    .abs_vol = avrcp_absvol,
    .play_status = avrcp_play_status,
};
#endif

#if CONFIG_BT_AVRCP
static void avrcp_chain(struct bt_conn *conn, uint8_t state)
{
    AT_BREDR_PRINTF("%s, conn: %p \n", __func__, conn);

    if (state == BT_AVRCP_CHAIN_CONNECTED) {
        AT_BREDR_PRINTF("avrcp connected. \n");
    } else if (state == BT_AVRCP_CHAIN_DISCONNECTED) {
        AT_BREDR_PRINTF("avrcp disconnected. \n");
    }
}

static void avrcp_absvol(uint8_t vol)
{
    AT_BREDR_PRINTF("%s, vol: %d \n", __func__, vol);
}

static void avrcp_play_status(uint32_t song_len, uint32_t song_pos, uint8_t status)
{
    AT_BREDR_PRINTF("%s, song length: %lu, song position: %lu, play status: %u \n", __func__, song_len, song_pos, status);
}
#endif

static void remote_name(const char *name)
{
    if (name) {
        AT_BREDR_PRINTF("%s, remote name len: %d,  : %s\n", __func__, strlen(name), name);
    } else {
        AT_BREDR_PRINTF("%s, remote name request fail\n", __func__);
    }
}

static int get_mac_from_string(char *string, uint8_t mac[6])
{
    int i, j = 0;
    char mac_string[13];

    if (strlen(string) != 17) {
        return -1;
    }

    memset(mac_string, 0, sizeof(mac_string));
    for (i = 0; i < strlen(string); i++) {
        if (i % 3 == 2) {
            if (string[i] != ':') {
                return -1;
            }
        } else {
            mac_string[j++] = string[i];
        }
    }

    if (utils_hex2bin((const char *)mac_string, strlen(mac_string), mac, 6) > 0) {
        return 0;
    }

    return -1;
}

static void bredr_connected(struct bt_conn *conn, u8_t err)
{
    if(conn->type != BT_CONN_TYPE_BR)
    {
        return;
    }

    char addr[BT_ADDR_STR_LEN];

    bt_conn_get_info(conn, &conn_info);
    bt_addr_to_str(conn_info.br.dst, addr, sizeof(addr));

    if (err) {
        AT_BREDR_PRINTF("bredr failed to connect to %s (%u) \r\n", addr, err);
        return;
    }

    AT_BREDR_PRINTF("bredr connected: %s \r\n", addr);

    if (!default_conn)
    {
        default_conn = conn;
    }

    at_response_string("+BREDR:CONNECTED:%s\r\n",addr);
}

static void bredr_disconnected(struct bt_conn *conn, u8_t reason)
{
    if(conn->type != BT_CONN_TYPE_BR)
    {
        return;
    }

    char addr[BT_ADDR_STR_LEN];

    bt_conn_get_info(conn, &conn_info);
    bt_addr_to_str(conn_info.br.dst, addr, sizeof(addr));

    AT_BREDR_PRINTF("bredr disconnected: %s (reason %u) \r\n", addr, reason);

    at_response_string(" +BREDR:DISCONNECTED%s\r\n",addr);
    if (default_conn == conn)
    {
        default_conn = NULL;
    }

}

static void bt_enable_cb(int err)
{
    if (!err) {
        bt_addr_le_t bt_addr;
        bt_get_local_public_address(&bt_addr);
        AT_BREDR_PRINTF("BD_ADDR:(MSB)%02x:%02x:%02x:%02x:%02x:%02x(LSB) \r\n",
               bt_addr.a.val[5], bt_addr.a.val[4], bt_addr.a.val[3], bt_addr.a.val[2], bt_addr.a.val[1], bt_addr.a.val[0]);
    }
}

#if CONFIG_BT_SPP

static void bt_recv_callback(u8_t *data, u16_t length)
{
    at_response_string("+BREDR:SPP:%s\r\n",bt_hex(data,length));
};
static void bt_spp_connected(void)
{
    at_response_string("+BREDR:SPPCONNECTED");
};
static void bt_spp_disconnected(void)
{
    at_response_string("+BREDR:SPPDISCONNECTED");
};

static struct spp_callback_t spp_conn_callbacks={
    .connected=bt_spp_connected,
    .disconnected=bt_spp_disconnected,
    .bt_spp_recv=bt_recv_callback,
};
#endif

static int at_exe_cmd_bredr_init(int argc, const char **argv)
{
    if(init){
        AT_BREDR_PRINTF("bredr has initialized\n");
        return AT_RESULT_CODE_OK;
    }
    else
    {
        if (!atomic_test_bit(bt_dev.flags, BT_DEV_ENABLE))
        {
            #if defined(QCC74x_undef) || defined(QCC74x_undef)
            ble_controller_init(configMAX_PRIORITIES - 1);
            #else
            btble_controller_init(configMAX_PRIORITIES - 1);
            #endif
            // Initialize BLE Host stack
            hci_driver_init();
            bt_enable(bt_enable_cb);
        }
    }

    default_conn = NULL;
    bt_conn_cb_register(&conn_callbacks);
#if CONFIG_BT_A2DP
    a2dp_cb_register(&a2dp_callbacks);
#endif
#if CONFIG_BT_AVRCP
    avrcp_cb_register(&avrcp_callbacks);
#endif
#if CONFIG_BT_SPP
    spp_cb_register(&spp_conn_callbacks);
#endif
    init = true;
    AT_BREDR_PRINTF("bredr init successfully\n");
    return AT_RESULT_CODE_OK;
}

#if CONFIG_BT_SPP
static int at_setup_cmd_bredr_sppsend(int argc, const char **argv)
{
    int err = 0;
    uint8_t string[255]={0};
    int strlen = 0;
    AT_CMD_PARSE_STRING(0, string, sizeof(string));
    AT_CMD_PARSE_NUMBER(1, &strlen);
    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }
    
    err=bt_spp_send(string,strlen);
    if(err)
    {
        AT_BREDR_PRINTF("bt spp send err:%d\r\n", err);
        at_response_string("%s", AT_CMD_MSG_SEND_FAIL);
        return AT_RESULT_CODE_IGNORE;
    }
    else
    {
        AT_BREDR_PRINTF("bt spp send successfully\r\n");
        at_response_string("%s", AT_CMD_MSG_SEND_OK);
        return AT_RESULT_CODE_PROCESS_DONE;
    }
        
}

static int at_exe_cmd_bredr_sppconnect(int argc, const char **argv)
{
    int err = 0;
    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }
    err= bt_spp_connect(default_conn);
    if(err)
    {
        AT_BREDR_PRINTF("bt spp connect err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("bt spp connect successfully\r\n");
        return AT_RESULT_CODE_OK;
    }
}

static int at_exe_cmd_bredr_sppdisconnect(int argc, const char **argv)
{
    int err = 0;
    if(!default_conn){
        
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }
    err= bt_spp_disconnect(default_conn);
    if(err)
    {
        AT_BREDR_PRINTF("bt spp disconnect err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("bt spp disconnect successfully\r\n");
        return AT_RESULT_CODE_OK;
    }

}
#endif

static int at_setup_cmd_bredr_connectable(int argc, const char **argv)
{
    int err;
    int action = 0;

    AT_CMD_PARSE_NUMBER(0, &action);

    if (action == 1) {
        err = bt_br_set_connectable(true);
    } else if (action == 0) {
        err = bt_br_set_connectable(false);
    } else {
        return AT_RESULT_CODE_ERROR;
    }


    if (err) {
        return AT_RESULT_CODE_ERROR;
        AT_BREDR_PRINTF("BR/EDR set connectable failed, (err %d)\n", err);
    } else {
        return AT_RESULT_CODE_OK;
    	AT_BREDR_PRINTF("BR/EDR set connectable done.\n");
    }
}

static int at_setup_cmd_bredr_discoverable(int argc, const char **argv)
{
    int err;
    int action = 0;

    AT_CMD_PARSE_NUMBER(0, &action);

    if (action == 1) {
        err = bt_br_set_discoverable(true);
    } else if (action == 0) {
        err = bt_br_set_discoverable(false);
    } else {
        return AT_RESULT_CODE_ERROR;
    }

    if (err) {
        AT_BREDR_PRINTF("BR/EDR set discoverable failed, (err %d)\n", err);
        return AT_RESULT_CODE_ERROR;
    } else {
    	AT_BREDR_PRINTF("BR/EDR set discoverable done.\n");
        return AT_RESULT_CODE_OK;
    }
}

static int at_setup_cmd_bredr_connect(int argc, const char **argv)
{
    struct bt_conn *conn;
    char addr_string[18];
    uint8_t remote_address[6];
    bt_addr_t peer_addr;
    struct bt_br_conn_param param;
    char addr_str[18];

    AT_CMD_PARSE_STRING(0, addr_string, sizeof(addr_string));
    if (get_mac_from_string(addr_string, remote_address) != 0) {
        return AT_RESULT_CODE_ERROR;
    }
    reverse_bytearray(remote_address, peer_addr.val, 6);
    bt_addr_to_str(&peer_addr, addr_str, sizeof(addr_str));

    AT_BREDR_PRINTF("%s, create bredr connection with : %s \n", __func__, addr_str);

    param.allow_role_switch = true;

    conn = bt_conn_create_br(&peer_addr, &param);
    if (conn) {
        AT_BREDR_PRINTF("Connect bredr ACL success.\n");
        return AT_RESULT_CODE_OK;
    } else {
        AT_BREDR_PRINTF("Connect bredr ACL fail.\n");
        return AT_RESULT_CODE_ERROR;
    }
}

static int at_exe_cmd_bredr_disconnect(int argc, const char **argv)
{
    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    int err = bt_conn_disconnect(default_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    if (err) {
        AT_BREDR_PRINTF("Disconnection failed.\n");
        return AT_RESULT_CODE_ERROR;
    } else {
        AT_BREDR_PRINTF("Disconnect successfully.\n");
        return AT_RESULT_CODE_OK;
    }

}

static int at_exe_cmd_bredr_write_local_name(int argc, const char **argv)
{
    int err;
    char *name = "QCC-BT";

    err = bt_br_write_local_name(name);
    if (err) {
        AT_BREDR_PRINTF("BR/EDR write local name failed, (err %d)\n", err);
        return AT_RESULT_CODE_ERROR;
    } else {
        AT_BREDR_PRINTF("BR/EDR write local name done.\n");
        return AT_RESULT_CODE_OK;
    }
}

static int at_exe_cmd_bredr_write_eir(int argc, const char **argv)
{
    int err;
    char *name = "qcc74x-classic-bluetooth";
    uint8_t fec = 1;
    uint8_t data[32] = {0};

    data[0] = 30;
    data[1] = 0x09;
    memcpy(data+2, name, strlen(name));

    for(int i = 0; i < strlen(name); i++)
    {
        AT_BREDR_PRINTF("0x%02x ", data[2+i]);
    }
    AT_BREDR_PRINTF("\n");

    err = bt_br_write_eir(fec, data);
    if (err) {
        AT_BREDR_PRINTF("BR/EDR write EIR failed, (err %d)\n", err);
        return AT_RESULT_CODE_ERROR;
    } else {
        AT_BREDR_PRINTF("BR/EDR write EIR done.\n");
        return AT_RESULT_CODE_OK;
    }
}

static int at_setup_cmd_bredr_remote_name(int argc, const char **argv)
{
    char addr_string[18];
    uint8_t remote_address[6];
    bt_addr_t peer_addr;

    AT_CMD_PARSE_STRING(0, addr_string, sizeof(addr_string));
    if (get_mac_from_string(addr_string, remote_address) != 0) {
        return AT_RESULT_CODE_ERROR;
    }
    reverse_bytearray(remote_address, peer_addr.val, 6);

    int err = remote_name_req(&peer_addr, remote_name);
    if (!err) {
        AT_BREDR_PRINTF("remote name request pending.\n");
        return AT_RESULT_CODE_OK;
    } else {
        AT_BREDR_PRINTF("remote name request fail.\n");
        return AT_RESULT_CODE_ERROR;
    }
}

static int at_setup_cmd_bredr_unpair(int argc, const char **argv)
{
    bt_addr_le_t addr;
    char addr_string[18];
    uint8_t remote_address[6];
    int err;

    /*Get addr type, 0:ADDR_PUBLIC, 1:ADDR_RAND, 2:ADDR_RPA_OR_PUBLIC, 3:ADDR_RPA_OR_RAND*/
    AT_CMD_PARSE_NUMBER(0, &addr.type);

    AT_CMD_PARSE_STRING(1, addr_string, sizeof(addr_string));

    if (get_mac_from_string(addr_string, remote_address) != 0) {
        return AT_RESULT_CODE_ERROR;
    }

    reverse_bytearray(remote_address, addr.a.val, 6);

    err = bt_unpair(0, &addr);

    if(err){
        AT_BREDR_PRINTF("Failed to unpair\r\n");
        return AT_RESULT_CODE_ERROR;
    }else{
        AT_BREDR_PRINTF("Unpair successfully\r\n");
        return AT_RESULT_CODE_OK;
    }
}

static void bt_br_discv_cb(struct bt_br_discovery_result *results,
				  size_t count)
{
    char addr_str[18];
    uint32_t dev_class;
    int i;

    if (!results || count== 0) {
        return;
    }

    for (i=0;i<count;i++) {
        dev_class = (results[i].cod[0] | (results[i].cod[1] << 8) | 
                     (results[i].cod[1] << 16));
        bt_addr_to_str(&results[i].addr, addr_str, sizeof(addr_str));
        AT_BREDR_PRINTF("addr %s,class 0x%lx,rssi %d\r\n",addr_str,
                     dev_class,results[i].rssi);
    }
}

static int at_exe_cmd_bredr_start_inquiry(int argc, const char **argv)
{
    struct bt_br_discovery_param param;

    //Valid range 0x01 - 0x30.
    param.length = 0x05;
    param.limited = 0;

    int err = bt_br_discovery_start(&param,result,10,bt_br_discv_cb);
    if (err) {
        AT_BREDR_PRINTF("BREDR discovery failed\n");
        return AT_RESULT_CODE_ERROR;
    }
    return AT_RESULT_CODE_OK;
}

static int at_exe_cmd_bredr_stop_inquiry(int argc, const char **argv)
{
    int err = bt_br_discovery_stop();
    if (err) {
        AT_BREDR_PRINTF("BREDR stop discovery failed\n");
        return AT_RESULT_CODE_ERROR;
    }
    return AT_RESULT_CODE_OK;
}

#if CONFIG_BT_HFP
static int at_exe_cmd_hfp_connect(int argc, const char **argv)
{
    int err;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    err = bt_hfp_hf_initiate_connect(default_conn);
    if (err) {
        AT_BREDR_PRINTF("hfp connect fail.\n");
        return AT_RESULT_CODE_ERROR;
    } else {
        AT_BREDR_PRINTF("hfp connect pending.\n");
        return AT_RESULT_CODE_OK;
    }
}

static int at_exe_cmd_hfp_disconnect(int argc, const char **argv)
{
    int err = 0;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    err = bt_hfp_hf_send_disconnect(default_conn);
    if(err){
        AT_BREDR_PRINTF("hf disconnect fail %d",err);
        return AT_RESULT_CODE_ERROR;
    }
    return AT_RESULT_CODE_OK;
}

static int at_exe_cmd_hfp_answer(int argc, const char **argv)
{
    int err = 0;
    
    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    err = bt_hfp_hf_send_cmd(default_conn, BT_HFP_HF_ATA, NULL);
    if(err)
    {
        AT_BREDR_PRINTF("Fail to send answer AT command with err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("send answer AT command successfully\r\n");
        return AT_RESULT_CODE_OK;
    }    
}

static int at_exe_cmd_hfp_terminate_call(int argc, const char **argv)
{
    int err = 0;
    
    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    err = bt_hfp_hf_send_cmd(default_conn, BT_HFP_HF_AT_CHUP, NULL);
    if(err)
    {
        AT_BREDR_PRINTF("Fail to send terminate call AT command with err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("send terminate call AT command successfully\r\n");
        return AT_RESULT_CODE_OK;
    }
        
}

static int at_exe_cmd_hfp_disable_nrec(int argc, const char **argv)
{
    int err = 0;
    
    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    err = bt_hfp_hf_send_cmd(default_conn, BT_HFP_HF_AT_NREC, NULL);
    if(err)
    {
        AT_BREDR_PRINTF("Fail to send disable nrec AT command with err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("send disable nrec AT command successfully\r\n");
        return AT_RESULT_CODE_OK;
    }
        
}

static int at_setup_cmd_hfp_voice_recognition(int argc, const char **argv)
{
    int err = 0;
    int enable = 0;

    AT_CMD_PARSE_NUMBER(0, &enable);

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    err = bt_hfp_hf_send_cmd_arg(default_conn, BT_HFP_HF_AT_BVRA, enable);
    if(err)
    {
        AT_BREDR_PRINTF("Fail to send voice recognition AT command with err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("send voice recognition AT command successfully\r\n");
        return AT_RESULT_CODE_OK;
    }
}

static int at_setup_cmd_hfp_voice_req_phone_num(int argc, const char **argv)
{
    int err = 0;
    int enable = 0;

    AT_CMD_PARSE_NUMBER(0, &enable);

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    err = bt_hfp_hf_send_cmd_arg(default_conn, BT_HFP_HF_AT_BINP, enable);
    if(err)
    {
        AT_BREDR_PRINTF("Fail to send reqeust phone number to the AG AT command with err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("send reqeust phone number to the AG AT command successfully\r\n");
        return AT_RESULT_CODE_OK;
    }
}

static int at_setup_cmd_hfp_accept_incoming_caller(int argc, const char **argv)
{
    int err = 0;
    int call_id = 0;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    AT_CMD_PARSE_NUMBER(0, &call_id);

    err = bt_hfp_hf_send_cmd_arg(default_conn, BT_HFP_ACCEPT_INCOMING_CALLER_ID, call_id);
    if(err)
    {
        AT_BREDR_PRINTF("Fail to accept a incoming call err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("Accept a incoming call successfully\r\n");
        return AT_RESULT_CODE_OK;
    }
}

static int at_setup_cmd_hfp_set_mic_volume(int argc, const char **argv)
{
    int err = 0;
    int vol = 0;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    AT_CMD_PARSE_NUMBER(0, &vol);

    if(vol > 15 ){
        AT_BREDR_PRINTF("Volume out of range %d\n",vol);
        return AT_RESULT_CODE_ERROR;
    }

    err = bt_hfp_hf_send_cmd_arg(default_conn, BT_HFP_SET_MIC_VOL,vol);
    if(err)
    {
        AT_BREDR_PRINTF("Fail to set mic volume err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("Set mic volume successfully\r\n");
        return AT_RESULT_CODE_OK;
    }
}

static int at_setup_cmd_hfp_set_speaker_volume(int argc, const char **argv)
{
    int err = 0;
    int vol = 0;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

     AT_CMD_PARSE_NUMBER(0, &vol);

    if(vol > 15 ){
        AT_BREDR_PRINTF("Volume out of range %d\n",vol);
        return AT_RESULT_CODE_ERROR;
    }

    err = bt_hfp_hf_send_cmd_arg(default_conn, BT_HFP_HF_AT_VGS,vol);
    if(err)
    {
        AT_BREDR_PRINTF("Fail to set speaker volume err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("Set speaker volume successfully\r\n");
        return AT_RESULT_CODE_OK;
    }

}

static int at_exe_cmd_hfp_query_list_calls(int argc, const char **argv)
{
    int err = 0;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    err = bt_hfp_hf_send_cmd(default_conn, BT_HFP_QUERY_LIST_CALLS,NULL);
    if(err)
    {
        AT_BREDR_PRINTF("Fail to query the list calls err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("Query the list calls successfully\r\n");
        return AT_RESULT_CODE_OK;
    }

}

static int at_setup_cmd_hfp_response_call(int argc, const char **argv)
{
    int err = 0;
    int method = 0;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    AT_CMD_PARSE_NUMBER(0, &method);
    if(method > 2){
        AT_BREDR_PRINTF("Unexception %d\n",method);
        return AT_RESULT_CODE_ERROR;
    }
    err = bt_hfp_hf_send_cmd_arg(default_conn, BT_HFP_RESPONSE_CALLS,method);
    if(err)
    {
        AT_BREDR_PRINTF("Fail to response a call err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("Response a call successfully\r\n");
        return AT_RESULT_CODE_OK;
    }
        
}

static int at_exe_cmd_hfp_subscriber_number_info(int argc, const char **argv)
{
    int err = 0;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    err = bt_hfp_hf_send_cmd(default_conn, BT_HFP_SUBSCRIBE_NUM_INFO,NULL);
    if(err)
    {
        AT_BREDR_PRINTF("Fail to response a call err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("Response a call successfully\r\n");
        return AT_RESULT_CODE_OK;    
    }
}

static int at_setup_cmd_hfp_hf_send_indicator(int argc, const char **argv)
{
    int err = 0;
    int id = 0;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    AT_CMD_PARSE_NUMBER(0, &id);

    err = bt_hfp_hf_send_cmd_arg(default_conn, BT_HFP_SEND_INDICATOR,id);
    if(err)
    {
        AT_BREDR_PRINTF("Fail to send a indicator err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("Send a indicato successfully\r\n");
        return AT_RESULT_CODE_OK;
    }

}

static int at_setup_cmd_hfp_hf_update_indicator(int argc, const char **argv)
{
    int val = 0;
    int err = 0;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    AT_CMD_PARSE_NUMBER(0, &val);

    err = bt_hfp_hf_send_cmd_arg(default_conn, BT_HFP_UPDATE_INDICATOR,val);
    if(err)
    {
        AT_BREDR_PRINTF("Fail to update indicator err:%d\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("Update indicator successfully\r\n");
        return AT_RESULT_CODE_OK;
    }
}

#endif

#if CONFIG_BT_A2DP
static int at_exe_cmd_a2dp_connect(int argc, const char **argv)
{
    struct bt_a2dp *a2dp;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    a2dp = bt_a2dp_connect(default_conn);
    if(a2dp) {
        AT_BREDR_PRINTF("a2dp connect successfully.\n");
        return AT_RESULT_CODE_OK;
    } else {
        AT_BREDR_PRINTF("a2dp connect fail. \n");
        return AT_RESULT_CODE_ERROR;
    }
}
#if 0
static int at_exe_cmd_a2dp_disconnect(int argc, const char **argv)
{
    int ret;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    ret = bt_a2dp_disconnect(default_conn);
    if(ret) {
        AT_BREDR_PRINTF("A2dp disconnect successfully.\n");
        return AT_RESULT_CODE_OK;
    } else {
        AT_BREDR_PRINTF("A2dp disconnect fail. ret(%d)\n",ret);
        return AT_RESULT_CODE_ERROR;
    }
}

static int at_exe_cmd_a2dp_suspend_stream(int argc, const char **argv)
{
    int ret = 0;

    if (!default_conn) {
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    ret = bt_a2dp_suspend_stream(default_conn);
    if (!ret) {
        AT_BREDR_PRINTF("A2dp suspend stream successfully.\n");
        return AT_RESULT_CODE_OK;
    } else {
        AT_BREDR_PRINTF("A2dp suspend stream fail. ret(%d)\n",ret);
        return AT_RESULT_CODE_ERROR;
    }
}

static int at_exe_cmd_a2dp_close_stream(int argc, const char **argv)
{
    int ret;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    ret = bt_a2dp_close_stream(default_conn);
    if(ret) {
        AT_BREDR_PRINTF("A2dp close stream successfully.\n");
        return AT_RESULT_CODE_OK;
    } else {
        AT_BREDR_PRINTF("A2dp close stream. ret(%d)\n",ret);
        return AT_RESULT_CODE_ERROR;
    }
}

static int at_exe_cmd_a2dp_open_stream(int argc, const char **argv)
{
    int ret;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    ret = bt_a2dp_open_stream(default_conn);
    if(ret) {
        AT_BREDR_PRINTF("A2dp open stream successfully.\n");
        return AT_RESULT_CODE_OK;
    } else {
        AT_BREDR_PRINTF("A2dp open stream  fail. ret(%d)\n",ret);
        return AT_RESULT_CODE_ERROR;
    }
}

static int at_exe_cmd_a2dp_start_stream(int argc, const char **argv)
{
    int ret;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    ret = bt_a2dp_start_stream(default_conn);
    if(ret) {
        AT_BREDR_PRINTF("A2dp start stream successfully.\n");
        return AT_RESULT_CODE_OK;
    } else {
        AT_BREDR_PRINTF("A2dp start stream fail. ret(%d)\n",ret);
        return AT_RESULT_CODE_ERROR;
    }
}
#endif
#endif

#if CONFIG_BT_AVRCP
static int at_exe_cmd_avrcp_connect(int argc, const char **argv)
{
    struct bt_avrcp *avrcp = NULL;

    if (!default_conn) {
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    avrcp = bt_avrcp_connect(default_conn);
    if(!avrcp) {
        AT_BREDR_PRINTF("avrcp connect failed\n");
        return AT_RESULT_CODE_ERROR;
    } else {
        AT_BREDR_PRINTF("avrcp connect successfully.\n");
        return AT_RESULT_CODE_OK;
    }
}

static int at_setup_cmd_avrcp_pth_key(int argc, const char **argv)
{
    int err;
    int key;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    AT_CMD_PARSE_NUMBER(0, &key);

    err = avrcp_pasthr_cmd(NULL, PASTHR_STATE_PRESSED, key);
    if(err) {
        AT_BREDR_PRINTF("avrcp key pressed failed, err: %d\n", err);
        return AT_RESULT_CODE_ERROR;
    } else {
        AT_BREDR_PRINTF("avrcp key pressed successfully.\n");
    }

    err = avrcp_pasthr_cmd(NULL, PASTHR_STATE_RELEASED, key);
    if(err) {
        AT_BREDR_PRINTF("avrcp key released failed, err: %d\n", err);
        return AT_RESULT_CODE_ERROR;
    } else {
        AT_BREDR_PRINTF("avrcp key play released successfully.\n");
        return AT_RESULT_CODE_OK;
    }
}

static int at_setup_cmd_avrcp_pth_key_act(int argc, const char **argv)
{
    int err;
    int key;
    int action;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    AT_CMD_PARSE_NUMBER(0, &key);
    AT_CMD_PARSE_NUMBER(1, &action); 

    if (action != PASTHR_STATE_PRESSED && action != PASTHR_STATE_RELEASED)
    {
        AT_BREDR_PRINTF("invalid key action.\n");
        return AT_RESULT_CODE_ERROR;
    }

    err = avrcp_pasthr_cmd(NULL, action, key);
    if(err) {
        AT_BREDR_PRINTF("avrcp key action failed, err: %d\n", err);
        return AT_RESULT_CODE_ERROR;
    } else {
        AT_BREDR_PRINTF("avrcp %s key %d successfully.\n", action ? "released":"pressed", key);
        return AT_RESULT_CODE_OK;
    }
}

static int at_setup_cmd_avrcp_change_vol(int argc, const char **argv)
{
    int err;
    int vol;
    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    AT_CMD_PARSE_NUMBER(0, &vol);
    err = avrcp_change_volume(vol);
    if (err) {
        AT_BREDR_PRINTF("avrcp change volume fail\n");
        return AT_RESULT_CODE_ERROR;
    } else {
        AT_BREDR_PRINTF("avrcp change volume success\n");
        return AT_RESULT_CODE_OK;
    }
}

static int at_exe_cmd_avrcp_get_play_status(int argc, const char **argv)
{
    int err;

    if(!default_conn){
        AT_BREDR_PRINTF("Not connected.\n");
        return AT_RESULT_CODE_ERROR;
    }

    err = avrcp_get_play_status_cmd(NULL);
    if(err) {
        AT_BREDR_PRINTF("avrcp get play status fail\n");
        return AT_RESULT_CODE_ERROR;
    } else {
        AT_BREDR_PRINTF("avrcp get play status success\n");
        return AT_RESULT_CODE_OK;
    }
}


#endif

static int at_setup_cmd_set_min_enc_key_size(int argc, const char **argv)
{
    int err;
    int key_size;
    
    AT_CMD_PARSE_NUMBER(0, &key_size);

    err = bt_br_set_min_enc_key_size(key_size);
    if (err)
    {
        AT_BREDR_PRINTF("set BREDR min encryption key size failed (err %d)\r\n", err);
        return AT_RESULT_CODE_ERROR;
    }
    else
    {
        AT_BREDR_PRINTF("set BREDR min encryption key size success\r\n");
        return AT_RESULT_CODE_OK;
    }
}

static int at_setup_cmd_bredr_set_tx_pwr(int argc, const char **argv)
{
    int err;
    int br_power,edr_power;

    AT_CMD_PARSE_NUMBER(0,&br_power);
    AT_CMD_PARSE_NUMBER(1,&edr_power);
    if ( (br_power > 10) && (br_power != 0xff) )
    {
        AT_BREDR_PRINTF("bt_set_tx_pwr, invalid value, br power value shall be in [%d - %d] or 0xff\r\n", 0, 10);
        return AT_RESULT_CODE_ERROR;
    }

    if ( (edr_power > 8) && (edr_power != 0xff) )
    {
        AT_BREDR_PRINTF("bt_set_tx_pwr, invalid value, edr power value shall be in [%d - %d] or 0xff\r\n", 0, 8);
        return AT_RESULT_CODE_ERROR;
    }

    err = bt_br_set_tx_pwr((int8_t)br_power, (int8_t)edr_power);

    if(err){
		AT_BREDR_PRINTF("bt_set_tx_pwr, Fail to set tx power (err %d)\r\n", err);
        return AT_RESULT_CODE_ERROR;
	}
	else{
		AT_BREDR_PRINTF("bt_set_tx_pwr, Set tx power successfully\r\n");
        return AT_RESULT_CODE_OK;
	}

}

static const at_cmd_struct at_bredr_cmd[] = {
    {"+BREDR_INIT", NULL,NULL,NULL,at_exe_cmd_bredr_init, 0, 0},
    {"+BREDR_START_INQUIRY", NULL,NULL,NULL,at_exe_cmd_bredr_start_inquiry, 0, 0},
    {"+BREDR_STOP_INQUIRY", NULL,NULL,NULL,at_exe_cmd_bredr_stop_inquiry, 0, 0},
    {"+BREDR_WRITE_EIR", NULL,NULL,NULL,at_exe_cmd_bredr_write_eir, 0, 0},
    {"+BREDR_WRITE_LOCAL_NAME", NULL,NULL,NULL,at_exe_cmd_bredr_write_local_name, 0, 0},
    {"+BREDR_CONNECTABLE", NULL,NULL,at_setup_cmd_bredr_connectable,NULL, 1, 1},
    {"+BREDR_DISCOVERABLE", NULL,NULL,at_setup_cmd_bredr_discoverable,NULL, 1, 1},
    {"+BREDR_CONNECT", NULL,NULL,at_setup_cmd_bredr_connect,NULL, 1, 1},
    {"+BREDR_REMOTE_NAME", NULL,NULL,at_setup_cmd_bredr_remote_name,NULL, 1, 1},
    {"+BREDR_SET_TX_POWER", NULL,NULL,at_setup_cmd_bredr_set_tx_pwr,NULL, 2, 2},
    {"+BREDR_UNPAIR", NULL,NULL,at_setup_cmd_bredr_unpair,NULL, 2, 2},
    {"+BREDR_DISCONNECT", NULL,NULL,NULL, at_exe_cmd_bredr_disconnect,0,0},
    #if CONFIG_BT_HFP
    {"+HFP_CONNECT", NULL,NULL,NULL, at_exe_cmd_hfp_connect,0,0},
    {"+HFP_DISCONNECT", NULL,NULL,NULL,at_exe_cmd_hfp_disconnect, 0, 0},
    {"+HFP_ANSWER", NULL,NULL,NULL,at_exe_cmd_hfp_answer, 0, 0},
    {"+HFP_TERMINATE_CALL", NULL,NULL,NULL,at_exe_cmd_hfp_terminate_call, 0, 0},
    {"+HFP_DISABLE_NREC", NULL,NULL,NULL,at_exe_cmd_hfp_disable_nrec, 0, 0},
    {"+HFP_QUERY_LIST_CALLS", NULL,NULL,NULL,at_exe_cmd_hfp_query_list_calls, 0, 0},
    {"+HFP_SUBSCRIBER_NUMBER_INFO", NULL,NULL,NULL,at_exe_cmd_hfp_subscriber_number_info, 0, 0},
    {"+HFP_VOICE_RECOGNITION", NULL,NULL,at_setup_cmd_hfp_voice_recognition,NULL, 1, 1},
    {"+HFP_VOICE_REQ_PHONE_NUM", NULL,NULL,at_setup_cmd_hfp_voice_req_phone_num,NULL, 1, 1},
    {"+HFP_ACCEPT_INCOMING_CALLER", NULL,NULL,at_setup_cmd_hfp_accept_incoming_caller,NULL, 1, 1},
    {"+HFP_SET_MIC_VOL", NULL,NULL,at_setup_cmd_hfp_set_mic_volume,NULL, 1, 1},
    {"+HFP_SET_SPEAKER_VOL", NULL,NULL,at_setup_cmd_hfp_set_speaker_volume,NULL, 1, 1},
    {"+HFP_RESPONSE_INCOMING_CALL", NULL,NULL,at_setup_cmd_hfp_response_call,NULL, 1, 1},
    {"+HFP_HF_SEND_INDICATOR", NULL,NULL,at_setup_cmd_hfp_hf_send_indicator,NULL, 1, 1},
    {"+HFP_HF_UPDATE_INDICATOR", NULL,NULL,at_setup_cmd_hfp_hf_update_indicator,NULL, 1, 1},
    #endif
    #if CONFIG_BT_A2DP
    {"+A2DP_CONNECT", NULL,NULL,NULL, at_exe_cmd_a2dp_connect,0,0},
    #if 0
    {"+A2DP_DISCONNECT", NULL,NULL,NULL, at_exe_cmd_a2dp_disconnect,0,0},
    {"+A2DP_SUSPEND_STREAM", NULL,NULL,NULL, at_exe_cmd_a2dp_suspend_stream,0,0},
    {"+A2DP_CLOSE_STREAM", NULL,NULL,NULL, at_exe_cmd_a2dp_close_stream,0,0},
    {"+A2DP_OPEN_STREAM", NULL,NULL,NULL, at_exe_cmd_a2dp_open_stream,0,0},
    {"+A2DP_STRAT_STREAM", NULL,NULL,NULL, at_exe_cmd_a2dp_start_stream,0,0},
    #endif
    #endif
    #if CONFIG_BT_AVRCP
    {"+AVRCP_CONNECT", NULL,NULL,NULL, at_exe_cmd_avrcp_connect,0,0},
    {"+AVRCP_KEY_PRESS", NULL,NULL,at_setup_cmd_avrcp_pth_key, NULL,1,1},
    {"+AVRCP_KEY_ACTION", NULL,NULL,at_setup_cmd_avrcp_pth_key_act, NULL,2,2},
    {"+AVRCP_CHANGE_VOL", NULL,NULL,at_setup_cmd_avrcp_change_vol, NULL,1,1},
    {"+AVRCP_GET_PLAY_STATUS", NULL,NULL,NULL, at_exe_cmd_avrcp_get_play_status,0,0},
    #endif
    #if CONFIG_BT_SPP
    {"+BREDRSPPCONNECT", NULL,NULL,NULL, at_exe_cmd_bredr_sppconnect,0,0},
    {"+BREDRSPPDISCONNECT", NULL,NULL,NULL, at_exe_cmd_bredr_sppdisconnect,0,0},
    {"+BREDRSPPSEND", NULL,NULL,at_setup_cmd_bredr_sppsend, NULL,2,2},
    #endif
    
};

int at_bredr_deinit(void)
{
    return 0;
}

int at_bredr_config_default(void)
{
    return 0;
}

bool at_bredr_cmd_regist(void)
{

    at_register_function(at_bredr_config_default, at_bredr_deinit);
    
    if (at_cmd_register(at_bredr_cmd, sizeof(at_bredr_cmd) / sizeof(at_bredr_cmd[0])) == 0)
        return true;
    else
        return false;
}
