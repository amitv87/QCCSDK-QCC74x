/**
  ******************************************************************************
  * @file    at_net_cmd.c
  * @version V1.0
  * @date
  * @brief   This file is part of AT command framework
  ******************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include <aos/kernel.h>
//#include <wifi_mgmr_ext.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
//#include <ping.h>
#include <at_net_ping.h>
#include <time.h>
//#include <utils_time.h>

#include "at_main.h"
#include "at_core.h"
#include "at_base_config.h"
#include "at_wifi_config.h"
#include "at_net_main.h"
#include "at_net_config.h"
#include "at_through.h"

#define AT_NET_CMD_PRINTF printf

/*static int string_is_valid_ip(char *string)
{
    uint32_t ipaddr;

    ipaddr = ipaddr_addr(string);
    if (ipaddr == IPADDR_NONE || ipaddr == IPADDR_ANY) {
        return 0;
    }

    return 1;
}*/

static int string_host_to_ip(char *host, uint32_t *ip)
{
    struct hostent *hostinfo = gethostbyname(host);
    if (hostinfo) {
        *ip = (uint32_t)((struct in_addr *) hostinfo->h_addr)->s_addr;
        return 0;
    }

    AT_NET_CMD_PRINTF("unknown host: %s\r\n", host);
    return -1;
}

static int at_exe_cmd_cifsr(int argc, const char **argv)
{
    ip4_addr_t sta_addr = {0}, sta_gw = {0}, sta_mask = {0}, dns = {0};
    ip4_addr_t ap_addr;

    if (at_wifi_config->dhcp_state.bit.sta_dhcp == 0) {
        sta_addr.addr = at_wifi_config->sta_ip.ip;
    } else {
        wifi_sta_ip4_addr_get(&sta_addr.addr, &sta_gw.addr, &sta_mask.addr, &dns.addr);
    }

    ap_addr.addr = at_wifi_config->ap_ip.ip;

    if (at_wifi_config->wifi_mode == WIFI_STATION_MODE || at_wifi_config->wifi_mode == WIFI_AP_STA_MODE) {
        at_response_string("+CIFSR:%s,\"%s\"\r\n", "STAIP", ip4addr_ntoa(&sta_addr));
        at_response_string("+CIFSR:%s,\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n", "STAMAC",
            at_wifi_config->sta_mac.addr[0],
            at_wifi_config->sta_mac.addr[1],
            at_wifi_config->sta_mac.addr[2],
            at_wifi_config->sta_mac.addr[3],
            at_wifi_config->sta_mac.addr[4],
            at_wifi_config->sta_mac.addr[5]);
    }

    if (at_wifi_config->wifi_mode == WIFI_SOFTAP_MODE || at_wifi_config->wifi_mode == WIFI_AP_STA_MODE) {
        at_response_string("+CIFSR:%s,\"%s\"\r\n", "APIP", ip4addr_ntoa(&ap_addr));
        at_response_string("+CIFSR:%s,\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n", "APMAC",
            at_wifi_config->ap_mac.addr[0],
            at_wifi_config->ap_mac.addr[1],
            at_wifi_config->ap_mac.addr[2],
            at_wifi_config->ap_mac.addr[3],
            at_wifi_config->ap_mac.addr[4],
            at_wifi_config->ap_mac.addr[5]);
    }
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipv6(int argc, const char **argv)
{
    return AT_RESULT_CODE_ERROR;
}

static int at_setup_cmd_cipv6(int argc, const char **argv)
{
    return AT_RESULT_CODE_ERROR;
}

static int at_query_cmd_cipdns(int argc, const char **argv)
{
    return AT_RESULT_CODE_ERROR;
}

static int at_setup_cmd_cipdns(int argc, const char **argv)
{
    return AT_RESULT_CODE_ERROR;
}

static int at_setup_cmd_cipdomain(int argc, const char **argv)
{
    char hostname[128];
    int ip_network_valid = 0, ip_network;
    struct hostent *hostinfo;

    AT_CMD_PARSE_STRING(0, hostname, sizeof(hostname));
    AT_CMD_PARSE_OPT_NUMBER(1, &ip_network, ip_network_valid);

    if (ip_network_valid && (ip_network != 1 && ip_network != 2)) {
        return AT_RESULT_CODE_ERROR;
    }

    hostinfo = gethostbyname(hostname);
    if (!hostinfo) {
        return AT_RESULT_CODE_FAIL;
    }

    at_response_string("+CIPDOMAIN:\"%s\"\r\n", ipaddr_ntoa((ip_addr_t *)hostinfo->h_addr));
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipstate(int argc, const char **argv)
{
    char type[8];
    uint32_t remote_ip;
    uint16_t remote_port, local_port;
    uint8_t tetype;
    ip4_addr_t ipaddr;
    int i;

    for (i = 0; i < AT_NET_CLIENT_HANDLE_MAX; i++) {
        if (at_net_client_get_info(i, type, &remote_ip, &remote_port, &local_port, &tetype) == 0) {
            ipaddr.addr = remote_ip;
            at_response_string("+CIPSTATUS:%d,\"%s\",\"%s\",%d,%d,%d\r\n", i, type, ip4addr_ntoa(&ipaddr), remote_port, local_port, tetype);
        }
    }
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipstart(int argc, const char **argv)
{
    char type[8];
    uint32_t remote_ip;
    uint16_t remote_port, local_port;
    uint8_t tetype;
    ip4_addr_t ipaddr;
    int i;

    for (i = 0; i < AT_NET_CLIENT_HANDLE_MAX; i++) {
        if (at_net_client_get_info(i, type, &remote_ip, &remote_port, &local_port, &tetype) == 0) {
            ipaddr.addr = remote_ip;
            at_response_string("+CIPSTART:%d,\"%s\",\"%s\",%d,%d,%d\r\n", i, type, ip4addr_ntoa(&ipaddr), remote_port, local_port, tetype);
        }
    }
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipstart(int argc, const char **argv)
{
    int linkid = 0;
    char type[8];
    char remote_host[128];
    int remote_port;
    int keepalive_valid = 0, keepalive = 0;
    int local_port_valid = 0, local_port = 0;
    int mode_valid = 0, mode = 0;
    char local_ip[16];
    int local_ip_valid = 0;
    uint32_t remote_ip = 0;
    int argc_index = 0, ret = 0;

    if (at_net_config->mux_mode == NET_LINK_MULT) {
        AT_CMD_PARSE_NUMBER(argc_index, &linkid);
        argc_index++;
    }

    AT_CMD_PARSE_STRING(argc_index, type, sizeof(type));
    argc_index++;
    AT_CMD_PARSE_STRING(argc_index, remote_host, sizeof(remote_host));
    argc_index++;
    AT_CMD_PARSE_NUMBER(argc_index, &remote_port);
    argc_index++;

    if (!at_net_client_id_is_valid(linkid)) {
        return AT_RESULT_CODE_ERROR;
    }
    if (string_host_to_ip(remote_host, &remote_ip) != 0) {
        return AT_RESULT_CODE_ERROR;
    }
    if (remote_port < 1 || remote_port > 65535) {
        return AT_RESULT_CODE_ERROR; 
    }
    if (strcasecmp(type, "TCP") == 0) {
        AT_CMD_PARSE_OPT_NUMBER(argc_index, &keepalive, keepalive_valid);
        argc_index++;
        AT_CMD_PARSE_OPT_STRING(argc_index, local_ip, sizeof(local_ip), local_ip_valid);
        argc_index++;
    } else if (strcasecmp(type, "UDP") == 0) {
        AT_CMD_PARSE_OPT_NUMBER(argc_index, &local_port, local_port_valid);
        argc_index++;
        AT_CMD_PARSE_OPT_NUMBER(argc_index, &mode, mode_valid);
        argc_index++;
        AT_CMD_PARSE_OPT_NUMBER(argc_index, &keepalive, keepalive_valid);
        argc_index++;
        AT_CMD_PARSE_OPT_STRING(argc_index, local_ip, sizeof(local_ip), local_ip_valid);
        argc_index++;
    } else if (strcasecmp(type, "SSL") == 0) {
        AT_CMD_PARSE_OPT_NUMBER(argc_index, &keepalive, keepalive_valid);
        argc_index++;
        AT_CMD_PARSE_OPT_STRING(argc_index, local_ip, sizeof(local_ip), local_ip_valid);
        argc_index++;
    } else {
        return AT_RESULT_CODE_ERROR;
    }
    if (keepalive_valid && (keepalive < 0 || keepalive > 7200)) {
        return AT_RESULT_CODE_ERROR;
    }
    if (local_port_valid && (local_port < 0 || local_port > 65535)) {
        return AT_RESULT_CODE_ERROR;
    }
    if (mode_valid && (mode < 0 || mode > 2)) {
        return AT_RESULT_CODE_ERROR;
    }

    if (strcasecmp(type, "TCP") == 0) {
        ret = at_net_client_tcp_connect(linkid, remote_ip, (uint16_t)remote_port, keepalive);
    } else if (strcasecmp(type, "UDP") == 0) {
        ret = at_net_client_udp_connect(linkid, remote_ip, (uint16_t)remote_port, (uint16_t)local_port, mode);
    } else if (strcasecmp(type, "SSL") == 0) {
        ret = at_net_client_ssl_connect(linkid, remote_ip, (uint16_t)remote_port, keepalive);
    }

    if (ret != 0) {
        return AT_RESULT_CODE_FAIL;
    }

    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipstartex(int argc, const char **argv)
{
    int linkid = 0;
    char type[8];
    char remote_host[128];
    int remote_port;
    int keepalive_valid = 0, keepalive = 0;
    int local_port_valid = 0, local_port = 0;
    int mode_valid = 0, mode = 0;
    char local_ip[16];
    int local_ip_valid = 0;
    uint32_t remote_ip = 0;
    int argc_index = 0, ret = 0;

    linkid = at_net_client_get_valid_id();
    if (linkid < 0 || (at_net_config->mux_mode == NET_LINK_SINGLE && linkid != 0))
        return AT_RESULT_CODE_ERROR;

    AT_CMD_PARSE_STRING(argc_index, type, sizeof(type));
    argc_index++;
    AT_CMD_PARSE_STRING(argc_index, remote_host, sizeof(remote_host));
    argc_index++;
    AT_CMD_PARSE_NUMBER(argc_index, &remote_port);
    argc_index++;

    if (!at_net_client_id_is_valid(linkid)) {
        return AT_RESULT_CODE_ERROR;
    }
    if (string_host_to_ip(remote_host, &remote_ip) != 0) {
        return AT_RESULT_CODE_ERROR;
    }
    if (remote_port < 1 || remote_port > 65535) {
        return AT_RESULT_CODE_ERROR; 
    }
    if (strcasecmp(type, "TCP") == 0) {
        AT_CMD_PARSE_OPT_NUMBER(argc_index, &keepalive, keepalive_valid);
        argc_index++;
        AT_CMD_PARSE_OPT_STRING(argc_index, local_ip, sizeof(local_ip), local_ip_valid);
        argc_index++;
    } else if (strcasecmp(type, "UDP") == 0) {
        AT_CMD_PARSE_OPT_NUMBER(argc_index, &local_port, local_port_valid);
        argc_index++;
        AT_CMD_PARSE_OPT_NUMBER(argc_index, &mode, mode_valid);
        argc_index++;
        AT_CMD_PARSE_OPT_NUMBER(argc_index, &keepalive, keepalive_valid);
        argc_index++;
        AT_CMD_PARSE_OPT_STRING(argc_index, local_ip, sizeof(local_ip), local_ip_valid);
        argc_index++;
    } else if (strcasecmp(type, "SSL") == 0) {
        AT_CMD_PARSE_OPT_NUMBER(argc_index, &keepalive, keepalive_valid);
        argc_index++;
        AT_CMD_PARSE_OPT_STRING(argc_index, local_ip, sizeof(local_ip), local_ip_valid);
        argc_index++;
    } else {
        return AT_RESULT_CODE_ERROR;
    }
    if (keepalive_valid && (keepalive < 0 || keepalive > 7200)) {
        return AT_RESULT_CODE_ERROR;
    }
    if (local_port_valid && (local_port < 0 || local_port > 65535)) {
        return AT_RESULT_CODE_ERROR;
    }
    if (mode_valid && (mode < 0 || mode > 2)) {
        return AT_RESULT_CODE_ERROR;
    }

    if (strcasecmp(type, "TCP") == 0) {
        ret = at_net_client_tcp_connect(linkid, remote_ip, (uint16_t)remote_port, keepalive);
    } else if (strcasecmp(type, "UDP") == 0) {
        ret = at_net_client_udp_connect(linkid, remote_ip, (uint16_t)remote_port, (uint16_t)local_port, mode);
    } else if (strcasecmp(type, "SSL") == 0) {
        ret = at_net_client_ssl_connect(linkid, remote_ip, (uint16_t)remote_port, keepalive);
    }

    if (ret != 0) {
        return AT_RESULT_CODE_FAIL;
    }

    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_ciptcport(int argc, const char **argv)
{
    int i;

    for (i = 0; i < sizeof(at_net_config->tcp_opt)/sizeof(net_tcp_opt); i++) {
        at_response_string("+CIPTCPOPT:%d,%d,%d,%d\r\n", i,
                at_net_config->tcp_opt[i].so_linger,
                at_net_config->tcp_opt[i].tcp_nodelay,
                at_net_config->tcp_opt[i].so_sndtimeo);
    }
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ciptcport(int argc, const char **argv)
{
    int linkid = 0;
    int so_linger_valid = 0, so_linger;
    int tcp_nodelay_valid = 0, tcp_nodelay;
    int so_sndtimeo_valid = 0, so_sndtimeo;
    int argc_index = 0;

    if (at_net_config->mux_mode == NET_LINK_MULT) {
        AT_CMD_PARSE_NUMBER(argc_index, &linkid);
        argc_index++;
    }

    AT_CMD_PARSE_OPT_NUMBER(argc_index, &so_linger, so_linger_valid);
    argc_index++;
    AT_CMD_PARSE_OPT_NUMBER(argc_index, &tcp_nodelay, tcp_nodelay_valid);
    argc_index++;
    AT_CMD_PARSE_OPT_NUMBER(argc_index, &so_sndtimeo, so_sndtimeo_valid);
    argc_index++;

    if (!at_net_client_id_is_valid(linkid)) {
        return AT_RESULT_CODE_ERROR;
    }
    if (so_linger_valid) {
        if (so_linger < 0 && so_linger != -1) {
            return AT_RESULT_CODE_ERROR;
        }
        at_net_config->tcp_opt[linkid].so_linger = so_linger;
    }
    if (tcp_nodelay_valid) {
        if (tcp_nodelay != 0 && tcp_nodelay != 1) {
            return AT_RESULT_CODE_ERROR;
        }
        at_net_config->tcp_opt[linkid].tcp_nodelay = tcp_nodelay;
    }
    if (so_sndtimeo_valid) {
        if (so_sndtimeo < 0) {
            return AT_RESULT_CODE_ERROR;
        }
        at_net_config->tcp_opt[linkid].so_sndtimeo = so_sndtimeo;
    }

    if (at->store) {
        at_net_config_save(AT_CONFIG_KEY_NET_TCP_OPT);
    }
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipclose(int argc, const char **argv)
{
    int linkid;

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
        return AT_RESULT_CODE_ERROR;
    }

    AT_CMD_PARSE_NUMBER(0, &linkid);

    if (at_net_client_id_is_valid(linkid)) {
        if (at_net_client_close(linkid) != 0) {
            return AT_RESULT_CODE_FAIL;
        }
    } else if (linkid == AT_NET_CLIENT_HANDLE_MAX) {
        at_net_client_close_all();
    } else {
        return AT_RESULT_CODE_ERROR;
    }

    return AT_RESULT_CODE_OK;
}

static int at_exe_cmd_cipclose(int argc, const char **argv)
{
    int linkid = 0;

    if (at_net_config->mux_mode == NET_LINK_MULT) {
        return AT_RESULT_CODE_ERROR;
    }

    if (at_net_client_close(linkid) != 0) {
        return AT_RESULT_CODE_FAIL;
    }

    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipsend(int argc, const char **argv)
{
    int linkid = 0;
    int length;
    char remote_host[128];
    int remote_port = 0;
    int remote_host_valid = 0;
    int remote_port_valid = 0;
    uint32_t remote_ip = 0;
    int recv_num = 0;
    int send_num = 0;

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
        AT_CMD_PARSE_NUMBER(0, &length);
        AT_CMD_PARSE_OPT_STRING(1, remote_host, sizeof(remote_host), remote_host_valid);
        AT_CMD_PARSE_OPT_NUMBER(2, &remote_port, remote_port_valid);
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
        AT_CMD_PARSE_NUMBER(1, &length);
        AT_CMD_PARSE_OPT_STRING(2, remote_host, sizeof(remote_host), remote_host_valid);
        AT_CMD_PARSE_OPT_NUMBER(3, &remote_port, remote_port_valid);
    }

    if (!at_net_client_id_is_valid(linkid)) {
        return AT_RESULT_CODE_ERROR;
    }
    if (length <= 0 || length > 8192) {
        return AT_RESULT_CODE_ERROR;
    }
    if (remote_host_valid) {
        if (string_host_to_ip(remote_host, &remote_ip) != 0) {
            return AT_RESULT_CODE_ERROR;
        }
    }
    if (remote_port_valid && (remote_port_valid <= 0 || remote_port_valid > 65535)) {
        return AT_RESULT_CODE_ERROR;
    }

    if (!at_net_client_is_connected(linkid)) {
        return AT_RESULT_CODE_FAIL;
    }
    if (remote_host_valid) {
        if (at_net_client_set_remote(linkid, remote_ip, (uint16_t)remote_port) != 0) {
            return AT_RESULT_CODE_FAIL;
        }
    }

    char *buffer = (char *)pvPortMalloc(length);
    if (!buffer) {
        AT_NET_CMD_PRINTF("malloc %d bytes failed\r\n", length);
        return AT_RESULT_CODE_FAIL;
    }
    at_response_result(AT_RESULT_CODE_OK);

    AT_CMD_RESPONSE(AT_CMD_MSG_WAIT_DATA);
    while(recv_num < length) {
        recv_num += AT_CMD_DATA_RECV(buffer + recv_num, length - recv_num);
    }
    at_response_string("Recv %d bytes\r\n", recv_num);

    send_num = at_net_client_send(linkid, buffer, recv_num);
    vPortFree(buffer);

    if (send_num == recv_num) {
        return AT_RESULT_CODE_SEND_OK;
    } else {
        return AT_RESULT_CODE_FAIL;
    }
}

static int at_exe_cmd_cipsend(int argc, const char **argv)
{
    int linkid = 0;

    //if (at_net_config->work_mode != NET_MODE_TRANS) {
    //    return AT_RESULT_CODE_ERROR;
    //}
    //if (at_net_config->mux_mode != NET_LINK_SINGLE) {
    //    return AT_RESULT_CODE_ERROR;
    //}

    //if (!at_net_client_is_connected(linkid)) {
    //    return AT_RESULT_CODE_ERROR;
    //}

    printf("at_set_work_mode AT_WORK_MODE_THROUGHPUT\r\n");
    at_set_work_mode(AT_WORK_MODE_THROUGHPUT);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipsendl(int argc, const char **argv)
{
    int linkid = 0;
    int length;
    char remote_host[128];
    int remote_port = 0;
    int remote_host_valid = 0;
    int remote_port_valid = 0;
    uint32_t remote_ip = 0;

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
        AT_CMD_PARSE_NUMBER(0, &length);
        AT_CMD_PARSE_OPT_STRING(1, remote_host, sizeof(remote_host), remote_host_valid);
        AT_CMD_PARSE_OPT_NUMBER(2, &remote_port, remote_port_valid);
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
        AT_CMD_PARSE_NUMBER(1, &length);
        AT_CMD_PARSE_OPT_STRING(2, remote_host, sizeof(remote_host), remote_host_valid);
        AT_CMD_PARSE_OPT_NUMBER(3, &remote_port, remote_port_valid);
    }

    if (!at_net_client_id_is_valid(linkid)) {
        return AT_RESULT_CODE_ERROR;
    }
    if (length <= 0) {
        return AT_RESULT_CODE_ERROR;
    }
    if (remote_host_valid) {
        if (string_host_to_ip(remote_host, &remote_ip) != 0) {
            return AT_RESULT_CODE_ERROR;
        }
    }
    if (remote_port_valid && (remote_port_valid <= 0 || remote_port_valid > 65535)) {
        return AT_RESULT_CODE_ERROR;
    }

    if (!at_net_client_is_connected(linkid)) {
        return AT_RESULT_CODE_FAIL;
    }
    if (remote_host_valid) {
        if (at_net_client_set_remote(linkid, remote_ip, (uint16_t)remote_port) != 0) {
            return AT_RESULT_CODE_FAIL;
        }
    }

    at_through_set_length(linkid, length, at_net_config->sendl_cfg.report_size, at_net_config->sendl_cfg.transmit_size);
    at_set_work_mode(AT_WORK_MODE_CMD_THROUGHPUT);
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipsendlcfg(int argc, const char **argv)
{
    at_response_string("+CIPSENDLCFG:%d,%d\r\n", at_net_config->sendl_cfg.report_size, at_net_config->sendl_cfg.transmit_size);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipsendlcfg(int argc, const char **argv)
{
    int report_size;
    int transmit_size;

    AT_CMD_PARSE_NUMBER(0, &report_size);
    AT_CMD_PARSE_NUMBER(1, &transmit_size);

    if (report_size < 100 || report_size > 65535) {
        return AT_RESULT_CODE_ERROR;
    }
    if (transmit_size < 100 || transmit_size > 2920) {
        return AT_RESULT_CODE_ERROR;
    }

    at_net_config->sendl_cfg.report_size = (uint16_t)report_size;
    at_net_config->sendl_cfg.transmit_size = (uint16_t)transmit_size;
    return AT_RESULT_CODE_OK;
}

static int at_senddata_parse(char *data, int length, int *index, int *finish)
{
    int i;
    int flag_slash = 0;

    for (i = *index; i < length; i++) {
        if (data[i] == 0x30 && flag_slash == 1) {
            *index = 0;
            *finish = 1;
            return i - 1;
        }

        if (flag_slash) {
            memmove(data + (i - 1), data + i, length - i);
            length--;
            i--;
            flag_slash = 0;
            *index = i;
        } else {
            if (data[i] == 0x5C) {
                flag_slash = 1;
            } else {
                flag_slash = 0;
            }
        }
    }

    return length;
}

static int at_setup_cmd_cipsendex(int argc, const char **argv)
{
    int linkid = 0;
    int length;
    char remote_host[128];
    int remote_port = 0;
    int remote_host_valid = 0;
    int remote_port_valid = 0;
    uint32_t remote_ip = 0;
    int recv_num = 0;
    int send_num = 0;
    int index = 0, finish = 0;
    int ret;

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
        AT_CMD_PARSE_NUMBER(0, &length);
        AT_CMD_PARSE_OPT_STRING(1, remote_host, sizeof(remote_host), remote_host_valid);
        AT_CMD_PARSE_OPT_NUMBER(2, &remote_port, remote_port_valid);
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
        AT_CMD_PARSE_NUMBER(1, &length);
        AT_CMD_PARSE_OPT_STRING(2, remote_host, sizeof(remote_host), remote_host_valid);
        AT_CMD_PARSE_OPT_NUMBER(3, &remote_port, remote_port_valid);
    }

    if (!at_net_client_id_is_valid(linkid)) {
        return AT_RESULT_CODE_ERROR;
    }
    if (length <= 0 || length > 8192) {
        return AT_RESULT_CODE_ERROR;
    }
    if (remote_host_valid) {
        if (string_host_to_ip(remote_host, &remote_ip) != 0) {
            return AT_RESULT_CODE_ERROR;
        }
    }
    if (remote_port_valid && (remote_port_valid <= 0 || remote_port_valid > 65535)) {
        return AT_RESULT_CODE_ERROR;
    }

    if (!at_net_client_is_connected(linkid)) {
        return AT_RESULT_CODE_FAIL;
    }
    if (remote_host_valid) {
        if (at_net_client_set_remote(linkid, remote_ip, (uint16_t)remote_port) != 0) {
            return AT_RESULT_CODE_FAIL;
        }
    }

    char *buffer = (char *)pvPortMalloc(length);
    if (!buffer) {
        AT_NET_CMD_PRINTF("malloc %d bytes failed\r\n", length);
        return AT_RESULT_CODE_FAIL;
    }
    at_response_result(AT_RESULT_CODE_OK);

    AT_CMD_RESPONSE(AT_CMD_MSG_WAIT_DATA);
    while(recv_num < length) {
        ret = AT_CMD_DATA_RECV(buffer + recv_num, length - recv_num);
        if (ret > 0) {
            recv_num += ret;
            recv_num = at_senddata_parse(buffer, recv_num, &index, &finish);
            if (finish)
                break;
        }
    }
    at_response_string("Recv %d bytes\r\n", recv_num);

    send_num = at_net_client_send(linkid, buffer, recv_num);
    vPortFree(buffer);

    if (send_num != recv_num) {
        return AT_RESULT_CODE_FAIL;
    }

    return AT_RESULT_CODE_SEND_OK;
}

static int at_query_cmd_cipdinfo(int argc, const char **argv)
{
    if (at_net_config->ipd_info == NET_IPDINFO_DISABLE_IPPORT) {
        at_response_string("+CIPDINFO:%s", "false");
    } else {
        at_response_string("+CIPDINFO:%s", "true");
    }
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipdinfo(int argc, const char **argv)
{
    int enable;

    AT_CMD_PARSE_NUMBER(0, &enable);
    if (enable != NET_IPDINFO_DISABLE_IPPORT && enable != NET_IPDINFO_ENABLE_IPPORT) {
        return AT_RESULT_CODE_ERROR;
    }

    at_net_config->ipd_info = (net_ipd_info)enable;
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_wips(int argc, const char **argv)
{
    int enable;

    AT_CMD_PARSE_NUMBER(0, &enable);

    at_net_config->wips_enable = (uint8_t)enable;
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_wips(int argc, const char **argv)
{
    at_response_string("+WIPS:%d\r\n", at_net_config->wips_enable);
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipmux(int argc, const char **argv)
{
    at_response_string("+CIPMUX:%d\r\n", at_net_config->mux_mode);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipmux(int argc, const char **argv)
{
    int mode;

    AT_CMD_PARSE_NUMBER(0, &mode);
    if(mode != NET_LINK_SINGLE && mode != NET_LINK_MULT) {
        return AT_RESULT_CODE_ERROR;
    }

    if (at_net_sock_is_build()) {
        AT_NET_CMD_PRINTF("link is builded\r\n");
        return AT_RESULT_CODE_ERROR;
    }

    at_net_config->mux_mode = mode;
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_ciprecvmode(int argc, const char **argv)
{
    at_response_string("+CIPRECVMODE::%d\r\n", at_net_config->recv_mode);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ciprecvmode(int argc, const char **argv)
{
    int mode;

    AT_CMD_PARSE_NUMBER(0, &mode);
    if(mode != NET_RECV_MODE_ACTIVE && mode != NET_RECV_MODE_PASSIVE) {
        return AT_RESULT_CODE_ERROR;
    }

    at_net_config->recv_mode = mode;
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ciprecvdata(int argc, const char **argv)
{
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ciprecvbuf(int argc, const char **argv)
{
    int linkid = 0, size;
    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
        AT_CMD_PARSE_NUMBER(0, &size);
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
        AT_CMD_PARSE_NUMBER(1, &size);
    }

    if (!at_net_client_id_is_valid(linkid)) {
        return AT_RESULT_CODE_ERROR;
    }

    if (at_net_client_is_connected(linkid)) {
        return AT_RESULT_CODE_FAIL;
    }
    at_net_recvbuf_size_set(linkid, size);
    return AT_RESULT_CODE_OK;
}


static int at_query_cmd_ciprecvbuf(int argc, const char **argv)
{
    int linkid = 0, linkid_valid = 0;
    
    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
        AT_CMD_PARSE_OPT_NUMBER(0, &linkid, linkid_valid);
        if (linkid_valid) {
            return AT_RESULT_CODE_ERROR;
        }
    } else {
    	if (argc <= 0) {
    		return AT_RESULT_CODE_ERROR;
    	}
        AT_CMD_PARSE_NUMBER(0, &linkid);
    }

    if (!at_net_client_id_is_valid(linkid)) {
        return AT_RESULT_CODE_ERROR;
    }

    at_response_string("+CIPRECVBUF:%d\r\n", at_net_recvbuf_size_get(linkid));
    return AT_RESULT_CODE_OK;
}
static int at_query_cmd_ciprecvlen(int argc, const char **argv)
{
    int id = 0;
    char buf[32] = {0};

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
        at_response_string("+CIPRECVLEN:%d\r\n", at_net_client_get_recvsize(id));
    } else {
        for (id = 0; id < AT_NET_CLIENT_HANDLE_MAX; id++) {
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%d,", at_net_client_get_recvsize(id));
        }
        if (strlen(buf) > 0) {
            buf[strlen(buf)] = '\0';
        }
        at_response_string("+CIPRECVLEN:%s\r\n", buf);
    }
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipserver(int argc, const char **argv)
{
    uint16_t port;
    char type[8];
    int ca_enable;

    if (at_net_server_is_created(&port, type, &ca_enable)) {
        at_response_string("+CIPSERVER:%d,%d,\"%s\",%d\r\n", 1, port, type, ca_enable);
    } else {
        at_response_string("+CIPSERVER:%d\r\n", 0);
    }

    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipserver(int argc, const char **argv)
{
    int mode;
    int param_valid = 0, param = 0;
    int port;
    char type[8];
    int type_valid = 0;
    int ca_enable_valid = 0, ca_enable = 0;
    int ret = 0;

    if (at_net_config->mux_mode != NET_LINK_MULT) {
        return AT_RESULT_CODE_ERROR;
    }

    AT_CMD_PARSE_NUMBER(0, &mode);
    if (mode == 0) {
        AT_CMD_PARSE_OPT_NUMBER(1, &param, param_valid);

        if (param == 0) {
            ret = at_net_server_close();
        } else if (param == 1) {
            ret = at_net_server_close();
            ret |= at_net_client_close_all();
        } else {
            return AT_RESULT_CODE_ERROR;
        }
    } else if (mode == 1) {
        if (argc < 2) {
            return AT_RESULT_CODE_ERROR;
        }

        AT_CMD_PARSE_NUMBER(1, &port);
        AT_CMD_PARSE_OPT_STRING(2, type, sizeof(type), type_valid);
        if (!type_valid) {
            strlcpy(type, "TCP", sizeof(type));
        }
        AT_CMD_PARSE_OPT_NUMBER(3, &ca_enable, ca_enable_valid);

        if (port <= 0 || port > 65535) {
            return AT_RESULT_CODE_ERROR;
        }
        if (ca_enable != 0 && ca_enable != 1) {
            return AT_RESULT_CODE_ERROR;
        }

        if (strcasecmp(type, "TCP") == 0) {
            ret = at_net_server_tcp_create((uint16_t)port, at_net_config->server_maxconn, at_net_config->server_timeout);
        } else if (strcasecmp(type, "UDP") == 0) {
            ret = at_net_server_udp_create((uint16_t)port, at_net_config->server_maxconn, at_net_config->server_timeout);
        }
        else  if (strcasecmp(type, "SSL") == 0) {
            ret = at_net_server_ssl_create((uint16_t)port, at_net_config->server_maxconn, at_net_config->server_timeout, ca_enable);
        } else {
            return AT_RESULT_CODE_ERROR;
        }
    } else {
        return AT_RESULT_CODE_ERROR;
    }

    if (ret != 0) {
        return AT_RESULT_CODE_FAIL;
    }
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipservermaxconn(int argc, const char **argv)
{
    at_response_string("+CIPSERVERMAXCONN:%d\r\n", at_net_config->server_maxconn);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipservermaxconn(int argc, const char **argv)
{
    int maxconn = 0;

    AT_CMD_PARSE_NUMBER(0, &maxconn);

    if (maxconn < 1 || maxconn > AT_NET_CLIENT_HANDLE_MAX)
        return AT_RESULT_CODE_ERROR;

    if (at_net_server_is_created(NULL, NULL, NULL))
        return AT_RESULT_CODE_ERROR;

    at_net_config->server_maxconn = maxconn;
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipsslcsni(int argc, const char **argv)
{
    int linkid = 0;

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
        if (!at_net_client_id_is_valid(linkid)) {
            return AT_RESULT_CODE_ERROR;
        }
    }
    at_response_string("+CIPSSLCSNI:%d,%s\r\n", linkid, at_net_ssl_sni_get(linkid));
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipsslcsni(int argc, const char **argv)
{
    int linkid = 0;
    char hostname[255] = {0};

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
        AT_CMD_PARSE_STRING(0, hostname, sizeof(hostname));
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
        if (!at_net_client_id_is_valid(linkid)) {
            return AT_RESULT_CODE_ERROR;
        }
        AT_CMD_PARSE_STRING(1, hostname, sizeof(hostname));
    }
    
    at_net_ssl_sni_set(linkid, hostname);

    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipsslcalpn(int argc, const char **argv)
{
    int i, n, offset = 0;
    int linkid = 0, count = 0;
    char **alpn;
    char output[255] = {0};

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
        if (!at_net_client_id_is_valid(linkid)) {
            return AT_RESULT_CODE_ERROR;
        }
    }
    alpn = at_net_ssl_alpn_get(linkid, &count);


    n = snprintf(output + offset, sizeof(output) - offset, "+CIPSSLCALPN:%d", linkid);
    if (n > 0) {
        offset += n;
    }

    for (i = 0; i < count; i++) {
        n = snprintf(output + offset, sizeof(output) - offset, ",\"%s\"", alpn[i]);
        if (n > 0) {
            offset += n;
        }
    }
    
    n = snprintf(output + offset, sizeof(output) - offset, "%s", "\r\n");
    if (n > 0) {
        offset += n;
    }
    at_response_string(output);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipsslcalpn(int argc, const char **argv)
{
    int i, offset;
    int linkid = 0, count = 0;
    char alpn[255] = {0};

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
        AT_CMD_PARSE_NUMBER(0, &count);
        if (count != argc - 1) {
            return AT_RESULT_CODE_ERROR;
        }
        offset = 1;
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
        if (!at_net_client_id_is_valid(linkid)) {
            return AT_RESULT_CODE_ERROR;
        }
        AT_CMD_PARSE_NUMBER(1, &count);
        if (count != argc - 2) {
            return AT_RESULT_CODE_ERROR;
        }
        offset = 2;
    }

    for (i = 0; i < count; i++) {
        AT_CMD_PARSE_STRING(i + offset, alpn, sizeof(alpn));
        at_net_ssl_alpn_set(linkid, i, alpn);
    }
    for (; i < 6; i++) {
        at_net_ssl_alpn_set(linkid, i, NULL);
    }
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipsslcpsk(int argc, const char **argv)
{
    int linkid = 0;
    char *psk, *hint;
    int psk_len, hint_len;

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
        if (!at_net_client_id_is_valid(linkid)) {
            return AT_RESULT_CODE_ERROR;
        }
    }

    at_net_ssl_psk_get(linkid, &psk, &psk_len, &hint, &hint_len);
    at_response_string("+CIPSSLCPSK:%d,\"%s\",\"%s\"\r\n", linkid, psk, hint);

    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipsslcpsk(int argc, const char **argv)
{
    int linkid = 0;
    char psk[32] = {0};
    char hint[32] = {0};

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
        AT_CMD_PARSE_STRING(0, psk, sizeof(psk));
        AT_CMD_PARSE_STRING(1, hint, sizeof(hint));
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
        if (!at_net_client_id_is_valid(linkid)) {
            return AT_RESULT_CODE_ERROR;
        }
        AT_CMD_PARSE_STRING(1, psk, sizeof(psk));
        AT_CMD_PARSE_STRING(2, hint, sizeof(hint));
    }
    at_net_ssl_psk_set(linkid, psk, strlen(psk), hint, strlen(hint));
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipsslcpskhex(int argc, const char **argv)
{
    int linkid = 0;
    char *psk, *hint;
    int psk_len, hint_len;
    char output[128];
    int offset = 0, n;

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
        if (!at_net_client_id_is_valid(linkid)) {
            return AT_RESULT_CODE_ERROR;
        }
    }

    at_net_ssl_psk_get(linkid, &psk, &psk_len, &hint, &hint_len);

    n = snprintf(output + offset, sizeof(output) - offset, "+CIPSSLCPSKHEX:%d,\"", linkid);
    if (n > 0) {
        offset += n;
    }

    for (int i = 0; i < psk_len; i++) {
        n = snprintf(output + offset, sizeof(output) - offset, "%02x", psk[i]);
        if (n > 0) {
            offset += n;
        }
    }

    n = snprintf(output + offset, sizeof(output) - offset, "\",\"%s\"\r\n", hint);
    if (n > 0) {
        offset += n;
    }

    at_response_string(output);

    return AT_RESULT_CODE_OK;
}

static int _str_to_hex(const char *str, uint8_t *hex)
{
    int i;
    int len = 0;

    len = strlen(str) / 2;

    for (i = 0; i < len; i++) {
        sscanf(str, "%02x", &hex[i]);
        str += 2;
    }
    
    return len;
}

static int at_setup_cmd_cipsslcpskhex(int argc, const char **argv)
{
    int linkid = 0, psk_len;
    char psk[64] = {0};
    char psk_hex[32] = {0};
    char hint[32] = {0};

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
        AT_CMD_PARSE_STRING(0, psk, sizeof(psk));
        AT_CMD_PARSE_STRING(1, hint, sizeof(hint));
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
        if (!at_net_client_id_is_valid(linkid)) {
            return AT_RESULT_CODE_ERROR;
        }
        AT_CMD_PARSE_STRING(1, psk, sizeof(psk));
        AT_CMD_PARSE_STRING(2, hint, sizeof(hint));
    }

    psk_len = _str_to_hex(psk, psk_hex);

    at_net_ssl_psk_set(linkid, psk_hex, psk_len, hint, strlen(hint));

    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipmode(int argc, const char **argv)
{
    at_response_string("+CIPMODE:%d\r\n", at_net_config->work_mode);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipmode(int argc, const char **argv)
{
    int mode;

    AT_CMD_PARSE_NUMBER(0, &mode);
    if(mode != NET_MODE_NORMAL && mode != NET_MODE_TRANS) {
        return AT_RESULT_CODE_ERROR;
    }

    if (at_net_config->mux_mode != NET_LINK_SINGLE) {
        AT_NET_CMD_PRINTF("CIPMUX must be %d\r\n", NET_LINK_SINGLE);
        return AT_RESULT_CODE_ERROR;
    }

    at_net_config->work_mode = mode;
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipsto(int argc, const char **argv)
{
    at_response_string("+CIPSTO:%d\r\n", at_net_config->server_timeout);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipsto(int argc, const char **argv)
{
    int timeout;

    AT_CMD_PARSE_NUMBER(0, &timeout);
    if(timeout < 0 || timeout > 7200) {
        return AT_RESULT_CODE_ERROR;
    }

    at_net_config->server_timeout = (net_server_timeout)timeout;
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_savetranslink(int argc, const char **argv)
{
    int mode;
    char remote_host[128];
    int remote_port;
    char type[8];
    int type_valid = 0;
    int param, param_valid = 0;
    int keepalive = 0;
    int local_port = 0;

    AT_CMD_PARSE_NUMBER(0, &mode);
    if (mode == 0) {
        if (argc != 1) {
            return AT_RESULT_CODE_ERROR;
        }

        at_net_config->trans_link.enable = 0;
        at_net_config_save(AT_CONFIG_KEY_NET_TRANS_LINK);
        return AT_RESULT_CODE_OK;
    } else if (mode == 1) {
        AT_CMD_PARSE_STRING(1, remote_host, sizeof(remote_host));
        AT_CMD_PARSE_NUMBER(2, &remote_port);
        AT_CMD_PARSE_OPT_STRING(3, type, sizeof(type), type_valid);
        AT_CMD_PARSE_OPT_NUMBER(4, &param, param_valid);
    } else {
        return AT_RESULT_CODE_ERROR;
    }

    if (remote_port < 0 || remote_port > 65535) {
        return AT_RESULT_CODE_ERROR;
    }
    if (type_valid == 0) {
        strlcpy(type, "TCP", sizeof(type));
    }

    if (strcmp(type, "TCP") == 0 || strcmp(type, "SSL") == 0) {
        if (param_valid) {
            keepalive = param;
        }
        if (keepalive < 0 || keepalive > 7200) {
            return AT_RESULT_CODE_ERROR;
        }
    } else if (strcmp(type, "UDP") == 0) {
        if (param_valid) {
            local_port = param;
        }
        if (local_port < 0 || local_port > 65535) {
            return AT_RESULT_CODE_ERROR;
        }
    } else {
        return AT_RESULT_CODE_ERROR;
    }

    at_net_config->trans_link.enable = 1;
    strlcpy(at_net_config->trans_link.type ,type, sizeof(at_net_config->trans_link.type));
    strlcpy(at_net_config->trans_link.remote_host ,remote_host, sizeof(at_net_config->trans_link.remote_host));
    at_net_config->trans_link.remote_port = (uint16_t)remote_port;
    at_net_config->trans_link.keep_alive = (uint16_t)keepalive;
    at_net_config->trans_link.local_port = (uint16_t)local_port;
    at_net_config_save(AT_CONFIG_KEY_NET_TRANS_LINK);
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipsntpcfg(int argc, const char **argv)
{
    int enable, servernum, timezone;

    enable = at_net_config->sntp_cfg.enable;
    servernum = at_net_config->sntp_cfg.servernum;
    timezone = at_net_config->sntp_cfg.timezone;
    if (timezone < -12 || timezone > 14) {
        timezone = timezone/100;
    }
    if (enable) {
        if (servernum == 1) {
            at_response_string("+CIPSNTPCFG:1,%d,\"%s\"\r\n", timezone,
                at_net_config->sntp_cfg.server1);
        } else if (servernum == 2) {
            at_response_string("+CIPSNTPCFG:1,%d,\"%s\",\"%s\"\r\n", timezone,
                at_net_config->sntp_cfg.server1,
                at_net_config->sntp_cfg.server2);
        } else if (servernum == 3) {
            at_response_string("+CIPSNTPCFG:1,%d,\"%s\",\"%s\",\"%s\"\r\n", timezone,
                at_net_config->sntp_cfg.server1,
                at_net_config->sntp_cfg.server2,
                at_net_config->sntp_cfg.server3);
        }
    } else {
        at_response_string("+CIPSNTPCFG:0\r\n");
    }
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipsntpcfg(int argc, const char **argv)
{
    int enable;
    int timezone;
    char server1[64];
    char server2[64];
    char server3[64];
    int timezone_valid = 0;
    int server1_valid = 0;
    int server2_valid = 0;
    int server3_valid = 0;

    AT_CMD_PARSE_NUMBER(0, &enable);
    AT_CMD_PARSE_OPT_NUMBER(1, &timezone, timezone_valid);
    AT_CMD_PARSE_OPT_STRING(2, server1, sizeof(server1), server1_valid);
    AT_CMD_PARSE_OPT_STRING(3, server2, sizeof(server2), server2_valid);
    AT_CMD_PARSE_OPT_STRING(4, server3, sizeof(server3), server3_valid);

    if (enable != 0 && enable != 1) {
        return AT_RESULT_CODE_ERROR;
    }
    if (enable == 1 && argc < 2) {
        return AT_RESULT_CODE_ERROR;
    }
    if (timezone < -1259 || timezone > 1459) {
        return AT_RESULT_CODE_ERROR;
    }

    at_net_config->sntp_cfg.enable = enable;
    at_net_config->sntp_cfg.servernum = 0;
    if (timezone_valid) {
        at_net_config->sntp_cfg.timezone = timezone;
    } else {
        at_net_config->sntp_cfg.timezone = 0;
    }
    if (server1_valid == 0 && server2_valid == 0 && server3_valid == 0) {
        at_net_config->sntp_cfg.servernum = 3;
        strlcpy(at_net_config->sntp_cfg.server1, "cn.ntp.org.cn", sizeof(at_net_config->sntp_cfg.server1));
        strlcpy(at_net_config->sntp_cfg.server2, "ntp.sjtu.edu.cn", sizeof(at_net_config->sntp_cfg.server2));
        strlcpy(at_net_config->sntp_cfg.server3, "us.pool.ntp.org", sizeof(at_net_config->sntp_cfg.server3));
    } else {
        if (server1_valid) {
            strlcpy(at_net_config->sntp_cfg.server1, server1, sizeof(at_net_config->sntp_cfg.server1));
            at_net_config->sntp_cfg.servernum++;
        }
        if (server2_valid) {
            strlcpy(at_net_config->sntp_cfg.server2, server2, sizeof(at_net_config->sntp_cfg.server2));
            at_net_config->sntp_cfg.servernum++;
        }
        if (server3_valid) {
            strlcpy(at_net_config->sntp_cfg.server3, server3, sizeof(at_net_config->sntp_cfg.server3));
            at_net_config->sntp_cfg.servernum++;
        }
    }

    if (enable == 0) {
        at_net_sntp_stop();
    } else {
        at_net_sntp_stop();
        at_net_sntp_start();
    }
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipsntptime(int argc, const char **argv)
{
    time_t ut;
    char time_buf[32];
    int zone = (int)at_net_config->sntp_cfg.timezone;

    if (zone >= -12 && zone <= 14)
        zone = zone*3600;
    else {
        int zone_h = zone/100;
        int zone_m = zone%100;
        zone = zone_h * 3600 + zone_m * 60;
    }
    ut = (time_t)((at_base_config->systime_stamp + at_current_ms_get())/1000 + zone);
    ctime_r(&ut, time_buf);

    time_buf[strlen(time_buf)-1] = '\0'; // delete last 0x0A

    at_response_string("+CIPSNTPTIME:%s", time_buf);
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipsntpintv(int argc, const char **argv)
{
    at_response_string("+CIPSNTPINTV:%d\r\n", at_net_config->sntp_intv.interval);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipsntpintv(int argc, const char **argv)
{
    int interval;

    AT_CMD_PARSE_NUMBER(0, &interval);

    if (interval < 15 || interval > 4294967) {
        return AT_RESULT_CODE_ERROR;
    }

    at_net_config->sntp_intv.interval = interval;
    if (at_net_sntp_is_start()) {
        at_net_sntp_stop();
        at_net_sntp_start();
    }
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipreconnintv(int argc, const char **argv)
{
    at_response_string("+CIPRECONNINTV:%d\r\n", at_net_config->reconn_intv);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_cipreconnintv(int argc, const char **argv)
{
    int reconn_intv;

    AT_CMD_PARSE_NUMBER(0, &reconn_intv);
    if (reconn_intv < 1 || reconn_intv > 36000) {
        return AT_RESULT_CODE_ERROR;
    }

    at_net_config->reconn_intv = (net_reconn_intv)reconn_intv;
    if (at->store) {
        at_net_config_save(AT_CONFIG_KEY_NET_RECONN_INTV);
    }
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ping(int argc, const char **argv)
{
    // TODO: FIXME
#if 1
    char hostname[128];
    struct hostent *hostinfo;
    struct ping_var *env;
    int ping_response_time = 0;
    int len, count, interval;
    int len_valid = 0, count_valid = 0, interval_valid = 0;

    AT_CMD_PARSE_STRING(0, hostname, sizeof(hostname));
    AT_CMD_PARSE_OPT_NUMBER(1, &len, len_valid);
    AT_CMD_PARSE_OPT_NUMBER(2, &count, count_valid);
    AT_CMD_PARSE_OPT_NUMBER(3, &interval, interval_valid);

    if (!len_valid) {
        len = 64;
    }
    if (!count_valid) {
        count = 4;
    }
    if (!interval_valid) {
        interval = 1000;
    }

    hostinfo = gethostbyname(hostname);
    if (hostinfo) {
        env = ping_api_init(interval, len, count, 1000, (ip_addr_t *)hostinfo->h_addr);
        if (env) {
            while (env->requests_count <= 0) //wait start
                vTaskDelay(1);
            while (env->node_num > 0) //wait finish
                vTaskDelay(1);
            ping_response_time = env->ping_time;
        }
    }

    if (ping_response_time > 0) {
        at_response_string("+PING:%d\r\n", ping_response_time);
        return AT_RESULT_CODE_OK;
    }
    else
    {
        at_response_string("+PING:%s\r\n", "TIMEOUT");
        return AT_RESULT_CODE_FAIL;
    }
#endif
}

static int at_query_cmd_ciupdate(int argc, const char **argv)
{
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_ciupdate(int argc, const char **argv)
{
    return AT_RESULT_CODE_OK;
}

static int at_exe_cmd_ciupdate(int argc, const char **argv)
{
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_iperf(int argc, const char **argv)
{
    int is_server = 0;
    int is_udp = 0;
    int t, t_valid = 0;
    int p, p_valid = 0;
    int ip_valid = 0;
    char ipaddr[32] = {0};
    char type[4] = {0};
    char direct[3] = {0};
    char buffer[128];
    
    AT_CMD_PARSE_STRING(0, direct, sizeof(direct));
    AT_CMD_PARSE_STRING(1, type, sizeof(&type));
    AT_CMD_PARSE_OPT_STRING(2, ipaddr, sizeof(ipaddr), ip_valid);
    AT_CMD_PARSE_OPT_NUMBER(3, &t, t_valid);
    AT_CMD_PARSE_OPT_NUMBER(4, &p, p_valid);
    
    if (strcmp(direct, "TX") == 0) {
        is_server = 0;
    } else if (strcmp(direct, "RX") == 0) {
        is_server = 1;
    } else {
        return AT_RESULT_CODE_ERROR;
    }
     
    if (strcmp(type, "TCP") == 0) {
        is_udp = 0;
    } else if (strcmp(type, "UDP") == 0) {
        is_udp = 1;
    } else {
        return AT_RESULT_CODE_ERROR;
    }

    if (!t_valid) {
        t = 10;
    }
    
    if (!p_valid) {
        p = 5001;
    }

    if (!is_server && !ip_valid) {
        return AT_RESULT_CODE_ERROR;
    }
 
    snprintf(buffer, sizeof(buffer), 
             "iperf "\
             "%s "\
             "%s "\
             "%s "\
             "%s "\
             "-i 1 "\
             "-t %d "\
             "-p %d\r\n", 
             is_udp?"-u":"", 
             is_server?"-s":"-c", 
             is_server?"":ipaddr, 
             (is_udp && !is_server)?"-b 100M":"", 
             t, 
             p);

    printf(buffer);
    shell_exe_cmd(buffer, strlen(buffer));

    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_iperf_stop(int argc, const char **argv)
{
    shell_exe_cmd("iperf stop\r\n", strlen("iperf stop\r\n"));
    
    return AT_RESULT_CODE_OK;
}

#define AT_NET_SSL_NOT_AUTH         0
#define AT_NET_SSL_CLIENT_AUTH      1
#define AT_NET_SSL_SERVER_AUTH      2
#define AT_NET_SSL_BOTH_AUTH        3

static int at_setup_cmd_cipsslcconf(int argc, const char **argv)
{
    int linkid = 0, auth_mode;
    char cert_file[32] = {0};
    char key_file[32] = {0};
    char ca_file[32] = {0};

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
        AT_CMD_PARSE_NUMBER(0, &auth_mode);
        AT_CMD_PARSE_STRING(1, cert_file, sizeof(cert_file));
        AT_CMD_PARSE_STRING(2, key_file, sizeof(key_file));
        AT_CMD_PARSE_STRING(3, ca_file, sizeof(ca_file));
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
        AT_CMD_PARSE_NUMBER(1, &auth_mode);
        AT_CMD_PARSE_STRING(2, cert_file, sizeof(cert_file));
        AT_CMD_PARSE_STRING(3, key_file, sizeof(key_file));
        AT_CMD_PARSE_STRING(4, ca_file, sizeof(ca_file));
    }

    if (!at_net_client_id_is_valid(linkid)) {
        return AT_RESULT_CODE_ERROR;
    }

    if (auth_mode < AT_NET_SSL_NOT_AUTH || auth_mode > AT_NET_SSL_BOTH_AUTH) {
        return AT_RESULT_CODE_ERROR;
    }

    if (auth_mode == AT_NET_SSL_NOT_AUTH) {
        at_net_ssl_path_set(linkid, NULL, NULL, NULL);
    } else if (auth_mode == AT_NET_SSL_CLIENT_AUTH) {
        at_net_ssl_path_set(linkid, ca_file, NULL, NULL);
    } else if (auth_mode == AT_NET_SSL_SERVER_AUTH) {
        at_net_ssl_path_set(linkid, NULL, cert_file, key_file);
    } else if (auth_mode == AT_NET_SSL_BOTH_AUTH) {
        at_net_ssl_path_set(linkid, ca_file, cert_file, key_file);
    }

    strlcpy(at_net_config->sslconf[linkid].ca_file, ca_file, sizeof(at_net_config->sslconf[linkid].ca_file));
    strlcpy(at_net_config->sslconf[linkid].cert_file, cert_file, sizeof(at_net_config->sslconf[linkid].cert_file));
    strlcpy(at_net_config->sslconf[linkid].key_file, key_file, sizeof(at_net_config->sslconf[linkid].key_file));

    if (at->store) {
        at_net_config_save(AT_CONFIG_KEY_NET_SSLCONF);
    }
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cipsslcconf(int argc, const char **argv)
{
    int linkid = 0, auth_mode = 0;
    char *ca, *cert, *key;

    if (at_net_config->mux_mode == NET_LINK_SINGLE) {
    } else {
        AT_CMD_PARSE_NUMBER(0, &linkid);
    }

    if (!at_net_client_id_is_valid(linkid)) {
        return AT_RESULT_CODE_ERROR;
    }

    at_net_ssl_path_get(linkid, &ca, &cert, &key);

    if (strlen(ca)) {
        auth_mode |= 1;
    }

    if (strlen(cert) && strlen(key)) {
        auth_mode |= 2;
    }

    at_response_string("+CIPSSLCCONF:%d,%d,\"%s\",\"%s\",\"%s\"\r\n", linkid, auth_mode, cert, key, ca);

    return AT_RESULT_CODE_OK;
}

static const at_cmd_struct at_net_cmd[] = {
    {"+CIFSR", NULL, NULL, NULL, at_exe_cmd_cifsr, 0, 0},
    {"+CIPV6", NULL, at_query_cmd_cipv6, at_setup_cmd_cipv6, NULL, 0, 0},
    {"+CIPDNS", NULL, at_query_cmd_cipdns, at_setup_cmd_cipdns, NULL, 0, 0},
    {"+CIPDOMAIN", NULL, NULL, at_setup_cmd_cipdomain, NULL, 1, 2},
    {"+CIPSTATE", NULL, at_query_cmd_cipstate, NULL, NULL, 0, 0},
    {"+CIPSTART", NULL, at_query_cmd_cipstart, at_setup_cmd_cipstart, NULL, 3, 7},
    {"+CIPSTARTEX", NULL, NULL, at_setup_cmd_cipstartex, NULL, 3, 6},
    {"+CIPTCPOPT", NULL, at_query_cmd_ciptcport, at_setup_cmd_ciptcport, NULL, 1, 5},
    {"+CIPCLOSE", NULL, NULL, at_setup_cmd_cipclose, at_exe_cmd_cipclose, 1, 1},
    {"+CIPSEND", NULL, NULL, at_setup_cmd_cipsend, at_exe_cmd_cipsend, 1, 4},
    {"+CIPSENDL", NULL, NULL, at_setup_cmd_cipsendl, NULL, 1, 4},
    {"+CIPSENDLCFG", NULL, at_query_cmd_cipsendlcfg, at_setup_cmd_cipsendlcfg, NULL, 2, 2},
    {"+CIPSENDEX", NULL, NULL, at_setup_cmd_cipsendex, NULL, 1, 4},
    {"+CIPDINFO", NULL, at_query_cmd_cipdinfo, at_setup_cmd_cipdinfo, NULL, 1, 1},
    {"+WIPS", NULL, at_query_cmd_wips, at_setup_cmd_wips, NULL, 1, 1},
    {"+CIPMUX", NULL, at_query_cmd_cipmux, at_setup_cmd_cipmux, NULL, 1, 1},
    {"+CIPRECVMODE", NULL, at_query_cmd_ciprecvmode, at_setup_cmd_ciprecvmode, NULL, 1, 1},
    {"+CIPRECVDATA", NULL, NULL, at_setup_cmd_ciprecvdata, NULL, 0, 0},
    {"+CIPRECVBUF", NULL, at_query_cmd_ciprecvbuf, at_setup_cmd_ciprecvbuf, NULL, 1, 2},
    {"+CIPRECVLEN", NULL, at_query_cmd_ciprecvlen, NULL, NULL, 0, 0},
    {"+CIPSERVER", NULL, at_query_cmd_cipserver, at_setup_cmd_cipserver, NULL, 1, 4},
    {"+CIPSERVERMAXCONN", NULL, at_query_cmd_cipservermaxconn, at_setup_cmd_cipservermaxconn, NULL, 1, 1},
    {"+CIPSSLCCONF", NULL, at_query_cmd_cipsslcconf, at_setup_cmd_cipsslcconf, NULL, 4, 5},
    //{"+CIPSSLCCN", NULL, at_query_cmd_cipsslccn, at_setup_cmd_cipsslccn, NULL, 0, 0},
    {"+CIPSSLCSNI", NULL, at_query_cmd_cipsslcsni, at_setup_cmd_cipsslcsni, NULL, 1, 2},
    {"+CIPSSLCALPN", NULL, at_query_cmd_cipsslcalpn, at_setup_cmd_cipsslcalpn, NULL, 2, 8},
    {"+CIPSSLCPSK", NULL, at_query_cmd_cipsslcpsk, at_setup_cmd_cipsslcpsk, NULL, 2, 3},
    {"+CIPSSLCPSKHEX", NULL, at_query_cmd_cipsslcpskhex, at_setup_cmd_cipsslcpskhex, NULL, 2, 3},
    {"+CIPMODE", NULL, at_query_cmd_cipmode, at_setup_cmd_cipmode, NULL, 1, 1},
    {"+CIPSTO", NULL, at_query_cmd_cipsto, at_setup_cmd_cipsto, NULL,  1, 1},
    {"+SAVETRANSLINK", NULL, NULL, at_setup_cmd_savetranslink, NULL, 1, 5},
    {"+CIPSNTPCFG", NULL, at_query_cmd_cipsntpcfg, at_setup_cmd_cipsntpcfg, NULL, 1, 5},
    {"+CIPSNTPTIME", NULL, at_query_cmd_cipsntptime, NULL, NULL, 0, 0},
    {"+CIPSNTPINTV", NULL, at_query_cmd_cipsntpintv, at_setup_cmd_cipsntpintv, NULL, 1, 1},
    {"+CIPRECONNINTV", NULL, at_query_cmd_cipreconnintv, at_setup_cmd_cipreconnintv, NULL, 1, 1},
    {"+PING", NULL, NULL, at_setup_cmd_ping, NULL, 1, 4},
    {"+CIUPDATE", NULL, at_query_cmd_ciupdate, at_setup_cmd_ciupdate, at_exe_cmd_ciupdate, 0, 0},
    {"+IPERF", NULL, NULL, at_setup_cmd_iperf, NULL, 2, 5},
    {"+IPERFSTOP", NULL, NULL, NULL, at_setup_cmd_iperf_stop, 0, 0},
};

bool at_net_cmd_regist(void)
{
    at_net_config_init();

    at_net_start();

    at_register_function(at_net_config_default, at_net_stop);

    if (at_cmd_register(at_net_cmd, sizeof(at_net_cmd) / sizeof(at_net_cmd[0])) == 0)
        return true;
    else
        return false;
}

