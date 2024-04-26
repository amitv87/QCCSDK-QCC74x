#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <shell.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stream_buffer.h>

#include "spisync_port.h"
#include "spisync.h"

#define spisync_TEST_TASK_PRI         (13)

typedef struct __spisync_ctx_test {
    spisync_t    *p_spisync;
    TaskHandle_t  rx_hdr;
    StaticTask_t  rx_task;
    uint32_t      rx_stack[2028/sizeof(uint32_t)];
    uint32_t      rx_echo;
} spisync_ctx_test_t;

spisync_ctx_test_t *mst = NULL;
spisync_ctx_test_t *slv = NULL;

static void __spisync_dump_cmd(spisync_ctx_test_t *rs, int argc, char **argv)
{
    spisync_dump(rs->p_spisync);
}

static void rx_task_entry(void *arg)
{
    uint8_t *msg;
    uint32_t msg_len;
    spisync_ctx_test_t *rs = (spisync_ctx_test_t *)arg;
    uint32_t flag = 0;

    msg = pvPortMalloc(SPISYNC_PAYLOADBUF_LEN);

    if (NULL == msg) {
        spisync_log("mem err len = %d\r\n", SPISYNC_PAYLOADBUF_LEN);
        return;
    }

    while (1) {
        msg_len = spisync_read(rs->p_spisync, msg, SPISYNC_PAYLOADBUF_LEN, portMAX_DELAY);
        if (msg_len) {
            spisync_buf("rx", msg, msg_len);
        }

        // check echo
        if (NULL != mst) {
            if (mst->rx_echo) {
                flag = 1;
            }
        }
        if (NULL != slv) {
            if (slv->rx_echo) {
                flag = 1;
            }
        }
        if (flag) {
            flag = 0;
            spisync_write(rs->p_spisync, msg, msg_len, portMAX_DELAY);
        }
#if 0
        //  to streambuffer h2d
        {
            void at_h2d_send(uint8_t *buf, int len);
            at_h2d_send(msg, msg_len);
        }
#endif
    }
}

static void __spisync_push(spisync_t * ramsync, int argc, char **argv)
{
    uint8_t *msg;
    uint32_t msg_len;
    uint32_t i;
    uint32_t sendtimes;

    if ((argc != 2) && (argc != 3)) {
        spisync_log("usage: rs_push arg error\r\n");
        return;
    }

    if (3 == argc) {
        sendtimes = (uint32_t)atoi(argv[2]);
    } else {
        sendtimes = 1;
    }

    msg_len = (uint32_t)atoi(argv[1]);
    if (msg_len == 0) {
        spisync_log("usage: rs_push argh error len = %ld\r\n", msg_len);
        return;
    }

    spisync_trace("sendtimes = %ld, sendlen = %ld\r\n", sendtimes, msg_len);

    msg = pvPortMalloc(msg_len);
    if (NULL == msg) {
        spisync_log("mem err len = %ld\r\n", msg_len);
    }

    for (i = 0; i < msg_len; i++) {
        msg[i] = i&0xFF;
    }

    for (i = 0; i < sendtimes; i++) {
        spisync_write(ramsync, msg, msg_len, portMAX_DELAY);
    }

    vPortFree(msg);
}

static void __spisync_pop(char *name, spisync_t * ramsync, int argc, char **argv)
{
    uint8_t *msg;
    uint32_t msg_len;

    msg = pvPortMalloc(SPISYNC_PAYLOADBUF_LEN);
    if (NULL == msg) {
        spisync_log("mem err len = %d\r\n", SPISYNC_PAYLOADBUF_LEN);
    }
    msg_len = SPISYNC_PAYLOADBUF_LEN;

    msg_len = spisync_read(ramsync, msg, SPISYNC_PAYLOADBUF_LEN, 0);

    if (msg_len) {
        spisync_buf(name, msg, msg_len);
    }

    vPortFree(msg);
}

static void __spisync_fakepush_forpop(char *name, spisync_t * ramsync, int argc, char **argv)
{
    uint8_t *msg = NULL;
    uint32_t msg_len;
    int cmd_len;
    int compare_len;
    int hascrlf;

    if (argc < 2) {
        printf("Usage: ss_fakepush <data>\r\n");
        return;
    }
    if (argc == 3) {
        hascrlf = atoi(argv[2]);
    } else {
        hascrlf = 1;// default add crlf
    }

    msg = pvPortMalloc(SPISYNC_PAYLOADBUF_LEN);
    if (NULL == msg) {
        spisync_log("mem err len = %d\r\n", SPISYNC_PAYLOADBUF_LEN);
        goto fakepush_err;
    }

    cmd_len = strlen(argv[1]);
    compare_len = hascrlf?(SPISYNC_PAYLOADBUF_LEN - 2):(SPISYNC_PAYLOADBUF_LEN);
    if (cmd_len > compare_len) {
        spisync_log("data too len %d > %d, Usage: ss_fakepush <data>\r\n",
                        cmd_len, compare_len);
        goto fakepush_err;
    }
    memcpy(msg, argv[1], cmd_len);
    if (hascrlf) {
        memcpy(msg + cmd_len, "\r\n", 2);
        cmd_len += 2;
    }
    msg_len = cmd_len;

    msg_len = spisync_fakewrite_forread(ramsync, msg, msg_len, 0);

    spisync_log("fakewrite len:%d, hascrlf:%d\r\n", msg_len, hascrlf);
fakepush_err:
    if (msg) {
        vPortFree(msg);
    }
}

void spisync_loop_cmd(int argc, char **argv)
{
    if (NULL != mst) {
        mst->rx_echo = (mst->rx_echo) ? 0 : 1;
    }
    if (NULL != slv) {
        slv->rx_echo = (slv->rx_echo) ? 0 : 1;
    }
}

static void spisync_reset_cmd(int argc, char **argv)
{
}

static void sspisync_push_cmd(int argc, char **argv)
{
    if (NULL == slv) {
        spisync_log("slv is NULL\r\n");
        return;
    }
    __spisync_push(slv->p_spisync, argc, argv);
}

static void sspisync_fakepush_forpop_cmd(int argc, char **argv)
{
    if (NULL == slv) {
        spisync_log("slv is NULL\r\n");
        return;
    }

    __spisync_fakepush_forpop("sfakepush", slv->p_spisync, argc, argv);
}

static void sspisync_pop_cmd(int argc, char **argv)
{
    if (NULL == slv) {
        spisync_log("slv is NULL\r\n");
        return;
    }
    __spisync_pop("spop", slv->p_spisync, argc, argv);
}

void sspisync_init_cmd(char *buf, int len, int argc, char **argv)
{
#if 0
    slv = pvPortMalloc(sizeof(spisync_ctx_test_t));
    if (NULL == slv) {
        spisync_log("mem error\r\n");
        return;
    }
    memset(slv, 0, sizeof(spisync_ctx_test_t));
    slv->p_spisync = pvPortMalloc(sizeof(spisync_t));
    if (NULL == slv->p_spisync) {
        spisync_log("mem error\r\n");
        return;
    }

    spisync_init(slv->p_spisync);

    // start sync
    slv->rx_hdr = xTaskCreateStatic(
            rx_task_entry,
            (char*)"srx_test",
            sizeof(slv->rx_stack)/sizeof(StackType_t), slv,
            spisync_TEST_TASK_PRI,
            slv->rx_stack,
            &slv->rx_task);
#endif
}

#if 0
static void mspisync_init_cmd(int argc, char **argv)
{
    mst = pvPortMalloc(sizeof(spisync_ctx_test_t));
    if (NULL == mst) {
        spisync_log("mem error\r\n");
        return;
    }
    memset(mst, 0, sizeof(spisync_ctx_test_t));
    mst->p_spisync = pvPortMalloc(sizeof(spisync_t));
    if (NULL == mst->p_spisync) {
        spisync_log("mem error\r\n");
        return;
    }

    spisync_init(mst->p_spisync);

    // start sync
    mst->rx_hdr = xTaskCreateStatic(
            rx_task_entry,
            (char*)"mrx_test",
            sizeof(mst->rx_stack)/sizeof(StackType_t), mst,
            spisync_TEST_TASK_PRI,
            mst->rx_stack,
            &mst->rx_task);
}

static void mspisync_push_cmd(int argc, char **argv)
{
    if (NULL == mst) {
        spisync_log("mst is NULL\r\n");
        return;
    }
    __spisync_push(mst->p_spisync, argc, argv);
}

static void mspisync_pop_cmd(int argc, char **argv)
{
    if (NULL == mst) {
        spisync_log("mst is NULL\r\n");
        return;
    }
    __spisync_pop("mpop", mst->p_spisync, argc, argv);
}

#endif

/* --------------------- debug dump --------------------- */
#if 0
static void mspisync_dump_cmd(int argc, char **argv)
{
    if (NULL == mst) {
        spisync_log("mst is NULL\r\n");
        return;
    }
    __spisync_dump_cmd(mst, argc, argv);
}
#endif

static void sspisync_dump_cmd(int argc, char **argv)
{
    if (NULL == slv) {
        spisync_log("slv is NULL\r\n");
        spisync_dump(NULL);
        return;
    }

    __spisync_dump_cmd(slv, argc, argv);
}

SHELL_CMD_EXPORT_ALIAS(spisync_loop_cmd,  ss_loop,  spisync loop);
SHELL_CMD_EXPORT_ALIAS(spisync_reset_cmd, ss_reset, spisync reset);
SHELL_CMD_EXPORT_ALIAS(sspisync_init_cmd, ss_sinit, spisync init);
SHELL_CMD_EXPORT_ALIAS(sspisync_pop_cmd,  ss_spop,  spisync spop rx);
SHELL_CMD_EXPORT_ALIAS(sspisync_push_cmd, ss_spush, spisync spush tx);
SHELL_CMD_EXPORT_ALIAS(sspisync_fakepush_forpop_cmd, ss_sfpush, spisync fake push for pop);
SHELL_CMD_EXPORT_ALIAS(sspisync_dump_cmd, ss_sdump, spisync dump);
#if 0
SHELL_CMD_EXPORT_ALIAS(mspisync_init_cmd, rs_minit, ramsync init);
SHELL_CMD_EXPORT_ALIAS(mspisync_pop_cmd, rs_mpop, ramsync mpop rx);
SHELL_CMD_EXPORT_ALIAS(mspisync_push_cmd, rs_mpush, ramsync mpush tx);
SHELL_CMD_EXPORT_ALIAS(mspisync_dump_cmd, rs_mdump, ramsync dump);
#endif
