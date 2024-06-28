/**
  ******************************************************************************
  * @file    at_net_main.c
  * @version V1.0
  * @date
  * @brief   This file is part of AT command framework
  ******************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>

#include <at_net_sntp.h>
#include <wifi_mgmr_ext.h>
#include <qcc74x_sec_trng.h>
#include <qcc74x_romfs.h>

#include "at_main.h"
#include "at_core.h"
#include "at_base_config.h"
#include "at_net_main.h"
#include "at_net_config.h"
#include "at_net_ssl.h"
#include "at_wifi_config.h"

#define AT_NET_TASK_STACK_SIZE     (2048)
#define AT_NET_TASK_PRIORITY       (15)
#define AT_NET_RECV_BUF_SIZE       (2920)
#define AT_NET_PRINTF              printf
#define AT_NET_DEBUG               printf

#define CHECK_NET_CLIENT_ID_VALID(id) \
    if (!at_net_client_id_is_valid(id)) { \
        AT_NET_PRINTF("client id is invalid\r\n"); \
        return -1; \
    }

#define CHECK_NET_SERVER_ID_VALID(id) \
    if (!at_net_server_id_is_valid(id)) { \
        AT_NET_PRINTF("server id is invalid\r\n"); \
        return -1; \
    }

#define os_get_time_ms() ((xPortIsInsideInterrupt())?(xTaskGetTickCountFromISR()):(xTaskGetTickCount()))

typedef enum {
    NET_CLIENT_TCP = 0,
    NET_CLIENT_UDP,
    NET_CLIENT_SSL,
} net_client_type;

typedef enum {
    NET_SERVER_TCP = 0,
	NET_SERVER_UDP,
    NET_SERVER_SSL,
} net_server_type;

typedef enum {
    NET_IPDINFO_CONNECTED = 0,
    NET_IPDINFO_DISCONNECTED,
    NET_IPDINFO_RECVDATA
} net_ipdinfo_type;

typedef struct {
    int valid;
    net_client_type type;
    int fd;
    void *priv;

    uint32_t remote_ip;
    uint16_t remote_port;
    uint16_t local_port;
    uint8_t udp_mode;
    uint8_t tetype;
    uint16_t recv_timeout;
    uint32_t recv_time;
    uint32_t disconnect_time;

    int keep_alive;
    int so_linger;
    int tcp_nodelay;
    int so_sndtimeo;
    uint8_t *recv_buf;
    uint32_t recvbuf_size;

    char ca_path[32];
    char cert_path[32];
    char priv_key_path[32];
    char *ssl_hostname;
    char *ssl_alpn[6];
    int ssl_alpn_num;
    char ssl_psk[32];
    uint8_t ssl_psklen;
    char ssl_pskhint[32];
    uint8_t ssl_pskhint_len;
}at_net_client_handle;

typedef struct {
    int valid;
    net_server_type type;
    int fd;

    uint16_t port;
    uint16_t recv_timeout;
    uint8_t ca_enable;
    int8_t client_max;
    int8_t client_num;
    
    char ca_path[32];
    char cert_path[32];
    char priv_key_path[32];
}at_net_server_handle;

static at_net_client_handle *g_at_client_handle = NULL;
static at_net_server_handle *g_at_server_handle = NULL;
static SemaphoreHandle_t net_mutex;
static uint8_t g_at_net_task_is_start = 0;
static uint8_t g_at_net_sntp_is_start = 0;
static char g_at_net_savelink_host[128];

static int ip_multicast_enable(int fd, uint32_t ip)
{
#if LWIP_IGMP
    char optval;
    /* Close multicast loop. */
    optval =0;
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *) &optval, sizeof(optval))) {
        AT_NET_PRINTF("sock set multicast loop failed\r\n");
        return -1;
    }

    /* Set multicast interface. */
    struct in_addr addr;	
    memset((void *)&addr, 0, sizeof(struct in_addr));
    addr.s_addr = htonl(INADDR_ANY);
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF,(char *)&addr, sizeof(addr))) {
        AT_NET_PRINTF("sock set multicast interface failed\r\n");
        return -1;
    }

    /* Setup time-to-live. */
    optval = 10; /* Hop count */
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &optval, sizeof(optval))) {
        AT_NET_PRINTF("sock set multicast ttl failed\r\n");
        return -1;
    }

    /* Add membership to receiving socket. */
    struct ip_mreq mreq;
    memset(&mreq, 0, sizeof(struct ip_mreq));
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    mreq.imr_multiaddr.s_addr = ip;  
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq, sizeof(mreq))) {
        AT_NET_PRINTF("sock set add membership failed\r\n");
        return -1;
    }

    AT_NET_PRINTF("sock add membership\r\n");
#endif
    return 0;
}

static int so_keepalive_enable(int fd, int idle, int interval, int count)
{
    int keepAlive = 1;   // 开启keepalive属性. 缺省值: 0(关闭)  
    int keepIdle = idle;   // 如果在60秒内没有任何数据交互,则进行探测. 缺省值:7200(s)  
    int keepInterval = interval;   // 探测时发探测包的时间间隔为5秒. 缺省值:75(s)  
    int keepCount = count;   // 探测重试的次数. 全部超时则认定连接失效..缺省值:9(次)  

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive))) {
        AT_NET_PRINTF("sock enable tcp keepalive failed\r\n");
        return -1;
    }
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle))) {
        AT_NET_PRINTF("sock set tcp keepidle failed\r\n");
        return -1;
    }
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, (void*)&keepInterval, sizeof(keepInterval))) {
        AT_NET_PRINTF("sock set tcp keepintvl failed\r\n");
        return -1;
    }
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (void*)&keepCount, sizeof(keepCount))) {
        AT_NET_PRINTF("sock set tcp keepcnt failed\r\n");
        return -1;
    }

    AT_NET_PRINTF("sock enable keepalive\r\n");
    return 0;
}

static int so_keepalive_disable(int fd)
{
    int keepAlive = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive))) {
        AT_NET_PRINTF("sock disable tcp keepalive failed\r\n");
        return -1;
    }

    AT_NET_PRINTF("sock disable keepalive\r\n");
    return 0;
}

static int so_linger_enable(int fd, int linger)
{
    struct linger so_linger;
    so_linger.l_onoff = 1;
    so_linger.l_linger = linger;
    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger))) {
        AT_NET_PRINTF("sock set enable linger failed\r\n");
        return -1;
    }

    AT_NET_PRINTF("sock enable so_linger\r\n");
    return 0;
}

static int so_linger_disable(int fd)
{
    struct linger so_linger;
    so_linger.l_onoff = 0;
    so_linger.l_linger = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger))) {
        AT_NET_PRINTF("sock set disable linger failed\r\n");
        return -1;
    }

    AT_NET_PRINTF("sock disable so_linger\r\n");
    return 0;
}

static int so_sndtimeo_enable(int fd, int timeout_ms)
{
    struct timeval tv;
    tv.tv_sec = timeout_ms/1000;
    tv.tv_usec = (timeout_ms%1000)*10000;

    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*) &tv, sizeof(struct timeval))) {
        AT_NET_PRINTF("sock enable send timeout failed\r\n");
        return -1;
    }

    AT_NET_PRINTF("sock enable sndtimeo\r\n");
    return 0;
}

static int so_localport_get(int fd)
{
    struct sockaddr_in local_addr;
    int n = sizeof(local_addr);

    getsockname(fd, (struct sockaddr *) &local_addr, (socklen_t *)&n);
    return ntohs(local_addr.sin_port);
}

static int so_recvsize_get(int fd)
{
    int bytes;

    if (ioctl(fd, FIONREAD, &bytes) != 0)
        return -1;
    return bytes;
}

static int tcp_nodelay_enable(int fd)
{
    int on = 1;

    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on))) {
        AT_NET_PRINTF("sock set enable tcp nodelay\r\n");
        return -1;
    }

    AT_NET_PRINTF("sock enable tcp_nodelay\r\n");
    return 0;
}

static int tcp_nodelay_disable(int fd)
{
    int on = 0;

    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on))) {
        AT_NET_PRINTF("sock set disable tcp nodelay\r\n");
        return -1;
    }

    AT_NET_PRINTF("sock disable tcp_nodelay\r\n");
    return 0;
}

static uint16_t udp_localport_rand(void)
{
    uint16_t port = 50000 + ((random())%10000);
    return port;
}

static int tcp_client_connect(uint32_t ip, uint16_t port)
{
    int fd;
    int res;

    //struct hostent *hostinfo;
    struct sockaddr_in addr;

    ip4_addr_t ipaddr;
    ipaddr.addr = ip;
    AT_NET_PRINTF("tcp client connect %s:%d\r\n", ip4addr_ntoa(&ipaddr), port);
    /*hostinfo = gethostbyname(host);
    if (!hostinfo) {
        AT_NET_PRINTF("gethostbyname failed\r\n");
        return -1;
    }*/

    if ( (fd =  socket(AF_INET, SOCK_STREAM, 0))  < 0) {
        AT_NET_PRINTF("socket create failed\r\n");
        return -2;
    }

    AT_NET_DEBUG("tcp_client_connect fd:%d\r\n", fd);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_len = sizeof(addr);
    addr.sin_port = htons(port);
    //addr.sin_addr.s_addr = ((struct in_addr *) hostinfo->h_addr)->s_addr;
    addr.sin_addr.s_addr = ip;

    int on= 1;
    res = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
    if (res != 0) {
        AT_NET_PRINTF("setsockopt failed, res:%d\r\n", res);
    }

    res = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (res < 0) {
        AT_NET_PRINTF("connect failed, res:%d\r\n", res);
        close(fd);
        return -3;
    }

    return fd;
}

static int tcp_client_close(int fd)
{
    AT_NET_PRINTF("tcp client close\r\n");
    if (fd >= 0) {
        close(fd);
    }

    return 0;
}

static int tcp_client_send(int fd, void *buffer, int length)
{
    if (fd >= 0) {
        return send(fd, buffer, length, 0);
    }
    return 0;
}

static int udp_client_connect(uint16_t port)
{
    int fd;	
    struct sockaddr_in addr;	
    unsigned char loop= 0;
    int so_broadcast=1;

    AT_NET_PRINTF("udp client connect port %d\r\n", port);
    if ( (fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {		
        return -1;	
    }

    setsockopt(fd, SOL_SOCKET,SO_BROADCAST, &so_broadcast, sizeof(so_broadcast));
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

    memset(&addr, 0,  sizeof(addr));	
    addr.sin_family = AF_INET;	
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);	
    if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -2;
    }

    return fd;
}

static int udp_client_close(int fd)
{
    AT_NET_PRINTF("udp client close\r\n");
    if (fd >= 0) {
        close(fd);
    }

    return 0;
}

static int udp_client_send(int fd, void *buffer, int length, uint32_t ip, uint16_t port)
{
    struct sockaddr_in toaddr;

    if (fd >= 0) {
        memset(&toaddr, 0, sizeof(toaddr));	
        toaddr.sin_family = AF_INET;
        toaddr.sin_addr.s_addr = ip;
        toaddr.sin_port = htons(port);
        
        int ret = sendto(fd, buffer, length, 0, (struct sockaddr *)&toaddr, sizeof(struct sockaddr_in));
        return (ret < 0) ? 0 : ret;
        //printf("sendto ret:%d len:%d\r\n", ret, length);
    }
    return 0;
}

static int get_romfs_file_content(const char *file, romfs_filebuf_t *buf) 
{
    char path[64] = {0};
    romfs_file_t fp;

    if (!strlen(file)) {
        return -1;
    }
    sprintf(path, AT_FS_ROOT_DIR"/%s", file);
    romfs_open(&fp, path, 0);
    romfs_filebuf_get(&fp, buf);
    romfs_close(&fp);

    buf->bufsize += 1;
    return 0;
}

static int ssl_client_connect(int id, uint32_t ip, uint16_t port, void **priv)
{
    int fd;
    void *handle;

    ip4_addr_t ipaddr;
    ipaddr.addr = ip;
    AT_NET_PRINTF("ssl client connect %s:%d\r\n", ip4addr_ntoa(&ipaddr), port);

    fd = tcp_client_connect(ip, port);
    if (fd < 0) {
        AT_NET_PRINTF("tcp_client_connect fd:%d\r\n", fd);
        return -1;
    }

    romfs_filebuf_t ca = {0}, cert = {0}, key = {0};

    get_romfs_file_content(g_at_client_handle[id].ca_path, &ca);
    get_romfs_file_content(g_at_client_handle[id].cert_path, &cert);
    get_romfs_file_content(g_at_client_handle[id].priv_key_path, &key);

    handle = mbedtls_ssl_connect(id, fd, ca.buf, ca.bufsize,
                                 cert.buf, cert.bufsize,
                                 key.buf, key.bufsize);
    if (handle == NULL) {
        AT_NET_PRINTF("mbedtls_ssl_connect handle NULL, fd:%d\r\n", fd);
        close(fd);
        return -1;
    }

    *priv = handle;
    return fd;
}

static int ssl_client_close(int fd, void *priv)
{
    AT_NET_PRINTF("ssl client close\r\n");
    if (priv)
        mbedtls_ssl_close(priv);
    if (fd >= 0)
        close(fd);
    
    return 0;
}

static int ssl_client_send(int fd, void *buffer, int length, void *priv)
{
    if (fd >= 0 && priv)
        return mbedtls_ssl_send(priv, buffer, length);

    return 0;
}

static int udp_server_create(uint16_t port, int listen)
{	
    int fd;	
    struct sockaddr_in servaddr;	
    int on;
    struct timeval timeout = { 0 };

    AT_NET_PRINTF("udp server create port %d\r\n", port);
    if ( (fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        return -1;	
    }

    memset(&servaddr, 0, sizeof(servaddr));	
    servaddr.sin_family = AF_INET;	
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	
    servaddr.sin_port = htons(port);	

    on= 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (bind(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {		
        close(fd);
        return -2;	
    }

    timeout.tv_sec = 5;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    return fd;
}

static int tcp_server_create(uint16_t port, int listen)
{	
    int fd;	
    struct sockaddr_in servaddr;	
    int on;

    AT_NET_PRINTF("tcp server create port %d\r\n", port);
    if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {		
        return -1;	
    }

    memset(&servaddr, 0, sizeof(servaddr));	
    servaddr.sin_family = AF_INET;	
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	
    servaddr.sin_port = htons(port);	

    on= 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (bind(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {		
        close(fd);
        return -2;	
    }

    if (listen(fd, listen) < 0) {		
        close(fd);
        return -3;	
    }

    return fd;
}

static int tcp_server_close(int fd)
{
    AT_NET_PRINTF("tcp server close\r\n");
    if (fd >= 0) {
        close(fd);
    }

    return 0;
}

static void net_lock(void)
{
    xSemaphoreTake(net_mutex, portMAX_DELAY);
}

static void net_unlock(void)
{
    xSemaphoreGive(net_mutex);
}

static int net_is_active(void)
{
    int state;

    //wifi_mgmr_state_get(&state);
    if (at_wifi_config->wifi_mode == WIFI_STATION_MODE) {
    	ip_addr_t sta_addr = {0};
    	wifi_sta_ip4_addr_get(&sta_addr.addr, NULL, NULL, NULL);
    	if (!ip_addr_isany(&sta_addr)) {
    		return 1;
    	}
    } else if (at_wifi_config->wifi_mode == WIFI_SOFTAP_MODE) {
#if 0
        if (WIFI_STATE_AP_IS_ENABLED(state)) {
            return 1;
        }
#endif
    } else if (at_wifi_config->wifi_mode == WIFI_AP_STA_MODE) {
#if 0
         if (state == WIFI_STATE_CONNECTED_IP_GOT || state == WIFI_STATE_WITH_AP_CONNECTED_IP_GOT || WIFI_STATE_AP_IS_ENABLED(state)) {
            return 1;
         }
#endif
    }
    
    return 0;
}

static int net_socket_ipd(net_ipdinfo_type ipd, int id, void *buffer, int length, uint32_t ip, uint16_t port)
{
    if (ipd == NET_IPDINFO_CONNECTED) {
        if (at_get_work_mode() != AT_WORK_MODE_THROUGHPUT ||  at_base_config->sysmsg_cfg.bit.link_state_msg) {
            if (at_base_config->sysmsg_cfg.bit.link_msg_type) {
                char type[8];
                uint16_t local_port;
                uint8_t tetype;
                ip4_addr_t ipaddr;

                ipaddr.addr = ip;
                at_net_client_get_info(id, type, NULL, NULL, &local_port, &tetype);
                if (at_net_config->wips_enable) {
                    at_response_string("+LINK_CONN:%d,%d,\"%s\",%d,\"%s\",%d,%d\r\n", 0, id, type, tetype, ip4addr_ntoa(&ipaddr), port, local_port);
                }
            }
            else {
                if (at_net_config->wips_enable) {
                    if (at_net_config->mux_mode == NET_LINK_SINGLE)
                        at_response_string("+IPS:CONNECT\r\n");
                    else
                        at_response_string("+IPS:%d,CONNECT\r\n", id);
                }
            }
        }
    }
    else if (ipd == NET_IPDINFO_DISCONNECTED) {
        if (at_net_config->wips_enable) {
            if (at_get_work_mode() != AT_WORK_MODE_THROUGHPUT ||  at_base_config->sysmsg_cfg.bit.link_state_msg) {     
                if (at_net_config->mux_mode == NET_LINK_SINGLE)
                    at_response_string("+IPS:CLOSED\r\n");
                else
                    at_response_string("+IPS:%d,CLOSED\r\n", id);
            }
        }
    }
    else if (ipd == NET_IPDINFO_RECVDATA) {
        if (at_get_work_mode() != AT_WORK_MODE_THROUGHPUT && at_net_config->wips_enable) {
            if (at_net_config->ipd_info == NET_IPDINFO_DISABLE_IPPORT) {
                if (at_net_config->mux_mode == NET_LINK_SINGLE)
                    at_response_string("\r\n+IPD,%d:", length);
                else
                    at_response_string("\r\n+IPD,%d,%d:", id, length);
            }
            else {
                ip4_addr_t ipaddr;
                ipaddr.addr = ip;

                if (at_net_config->mux_mode == NET_LINK_SINGLE)
                    at_response_string("\r\n+IPD,%d,\"%s\",%d:", length, ip4addr_ntoa(&ipaddr), port);
                else
                    at_response_string("\r\n+IPD,%d,%d,\"%s\",%d:", id, length, ip4addr_ntoa(&ipaddr), port);
            }

            AT_CMD_DATA_SEND(buffer, length);
            at_response_string("\r\n");
        }
        else {
            AT_CMD_DATA_SEND(buffer, length);
        }
    }

    return 0;
}

static int net_socket_setopt(int fd, int keepalive, int so_linger, int tcp_nodelay, int sndtimeo)
{
    if (fd < 0)
        return -1;

    if (keepalive)
        so_keepalive_enable(fd, keepalive, 1, 3);
    else
        so_keepalive_disable(fd);

    if (so_linger>= 0)
        so_linger_enable(fd, so_linger);
    else
        so_linger_disable(fd);

    if (tcp_nodelay)
        tcp_nodelay_enable(fd);
    else
        tcp_nodelay_disable(fd);

    if (sndtimeo)
        so_sndtimeo_enable(fd, sndtimeo);

    return 0;
}

static int net_socket_connect(int id, net_client_type type, uint32_t ip, uint16_t port, int keepalive, uint16_t local_port, int mode)
{
    int fd;
    void *priv = NULL;
    int keep_alive = 0;
    int so_linger = -1;
    int tcp_nodelay = 0;
    int so_sndtimeo = 0;

    if (g_at_client_handle[id].valid) {
        AT_NET_PRINTF("already connected\r\n");
        return -1;
    }

    if (type == NET_CLIENT_TCP) {
        fd = tcp_client_connect(ip, port);
        AT_NET_PRINTF("tcp_client_connect fd:%d\r\n", fd);
        if (fd >= 0) {
            keep_alive = keepalive;
            so_linger = at_net_config->tcp_opt[id].so_linger;
            tcp_nodelay = at_net_config->tcp_opt[id].tcp_nodelay;
            so_sndtimeo = at_net_config->tcp_opt[id].so_sndtimeo;
            local_port = so_localport_get(fd);

            net_socket_setopt(fd, keep_alive, so_linger, tcp_nodelay, so_sndtimeo);    
        }
    }
    else if (type == NET_CLIENT_UDP) {
        if (local_port == 0)
            local_port = udp_localport_rand();
        fd = udp_client_connect(local_port);
        if (fd >= 0 && (htonl(ip) > 0xE0000000 && htonl(ip) <= 0xEFFFFFFF))
            ip_multicast_enable(fd, ip);
    }
    else if (type == NET_CLIENT_SSL) {
        fd = ssl_client_connect(id, ip, port, &priv);
        if (fd >= 0) {
            if (keepalive) {
                so_keepalive_enable(fd, keepalive, 1, 3);
                keep_alive = keepalive;
            }

            local_port = so_localport_get(fd);
        }
    }
    else {
        AT_NET_PRINTF("type error\r\n");
        return -1;
    }

    if (fd < 0) {
        AT_NET_PRINTF("net_socket_connect connect failed, fd:%d\r\n", fd);
        return -1;
    }

    net_lock();
    g_at_client_handle[id].valid = 1;
    g_at_client_handle[id].type = type;
    g_at_client_handle[id].fd = fd;
    g_at_client_handle[id].priv = priv;
    g_at_client_handle[id].remote_ip = ip;
    g_at_client_handle[id].remote_port = port;
    g_at_client_handle[id].local_port = local_port;
    g_at_client_handle[id].udp_mode = mode;
    g_at_client_handle[id].tetype = 0;
    g_at_client_handle[id].recv_timeout = 0;
    g_at_client_handle[id].recv_time = 0;
    g_at_client_handle[id].keep_alive = keep_alive;
    g_at_client_handle[id].so_linger = so_linger;
    g_at_client_handle[id].tcp_nodelay = tcp_nodelay;
    g_at_client_handle[id].so_sndtimeo = so_sndtimeo;
    if (!g_at_client_handle[id].recv_buf) {
        g_at_client_handle[id].recv_buf = pvPortMalloc(g_at_client_handle[id].recvbuf_size);
    }
    net_unlock();

    net_socket_ipd(NET_IPDINFO_CONNECTED, id, NULL, 0, g_at_client_handle[id].remote_ip, g_at_client_handle[id].remote_port);
    return 0;
}

static int net_socket_close(int id)
{
    int valid = g_at_client_handle[id].valid;
    net_client_type type = g_at_client_handle[id].type;
    int fd = g_at_client_handle[id].fd;
    void *priv = g_at_client_handle[id].priv;
    int servid = 0;
    
    if (!valid) {
        AT_NET_PRINTF("socket is not inited\r\n");
        return -1;
    }

    if (type == NET_CLIENT_TCP)
       tcp_client_close(fd);
    else if (type == NET_CLIENT_UDP)
       udp_client_close(fd);
    else if (type == NET_CLIENT_SSL)
       ssl_client_close(fd, priv);

    net_lock();
    if (at_get_work_mode() != AT_WORK_MODE_THROUGHPUT)
        g_at_client_handle[id].valid = 0;
    g_at_client_handle[id].fd = -1;
    g_at_client_handle[id].priv = NULL;
    g_at_client_handle[id].disconnect_time = os_get_time_ms();
    
    vPortFree(g_at_client_handle[id].recv_buf);
    g_at_client_handle[id].recv_buf = NULL;
    if (g_at_client_handle[id].tetype && g_at_server_handle[servid].valid) {
        g_at_server_handle[servid].client_num--;
        if (g_at_server_handle[servid].client_num < 0)
            g_at_server_handle[servid].client_num = 0;
    }
    net_unlock();

    net_socket_ipd(NET_IPDINFO_DISCONNECTED, id, NULL, 0, 0, 0);
    return 0;
}

static int net_socket_send(int id, void *buffer, int length)
{
    int ret = 0;

    int valid = g_at_client_handle[id].valid;
    int type = g_at_client_handle[id].type;
    int fd = g_at_client_handle[id].fd;   
    void *priv = g_at_client_handle[id].priv;
    uint32_t remote_ip = g_at_client_handle[id].remote_ip;
    uint16_t remote_port = g_at_client_handle[id].remote_port;

    if (!valid)
        return -1;

    if (type == NET_CLIENT_TCP)
       ret = tcp_client_send(fd, buffer, length);
    else if (type == NET_CLIENT_UDP)
       ret = udp_client_send(fd, buffer, length, remote_ip, remote_port);
    else if (type == NET_CLIENT_SSL)
       ret = ssl_client_send(fd, buffer, length, priv);

    return ret;
}

static int net_socket_recv(int id)
{
    int num = 0;
    struct sockaddr_in remote_addr;
    int len = sizeof(remote_addr);

    int type = g_at_client_handle[id].type;
    int fd = g_at_client_handle[id].fd;
    void *priv = g_at_client_handle[id].priv;
    uint32_t remote_ip = g_at_client_handle[id].remote_ip;
    uint16_t remote_port = g_at_client_handle[id].remote_port;
    int udp_mode = g_at_client_handle[id].udp_mode;

    if (type == NET_CLIENT_TCP) {
        num = recv(fd, 
                g_at_client_handle[id].recv_buf, 
                g_at_client_handle[id].recvbuf_size, 
                0);
        //num = so_recvsize_get(fd);
        //at_response_string("\r\n+IPD,%d:", num);
    }
    else if (type == NET_CLIENT_UDP) {
        num = recvfrom(fd, 
                g_at_client_handle[id].recv_buf, 
                g_at_client_handle[id].recvbuf_size, 
                0, 
                (struct sockaddr *)&remote_addr, 
                (socklen_t *)(&len));

        //update remote addr
        if (udp_mode == 1 || udp_mode == 2) {
            if (remote_ip != remote_addr.sin_addr.s_addr || remote_port != ntohs(remote_addr.sin_port)) {
                net_lock();
                g_at_client_handle[id].remote_ip = remote_addr.sin_addr.s_addr;
                g_at_client_handle[id].remote_port = ntohs(remote_addr.sin_port);
                if (udp_mode == 1)
                    g_at_client_handle[id].udp_mode = 0;
                net_unlock();
            }
        }
    }
    else if (type == NET_CLIENT_SSL) {
        num = mbedtls_ssl_recv(priv, 
                g_at_client_handle[id].recv_buf, 
                g_at_client_handle[id].recvbuf_size);
    }
    else
        return 0;

    if (num <= 0 && type != NET_CLIENT_UDP) {
        net_socket_close(id);
    }
    else {

        static uint32_t count = 0;
        static uint32_t sum = 0;
        uint32_t diff = xTaskGetTickCount() - count;
            
        sum += num;
        if (diff >= 1000) {
            printf("RX:%.4f Mbps\r\n", (float)sum * 8 / 1000 / 1000 * diff / 1000);
            count = xTaskGetTickCount();
            sum = 0;
        }

        net_socket_ipd(NET_IPDINFO_RECVDATA, 
                       id, 
                       g_at_client_handle[id].recv_buf, 
                       num, 
                       g_at_client_handle[id].remote_ip, 
                       g_at_client_handle[id].remote_port);

        net_lock();
        g_at_client_handle[id].recv_time = os_get_time_ms();
        net_unlock();
    }

    return num;
}

static int net_socket_accept(int fd, int type, uint16_t port, uint16_t timeout, uint8_t num, uint8_t max_conn)
{
    int sock;
    struct sockaddr_in remote_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int id;
    void *priv = NULL;

    if (type == NET_SERVER_UDP) {
    	for (int i = 0; i < AT_NET_CLIENT_HANDLE_MAX; i++) {
    		if (g_at_client_handle[id].valid && g_at_client_handle[id].fd == fd) {
    			return 0;
    		}
    	}
    	sock = fd;
    } else {
    	sock = accept(fd, (struct sockaddr*)&remote_addr, &len);
    }

    if (sock >= 0) {
        if (num+1 > max_conn) {
            close(sock);
            return 0;
        }

        id = at_net_client_get_valid_id();
        if (id < 0) {
            close(sock);
            return 0;
        }

        if (type == NET_SERVER_SSL) {

            romfs_filebuf_t ca = {0}, cert = {0}, key = {0};

            get_romfs_file_content(g_at_server_handle[id].ca_path, &ca);
            get_romfs_file_content(g_at_server_handle[id].cert_path, &cert);
            get_romfs_file_content(g_at_server_handle[id].priv_key_path, &key);

            priv = mbedtls_ssl_accept(sock, ca.buf, ca.bufsize, cert.buf, cert.bufsize, key.buf, key.bufsize);
            //priv = mbedtls_ssl_accept(sock, NULL, 0, NULL, 0, NULL, 0);
            if (priv == NULL) {
                close(sock);
                return 0;
            }
        }

        net_lock();
        g_at_client_handle[id].valid = 1;
        g_at_client_handle[id].type = type;
        g_at_client_handle[id].fd = sock;
        g_at_client_handle[id].priv = priv;
        g_at_client_handle[id].remote_ip = remote_addr.sin_addr.s_addr;
        g_at_client_handle[id].remote_port = ntohs(remote_addr.sin_port);
        g_at_client_handle[id].local_port = port;
        g_at_client_handle[id].udp_mode = 0;
        g_at_client_handle[id].tetype = 1;
        g_at_client_handle[id].recv_timeout = timeout;
        g_at_client_handle[id].recv_time = os_get_time_ms();
        g_at_client_handle[id].keep_alive = 0;
        g_at_client_handle[id].so_linger = -1;
        g_at_client_handle[id].tcp_nodelay = 0;
        g_at_client_handle[id].so_sndtimeo = 0;
        if (!g_at_client_handle[id].recv_buf) {
            g_at_client_handle[id].recv_buf = pvPortMalloc(g_at_client_handle[id].recvbuf_size);
        }
        net_unlock();

        net_socket_ipd(NET_IPDINFO_CONNECTED, id, NULL, 0, g_at_client_handle[id].remote_ip, g_at_client_handle[id].remote_port);
        return 1;
    }

    return 0;
}

static void net_poll_reconnect(void)
{
    int id = 0;
    int fd;
    void *priv;
    uint32_t ip;
    uint16_t port;
    uint16_t local_port;
    struct hostent *hostinfo;

    if (g_at_client_handle[id].valid && g_at_client_handle[id].fd < 0) {
        if ((at_get_work_mode() != AT_WORK_MODE_THROUGHPUT) && (at_get_work_mode() != AT_WORK_MODE_CMD_THROUGHPUT)) {
            net_lock();
            g_at_client_handle[id].valid = 0;
            net_unlock();
            return;
        }

        if (os_get_time_ms() - g_at_client_handle[id].disconnect_time <= at_net_config->reconn_intv*100)
            return;
        g_at_client_handle[id].disconnect_time = os_get_time_ms();

        if (!net_is_active())
            return;

        if (g_at_client_handle[id].remote_ip == 0) {
            hostinfo = gethostbyname(g_at_net_savelink_host);
            if (!hostinfo) {
                return;
            }
            ip = (uint32_t)((struct in_addr *) hostinfo->h_addr)->s_addr;
        }
        else
            ip = g_at_client_handle[id].remote_ip;
        port = g_at_client_handle[id].remote_port;
        local_port = g_at_client_handle[id].local_port;
    
        if (g_at_client_handle[id].type == NET_CLIENT_TCP) {
            fd = tcp_client_connect(ip, port);
            if (fd >= 0) {
                net_socket_setopt(fd, g_at_client_handle[id].keep_alive, g_at_client_handle[id].so_linger, g_at_client_handle[id].tcp_nodelay, g_at_client_handle[id].so_sndtimeo);
                local_port = so_localport_get(fd);
            }
        }
        else if (g_at_client_handle[id].type == NET_CLIENT_UDP) {
            if (local_port == 0)
                local_port = udp_localport_rand();
            fd = udp_client_connect(local_port);
            if (fd >= 0 && (htonl(ip) > 0xE0000000 && htonl(ip) <= 0xEFFFFFFF))
                ip_multicast_enable(fd, ip);
        }
        else if (g_at_client_handle[id].type == NET_CLIENT_SSL) {
            fd = ssl_client_connect(id, ip, port, &priv);
            if (fd >= 0) {
                if (g_at_client_handle[id].keep_alive)
                    so_keepalive_enable(fd, g_at_client_handle[id].keep_alive, 1, 3);
                local_port = so_localport_get(fd);
            }
        }
        else
            return;

        if (fd < 0)
            return;

        net_lock();
        g_at_client_handle[id].fd = fd;
        g_at_client_handle[id].priv = priv;
        g_at_client_handle[id].remote_ip = ip;
        g_at_client_handle[id].remote_port = port;
        g_at_client_handle[id].local_port = local_port;
        if (!g_at_client_handle[id].recv_buf) {
            g_at_client_handle[id].recv_buf = pvPortMalloc(g_at_client_handle[id].recvbuf_size);
        }
        net_unlock();
    }
}

static void net_poll_recv(void)
{
    fd_set fdR;
    struct timeval timeout;
    int maxfd = -1;
    int i;

    FD_ZERO(&fdR);
    for (i = 0; i < AT_NET_CLIENT_HANDLE_MAX; i++) {
        if (g_at_client_handle[i].valid && g_at_client_handle[i].fd >= 0) {
            FD_SET(g_at_client_handle[i].fd, &fdR);
            if (g_at_client_handle[i].fd > maxfd)
                maxfd = g_at_client_handle[i].fd;
        }
    }
    for (i = 0; i < AT_NET_SERVER_HANDLE_MAX; i++) {
        if (g_at_server_handle[i].valid) {
            FD_SET(g_at_server_handle[i].fd, &fdR);
            if (g_at_server_handle[i].fd > maxfd)
                maxfd = g_at_server_handle[i].fd;
        }
    }

    if (maxfd == -1) {
#ifdef LP_APP
        vTaskDelay(30000);
#else
        vTaskDelay(100);
#endif
        return;
    }

    timeout.tv_sec= 0;	
    timeout.tv_usec= 10000;
#ifdef LP_APP
    if(select(maxfd+1, &fdR, NULL, NULL, NULL) > 0) {
#else
    if(select(maxfd+1, &fdR, NULL, NULL, &timeout) > 0) {
#endif
         for (i = 0; i < AT_NET_CLIENT_HANDLE_MAX; i++) {
            if (g_at_client_handle[i].valid && g_at_client_handle[i].fd >= 0 && FD_ISSET(g_at_client_handle[i].fd, &fdR)) {
                net_socket_recv(i);
            }
        }
        for (i = 0; i < AT_NET_SERVER_HANDLE_MAX; i++) {
            if (g_at_server_handle[i].valid && FD_ISSET(g_at_server_handle[i].fd, &fdR)) {
                if (net_socket_accept(g_at_server_handle[i].fd, g_at_server_handle[i].type,
                            g_at_server_handle[i].port, g_at_server_handle[i].recv_timeout,
                            g_at_server_handle[i].client_num, g_at_server_handle[i].client_max) ) {
                    g_at_server_handle[i].client_num++;
                }
            }
        }
    }
}

static void net_poll_timeout(void)
{
    int i;

    for (i = 0; i < AT_NET_CLIENT_HANDLE_MAX; i++) {
        if (g_at_client_handle[i].valid && g_at_client_handle[i].recv_timeout > 0) {
            if ((os_get_time_ms() - g_at_client_handle[i].recv_time)/1000 > g_at_client_handle[i].recv_timeout)
                net_socket_close(i);
        }
    }
}

static void net_init_save_link(void)
{
    net_client_type type;
    int id;

    if (at_net_config->trans_link.enable) {
        if (strcmp(at_net_config->trans_link.type, "TCP") == 0)
            type = NET_CLIENT_TCP;
        else if (strcmp(at_net_config->trans_link.type, "UDP") == 0)
            type = NET_CLIENT_UDP;
        else if (strcmp(at_net_config->trans_link.type, "SSL") == 0)
            type = NET_CLIENT_SSL;
        else
            return;

        at_set_work_mode(AT_WORK_MODE_CMD_THROUGHPUT);
        at_net_config->work_mode = NET_MODE_TRANS;

        id = at_net_client_get_valid_id();
        net_lock();
        g_at_client_handle[id].valid = 1;
        g_at_client_handle[id].type = type;
        g_at_client_handle[id].fd = -1;
        g_at_client_handle[id].remote_ip = 0;
        strlcpy(g_at_net_savelink_host, at_net_config->trans_link.remote_host, sizeof(g_at_net_savelink_host));
        g_at_client_handle[id].remote_port = at_net_config->trans_link.remote_port;
        g_at_client_handle[id].local_port = at_net_config->trans_link.local_port;
        g_at_client_handle[id].udp_mode = 0;
        g_at_client_handle[id].tetype = 0;
        g_at_client_handle[id].recv_timeout = 0;
        g_at_client_handle[id].recv_time = 0;
        g_at_client_handle[id].keep_alive = at_net_config->trans_link.keep_alive;
        g_at_client_handle[id].so_linger = -1;
        g_at_client_handle[id].tcp_nodelay = 0;
        g_at_client_handle[id].so_sndtimeo = 0;
        net_unlock();
    }
}

static void net_main_task(void *pvParameters)
{
    g_at_net_task_is_start = 1;

    net_init_save_link();

    while(1) {
        net_poll_reconnect();

        net_poll_recv();

        net_poll_timeout();

        if (g_at_net_task_is_start == 2)
            break;
    }

    g_at_net_task_is_start = 0;
    vTaskDelete(NULL);
}

static int at_net_init(void)
{
    if (g_at_client_handle)
        return 0;

    g_at_client_handle = (at_net_client_handle *)pvPortMalloc(sizeof(at_net_client_handle) * AT_NET_CLIENT_HANDLE_MAX);
    if (!g_at_client_handle) {
        AT_NET_PRINTF("at_net malloc failed!\r\n");
        return -1;
    }

    g_at_server_handle = (at_net_server_handle *)pvPortMalloc(sizeof(at_net_server_handle) * AT_NET_SERVER_HANDLE_MAX);
    if (!g_at_server_handle) {
        AT_NET_PRINTF("at_net malloc failed!\r\n");
        vPortFree(g_at_client_handle);
        g_at_client_handle = NULL;
        return -1;
    }
    net_mutex = xSemaphoreCreateMutex();

    memset(g_at_client_handle, 0, sizeof(at_net_client_handle) * AT_NET_CLIENT_HANDLE_MAX);
    memset(g_at_server_handle, 0, sizeof(at_net_server_handle) * AT_NET_SERVER_HANDLE_MAX);

    for (int id = 0; id < AT_NET_CLIENT_HANDLE_MAX; id++) {
        g_at_client_handle[id].recvbuf_size = AT_NET_RECV_BUF_SIZE;
        at_net_ssl_path_set(id, at_net_config->sslconf[id].ca_file, 
                            at_net_config->sslconf[id].cert_file, 
                            at_net_config->sslconf[id].key_file); 
    }

    return 0;
}

static int at_net_deinit(void)
{
    if (g_at_client_handle) {
        vPortFree(g_at_client_handle);
        g_at_client_handle = NULL;

        vPortFree(g_at_server_handle);
        g_at_server_handle = NULL;
    }
    return 0;
}

int at_net_sock_is_build(void)
{
    int i;

    for (i = 0; i < AT_NET_CLIENT_HANDLE_MAX; i++) {
        if (g_at_client_handle[i].valid)
            return 1;
    }

    for (i = 0; i < AT_NET_SERVER_HANDLE_MAX; i++) {
        if (g_at_server_handle[i].valid)
            return 1;
    }

    return 0;
}

int at_net_client_get_valid_id(void)
{
    int i;

    for (i = 0; i < AT_NET_CLIENT_HANDLE_MAX; i++) {
        if (g_at_client_handle[i].valid == 0)
            return i;
    }
    return -1;
}

int at_net_client_id_is_valid(int id)
{
    if (id < 0 || id >= AT_NET_CLIENT_HANDLE_MAX)
        return 0;
    else
        return 1;
}

int at_net_client_tcp_connect(int id, uint32_t remote_ip, uint16_t remote_port, int keepalive)
{
    CHECK_NET_CLIENT_ID_VALID(id);

    return net_socket_connect(id, NET_CLIENT_TCP, remote_ip, remote_port, keepalive, 0, 0);
}

int at_net_client_udp_connect(int id, uint32_t remote_ip, uint16_t remote_port, uint16_t local_port, int mode)
{
    CHECK_NET_CLIENT_ID_VALID(id);

    return net_socket_connect(id, NET_CLIENT_UDP, remote_ip, remote_port, 0, local_port, mode);
}

int at_net_client_ssl_connect(int id, uint32_t remote_ip, uint16_t remote_port, int keepalive)
{
    CHECK_NET_CLIENT_ID_VALID(id);

    return net_socket_connect(id, NET_CLIENT_SSL, remote_ip, remote_port, keepalive, 0, 0);
}

int at_net_client_is_connected(int id)
{
    CHECK_NET_CLIENT_ID_VALID(id);

    if (g_at_client_handle[id].valid && g_at_client_handle[id].fd >= 0)
        return 1;

    return 0;
}

int at_net_client_set_remote(int id, uint32_t ip, uint16_t port)
{
    CHECK_NET_CLIENT_ID_VALID(id);

    if (!g_at_client_handle[id].valid)
        return -1;

    if (g_at_client_handle[id].type != NET_CLIENT_UDP)
        return -1;

    g_at_client_handle[id].remote_ip = ip;
    g_at_client_handle[id].remote_port = port;
    AT_NET_PRINTF("net set id: %d remote_ip: 0x%x remote_port: %d\r\n", id, ip, port);
    return 0;
}

int at_net_client_get_info(int id, char *type, uint32_t *remote_ip, uint16_t *remote_port, uint16_t *local_port, uint8_t *tetype)
{
    CHECK_NET_CLIENT_ID_VALID(id);

    if (g_at_client_handle[id].valid) {
        if (type) {
            if (g_at_client_handle[id].type == NET_CLIENT_TCP)
                strlcpy(type, "TCP", sizeof(type));
            else if (g_at_client_handle[id].type == NET_CLIENT_UDP)
                strlcpy(type, "UDP", sizeof(type));
            if (g_at_client_handle[id].type == NET_CLIENT_SSL)
                strlcpy(type, "SSL", sizeof(type));
        }
        if (remote_ip)
            *remote_ip = g_at_client_handle[id].remote_ip;
        if (remote_port)
            *remote_port = g_at_client_handle[id].remote_port;
        if (local_port)
            *local_port = g_at_client_handle[id].local_port;
        if (tetype)
            *tetype = g_at_client_handle[id].tetype;
        return 0;
    }
    return -1;
}

int at_net_client_get_recvsize(int id)
{
    CHECK_NET_CLIENT_ID_VALID(id);

    if (g_at_client_handle[id].valid && g_at_client_handle[id].fd >= 0)
        return so_recvsize_get(g_at_client_handle[id].fd);
    else
        return -1;
}

int at_net_client_send(int id, void * buffer, int length)
{
    CHECK_NET_CLIENT_ID_VALID(id);

    return net_socket_send(id, buffer, length);
}

int at_net_client_close(int id)
{
    CHECK_NET_CLIENT_ID_VALID(id);

    return net_socket_close(id);
}

int at_net_client_close_all(void)
{
    int i;

    for (i = 0; i < AT_NET_CLIENT_HANDLE_MAX; i++) {
        if (g_at_client_handle[i].valid)
            net_socket_close(i);
    }

    return 0;
}

int at_net_server_id_is_valid(int id)
{
    if (id < 0 || id >= AT_NET_SERVER_HANDLE_MAX)
        return 0;
    else
        return 1;
}

int at_net_server_udp_create(uint16_t port, int max_conn,  int timeout)
{
    int id = 0;
    int fd;

    if (g_at_server_handle[id].valid) {
        AT_NET_PRINTF("already create\r\n");
        return -1;
    }

    fd = udp_server_create(port, max_conn);
    if (fd < 0) {
        AT_NET_PRINTF("create failed\r\n");
        return -1;
    }

    net_lock();
    g_at_server_handle[id].valid = 1;
    g_at_server_handle[id].type = NET_SERVER_UDP;
    g_at_server_handle[id].fd = fd;
    g_at_server_handle[id].port = port;
    g_at_server_handle[id].recv_timeout = timeout;
    g_at_server_handle[id].ca_enable = 0;
    g_at_server_handle[id].client_max = max_conn;
    g_at_server_handle[id].client_num = 0;
    net_unlock();
    return 0;
}

int at_net_server_tcp_create(uint16_t port, int max_conn,  int timeout)
{
    int id = 0;
    int fd;

    if (g_at_server_handle[id].valid) {
        AT_NET_PRINTF("already create\r\n");
        return -1;
    }

    fd = tcp_server_create(port, max_conn);
    if (fd < 0) {
        AT_NET_PRINTF("create failed\r\n");
        return -1;
    }

    net_lock();
    g_at_server_handle[id].valid = 1;
    g_at_server_handle[id].type = NET_SERVER_TCP;
    g_at_server_handle[id].fd = fd;
    g_at_server_handle[id].port = port;
    g_at_server_handle[id].recv_timeout = timeout;
    g_at_server_handle[id].ca_enable = 0;
    g_at_server_handle[id].client_max = max_conn;
    g_at_server_handle[id].client_num = 0;
    net_unlock();
    return 0;
}

int at_net_server_ssl_create(uint16_t port, int max_conn,  int timeout, int ca_enable)
{
    int id = 0;
    int fd;

    if (g_at_server_handle[id].valid) {
        AT_NET_PRINTF("already create\r\n");
        return -1;
    }

    fd = tcp_server_create(port, max_conn);
    if (fd < 0) {
        AT_NET_PRINTF("create failed\r\n");
        return -1;
    }

    net_lock();
    g_at_server_handle[id].valid = 1;
    g_at_server_handle[id].type = NET_SERVER_SSL;
    g_at_server_handle[id].fd = fd;
    g_at_server_handle[id].port = port;
    g_at_server_handle[id].recv_timeout = timeout;
    g_at_server_handle[id].ca_enable = 0;
    g_at_server_handle[id].client_max = max_conn;
    g_at_server_handle[id].client_num = 0;
    net_unlock();
    return 0;
}

int at_net_server_is_created(uint16_t *port, char *type, int *ca_enable)
{
    int id = 0;
    CHECK_NET_SERVER_ID_VALID(id);

    if (!g_at_server_handle[id].valid)
        return 0;

    if (port) {
        *port = g_at_server_handle[id].port;
    }
    if (type) {
        if ( g_at_server_handle[id].type == NET_SERVER_TCP)
            strlcpy(type, "TCP", sizeof(type));
        else if ( g_at_server_handle[id].type == NET_SERVER_SSL)
            strlcpy(type, "SSL", sizeof(type));
    }
    if (ca_enable) {
        *ca_enable = g_at_server_handle[id].ca_enable;
    }
    return 1;
}

int at_net_server_close(void)
{
    int id = 0;
    int valid = g_at_server_handle[id].valid;
    net_server_type type = g_at_server_handle[id].type;
    int fd = g_at_server_handle[id].fd;;
    
    if (!valid) {
        AT_NET_PRINTF("socket is not inited\r\n");
        return -1;
    }

    net_lock();
    g_at_server_handle[id].valid = 0;
    net_unlock();

    if (type == NET_SERVER_TCP || type == NET_SERVER_SSL)
       tcp_server_close(fd);

    return 0;
}

static void at_net_sntp_sync(void)
{
    uint64_t time_stamp, time_stamp_ms;
    uint32_t seconds = 0, frags = 0;

    sntp_get_time(&seconds, &frags);
    time_stamp = seconds;
    time_stamp_ms = time_stamp*1000 + frags/1000;
    at_base_config->systime_stamp = time_stamp_ms - at_current_ms_get();
}

int at_net_sntp_start(void)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    if (at_net_config->sntp_cfg.servernum >= 1)
        sntp_setservername(0, at_net_config->sntp_cfg.server1);
    else
        sntp_setservername(0, NULL);
    if (at_net_config->sntp_cfg.servernum >= 2)
        sntp_setservername(1, at_net_config->sntp_cfg.server2);
    else
        sntp_setservername(1, NULL);
    if (at_net_config->sntp_cfg.servernum >= 3)
        sntp_setservername(2, at_net_config->sntp_cfg.server3);
    else
        sntp_setservername(2, NULL);
    sntp_settimesynccb(at_net_sntp_sync);
    sntp_setupdatedelay(at_net_config->sntp_intv.interval);
    sntp_init();
    g_at_net_sntp_is_start = 1;
    return 0;
}

uint64_t at_current_ms_get()
{
    uint64_t current_ms;
    TimeOut_t xCurrentTime = {0};
    vTaskSetTimeOutState(&xCurrentTime);
    current_ms = ( uint64_t ) ( xCurrentTime.xOverflowCount ) << ( sizeof( TickType_t ) * 8 );
    current_ms += xCurrentTime.xTimeOnEntering;
    current_ms = current_ms * portTICK_PERIOD_MS;
    return current_ms;
}

int at_net_sntp_stop(void)
{
    sntp_stop();
    g_at_net_sntp_is_start = 0;
    return 0;
}

int at_net_sntp_is_start(void)
{
    return g_at_net_sntp_is_start;
}

int at_net_start(void)
{
    int ret;

    at_net_init();


    ret = xTaskCreate(net_main_task, (char*)"net_main_task", AT_NET_TASK_STACK_SIZE, NULL, AT_NET_TASK_PRIORITY, NULL);
    if (ret != pdPASS) {
        AT_NET_PRINTF("ERROR: create net_main_task failed, ret = %d\r\n", ret);
        return -1;
    }
    return 0;
}

int at_net_stop(void)
{
    at_net_client_close_all();
    if (g_at_net_task_is_start) {
        g_at_net_task_is_start = 2;
        while(g_at_net_task_is_start != 0)
            vTaskDelay(100);
    }
    at_net_deinit();
    return 0;
}

int at_net_recvbuf_size_set(int linkid, uint32_t size)
{
    net_lock();
    if (!g_at_client_handle[linkid].recv_buf) {
        g_at_client_handle[linkid].recvbuf_size = size;
    }
    net_unlock();
    return 0;
}

int at_net_recvbuf_size_get(int linkid)
{
    return g_at_client_handle[linkid].recvbuf_size;
}

int at_net_ssl_path_set(int linkid, const char *ca, const char *cert, const char *key)
{
    if (ca) {
        strlcpy(g_at_client_handle[linkid].ca_path, ca, sizeof(g_at_client_handle[linkid].ca_path));
    } else {
        g_at_client_handle[linkid].ca_path[0] = '\0';
    }

    if (cert) {
        strlcpy(g_at_client_handle[linkid].cert_path, cert, sizeof(g_at_client_handle[linkid].cert_path));
    } else {
        g_at_client_handle[linkid].cert_path[0] = '\0';
    }

    if (key) {
        strlcpy(g_at_client_handle[linkid].priv_key_path, key, sizeof(g_at_client_handle[linkid].priv_key_path));
    } else {
        g_at_client_handle[linkid].priv_key_path[0] = '\0';
    }
    return 0;
}

int at_net_ssl_path_get(int linkid, const char **ca, const char **cert, const char **key)
{
    *ca = g_at_client_handle[linkid].ca_path;
    *cert = g_at_client_handle[linkid].cert_path;
    *key = g_at_client_handle[linkid].priv_key_path;
    return 0;
}

int at_net_ssl_sni_set(int linkid, const char *sni)
{
    if (g_at_client_handle[linkid].ssl_hostname) {
        vPortFree(g_at_client_handle[linkid].ssl_hostname);
    }
    g_at_client_handle[linkid].ssl_hostname = strdup(sni);
    return 0;
}

char *at_net_ssl_sni_get(int linkid)
{
    return g_at_client_handle[linkid].ssl_hostname;
}

int at_net_ssl_alpn_set(int linkid, int alpn_num, const char *alpn)
{
    net_lock();

    if (alpn == NULL) {
        vPortFree(g_at_client_handle[linkid].ssl_alpn[alpn_num]);
        g_at_client_handle[linkid].ssl_alpn[alpn_num] = NULL;
    } else {
        if (g_at_client_handle[linkid].ssl_alpn[alpn_num]) {
            vPortFree(g_at_client_handle[linkid].ssl_alpn[alpn_num]);
        }
        g_at_client_handle[linkid].ssl_alpn[alpn_num] = strdup(alpn);
    }

    g_at_client_handle[linkid].ssl_alpn_num = 0;
    for (int i = 0; i < 6; i++) {
        if (g_at_client_handle[linkid].ssl_alpn[i]) {
            g_at_client_handle[linkid].ssl_alpn_num++;
        }
    }
    net_unlock();
    return 0;
}

char **at_net_ssl_alpn_get(int linkid, int *alpn_num)
{
    if (alpn_num) {
        *alpn_num = g_at_client_handle[linkid].ssl_alpn_num;
    }
    return g_at_client_handle[linkid].ssl_alpn;
}

int at_net_ssl_psk_set(int linkid, char *psk, int psk_len, char *pskhint, int pskhint_len)
{
    memcpy(g_at_client_handle[linkid].ssl_psk, psk, psk_len);
    g_at_client_handle[linkid].ssl_psklen = psk_len;
    
    memcpy(g_at_client_handle[linkid].ssl_pskhint, pskhint, pskhint_len);
    g_at_client_handle[linkid].ssl_pskhint_len = pskhint_len;
    return 0;
}

int at_net_ssl_psk_get(int linkid, char **psk, int *psk_len, char **pskhint, int *pskhint_len)
{
    if (psk) {
        *psk = g_at_client_handle[linkid].ssl_psk;
    }
    if (psk_len) {
        *psk_len = g_at_client_handle[linkid].ssl_psklen;
    }
    
    if (pskhint) {
        *pskhint = g_at_client_handle[linkid].ssl_pskhint;
    }
    if (pskhint_len) {
        *pskhint_len = g_at_client_handle[linkid].ssl_pskhint_len;
    }
    return 0;
}

