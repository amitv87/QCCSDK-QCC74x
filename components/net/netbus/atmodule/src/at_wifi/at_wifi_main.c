/**
  ******************************************************************************
  * @file    at_wifi_main.c
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

#include <lwip/tcpip.h>
#include <lwip/netdb.h>
#include <lwip/tcp.h>
#include <lwip/udp.h>
#include <lwip/altcp.h>
#include <lwip/altcp_tcp.h>
#include <lwip/altcp_tls.h>
#include <lwip/err.h>
#include <lwip/netif.h>

#include "export/qcc74x_fw_api.h"
#include <wifi_mgmr.h>
#include <wifi_mgmr_ext.h>
#include <wifi_mgmr_cli.h>
#include "fhost.h"

#include "at_main.h"
#include "at_core.h"
#include "at_port.h"
#include "at_base_config.h"
#include "at_wifi_config.h"
#include "at_wifi_main.h"

#define DBG_TAG "MAIN"
#include "log.h"

#define AT_WIFI_SUPPORT_STORE_CHANNEL

#define AT_WIFI_TASK_STACK_SIZE 1024
#define AT_WIFI_TASK_PRIORITY 15
#define AT_WIFI_MAX_STA_NUM 10

static void wifi_ap_update_sta_ip(uint8_t mac[6], uint32_t ip);

//static wifi_interface_t g_wifi_sta_interface = NULL;
//static wifi_interface_t g_wifi_ap_interface = NULL;
static int g_wifi_sta_is_connected = 0;
static int g_wifi_sta_use_dhcp = 0;
static int g_wifi_sta_disconnect_reason = 0;
static int g_wifi_waiting_connect_result = 0;
static int g_wifi_sniffer_is_start = 0;

/** Mac address length  */
#define DHCP_MAX_HLEN               6
struct dhcp_client_node
{
    struct dhcp_client_node *next;
    uint8_t chaddr[DHCP_MAX_HLEN];
    ip4_addr_t ipaddr;
    uint32_t lease_end;
};

struct wifi_ap_sta_info
{
    long long valid_time;
    uint8_t mac[6];
    uint32_t ip;
    uint8_t sta_index;
};

struct wifi_ap_sta_info g_wifi_ap_sta_info[AT_WIFI_MAX_STA_NUM];

/* todo: wifi_mgmr_ext.c */
int wifi_mgmr_sta_disconnect(void)
{
    return wifi_sta_disconnect();
}

int wifi_mgmr_sta_mac_set(uint8_t mac[6])
{
    memcpy(wifiMgmr.wlan_sta.mac, mac, 6);

    return 0;
}

int wifi_mgmr_ap_mac_set(uint8_t mac[6])
{
    memcpy(wifiMgmr.wlan_ap.mac, mac, 6);
    return 0;
}
/* todo: */

static void wifi_background_init(uint8_t *ap_mac, uint8_t *sta_mac, uint8_t country_code)
{
    wifi_conf_t conf =
    {
        .country_code = "CN",
    };
    char *country_code_string[WIFI_COUNTRY_CODE_MAX] = {"CN", "JP", "US", "EU"};
    uint8_t mac[6];

    /* init ap mac address */
    wifi_mgmr_ap_mac_get(mac);
    if (memcmp(mac, ap_mac, 6)) {
        wifi_mgmr_ap_mac_set(ap_mac);
    }

    /* init sta mac address */
    wifi_mgmr_sta_mac_get(mac);
    if (memcmp(mac, sta_mac, 6)) {
        wifi_mgmr_sta_mac_set(sta_mac);
    }

    /* init wifi background*/
    strlcpy(conf.country_code, country_code_string[country_code], sizeof(conf.country_code));
#if 0
    wifi_mgmr_start_background(&conf);
#endif

#if 0
    /* enable scan hide ssid */
    wifi_mgmr_scan_filter_hidden_ssid(0);
#endif
}

static void wifiopt_sta_disconnect(int force)
{
#if 0
    int i;
    int state;

    if (g_wifi_sta_interface) {
        if (force == 0) {
            wifi_mgmr_sta_autoconnect_disable();
            /*XXX Must make sure sta is already disconnect, otherwise sta disable won't work*/
            vTaskDelay(1000);

            wifi_mgmr_sta_disconnect();
            for(i = 0; i < 200; i++) {
                wifi_mgmr_sta_state_get(&state);
                if (state == WIFI_STATE_IDLE || state == WIFI_STATE_WITH_AP_IDLE ||
                    state == WIFI_STATE_DISCONNECT || state == WIFI_STATE_WITH_AP_DISCONNECT) {
                    break;
                }
                vTaskDelay(100);
            }

            wifi_mgmr_sta_disable(NULL);
            for(i = 0; i < 200; i++) {
                wifi_mgmr_sta_state_get(&state);
                if (state == WIFI_STATE_IDLE || state == WIFI_STATE_WITH_AP_IDLE)
                    break;
                vTaskDelay(100);
            }
        }
        else
        {
            wifi_mgmr_sta_autoconnect_disable();
            wifi_mgmr_sta_disconnect();
            vTaskDelay(500);
            wifi_mgmr_sta_disable(NULL);
        }

        g_wifi_sta_interface = NULL;
        g_wifi_sta_is_connected = 0;
    }
#else
        wifi_mgmr_sta_autoconnect_disable();
        wifi_mgmr_sta_disconnect();
        vTaskDelay(500);
#endif
}

void wifiopt_sta_connect(void)
{
    char *ssid = at_wifi_config->sta_info.ssid;
    char *psk = at_wifi_config->sta_info.psk;

    int pmf_cfg = 1;

    char *pmk = at_wifi_config->sta_info.pmk;
    uint16_t freq = at_wifi_config->sta_info.freq;
    uint8_t *bssid = at_wifi_config->sta_info.bssid;
    uint16_t listen_interval = (uint16_t)at_wifi_config->sta_info.listen_interval;
    uint32_t flags = 0;
    char *hostname = at_wifi_config->hostname;
    int dhcp_en = (int)at_wifi_config->dhcp_state.bit.sta_dhcp;
    uint32_t ip = at_wifi_config->sta_ip.ip;
    uint32_t mask = at_wifi_config->sta_ip.netmask;
    uint32_t gateway = at_wifi_config->sta_ip.gateway;
    const uint8_t invalid_mac[6] = {0, 0, 0, 0, 0, 0};
    //struct ap_connect_adv ext_param;

    if (strlen(ssid) <= 0) {
        return;
    }

    if (memcmp(bssid, invalid_mac, sizeof(invalid_mac)) == 0) {
        bssid = NULL;
    }

#if 0
    if (at_wifi_config->sta_info.pci_en) {
        flags |= WIFI_CONNECT_PCI_EN;
    }
    if (at_wifi_config->sta_info.scan_mode == 0) {
        flags |= WIFI_CONNECT_STOP_SCAN_ALL_CHANNEL_IF_TARGET_AP_FOUND;
    }
    if (at_wifi_config->sta_info.pmf & 0x01) {
        flags |= WIFI_CONNECT_PMF_CAPABLE;
    }
    if (at_wifi_config->sta_info.pmf & 0x02) {
        flags |= WIFI_CONNECT_PMF_REQUIRED;
    }

    if (!g_wifi_sta_interface) {
        g_wifi_sta_interface = wifi_mgmr_sta_enable();
        wifi_mgmr_set_hostname(hostname);

        ext_param.psk = pmk;
        ext_param.ap_info.type = AP_INFO_TYPE_PRESIST;
        ext_param.ap_info.time_to_live = -1;
        ext_param.ap_info.bssid = bssid;
        ext_param.ap_info.band = 0;
#ifdef AT_WIFI_SUPPORT_STORE_CHANNEL
        ext_param.ap_info.freq = freq;
#else
        ext_param.ap_info.freq = 0;
#endif
        if (dhcp_en) {
            ext_param.ap_info.use_dhcp = 1;
            g_wifi_sta_use_dhcp = 1;
            wifi_mgmr_sta_ip_unset();
        }
        else {
            ext_param.ap_info.use_dhcp = 0;
            g_wifi_sta_use_dhcp = 0;
            wifi_mgmr_sta_ip_set(ip, mask, gateway, gateway, 0);
        }
        ext_param.flags = flags;
        wifi_mgmr_set_listen_interval(listen_interval);
        wifi_mgmr_sta_connect_ext(g_wifi_sta_interface, ssid, psk, &ext_param);
    }
#else
    wifi_sta_connect(ssid, psk, NULL, NULL, pmf_cfg, freq, freq, dhcp_en);
#endif
}

int at_wifi_state_get(void)
{
    return fhost_get_vif_state(MGMR_VIF_STA);
}

static int wifi_sta_wait(int timeout)
{
#if 0
    int ret  = 0;
    long long nTime = 0;

    nTime = aos_now_ms();
    g_wifi_waiting_connect_result = 1;
    while(aos_now_ms() - nTime < timeout) {
        if (!g_wifi_waiting_connect_result)
            break;
    }

    if (g_wifi_sta_is_connected) {
        ret = 0;
    }
    else {
        if (g_wifi_sta_disconnect_reason == WLAN_FW_AUTH_OR_ASSOC_RESPONSE_TIMEOUT_FAILURE)
            ret = 1; // connect timeout
        else if (g_wifi_sta_disconnect_reason == WLAN_FW_4WAY_HANDSHAKE_ERROR_PSK_TIMEOUT_FAILURE
            || g_wifi_sta_disconnect_reason == WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_TRANSMIT_FAILURE
            || g_wifi_sta_disconnect_reason == WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_ALLOCATE_FAIILURE
            || g_wifi_sta_disconnect_reason == WLAN_FW_DEAUTH_BY_AP_WHEN_NOT_CONNECTION)
            ret = 2; // password error
        else if (g_wifi_sta_disconnect_reason == WLAN_FW_SCAN_NO_BSSID_AND_CHANNEL)
            ret = 3; // not found ap
        else
            ret = 4; // connect fail
    }
    return ret;
#endif
}

static void reconnect_event(void *arg)
{
    TaskHandle_t *ptask = (TaskHandle_t *)arg;
    uint32_t interval;
    uint16_t repeat_count;
    int state;
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    for (repeat_count = 0; repeat_count < at_wifi_config->reconn_cfg.repeat_count;) {

        interval = (at_wifi_config->reconn_cfg.interval_second > 0 ) ? at_wifi_config->reconn_cfg.interval_second * 1000 : 5000;
        vTaskDelay(pdMS_TO_TICKS(interval));

        state = at_wifi_state_get();
        if (at_wifi_config->wifi_mode != WIFI_STATION_MODE || state == FHOST_STA_CONNECTED) {
            break;
        }
        printf("xxxxxxxxxxxxx %d %d\r\n", state, wifi_mgmr_sta_state_get());
        if (state == FHOST_STA_DISCONNECTED) {
            repeat_count++;
            printf("reconnect_event interval:%d cfg.repeat_count:%d repeat_count:%d\r\n", interval, at_wifi_config->reconn_cfg.repeat_count, repeat_count);     
            at_wifi_config->reconnect_state = 1;
            wifiopt_sta_connect();
        }
    }
    at_wifi_config->reconnect_state = 0;
    *ptask = NULL;
    vTaskDelete(NULL);
}

static void wifi_sta_enable_reconnect(int enable)
{
    static TaskHandle_t task = NULL;
    if ((at_wifi_config->reconn_cfg.interval_second > 0) && (at_wifi_config->reconn_cfg.repeat_count > 0)) {
        if (!task) {
            xTaskCreate(reconnect_event, (char*)"reconnect", 1024, &task, 15, &task);
        }
    }
}

static void _wifi_ap_status_callback(struct netif *netif)
{
    wifi_ap_update_sta_ip((uint8_t *)netif->hwaddr, netif->ip_addr.addr);

    if (at_get_work_mode() != AT_WORK_MODE_THROUGHPUT ||  at_base_config->sysmsg_cfg.bit.link_state_msg) {
        at_response_string("+DIST_STA_IP:\"%02x:%02x:%02x:%02x:%02x:%02x\",\"%s\"\r\n",
                netif->hwaddr[0],
                netif->hwaddr[1],
                netif->hwaddr[2],
                netif->hwaddr[3],
                netif->hwaddr[4],
                netif->hwaddr[5],
                ip4addr_ntoa(&netif->ip_addr));
    }
}

static int wifi_ap_start(void)
{
    wifi_mgmr_ap_params_t config = {0};

    config.ssid = at_wifi_config->ap_info.ssid;
    config.key = at_wifi_config->ap_info.pwd;
    config.hidden_ssid = at_wifi_config->ap_info.ssid_hidden;
    config.channel = at_wifi_config->ap_info.channel;
    config.ap_max_inactivity = at_wifi_config->ap_info.max_conn;
    config.use_dhcpd = at_wifi_config->dhcp_state.bit.ap_dhcp;
    config.start = at_wifi_config->dhcp_server.start;
    config.limit = at_wifi_config->dhcp_server.end - at_wifi_config->dhcp_server.start;
    config.ap_ipaddr = at_wifi_config->ap_ip.ip;
    config.ap_mask = at_wifi_config->ap_ip.netmask;

    struct netif *netif = fhost_to_net_if(MGMR_VIF_AP);

    wifi_mgmr_ap_start(&config);
    vTaskDelay(100);
    dhcpd_status_callback_set(netif, _wifi_ap_status_callback);

    return 0;
}

static int wifiopt_ap_stop(int force)
{
    wifi_mgmr_ap_stop();

    return 0;
}

static int wifi_ap_get_sta_info_index(uint8_t mac[6])
{
    int i, index = 0;
    long long oldest_time = 0;

    /* find exist record */
    for (i = 0; i < AT_WIFI_MAX_STA_NUM; i++) {
        if (g_wifi_ap_sta_info[i].valid_time != 0 && memcmp(g_wifi_ap_sta_info[i].mac, mac, 6) == 0) {
            return i;
        }   
    }

    /* find empty record */
    for (i = 0; i < AT_WIFI_MAX_STA_NUM; i++) {
        if (g_wifi_ap_sta_info[i].valid_time == 0) {
            return i;
        }   
    }

    /* find oldest record */
    oldest_time = at_current_ms_get();
    for (i = 0; i < AT_WIFI_MAX_STA_NUM; i++) {
        if (g_wifi_ap_sta_info[i].valid_time < oldest_time) {
            oldest_time = g_wifi_ap_sta_info[i].valid_time;
            index = i;
        }   
    }
    return index;
}

static void wifi_ap_update_sta_ip(uint8_t mac[6], uint32_t ip)
{
    int index = wifi_ap_get_sta_info_index(mac);

    g_wifi_ap_sta_info[index].valid_time = at_current_ms_get();
    memcpy(g_wifi_ap_sta_info[index].mac, mac, 6);
    g_wifi_ap_sta_info[index].ip = ip;
}

static int wifi_ap_update_sta_index()
{
    int idx = -1;
    struct wifi_sta_basic_info sta_info = {0};

    for(int i = 0; i < NX_REMOTE_STA_MAX; i++) {
        wifi_mgmr_ap_sta_info_get(&sta_info, i);
        if(!sta_info.is_used || (sta_info.sta_idx == 0xef)) {
            continue;
        }
        if (memcmp(g_wifi_ap_sta_info[i].mac, sta_info.sta_mac, 6) != 0) {
            idx = i;
            g_wifi_ap_sta_info[i].valid_time = at_current_ms_get();
            memcpy(g_wifi_ap_sta_info[i].mac, sta_info.sta_mac, 6);
            g_wifi_ap_sta_info[i].sta_index = sta_info.sta_idx;
            printf("update sta add idx:%d\r\n", idx);
            break;
        }
    }
    return idx;
}

static void wifi_ap_delete_sta_info(uint8_t mac[6])
{
    struct wifi_sta_basic_info sta_info = {0};

    for(int i = 0; i < NX_REMOTE_STA_MAX; i++) {
        wifi_mgmr_ap_sta_info_get(&sta_info, i);
        if(!sta_info.is_used || (sta_info.sta_idx == 0xef)) {
            if (g_wifi_ap_sta_info[i].valid_time) {
                memcpy(mac, g_wifi_ap_sta_info[i].mac, 6);
                memset(&g_wifi_ap_sta_info[i], 0, sizeof(g_wifi_ap_sta_info[i]));
                printf("delete sta idx:%d\r\n", i);
                break;
            }
        }
    }
}

static int wifi_ap_get_sta_ip(uint8_t mac[6], uint32_t *ip)
{
    int i;

    for (i = 0; i < AT_WIFI_MAX_STA_NUM; i++) {
        if (g_wifi_ap_sta_info[i].valid_time != 0 && memcmp(g_wifi_ap_sta_info[i].mac, mac, 6) == 0) {
            *ip = g_wifi_ap_sta_info[i].ip;
            return 0;
        }   
    }
    return -1;
}

static void wifi_sniffer_data_recv(void *env, uint8_t *pkt, int pkt_len)
{
#if 0
    int filter_len = (int)env;

    if (pkt_len >= filter_len) {
        printf("pkt_len: %d\r\n", pkt_len );
    }
#endif
}

static void at_wifi_event_cb(uint32_t code, void *private_data)
{
    switch (code) {
        case CODE_WIFI_ON_INIT_DONE: {
            wifi_background_init(   at_wifi_config->ap_mac.addr,
                                    at_wifi_config->sta_mac.addr,
                                    at_wifi_config->wifi_country.country_code);
        }
        break;

        case CODE_WIFI_ON_MGMR_DONE: {
            wifi_sta_enable_reconnect(1);
            if (at_wifi_config->wifi_mode == WIFI_SOFTAP_MODE || at_wifi_config->wifi_mode == WIFI_AP_STA_MODE) {
            	//FIXME: This is Timer context, not allowed call block API
                //wifi_ap_start();
            }
            if (at_wifi_config->wifi_mode == WIFI_STATION_MODE || at_wifi_config->wifi_mode == WIFI_AP_STA_MODE) {
                if (at_wifi_config->auto_conn == WIFI_AUTOCONN_ENABLE) {
                    wifiopt_sta_connect();
                }
            }
        }
        break;

        case CODE_WIFI_ON_DISCONNECT: {
            if (g_wifi_sta_is_connected) {
                if (at_get_work_mode() != AT_WORK_MODE_THROUGHPUT ||  at_base_config->sysmsg_cfg.bit.link_state_msg) {
                    at_response_string("WIFI DISCONNECT\r\n");
                }
                g_wifi_sta_is_connected = 0;
                wifi_sta_enable_reconnect(1);
            }
            g_wifi_sta_disconnect_reason = wifi_mgmr_sta_info_status_code_get();// qcc74x_fw_api.h eg: WLAN_FW_BEACON_LOSS
            g_wifi_waiting_connect_result = 0;
        }
        break;
 
        case CODE_WIFI_ON_CONNECTED: {
            if (!g_wifi_sta_is_connected) {
                if (at_get_work_mode() != AT_WORK_MODE_THROUGHPUT ||  at_base_config->sysmsg_cfg.bit.link_state_msg) {
                    at_response_string("WIFI CONNECTED\r\n");
                }
                g_wifi_sta_is_connected = 1;

                //if (g_wifi_sta_use_dhcp == 0)
                //    aos_post_event(EV_WIFI, CODE_WIFI_ON_GOT_IP, 0);

#if 0
#ifdef AT_WIFI_SUPPORT_STORE_CHANNEL
                wifi_mgmr_sta_connect_ind_stat_info_t stat;
                wifi_mgmr_sta_connect_ind_stat_get(&stat);
                if (at_wifi_config->sta_info.freq != stat.chan_freq) {
                    at_wifi_config->sta_info.freq = stat.chan_freq;
                    if (at->store) {
                        at_wifi_config_save(AT_CONFIG_KEY_WIFI_STA_INFO);
                    }
                }
#endif
#endif
            }
        }
        break;

        case CODE_WIFI_ON_GOT_IP: {
            if (g_wifi_sta_is_connected) {
                if (at_get_work_mode() != AT_WORK_MODE_THROUGHPUT ||  at_base_config->sysmsg_cfg.bit.link_state_msg) {
                    at_response_string("WIFI GOT IP\r\n");
                }

                g_wifi_sta_disconnect_reason = 0;
                g_wifi_waiting_connect_result = 0;
            }
        }
        break;

        case CODE_WIFI_ON_AP_STA_ADD: {
            struct wifi_sta_basic_info sta_info;
            int idx = wifi_ap_update_sta_index();
            wifi_mgmr_ap_sta_info_get(&sta_info, idx);
            if (at_get_work_mode() != AT_WORK_MODE_THROUGHPUT ||  at_base_config->sysmsg_cfg.bit.link_state_msg) {
                at_response_string("+STA_CONNECTED:\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n",
                        sta_info.sta_mac[0],
                        sta_info.sta_mac[1],
                        sta_info.sta_mac[2],
                        sta_info.sta_mac[3],
                        sta_info.sta_mac[4],
                        sta_info.sta_mac[5]);
            }
        }
        break;

        case CODE_WIFI_ON_AP_STA_DEL: {
            uint8_t sta_mac[6] = {0, 0, 0, 0, 0, 0};
            wifi_ap_delete_sta_info(sta_mac);
            if (at_get_work_mode() != AT_WORK_MODE_THROUGHPUT ||  at_base_config->sysmsg_cfg.bit.link_state_msg) {
                at_response_string("+STA_DISCONNECTED:\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n",
                        sta_mac[0],
                        sta_mac[1],
                        sta_mac[2],
                        sta_mac[3],
                        sta_mac[4],
                        sta_mac[5]);
            }
        }
        break;
        default: {
            /*nothing*/
        }
    }
}

static wifi_conf_t conf = {
    .country_code = "CN",
};
void wifi_event_handler(uint32_t code)
{
    switch (code) {
        case CODE_WIFI_ON_INIT_DONE: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_INIT_DONE\r\n", __func__);
            //at_wifi_event_cb(code, NULL);
            char *country_code_string[WIFI_COUNTRY_CODE_MAX] = {"CN", "JP", "US", "EU"};
            strlcpy(conf.country_code, country_code_string[at_wifi_config->wifi_country.country_code], sizeof(conf.country_code));
            wifi_mgmr_init(&conf);
        } break;
        case CODE_WIFI_ON_MGMR_DONE: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_MGMR_DONE\r\n", __func__);
            at_wifi_event_cb(code, NULL);
        } break;
        case CODE_WIFI_ON_SCAN_DONE: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_SCAN_DONE\r\n", __func__);
            extern void at_scan_done_event_tigger(void);
            at_scan_done_event_tigger();
        } break;
        case CODE_WIFI_ON_CONNECTED: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_CONNECTED\r\n", __func__);
            extern void mm_sec_keydump();
            mm_sec_keydump();
            at_wifi_event_cb(code, NULL);
        } break;
        case CODE_WIFI_ON_GOT_IP: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_GOT_IP\r\n", __func__);
            LOG_I("[SYS] Memory left is %d Bytes\r\n", kfree_size());
            at_wifi_event_cb(code, NULL);
        } break;
        case CODE_WIFI_ON_DISCONNECT: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_DISCONNECT\r\n", __func__);
            at_wifi_event_cb(code, NULL);
        } break;
        case CODE_WIFI_ON_AP_STARTED: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_AP_STARTED\r\n", __func__);
        } break;
        case CODE_WIFI_ON_AP_STOPPED: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_AP_STOPPED\r\n", __func__);
        } break;
        case CODE_WIFI_ON_AP_STA_ADD: {
            LOG_I("[APP] [EVT] [AP] [ADD] %lld\r\n", xTaskGetTickCount());
            at_wifi_event_cb(code, NULL);
        } break;
        case CODE_WIFI_ON_AP_STA_DEL: {
            LOG_I("[APP] [EVT] [AP] [DEL] %lld\r\n", xTaskGetTickCount());
            at_wifi_event_cb(code, NULL);
        } break;
        default: {
            LOG_I("[APP] [EVT] Unknown code %u \r\n", code);
        }
    }
}

int at_wifi_set_mode(void)
{
    wifiopt_ap_stop(0);
    wifiopt_sta_disconnect(0);

    if(at_wifi_config->wifi_mode == WIFI_STATION_MODE) {
        if (at_wifi_config->switch_mode_auto_conn == WIFI_AUTOCONN_ENABLE) {
            wifi_sta_enable_reconnect(1);
            wifiopt_sta_connect();
        }
    } else if(at_wifi_config->wifi_mode == WIFI_SOFTAP_MODE) {
        wifi_ap_start();
    } else if(at_wifi_config->wifi_mode == WIFI_AP_STA_MODE) {
        wifi_ap_start();
        if (at_wifi_config->switch_mode_auto_conn == WIFI_AUTOCONN_ENABLE) {
            wifiopt_sta_connect();
        }
    }

    return 0;
}

int at_wifi_sta_connect(int timeout)
{
#if 0
    int ret;

    wifiopt_sta_disconnect(0);
    wifiopt_sta_connect();
    ret = wifi_sta_wait(timeout);
    if (ret == 0) {
        wifi_sta_enable_reconnect(1);
    }

    return ret;
#else
    wifiopt_sta_disconnect(0);
    wifiopt_sta_connect();
    return 0;
#endif
}

int at_wifi_sta_disconnect(void)
{
    wifiopt_sta_disconnect(0);
    return 0;
}

int at_wifi_sta_set_reconnect(void)
{
    wifi_sta_enable_reconnect(1);
    return 0;
}

int at_wifi_ap_start(void)
{
    wifiopt_ap_stop(0);
    wifi_ap_start();
    return 0;
}

int at_wifi_ap_stop(void)
{
#if 0
    wifiopt_ap_stop(0);
#endif
    return 0;
}

int at_wifi_ap_get_sta_ip(uint8_t *mac, char *ip, uint32_t ipbuf_size, int check_flag)
{
    uint32_t ip_addr;
    ip4_addr_t ipaddr;

    if (wifi_ap_get_sta_ip(mac, &ip_addr) != 0) {
        strlcpy(ip, "", ipbuf_size);
        return -1;
    }

    if (check_flag) {
        uint32_t start_ip, end_ip;
        start_ip = (at_wifi_config->ap_ip.ip & at_wifi_config->ap_ip.netmask) | (at_wifi_config->dhcp_server.start << 24);
        end_ip = (at_wifi_config->ap_ip.ip & at_wifi_config->ap_ip.netmask) | (at_wifi_config->dhcp_server.end << 24);
        if (ntohl(ip_addr) < ntohl(start_ip) || ntohl(ip_addr) > ntohl(end_ip))
            ip_addr = 0;
    }

    ipaddr.addr = ip_addr;
    strlcpy(ip, ip4addr_ntoa(&ipaddr), ipbuf_size);
    return 0;
}

int at_wifi_ap_set_dhcp_range(int start, int end)
{
#if 0
    wifi_mgmr_ap_dhcp_range_set(0, 0, start, end);
#endif
    return 0;
}

int at_wifi_sniffer_start(int min_pkt_len)
{
    if (g_wifi_sniffer_is_start) {
        return -1;
    }

#if 0
    wifiopt_ap_stop(0);
    wifiopt_sta_disconnect(0);

    wifi_mgmr_sniffer_register((void *)min_pkt_len, wifi_sniffer_data_recv);
    wifi_mgmr_sniffer_enable();
#endif
    g_wifi_sniffer_is_start = 1;
    return 0;
}

int at_wifi_sniffer_set_channel(int channel)
{
    if (!g_wifi_sniffer_is_start) {
        return -1;
    }
#if 0

    wifi_mgmr_channel_set(channel, 0);
#endif
    return 0;
}

int at_wifi_sniffer_stop(void)
{
    if (!g_wifi_sniffer_is_start) {
        return -1;
    }

    //wifi_mgmr_sniffer_disable();
    //wifi_mgmr_sniffer_unregister(NULL);

    g_wifi_sniffer_is_start = 0;
    return 0;
}

int at_wifi_start(void)
{
#if 0
    hal_wifi_start_firmware_task();
    /*Trigger to start Wi-Fi*/
    aos_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE, 0);

    aos_register_event_filter(EV_WIFI, at_wifi_event_cb, NULL);

    memset(&g_wifi_ap_sta_info, 0, sizeof(g_wifi_ap_sta_info));
#endif
    return 0;
}

int at_wifi_stop(void)
{
    wifiopt_ap_stop(1);
    wifiopt_sta_disconnect(1);

    return 0;
}
