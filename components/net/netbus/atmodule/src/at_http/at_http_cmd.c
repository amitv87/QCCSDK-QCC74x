/**
  ******************************************************************************
  * @file    at_http_cmd.c
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
#include "at_httpc_main.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include <stream_buffer.h>

#define AT_HTTPC_DEFAULT_TIMEOUT (5000)
#define AT_HTTPC_RECVBUF_SIZE_DEFAULT (1024)

#define AT_HTTPC_RECV_MODE_ACTIVE  0
#define AT_HTTPC_RECV_MODE_PASSIVE 1

#define AT_HTTP_EVT_HEAD(s)      (g_https_cfg.recv_mode == AT_HTTPC_RECV_MODE_PASSIVE)?s"\r\n":s","

struct at_http_ctx {
    httpc_connection_t settings;
    uint8_t *data;
    char url_buf[256];
};

struct at_https_global_config {
#define AT_HTTPS_NOT_AUTH        0
#define AT_HTTPS_SERVER_AUTH     1
#define AT_HTTPS_CLIENT_AUTH     2
#define AT_HTTPS_BOTH_AUTH       3
    uint8_t https_auth_type;
    uint8_t recv_mode;
    char ca_file[32];
    char cert_file[32];
    char key_file[32];

    char *url;
    uint32_t url_size;
    SemaphoreHandle_t mutex;
    StreamBufferHandle_t recv_buf;
    uint32_t recvbuf_size;
    struct altcp_pcb *altcp_conn;
};

struct at_https_global_config g_https_cfg = {0};

static int httpc_get_recvsize(void)
{
    if (g_https_cfg.recv_buf)
        return xStreamBufferBytesAvailable(g_https_cfg.recv_buf);
    else
        return 0;
}

static int httpc_buffer_write(struct at_http_ctx *ctx, void *buf, uint32_t len)
{
    int ret = 0;

    xSemaphoreTake(g_https_cfg.mutex, portMAX_DELAY);
    ret = xStreamBufferSend(g_https_cfg.recv_buf, buf, len, 3000);
    xSemaphoreGive(g_https_cfg.mutex);

    ret = ret < 0 ? 0 : ret;

    if (ret < len) {
        at_write("+HTTPCLOST,%d\r\n", len - ret);
    }
    return ret;
}

static err_t cb_altcp_recv_fn(void *arg, struct altcp_pcb *conn, struct pbuf *p, err_t err)
{
    struct at_http_ctx *ctx = (struct at_http_ctx *)arg;

    if (g_https_cfg.recv_mode == AT_HTTPC_RECV_MODE_ACTIVE) {
        altcp_recved(conn, p->tot_len);
        if (p->tot_len) {
            AT_CMD_DATA_SEND(p->payload, p->tot_len);
        }
    } else {
        g_https_cfg.altcp_conn = conn;
        httpc_buffer_write(ctx, p->payload, p->tot_len);
        //altcp_recved(conn, p->tot_len);
    }
    
    pbuf_free(p);
    return 0;
}

static void cb_httpc_result(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err)
{
    struct at_http_ctx *ctx = (struct at_http_ctx *)arg;

    printf("[DATA] Transfer finished err %d. rx_content_len is %lu httpc_result: %d\r\n", err, rx_content_len, httpc_result);
    
    if ((err == 0 && httpc_result == HTTPC_RESULT_OK) || 
        (ctx->settings.req_type == REQ_TYPE_HEAD && httpc_result == HTTPC_RESULT_ERR_CONTENT_LEN)) {
        at_response_string("+HTTPSTATUS:OK\r\n");
    } else {
        at_response_string("+HTTPSTATUS:ERROR\r\n");
    }
    free(ctx->data);
    ctx->data = NULL;
#if LWIP_ALTCP_TLS && LWIP_ALTCP_TLS_MBEDTLS 
    if (ctx->settings.tls_config) {
        altcp_tls_free_config(ctx->settings.tls_config);
    }
#endif
    g_https_cfg.altcp_conn = NULL;
    free(ctx);
}

static err_t cb_httpc_headers_done_fn(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len)
{
    struct at_http_ctx *ctx = (struct at_http_ctx *)arg;

    printf("[HEAD] hdr->tot_len is %u, hdr_len is %u, content_len is %lu\r\n", hdr->tot_len, hdr_len, content_len);

    //printf((char *)hdr->payload);

    if (ctx->settings.req_type == REQ_TYPE_HEAD) {
        if (hdr->tot_len) {
            at_write(AT_HTTP_EVT_HEAD("+HTTPC:%d"), hdr_len);
    
            if (g_https_cfg.recv_mode == AT_HTTPC_RECV_MODE_ACTIVE) {
                AT_CMD_DATA_SEND(hdr->payload, hdr->tot_len);
            } else {
                httpc_buffer_write(ctx, hdr->payload, hdr->tot_len);
            }
        }
    } else {
        at_write(AT_HTTP_EVT_HEAD("+HTTPC:%d"), content_len);
    }
    return ERR_OK;
}

static int at_httpc_request(struct at_http_ctx *ctx, 
                            const char *url_buf,
                            httpc_result_fn result_fn, 
                            httpc_headers_done_fn headers_done_fn,
                            altcp_recv_fn recv_fn,
                            void *parg)
{

    char *param, *host_name, *url;
    httpc_state_t *req = NULL;
    ip_addr_t ip_addr;
    char *p_port;
    int port;

#if LWIP_ALTCP_TLS && LWIP_ALTCP_TLS_MBEDTLS
    ctx->settings.tls_config = NULL;
#endif
    ctx->settings.result_fn = result_fn;
    ctx->settings.headers_done_fn = headers_done_fn;

    if (0 == strncmp(url_buf, "http://", 7)) {
        port = 80;
        url = url_buf + 7;
    } else if (0 == strncmp(url_buf, "https://", 8)) {
        port = 443;
        url = url_buf + 8;
#if LWIP_ALTCP_TLS && LWIP_ALTCP_TLS_MBEDTLS 
        uint8_t *ca_buf = NULL, *cert_buf = NULL, *privkey_buf = NULL;
        uint32_t ca_len, cert_len, privkey_len;

        if (g_https_cfg.https_auth_type == AT_HTTPS_NOT_AUTH) {

            ctx->settings.tls_config = altcp_tls_create_config_client(NULL, 0);

        } else if (g_https_cfg.https_auth_type == AT_HTTPS_CLIENT_AUTH) {

            at_load_file(g_https_cfg.cert_file, &cert_buf, &cert_len);
            at_load_file(g_https_cfg.key_file, &privkey_buf, &privkey_len);

            ctx->settings.tls_config = altcp_tls_create_config_client_2wayauth(NULL, 0,
                                                                               privkey_buf, privkey_len,
                                                                               NULL, 0,
                                                                               cert_buf, cert_len);

            free(cert_buf);
            free(privkey_buf);
        } else if (g_https_cfg.https_auth_type == AT_HTTPS_SERVER_AUTH) {

            at_load_file(g_https_cfg.ca_file, &ca_buf, &ca_len);
            ctx->settings.tls_config = altcp_tls_create_config_client(ca_buf, ca_len);
            free(ca_buf);

        } else if (g_https_cfg.https_auth_type == AT_HTTPS_BOTH_AUTH) {

            at_load_file(g_https_cfg.cert_file, &cert_buf, &cert_len);
            at_load_file(g_https_cfg.key_file, &privkey_buf, &privkey_len);
            at_load_file(g_https_cfg.ca_file, &ca_buf, &ca_len);

            ctx->settings.tls_config = altcp_tls_create_config_client_2wayauth(ca_buf, ca_len,
                                                                               privkey_buf, privkey_len,
                                                                               NULL, 0,
                                                                               cert_buf, cert_len);
            free(cert_buf);
            free(privkey_buf);
            free(ca_buf);
        }

#endif
    } else {
        free(ctx);
        return AT_RESULT_CODE_ERROR;
    }

    if ((param = strstr(url, "/")) == NULL) {
        param = url + strlen(url);
    }

    host_name = strdup(url);
    host_name[param - url] = '\0';
    
    if ((p_port = strstr(host_name, ":")) != NULL) {
        *p_port = '\0';
        p_port++;
        port = atoi(p_port);
    }

    if (ipaddr_aton(host_name, &ip_addr)) {
        printf("host_name:%s port:%d uri:%s\r\n", ipaddr_ntoa(&ip_addr), port, param);
        at_httpc_get_file((const ip_addr_t* )&ip_addr,
                        port,
                        param,
                        &ctx->settings,
                        recv_fn,
                        (void *)ctx,
                        &req);
    } else {
        printf("host_name:%s port:%d uri:%s\r\n", host_name, port, param);
        at_httpc_get_file_dns(
                host_name,
                port,
                param,
                &ctx->settings,
                recv_fn,
                (void *)ctx,
                &req);
    }

    free(host_name);

    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_httpsslcfg(int argc, const char **argv)
{
    int scheme;
    char cert_file[32] = {0};
    char key_file[32] = {0};
    char ca_file[32] = {0};
    int cert_file_valid = 0, key_file_valid = 0, ca_file_valid = 0;

    AT_CMD_PARSE_NUMBER(0, &scheme);
    AT_CMD_PARSE_OPT_STRING(1, cert_file, sizeof(cert_file), cert_file_valid);
    AT_CMD_PARSE_OPT_STRING(2, key_file, sizeof(key_file), key_file_valid);
    AT_CMD_PARSE_OPT_STRING(3, ca_file, sizeof(ca_file), ca_file_valid);

    if (scheme == AT_HTTPS_NOT_AUTH) {
        cert_file[0] = '\0';
        key_file[0] = '\0';
        ca_file[0] = '\0';
    } else if (scheme == AT_HTTPS_SERVER_AUTH) {
        if ((!ca_file_valid)) {
            return AT_RESULT_CODE_ERROR;
        }
        cert_file[0] = '\0';
        key_file[0] = '\0';
    } else if (scheme == AT_HTTPS_CLIENT_AUTH) {
        if ((!key_file_valid) || (!cert_file_valid)) {
            return AT_RESULT_CODE_ERROR;
        }
        ca_file[0] = '\0';
    } else if (scheme == AT_HTTPS_BOTH_AUTH) {
        if ((!key_file_valid) || (!cert_file_valid) || (!ca_file_valid)) {
            return AT_RESULT_CODE_ERROR;
        }
    }
    g_https_cfg.https_auth_type = scheme;

    strlcpy(g_https_cfg.cert_file, cert_file, sizeof(g_https_cfg.cert_file));
    strlcpy(g_https_cfg.key_file, key_file, sizeof(g_https_cfg.key_file));
    strlcpy(g_https_cfg.ca_file, ca_file, sizeof(g_https_cfg.ca_file));

    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_httpsslcfg(int argc, const char **argv)
{
    at_response_string("+HTTPSSLCFG:%d,\"%s\",\"%s\",\"%s\"\r\n", 
            g_https_cfg.https_auth_type, g_https_cfg.cert_file, g_https_cfg.key_file, g_https_cfg.ca_file);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_httpclient(int argc, const char **argv)
{
    int opt, content_type;
    struct at_http_ctx *ctx = NULL;
    uint8_t data_valid = 0;
    char *url_buf;
    
    ctx = malloc(sizeof(struct at_http_ctx));
    if (!ctx) {
        return AT_RESULT_CODE_ERROR;
    }
    memset(ctx, 0, sizeof(struct at_http_ctx));

    ctx->data = malloc(256);
    if (!ctx->data) {
        free(ctx);
        return AT_RESULT_CODE_ERROR;
    }
    memset(ctx->data, 0, 256);

    url_buf = ctx->url_buf;
    AT_CMD_PARSE_NUMBER(0, &opt);
    AT_CMD_PARSE_NUMBER(1, &content_type);
    AT_CMD_PARSE_STRING(2, ctx->url_buf, sizeof(ctx->url_buf));
    AT_CMD_PARSE_OPT_STRING(3, ctx->data, 256, data_valid);

 
    if (strlen(url_buf) == 0) {
        if ((g_https_cfg.url == NULL) && (g_https_cfg.url_size == 0)) {
            free(ctx->data);
            free(ctx);
            return AT_RESULT_CODE_ERROR;
        }
        url_buf = g_https_cfg.url;
    }   
    ctx->settings.timeout = AT_HTTPC_DEFAULT_TIMEOUT;
    ctx->settings.use_proxy = 0;
    ctx->settings.req_type  = opt;
    ctx->settings.content_type = content_type;
    ctx->settings.data = ctx->data;

    int ret = at_httpc_request(ctx, url_buf, cb_httpc_result, cb_httpc_headers_done_fn, cb_altcp_recv_fn, ctx);
    if (ret != 0) {
        free(ctx->data);
        ctx->data = NULL;
        return AT_RESULT_CODE_ERROR;
    }

    return AT_RESULT_CODE_OK;
}

static err_t cb_httpgetsize_headers_done_fn(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len)
{
    struct at_http_ctx *ctx = (struct at_http_ctx *)arg;

    at_response_string("+HTTPGETSIZE:%d\r\n", content_len);

    return ERR_OK;
}

static err_t cb_httpgetsize_recv_fn(void *arg, struct altcp_pcb *conn, struct pbuf *p, err_t err)
{
    altcp_recved(conn, p->tot_len);
    pbuf_free(p);
    return 0;
}

static int at_setup_cmd_httpgetsize(int argc, const char **argv)
{
    int timeout, timeout_valild = 0;
    struct at_http_ctx *ctx = NULL;
    char *url_buf;
    
    ctx = malloc(sizeof(struct at_http_ctx));
    if (!ctx) {
        return AT_RESULT_CODE_ERROR;
    }
    url_buf = ctx->url_buf;
    memset(ctx, 0, sizeof(struct at_http_ctx));

    AT_CMD_PARSE_STRING(0, ctx->url_buf, sizeof(ctx->url_buf));
    AT_CMD_PARSE_OPT_NUMBER(1, &timeout, timeout_valild);

    if (timeout_valild) {
        if (timeout < 0 || timeout > 180000) {
            free(ctx);
            return AT_RESULT_CODE_ERROR;
        }
    } else {
        timeout = AT_HTTPC_DEFAULT_TIMEOUT;
    }

    if (strlen(url_buf) == 0) {
        if ((g_https_cfg.url == NULL) && (g_https_cfg.url_size == 0)) {
            free(ctx);
            return AT_RESULT_CODE_ERROR;
        }
        url_buf = g_https_cfg.url;
    }

    ctx->data = NULL;
    ctx->settings.timeout = timeout;
    ctx->settings.use_proxy = 0;
    ctx->settings.req_type  = REQ_TYPE_HEAD;
    ctx->settings.content_type = 0;
    ctx->settings.data = NULL;

    int ret = at_httpc_request(ctx, url_buf, cb_httpc_result, cb_httpgetsize_headers_done_fn, cb_httpgetsize_recv_fn, ctx);
    if (ret != 0) {
        return AT_RESULT_CODE_ERROR;
    }

    return AT_RESULT_CODE_OK;
}

static err_t cb_httpcget_headers_done_fn(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len)
{
    char buf[32];
    struct at_http_ctx *ctx = (struct at_http_ctx *)arg;

    snprintf(buf, sizeof(buf), "+HTTPCGET:%d,", content_len);
    AT_CMD_DATA_SEND(buf, strlen(buf));

    return ERR_OK;
}

static int at_setup_cmd_httpcget(int argc, const char **argv)
{
    int timeout, timeout_valild = 0;
    struct at_http_ctx *ctx = NULL;
    char *url_buf;
    
    ctx = malloc(sizeof(struct at_http_ctx));
    if (!ctx) {
        return AT_RESULT_CODE_ERROR;
    }
    url_buf = ctx->url_buf;
    memset(ctx, 0, sizeof(struct at_http_ctx));
    
    AT_CMD_PARSE_STRING(0, ctx->url_buf, sizeof(ctx->url_buf));
    AT_CMD_PARSE_OPT_NUMBER(1, &timeout, timeout_valild);

    if (timeout_valild) {
        if (timeout < 0 || timeout > 180000) {
            free(ctx);
            return AT_RESULT_CODE_ERROR;
        }
    } else {
        timeout = AT_HTTPC_DEFAULT_TIMEOUT;
    }

    if (strlen(url_buf) == 0) {
        if ((g_https_cfg.url == NULL) && (g_https_cfg.url_size == 0)) {
            free(ctx);
            return AT_RESULT_CODE_ERROR;
        }
        url_buf = g_https_cfg.url;
    }
    
    ctx->data = NULL;
    ctx->settings.timeout = timeout;
    ctx->settings.use_proxy = 0;
    ctx->settings.req_type  = REQ_TYPE_GET;
    ctx->settings.content_type = 0;
    ctx->settings.data = NULL;

    int ret = at_httpc_request(ctx, url_buf, cb_httpc_result, cb_httpc_headers_done_fn, cb_altcp_recv_fn, ctx);
    if (ret != 0) {
        return AT_RESULT_CODE_ERROR;
    }

    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_httpcpost(int argc, const char **argv)
{
    int ret;
    int len, recv_num = 0;
    struct at_http_ctx *ctx = NULL;
    uint8_t header_cnt_valid = 0;
    char *url_buf;
    
    ctx = malloc(sizeof(struct at_http_ctx));
    if (!ctx) {
        return AT_RESULT_CODE_ERROR;
    }
    memset(ctx, 0, sizeof(struct at_http_ctx));

    url_buf = ctx->url_buf;
    AT_CMD_PARSE_STRING(0, ctx->url_buf, sizeof(ctx->url_buf));
    AT_CMD_PARSE_NUMBER(1, &len);
 
    if (strlen(url_buf) == 0) {
        if ((g_https_cfg.url == NULL) && (g_https_cfg.url_size == 0)) {
            free(ctx);
            return AT_RESULT_CODE_ERROR;
        }
        url_buf = g_https_cfg.url;
    }   

    ctx->data = malloc(len + 1);
    if (!ctx->data) {
        free(ctx);
        return AT_RESULT_CODE_ERROR;
    }
    memset(ctx->data, 0, len + 1);

    ctx->settings.timeout = AT_HTTPC_DEFAULT_TIMEOUT;
    ctx->settings.use_proxy = 0;
    ctx->settings.req_type  = REQ_TYPE_POST;
    ctx->settings.content_type = 0;
    ctx->settings.data = ctx->data;

    at_response_result(AT_RESULT_CODE_OK);
    AT_CMD_RESPONSE(AT_CMD_MSG_WAIT_DATA);
    while(recv_num < len) {
        recv_num += AT_CMD_DATA_RECV(ctx->data + recv_num, len - recv_num);
    }
    at_response_string("Recv %d bytes\r\n", recv_num);

    if (len == recv_num) {
        ret = AT_RESULT_CODE_SEND_OK;
    } else {
        ret = AT_RESULT_CODE_SEND_FAIL;
    }
    
    ret = at_httpc_request(ctx, url_buf, cb_httpc_result, cb_httpc_headers_done_fn, cb_altcp_recv_fn, ctx);
    if (ret != 0) {
        free(ctx->data);
        ctx->data = NULL;
        return AT_RESULT_CODE_ERROR;
    }

    return AT_RESULT_CODE_IGNORE;
}

static int at_setup_cmd_httpcput(int argc, const char **argv)
{
    int ret;
    int len, content_type, recv_num = 0;
    struct at_http_ctx *ctx = NULL;
    uint8_t header_cnt_valid = 0;
    char *url_buf;
    
    ctx = malloc(sizeof(struct at_http_ctx));
    if (!ctx) {
        return AT_RESULT_CODE_ERROR;
    }
    memset(ctx, 0, sizeof(struct at_http_ctx));

    url_buf = ctx->url_buf;
    AT_CMD_PARSE_STRING(0, ctx->url_buf, sizeof(ctx->url_buf));
    AT_CMD_PARSE_NUMBER(1, &content_type);
    AT_CMD_PARSE_NUMBER(2, &len);
 
    if (strlen(url_buf) == 0) {
        if ((g_https_cfg.url == NULL) && (g_https_cfg.url_size == 0)) {
            free(ctx);
            return AT_RESULT_CODE_ERROR;
        }
        url_buf = g_https_cfg.url;
    }   

    ctx->data = malloc(len + 1);
    if (!ctx->data) {
        free(ctx);
        return AT_RESULT_CODE_ERROR;
    }
    memset(ctx->data, 0, len + 1);

    ctx->settings.timeout = AT_HTTPC_DEFAULT_TIMEOUT;
    ctx->settings.use_proxy = 0;
    ctx->settings.req_type  = REQ_TYPE_PUT;
    ctx->settings.content_type = content_type;
    ctx->settings.data = ctx->data;

    at_response_result(AT_RESULT_CODE_OK);
    AT_CMD_RESPONSE(AT_CMD_MSG_WAIT_DATA);
    while(recv_num < len) {
        recv_num += AT_CMD_DATA_RECV(ctx->data + recv_num, len - recv_num);
    }

    at_response_string("Recv %d bytes\r\n", recv_num);

    if (len == recv_num) {
        ret = AT_RESULT_CODE_SEND_OK;
    } else {
        ret = AT_RESULT_CODE_SEND_FAIL;
    }
    
    ret = at_httpc_request(ctx, url_buf, cb_httpc_result, cb_httpc_headers_done_fn, cb_altcp_recv_fn, ctx);
    if (ret != 0) {
        free(ctx->data);
        ctx->data = NULL;
        return AT_RESULT_CODE_ERROR;
    }

    return AT_RESULT_CODE_IGNORE;
}

static int at_setup_cmd_httpcurlcfg(int argc, const char **argv)
{
    int len, recv_num = 0;

    AT_CMD_PARSE_NUMBER(0, &len);
    if (len < 0) {
        return AT_RESULT_CODE_ERROR;
    }
    if (len == 0) {
        free(g_https_cfg.url);
        g_https_cfg.url_size = 0;
        g_https_cfg.url = NULL;
        return AT_RESULT_CODE_OK;
    }

    if (g_https_cfg.url != NULL) {
        return AT_RESULT_CODE_ERROR;
    }
    g_https_cfg.url = malloc(len);
    if (!g_https_cfg.url) {
        return AT_RESULT_CODE_ERROR;
    }
    memset(g_https_cfg.url, 0, len);

    at_response_result(AT_RESULT_CODE_OK);
    AT_CMD_RESPONSE(AT_CMD_MSG_WAIT_DATA);
    while(recv_num < len) {
        recv_num += AT_CMD_DATA_RECV(g_https_cfg.url + recv_num, len - recv_num);
    }
    at_response_string("Recv %d bytes\r\n", recv_num);
    g_https_cfg.url_size = len;

    g_https_cfg.url[len] = '\0';
    if (len == recv_num) {
        return AT_RESULT_CODE_SEND_OK;
    }
    return AT_RESULT_CODE_SEND_FAIL;
}

static int at_query_cmd_httpcurlcfg(int argc, const char **argv)
{
    at_response_string("+HTTPURLCFG:%d,%s\r\n", g_https_cfg.url_size, g_https_cfg.url);
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_httprecvmode(int argc, const char **argv)
{
    at_response_string("+HTTPRECVMODE:%d\r\n", g_https_cfg.recv_mode);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_httprecvmode(int argc, const char **argv)
{
    int mode;

    AT_CMD_PARSE_NUMBER(0, &mode);
    if(mode != AT_HTTPC_RECV_MODE_ACTIVE && mode != AT_HTTPC_RECV_MODE_PASSIVE) {
        return AT_RESULT_CODE_ERROR;
    }
   
    if (at_get_work_mode() != AT_WORK_MODE_CMD) {
        return AT_RESULT_CODE_ERROR;
    }
    g_https_cfg.recv_mode = mode;

    xSemaphoreTake(g_https_cfg.mutex, portMAX_DELAY);
    if (mode == AT_HTTPC_RECV_MODE_PASSIVE) {
    
        if (!g_https_cfg.recv_buf) {
            g_https_cfg.recv_buf = xStreamBufferCreate(g_https_cfg.recvbuf_size, 1);
        }
    } else {
        if (g_https_cfg.recv_buf) {
            vStreamBufferDelete(g_https_cfg.recv_buf);
            g_https_cfg.recv_buf = NULL;
        }
    }
    xSemaphoreGive(g_https_cfg.mutex);

    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_httprecvdata(int argc, const char **argv)
{
    int read_len, size, ret = 0, n, offset = 0;
    uint8_t *buffer;

    AT_CMD_PARSE_NUMBER(0, &size);

    if (size <= 0 || size > g_https_cfg.recvbuf_size) {
        return AT_RESULT_CODE_ERROR;
    }

    buffer = (char *)pvPortMalloc(size + 48);

    read_len = httpc_get_recvsize();
    read_len = read_len > size ? size : read_len;

    n = snprintf(buffer + offset, 48, "+HTTPRECVDATA:%d,", read_len);
    if (n > 0) {
        offset += n;
    }

    if (read_len) {
        ret = xStreamBufferReceive(g_https_cfg.recv_buf, buffer + offset, read_len, 0);
        if (ret != read_len) {
            printf("at_net_recvbuf_read error %d\r\n", ret);
        }
        if (g_https_cfg.altcp_conn) {
            altcp_recved(g_https_cfg.altcp_conn, ret);
        }
    }
    
    offset += ret;
   
    memcpy(buffer + offset, "\r\n", 2);
    offset += 2;
    AT_CMD_DATA_SEND((uint8_t *)buffer, offset);
    
    vPortFree(buffer);

    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_httprecvbuf(int argc, const char **argv)
{
    int linkid = 0, size;
        
    AT_CMD_PARSE_NUMBER(0, &size);

    if (size <= 0) {
        return AT_RESULT_CODE_ERROR;
    }

	if (g_https_cfg.recv_buf) {
		return AT_RESULT_CODE_ERROR;
	}
    g_https_cfg.recvbuf_size = size;

    return AT_RESULT_CODE_OK;
}


static int at_query_cmd_httprecvbuf(int argc, const char **argv)
{
    at_response_string("+HTTPRECVBUF:%d\r\n", g_https_cfg.recvbuf_size);
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_httprecvlen(int argc, const char **argv)
{
    int id = 0;

    at_response_string("+HTTPRECVLEN:%d\r\n", httpc_get_recvsize());
    return AT_RESULT_CODE_OK;
}

static const at_cmd_struct at_http_cmd[] = {
    {"+HTTPRECVDATA", NULL, NULL, at_setup_cmd_httprecvdata, NULL, 1, 1},
    {"+HTTPRECVMODE", NULL, at_query_cmd_httprecvmode, at_setup_cmd_httprecvmode, NULL, 1, 1},
    {"+HTTPRECVBUF", NULL, at_query_cmd_httprecvbuf, at_setup_cmd_httprecvbuf, NULL, 1, 1},
    {"+HTTPRECVLEN", NULL, at_query_cmd_httprecvlen, NULL, NULL, 0, 0},
    {"+HTTPCLIENT", NULL, NULL, at_setup_cmd_httpclient, NULL, 3, 4},
    {"+HTTPGETSIZE", NULL, NULL, at_setup_cmd_httpgetsize, NULL, 1, 2},
    {"+HTTPCGET", NULL, NULL, at_setup_cmd_httpcget, NULL, 1, 2},
    {"+HTTPCPOST", NULL, NULL, at_setup_cmd_httpcpost, NULL, 2, 2},
    {"+HTTPCPUT", NULL, NULL, at_setup_cmd_httpcput, NULL, 3, 3},
    {"+HTTPURLCFG", NULL, at_query_cmd_httpcurlcfg, at_setup_cmd_httpcurlcfg, NULL, 1, 1},
    {"+HTTPSSLCFG", NULL, at_query_cmd_httpsslcfg, at_setup_cmd_httpsslcfg, NULL, 1, 4},
};

bool at_http_cmd_regist(void)
{
    g_https_cfg.recvbuf_size = AT_HTTPC_RECVBUF_SIZE_DEFAULT;
    g_https_cfg.mutex = xSemaphoreCreateMutex();

    if (at_cmd_register(at_http_cmd, sizeof(at_http_cmd) / sizeof(at_http_cmd[0])) == 0)
        return true;
    else
        return false;
}

