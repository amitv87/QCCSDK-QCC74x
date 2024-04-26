/**
  ******************************************************************************
  * @file    at_ble_cmd.c
  * @version V1.0
  * @date
  * @brief   This file is part of AT command framework
  ******************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "at_main.h"
#include "at_core.h"

//////////////////////////////////
#include <FreeRTOS.h>
#include <timers.h>
#include <semphr.h>

#include "at_main.h"
#include "at_core.h"
#include "at_ble_config.h"
#include "at_ble_main.h"

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

static hex_to_string(uint8_t *hex, int hex_len, char *string, int string_len)
{
    int i;

    memset(string, 0, string_len);
    for (i = 0; i < hex_len; i++) {
        snprintf(string + strlen(string), string_len - strlen(string), "%02x", hex[i]);
    }
}

static int at_query_cmd_ble_init(int argc, const char **argv)
{
    at_response_string("+BLEINIT:%d\r\n", at_ble_config->work_role);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ble_init(int argc, const char **argv)
{
    int role = 0;

    AT_CMD_PARSE_NUMBER(0, &role);

    if (role != BLE_DISABLE && role != BLE_CLIENT && role != BLE_SERVER)
        return AT_RESULT_CODE_ERROR;

    at_ble_config->work_role = role;
    if (at_ble_init(role) != 0) {
        return AT_RESULT_CODE_FAIL;
    }

    return AT_RESULT_CODE_OK;
}

/*static int at_query_cmd_ble_addr(int argc, const char **argv)
{
    uint8_t addr[6];

    at_ble_get_public_addr(addr);
    at_response_string("+BLEADDR:\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ble_addr(int argc, const char **argv)
{
    int addr_type = 0;
    char addr_string[18];
    int addr_string_valid = 0;
    uint8_t addr[6];

    AT_CMD_PARSE_NUMBER(0, &addr_type);
    AT_CMD_PARSE_OPT_STRING(1, addr_string, sizeof(addr_string), addr_string_valid);

    if (addr_type != 0 && addr_type != 1) {
        return AT_RESULT_CODE_ERROR;
    }
    if (addr_string_valid) {
        if (get_mac_from_string(addr_string, addr) != 0) {
            return AT_RESULT_CODE_ERROR;
        }
    }

    if (addr_type == 0) {
        at_ble_set_public_addr(addr);
    }
    else if (addr_type == 1) {
        at_ble_set_random_addr(addr);
    }
    return AT_RESULT_CODE_OK;
}*/

static int at_query_cmd_ble_name(int argc, const char **argv)
{
    at_response_string("+BLENAME:%s\r\n", at_ble_config->ble_name);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ble_name(int argc, const char **argv)
{
    char ble_name[32+1];

    AT_CMD_PARSE_STRING(0, ble_name, sizeof(ble_name));

    strlcpy(at_ble_config->ble_name, ble_name, sizeof(at_ble_config->ble_name));
    if (at->store)
        at_ble_config_save(AT_CONFIG_KEY_BLE_NAME);
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_ble_scan_param(int argc, const char **argv)
{
    at_response_string("+BLESCANPARAM:%d,%d,%d,%d,%d\r\n",
            at_ble_config->scan_param.scan_type,
            at_ble_config->scan_param.own_addr_type,
            at_ble_config->scan_param.filter_policy,
            at_ble_config->scan_param.scan_interval,
            at_ble_config->scan_param.scan_window);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ble_scan_param(int argc, const char **argv)
{
    int scan_type = 0;
    int own_addr_type = 0;
    int filter_policy = 0;
    int scan_interval = 0;
    int scan_window = 0;

    AT_CMD_PARSE_NUMBER(0, &scan_type);
    AT_CMD_PARSE_NUMBER(1, &own_addr_type);
    AT_CMD_PARSE_NUMBER(2, &filter_policy);
    AT_CMD_PARSE_NUMBER(3, &scan_interval);
    AT_CMD_PARSE_NUMBER(4, &scan_window);

    if (scan_type != 0 && scan_type != 1)
        return AT_RESULT_CODE_ERROR;
    if (own_addr_type != 0 && own_addr_type != 1 && own_addr_type != 2 && own_addr_type != 3)
        return AT_RESULT_CODE_ERROR;
    if (filter_policy != 0 && filter_policy != 1 && filter_policy != 2 && filter_policy != 3)
        return AT_RESULT_CODE_ERROR;
    if (scan_interval < 0x4 || scan_interval > 0x4000)
        return AT_RESULT_CODE_ERROR;
    if (scan_window < 0x4 || scan_window > 0x4000)
        return AT_RESULT_CODE_ERROR;

    at_ble_config->scan_param.scan_type = scan_type;
    at_ble_config->scan_param.own_addr_type = own_addr_type;
    at_ble_config->scan_param.filter_policy = filter_policy;
    at_ble_config->scan_param.scan_interval = scan_interval;
    at_ble_config->scan_param.scan_window = scan_window;

    return AT_RESULT_CODE_OK;
}

static int at_ble_scan_callback(uint8_t addr_type, uint8_t *addr, int8_t rssi, uint8_t *adv_data, int adv_len)
{
    char ble_scan_result[200] = {0};
    char adv_data_string[65] = {0};
    char *scan_rsp_data_string = "";

    hex_to_string(adv_data, adv_len, adv_data_string, sizeof(adv_data_string));
    snprintf(ble_scan_result, sizeof(ble_scan_result), "+BLESCAN:\"%02x:%02x:%02x:%02x:%02x:%02x\",%d,%s,%s,%d\r\n",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5],
            rssi,
            adv_data_string,
            scan_rsp_data_string,
            addr_type);
    AT_CMD_RESPONSE(ble_scan_result);
}

static int at_setup_cmd_ble_scan(int argc, const char **argv)
{
    int enable = 0;
    int duration_valid = 0, duration = 0;
    int filter_type_valid = 0, filter_type = 0;
    int filter_param_valid = 0, filter_param = 0;

    AT_CMD_PARSE_NUMBER(0, &enable);
    AT_CMD_PARSE_OPT_NUMBER(1, &duration, duration_valid);
    AT_CMD_PARSE_OPT_NUMBER(2, &filter_type, filter_type_valid);
    AT_CMD_PARSE_OPT_NUMBER(3, &filter_param, filter_param_valid);

    if (enable != 0 && enable != 1)
        return AT_RESULT_CODE_ERROR;

    if (at_ble_config->work_role == BLE_DISABLE)
        return AT_RESULT_CODE_ERROR;

    if (enable == 0) {
        at_ble_scan_stop();
    }
    else if (enable == 1) {
        at_ble_scan_start(at_ble_scan_callback);
    }
    
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ble_scan_rsp_data(int argc, const char **argv)
{
    char scan_rsp_data[62 + 1];

    AT_CMD_PARSE_STRING(0, scan_rsp_data, sizeof(scan_rsp_data));

    at_ble_config->scan_rsp_data.len = utils_hex2bin(scan_rsp_data, strlen(scan_rsp_data), at_ble_config->scan_rsp_data.data);
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_ble_adv_param(int argc, const char **argv)
{
    at_response_string("+BLEADVPARAM:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
            at_ble_config->adv_param.adv_int_min, at_ble_config->adv_param.adv_int_max,
            at_ble_config->adv_param.adv_type,
            at_ble_config->adv_param.own_addr_type,
            at_ble_config->adv_param.channel_map,
            at_ble_config->adv_param.adv_filter_policy,
            at_ble_config->adv_param.peer_addr_type,
            at_ble_config->adv_param.peer_addr,
            at_ble_config->adv_param.primary_PHY,
            at_ble_config->adv_param.secondary_PHY);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ble_adv_param(int argc, const char **argv)
{
    int adv_int_min = 0;
    int adv_int_max = 0;
    int adv_type = 0;
    int own_addr_type = 0;
    int channel_map = 0;
    int adv_filter_policy_valid = 0, adv_filter_policy;
    int peer_addr_type_valid = 0, peer_addr_type;
    int peer_addr_valid = 0, peer_addr;
    int primary_PHY_valid = 0, primary_PHY;
    int secondary_PHY_valid = 0, secondary_PHY;

    AT_CMD_PARSE_NUMBER(0, &adv_int_min);
    AT_CMD_PARSE_NUMBER(1, &adv_int_max);
    AT_CMD_PARSE_NUMBER(2, &adv_type);
    AT_CMD_PARSE_NUMBER(3, &own_addr_type);
    AT_CMD_PARSE_NUMBER(4, &channel_map);
    AT_CMD_PARSE_OPT_NUMBER(5, &adv_filter_policy, adv_filter_policy_valid);
    AT_CMD_PARSE_OPT_NUMBER(6, &peer_addr_type, peer_addr_type_valid);
    AT_CMD_PARSE_OPT_NUMBER(7, &peer_addr, peer_addr_valid);
    AT_CMD_PARSE_OPT_NUMBER(8, &primary_PHY, primary_PHY_valid);
    AT_CMD_PARSE_OPT_NUMBER(9, &secondary_PHY, secondary_PHY_valid);

    if (adv_int_min < 0x0020 || adv_int_min > 0x4000)
        return AT_RESULT_CODE_ERROR;
    if (adv_int_max < 0x0020 || adv_int_max > 0x4000)
        return AT_RESULT_CODE_ERROR;
    if (adv_int_min >= adv_int_max)
        return AT_RESULT_CODE_ERROR;
    if (adv_type < 0 || adv_type > 7)
        return AT_RESULT_CODE_ERROR;
    if (own_addr_type != 0 && own_addr_type != 1)
        return AT_RESULT_CODE_ERROR;
    if (channel_map != 1 && channel_map != 2 && channel_map != 4 && channel_map != 7)
        return AT_RESULT_CODE_ERROR;
    if (adv_filter_policy_valid) {
        if (adv_filter_policy != 0 && adv_filter_policy != 1 && adv_filter_policy != 2 && adv_filter_policy != 3)
            return AT_RESULT_CODE_ERROR;
    }
    if (peer_addr_type_valid) {
        if (peer_addr_type != 0 && peer_addr_type != 1)
            return AT_RESULT_CODE_ERROR;
    }
    if (peer_addr_valid) {
    }
    if (primary_PHY_valid) {
        if (primary_PHY != 1 && primary_PHY != 3)
            return AT_RESULT_CODE_ERROR;
    }
    if (secondary_PHY_valid) {
        if (secondary_PHY != 1 && secondary_PHY != 2 && secondary_PHY != 3)
            return AT_RESULT_CODE_ERROR;
    }
    
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ble_adv_data(int argc, const char **argv)
{
    char adv_data[62 + 1];

    AT_CMD_PARSE_STRING(0, adv_data, sizeof(adv_data));

    at_ble_config->adv_data.len = utils_hex2bin(adv_data, strlen(adv_data), at_ble_config->adv_data.data);
    return AT_RESULT_CODE_OK;
}

static int at_exe_cmd_ble_adv_start(int argc, const char **argv)
{
    if (at_ble_adv_start() != 0) {
        return AT_RESULT_CODE_FAIL;
    }

    return AT_RESULT_CODE_OK;
}

static int at_exe_cmd_ble_adv_stop(int argc, const char **argv)
{
    if (at_ble_adv_stop() != 0) {
        return AT_RESULT_CODE_FAIL;
    }

    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_ble_conn(int argc, const char **argv)
{
    if (at_ble_config->work_role != BLE_DISABLE) {
        at_response_string("+BLECONN:\r\n");
    }

    return AT_RESULT_CODE_ERROR;
}

static int at_setup_cmd_ble_conn(int argc, const char **argv)
{
    int conn_index = 0;
    char addr_string[18];
    uint8_t remote_address[6];
    int addr_type_valid = 0, addr_type = 0;
    int timeout_valid = 0, timeout = 5;

    AT_CMD_PARSE_NUMBER(0, &conn_index);
    AT_CMD_PARSE_STRING(1, addr_string, sizeof(addr_string));
    AT_CMD_PARSE_OPT_NUMBER(2, &addr_type_valid, addr_type);
    AT_CMD_PARSE_OPT_NUMBER(3, &timeout_valid, timeout);

    if (conn_index != 0 && conn_index != 1 && conn_index != 2)
        return AT_RESULT_CODE_ERROR;
    if (get_mac_from_string(addr_string, remote_address) != 0) {
        return AT_RESULT_CODE_ERROR;
    }
    if (addr_type_valid) {
        if (addr_type != 0 && addr_type != 1)
            return AT_RESULT_CODE_ERROR;
    }
    if (timeout_valid) {
        if (timeout < 3 || timeout > 30)
            return AT_RESULT_CODE_ERROR;
    }

    if (at_ble_config->work_role != BLE_CLIENT)
        return AT_RESULT_CODE_ERROR;

    at_ble_conn(conn_index, remote_address, addr_type, timeout);
    return AT_RESULT_CODE_ERROR;
}

static int at_query_cmd_ble_conn_param(int argc, const char **argv)
{
    int min_interval = 0;
    int max_interval = 0;
    int cur_interval = 0;
    int latency = 0;
    int timeout = 0;
    int i = 0;
    int connected_num = 0;

    for (i = 0; i < 2; i++) {
        if (at_ble_is_connected(i)) {
            at_ble_conn_get_param(i, &min_interval, &max_interval, &cur_interval, &latency, &timeout);
            at_response_string("+BLECONNPARAM:%d,%d,%d,%d,%d,%d\r\n",
                i, min_interval, max_interval, cur_interval, latency, timeout);
            connected_num++;
        }
    }

    if (connected_num <= 0)
        return AT_RESULT_CODE_ERROR;

    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ble_conn_param(int argc, const char **argv)
{
    int conn_index = 0;
    int min_interval = 0;
    int max_interval = 0;
    int latency = 0;
    int timeout = 0;

    AT_CMD_PARSE_NUMBER(0, &conn_index);
    AT_CMD_PARSE_NUMBER(1, &min_interval);
    AT_CMD_PARSE_NUMBER(2, &max_interval);
    AT_CMD_PARSE_NUMBER(3, &latency);
    AT_CMD_PARSE_NUMBER(4, &timeout);

    if (conn_index != 0 && conn_index != 1 && conn_index != 2)
        return AT_RESULT_CODE_ERROR;
    if (min_interval < 0x6 || min_interval > 0xC80)
        return AT_RESULT_CODE_ERROR;
    if (max_interval < 0x6 || max_interval > 0xC80)
        return AT_RESULT_CODE_ERROR;
    if (min_interval < max_interval)
        return AT_RESULT_CODE_ERROR;
    if (latency < 0x0 || latency > 0x1F3)
        return AT_RESULT_CODE_ERROR;
    if (timeout < 0xA || timeout > 0xC80)
        return AT_RESULT_CODE_ERROR;

    if (!at_ble_is_connected(conn_index))
        return AT_RESULT_CODE_ERROR;
    if (!at_ble_conn_update_param(conn_index, min_interval, max_interval, latency, timeout))
        return AT_RESULT_CODE_ERROR;

    return AT_RESULT_CODE_OK;
}

static int at_exe_cmd_ble_disconn(int argc, const char **argv)
{
    int conn_index = 0;

    AT_CMD_PARSE_NUMBER(0, &conn_index);

    if (conn_index != 0 && conn_index != 1 && conn_index != 2)
        return AT_RESULT_CODE_ERROR;

    if (at_ble_config->work_role == BLE_DISABLE)
        return AT_RESULT_CODE_ERROR;

    if (!at_ble_is_connected(conn_index))
        return AT_RESULT_CODE_ERROR;
    if (!at_ble_disconn(conn_index))
        return AT_RESULT_CODE_ERROR;

    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ble_datalen(int argc, const char **argv)
{
    int conn_index = 0;
    int pkt_data_len = 0;

    AT_CMD_PARSE_NUMBER(0, &conn_index);
    AT_CMD_PARSE_NUMBER(1, &pkt_data_len);

    if (conn_index != 0 && conn_index != 1 && conn_index != 2)
        return AT_RESULT_CODE_ERROR;
    if (pkt_data_len < 0x1B || pkt_data_len > 0xFB)
        return AT_RESULT_CODE_ERROR;

    if (!at_ble_is_connected(conn_index))
        return AT_RESULT_CODE_ERROR;
    if (!at_ble_conn_update_datalen(conn_index, pkt_data_len))
        return AT_RESULT_CODE_ERROR;

    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_ble_cfg_mtu(int argc, const char **argv)
{
    int i = 0;
    int mtu_size = 0;
    int connected_num = 0;

    for (i = 0; i < 2; i++) {
        if (at_ble_is_connected(i)) {
            at_ble_conn_get_mtu(i, &mtu_size);
            at_response_string("+BLECFGMTU:%d,%d,%d\r\n", i, mtu_size);
            connected_num++;
        }
    }

    if (connected_num <= 0)
        return AT_RESULT_CODE_ERROR;

    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ble_cfg_mtu(int argc, const char **argv)
{
    int conn_index = 0;
    int mtu_size = 0;

    AT_CMD_PARSE_NUMBER(0, &conn_index);
    AT_CMD_PARSE_NUMBER(1, &mtu_size);

    if (conn_index != 0 && conn_index != 1 && conn_index != 2)
        return AT_RESULT_CODE_ERROR;
    if (mtu_size < 23 || mtu_size > 517)
        return AT_RESULT_CODE_ERROR;

    if (!at_ble_is_connected(conn_index))
        return AT_RESULT_CODE_ERROR;
    if (!at_ble_conn_update_mtu(conn_index, mtu_size))
        return AT_RESULT_CODE_ERROR;

    return AT_RESULT_CODE_OK;
}

static const at_cmd_struct at_ble_cmd[] = {
    {"+BLEINIT", NULL, at_query_cmd_ble_init, at_setup_cmd_ble_init, NULL, 1, 1},
    //{"+BLEADDR", NULL, at_query_cmd_ble_addr, at_setup_cmd_ble_addr, NULL, 1, 2},
    {"+BLENAME", NULL, at_query_cmd_ble_name, at_setup_cmd_ble_name, NULL, 1, 1},
    {"+BLESCANPARAM", NULL, at_query_cmd_ble_scan_param, at_setup_cmd_ble_scan_param, NULL, 5, 5},
    {"+BLESCAN", NULL, NULL, at_setup_cmd_ble_scan, NULL, 1, 4},
    {"+BLESCANRSPDATA", NULL, NULL, at_setup_cmd_ble_scan_rsp_data, NULL, 1, 1},
    {"+BLEADVPARAM", NULL, at_query_cmd_ble_adv_param, at_setup_cmd_ble_adv_param, NULL, 5, 10},
    {"+BLEADVDATA", NULL, NULL, at_setup_cmd_ble_adv_data, NULL, 1, 1},
    {"+BLEADVSTART", NULL, NULL, NULL, at_exe_cmd_ble_adv_start, 0, 0},
    {"+BLEADVSTOP", NULL, NULL, NULL, at_exe_cmd_ble_adv_stop, 0, 0},
    {"+BLECONN", NULL, at_query_cmd_ble_conn, at_setup_cmd_ble_conn, NULL, 2, 4},
    {"+BLECONNPARAM", NULL, at_query_cmd_ble_conn_param, at_setup_cmd_ble_conn_param, NULL, 5, 5},
    {"+BLEDISCONN", NULL, NULL, NULL, at_exe_cmd_ble_disconn, 0, 0},
    {"+BLEDATALEN", NULL, NULL, at_setup_cmd_ble_datalen, NULL, 2, 2},
    {"+BLECFGMTU", NULL, at_query_cmd_ble_cfg_mtu, at_setup_cmd_ble_cfg_mtu, NULL, 2, 2},
    /*{"+BLEGATTSNTFY", NULL, NULL, NULL, NULL, 0, 0},
    {"+BLEGATTSIND", NULL, NULL, NULL, NULL, 0, 0},*/
};

bool at_ble_cmd_regist(void)
{
    at_ble_config_init();

    at_register_function(at_ble_config_default, at_ble_stop);
    
    if (at_cmd_register(at_ble_cmd, sizeof(at_ble_cmd) / sizeof(at_ble_cmd[0])) == 0)
        return true;
    else
        return false;
}

