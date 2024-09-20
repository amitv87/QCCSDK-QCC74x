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

struct at_http_ctx {
    httpc_connection_t settings;
    SemaphoreHandle_t sem;
    uint8_t *data;
    char url_buf[256];
};

static char *g_url = NULL;
static uint32_t g_url_size = 0;

static err_t cb_altcp_recv_fn(void *arg, struct altcp_pcb *conn, struct pbuf *p, err_t err)
{
    //printf("%s\r\n", (char *)p->payload);
    altcp_recved(conn, p->tot_len);
    if (p->tot_len) {
        AT_CMD_DATA_SEND(p->payload, p->tot_len);
    }
    pbuf_free(p);
    return 0;
}

static void cb_httpc_result(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err)
{
    struct at_http_ctx *ctx = (struct at_http_ctx *)arg;

    printf("[DATA] Transfer finished err %d. rx_content_len is %lu httpc_result: %d\r\n", err, rx_content_len, httpc_result);
    if (err == 0) {
        at_response_string("\r\n+HTTPC:OK\r\n");
    } else {
        at_response_string("\r\n+HTTPC:ERROR\r\n");
    }
    free(ctx->data);
    free(ctx);
}

static err_t cb_httpc_headers_done_fn(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len)
{
    char buf[16];
    struct at_http_ctx *ctx = (struct at_http_ctx *)arg;

    printf("[HEAD] hdr->tot_len is %u, hdr_len is %u, content_len is %lu\r\n", hdr->tot_len, hdr_len, content_len);
    printf((char *)hdr->payload);
    if (ctx->settings.req_type == REQ_TYPE_HEAD) {
        snprintf(buf, sizeof(buf), "+HTTPC:%d,", hdr_len);
        AT_CMD_DATA_SEND(buf, strlen(buf));
        if (hdr->tot_len) {
            AT_CMD_DATA_SEND(hdr->payload, hdr->tot_len);
        }
    } else {
        snprintf(buf, sizeof(buf), "+HTTPC:%d,", content_len);
        AT_CMD_DATA_SEND(buf, strlen(buf));
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
        ctx->settings.tls_config = altcp_tls_create_config_client(NULL, 0);
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
        if ((g_url == NULL) && (g_url_size == 0)) {
            free(ctx->data);
            free(ctx);
            return AT_RESULT_CODE_ERROR;
        }
        url_buf = g_url;
    }   
    ctx->settings.use_proxy = 0;
    ctx->settings.req_type  = opt;
    ctx->settings.content_type = content_type;
    ctx->settings.data = ctx->data;

    return at_httpc_request(ctx, url_buf, cb_httpc_result, cb_httpc_headers_done_fn, cb_altcp_recv_fn, ctx);
}

static void cb_httpgetsize_result(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err)
{
    struct at_http_ctx *ctx = (struct at_http_ctx *)arg;

    printf("[DATA] Transfer finished err %d. rx_content_len is %lu httpc_result: %d\r\n", err, rx_content_len, httpc_result);
    
    free(ctx);
}

static err_t cb_httpgetsize_headers_done_fn(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len)
{
    struct at_http_ctx *ctx = (struct at_http_ctx *)arg;

    at_response_string("+HTTPGETSIZE:%d\r\n", content_len);
    xSemaphoreGive(ctx->sem);

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
        timeout = 5000;
    }

    if (strlen(url_buf) == 0) {
        if ((g_url == NULL) && (g_url_size == 0)) {
            free(ctx);
            return AT_RESULT_CODE_ERROR;
        }
        url_buf = g_url;
    }
    ctx->settings.use_proxy = 0;
    ctx->settings.req_type  = REQ_TYPE_HEAD;
    ctx->settings.content_type = 0;
    ctx->settings.data = NULL;

    ctx->sem = xSemaphoreCreateBinary();
    if (!ctx->sem) {
        free(ctx);
        return AT_RESULT_CODE_ERROR;
    }

    int ret = at_httpc_request(ctx, url_buf, cb_httpgetsize_result, cb_httpgetsize_headers_done_fn, cb_httpgetsize_recv_fn, ctx);

    xSemaphoreTake(ctx->sem, timeout);
    vSemaphoreDelete(ctx->sem);

    return ret;
}

static void cb_httpcget_result(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err)
{
    struct at_http_ctx *ctx = (struct at_http_ctx *)arg;

    free(ctx);
}

static err_t cb_httpcget_headers_done_fn(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len)
{
    char buf[32];
    struct at_http_ctx *ctx = (struct at_http_ctx *)arg;

    snprintf(buf, sizeof(buf), "+HTTPCGET:%d,", content_len);
    AT_CMD_DATA_SEND(buf, strlen(buf));
    xSemaphoreGive(ctx->sem);

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
        timeout = 5000;
    }

    if (strlen(url_buf) == 0) {
        if ((g_url == NULL) && (g_url_size == 0)) {
            free(ctx);
            return AT_RESULT_CODE_ERROR;
        }
        url_buf = g_url;
    }
    
    ctx->settings.use_proxy = 0;
    ctx->settings.req_type  = REQ_TYPE_GET;
    ctx->settings.content_type = 0;
    ctx->settings.data = NULL;

    ctx->sem = xSemaphoreCreateBinary();
    if (!ctx->sem) {
        return AT_RESULT_CODE_ERROR;
    }

    int ret = at_httpc_request(ctx, url_buf, cb_httpcget_result, cb_httpcget_headers_done_fn, cb_altcp_recv_fn, ctx);

    xSemaphoreTake(ctx->sem, timeout);
    vSemaphoreDelete(ctx->sem);

    return ret;
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
        if ((g_url == NULL) && (g_url_size == 0)) {
            free(ctx);
            return AT_RESULT_CODE_ERROR;
        }
        url_buf = g_url;
    }   

    ctx->data = malloc(len + 1);
    if (!ctx->data) {
        free(ctx);
        return AT_RESULT_CODE_ERROR;
    }
    memset(ctx->data, 0, len + 1);

    ctx->settings.use_proxy = 0;
    ctx->settings.req_type  = REQ_TYPE_POST;
    ctx->settings.content_type = 0;
    ctx->settings.data = ctx->data;

    at_response_result(AT_RESULT_CODE_OK);
    AT_CMD_RESPONSE(AT_CMD_MSG_WAIT_DATA);
    while(recv_num < len) {
        recv_num += AT_CMD_DATA_RECV(ctx->data + recv_num, len - recv_num);
    }

    if (len == recv_num) {
        ret = AT_RESULT_CODE_SEND_OK;
    } else {
        ret = AT_RESULT_CODE_SEND_FAIL;
    }
    
    if (at_httpc_request(ctx, url_buf, cb_httpc_result, cb_httpc_headers_done_fn, cb_altcp_recv_fn, ctx)) {
        ret = AT_RESULT_CODE_ERROR;
    }

    return ret;
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
        if ((g_url == NULL) && (g_url_size == 0)) {
            free(ctx);
            return AT_RESULT_CODE_ERROR;
        }
        url_buf = g_url;
    }   

    ctx->data = malloc(len + 1);
    if (!ctx->data) {
        free(ctx);
        return AT_RESULT_CODE_ERROR;
    }
    memset(ctx->data, 0, len + 1);

    ctx->settings.use_proxy = 0;
    ctx->settings.req_type  = REQ_TYPE_PUT;
    ctx->settings.content_type = content_type;
    ctx->settings.data = ctx->data;

    at_response_result(AT_RESULT_CODE_OK);
    AT_CMD_RESPONSE(AT_CMD_MSG_WAIT_DATA);
    while(recv_num < len) {
        recv_num += AT_CMD_DATA_RECV(ctx->data + recv_num, len - recv_num);
    }

    if (len == recv_num) {
        ret = AT_RESULT_CODE_SEND_OK;
    } else {
        ret = AT_RESULT_CODE_SEND_FAIL;
    }
    
    if (at_httpc_request(ctx, url_buf, cb_httpc_result, cb_httpc_headers_done_fn, cb_altcp_recv_fn, ctx)) {
        ret = AT_RESULT_CODE_ERROR;
    }

    return ret;
}

static int at_setup_cmd_httpcurlcfg(int argc, const char **argv)
{
    int len, recv_num = 0;

    AT_CMD_PARSE_NUMBER(0, &len);
    if (len < 0) {
        return AT_RESULT_CODE_ERROR;
    }
    if (len == 0) {
        free(g_url);
        g_url_size = 0;
        g_url = NULL;
        return AT_RESULT_CODE_OK;
    }

    if (g_url != NULL) {
        return AT_RESULT_CODE_ERROR;
    }
    g_url = malloc(len);
    if (!g_url) {
        return AT_RESULT_CODE_ERROR;
    }
    memset(g_url, 0, len);

    at_response_result(AT_RESULT_CODE_OK);
    AT_CMD_RESPONSE(AT_CMD_MSG_WAIT_DATA);
    while(recv_num < len) {
        recv_num += AT_CMD_DATA_RECV(g_url + recv_num, len - recv_num);
    }
    g_url_size = len;

    g_url[len] = '\0';
    if (len == recv_num) {
        return AT_RESULT_CODE_SEND_OK;
    }
    return AT_RESULT_CODE_SEND_FAIL;
}

static int at_query_cmd_httpcurlcfg(int argc, const char **argv)
{
    at_response_string("+HTTPURLCFG:%d,%s\r\n", g_url_size, g_url);
    return AT_RESULT_CODE_OK;
}

static const at_cmd_struct at_http_cmd[] = {
    {"+HTTPCLIENT", NULL, NULL, at_setup_cmd_httpclient, NULL, 3, 4},
    {"+HTTPGETSIZE", NULL, NULL, at_setup_cmd_httpgetsize, NULL, 1, 2},
    {"+HTTPCGET", NULL, NULL, at_setup_cmd_httpcget, NULL, 1, 2},
    {"+HTTPCPOST", NULL, NULL, at_setup_cmd_httpcpost, NULL, 2, 2},
    {"+HTTPCPUT", NULL, NULL, at_setup_cmd_httpcput, NULL, 3, 3},
    {"+HTTPURLCFG", NULL, at_query_cmd_httpcurlcfg, at_setup_cmd_httpcurlcfg, NULL, 1, 1},
};

bool at_http_cmd_regist(void)
{
    if (at_cmd_register(at_http_cmd, sizeof(at_http_cmd) / sizeof(at_http_cmd[0])) == 0)
        return true;
    else
        return false;
}

