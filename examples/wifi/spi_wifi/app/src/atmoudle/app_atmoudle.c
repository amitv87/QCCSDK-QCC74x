#include <stdio.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <mem.h>
#include <shell.h>
#include <spisync.h>
#include "at_main.h"
#include "qcc74x_dma.h"

static void __spisync_gpio_init(void *arg);

const struct spisync_config spisync_config = {
    .spi_name = "spi0",
    .spi_tx_dmach = "dma0_ch0",
    .spi_rx_dmach = "dma0_ch1",

    .spi_port = 0,
    .spi_mode = 0,
#ifdef CONFIG_SPI_3PIN_MODE_ENABLE
    .spi_3pin_mode = 1,
#else
    .spi_3pin_mode = 0,
#endif
    .spi_speed = 80000000,

    .spi_dma_req_tx = DMA_REQUEST_SPI0_TX,
    .spi_dma_req_rx = DMA_REQUEST_SPI0_RX,
    .spi_dma_tdr = DMA_ADDR_SPI0_TDR,
    .spi_dma_rdr = DMA_ADDR_SPI0_RDR,
    .reset_cb = __spisync_gpio_init,
    .reset_arg = NULL,
};
spisync_t    *at_spisync = NULL;

static void __spisync_gpio_init(void *arg)
{
#if CONFIG_SPI_3PIN_MODE_ENABLE
    board_spi0_gpio_3pin_init();
#else
    board_spi0_gpio_init();
#endif
}

static void atspisync_fakepush_forpop_cmd(int argc, char **argv)
{
    uint8_t *msg = NULL;
    uint32_t msg_len;
    int cmd_len;
    int compare_len;
    int hascrlf;

    if (argc < 2) {
        printf("Usage: at_fake <data_str> [hascrlf]\r\n");
        return;
    }
    if (argc == 3) {
        hascrlf = atoi(argv[2]);
    } else {
        /* default add crlf */
        hascrlf = 1;
    }

    msg = pvPortMalloc(SPISYNC_PAYLOADBUF_LEN);
    if (NULL == msg) {
        printf("mem err len = %d\r\n", SPISYNC_PAYLOADBUF_LEN);
        goto fakepush_err;
    }

    cmd_len = strlen(argv[1]);
    compare_len = hascrlf?(SPISYNC_PAYLOADBUF_LEN - 2):(SPISYNC_PAYLOADBUF_LEN);
    if (cmd_len > compare_len) {
        printf("data too len %d > %d, Usage: ss_fakepush <data>\r\n",
                        cmd_len, compare_len);
        goto fakepush_err;
    }
    memcpy(msg, argv[1], cmd_len);
    if (hascrlf) {
        memcpy(msg + cmd_len, "\r\n", 2);
        cmd_len += 2;
    }
    msg_len = cmd_len;

    //extern struct at_struct *at;
    //at->fakeoutput = 1;
    msg_len = spisync_fakewrite_forread(at_spisync, msg, msg_len, 0);

    printf("fakewrite len:%d, hascrlf:%d\r\n", msg_len, hascrlf);
fakepush_err:
    if (msg) {
        vPortFree(msg);
    }
}
SHELL_CMD_EXPORT_ALIAS(atspisync_fakepush_forpop_cmd, atfake, spisync fake push for pop);

static void at_dump_cmd(int argc, char **argv)
{
    spisync_dump_internal(at_spisync);
}
SHELL_CMD_EXPORT_ALIAS(at_dump_cmd, at, at dump spisync dump);

static void at_iperf_cmd(int argc, char **argv)
{
    int enable = 1;

    if (argc == 2) {
        enable = atoi(argv[1]);
    }

    spisync_iperf(at_spisync, enable);
}
SHELL_CMD_EXPORT_ALIAS(at_iperf_cmd, at_iperf, at_iperf [0 or 1]);

static void at_iperftx_cmd(int argc, char **argv)
{
    uint32_t time_sec;

    if (argc < 2) {
        time_sec = 20;
    } else if (argc == 2) {
        time_sec = atoi(argv[1]);
    }
    printf("Usage: at_iperftx [time_sec]\r\n");

    spisync_iperftx(at_spisync, time_sec);
}
SHELL_CMD_EXPORT_ALIAS(at_iperftx_cmd, at_iperftx, at_iperftx [20]);

void spisync_iperf_cmd(int argc, char **argv)
{
    int tx_en;
    int type;

    // check arg
    if (argc != 3) {
        printf("arg error, spisync_iperf <tx_en> <type>\r\n");
        return;
    }

    // update tx/rx
    tx_en = atoi(argv[1]);
    if (tx_en > 1) {
        printf("arg error, tx_en;%d\r\n", tx_en);
        return;
    }

    // update type
    type = atoi(argv[2]);
    if (type >= 3) {
        printf("arg error, type;%d\r\n", type);
        return;
    }

    spisync_iperf_test(at_spisync, tx_en, type, 1, 30);
}
SHELL_CMD_EXPORT_ALIAS(spisync_iperf_cmd, spisync_iperf, spisync_iperf <tx_en> <type>);

void spisync_type_cmd(int argc, char **argv)
{
    int period;
    int timeout;

    // check arg
    if (argc != 3) {
        printf("arg error, spisync_type <period> <timeout>\r\n");
        return;
    }

    // update tx/rx
    period = atoi(argv[1]);
    if (period < 0) {
        printf("arg error, period;%d\r\n", period);
        return;
    }

    // update timeout
    timeout = atoi(argv[2]);
    if (timeout < period) {
        printf("arg error, period:%d, timeout:%d\r\n", period, timeout);
        return;
    }

    printf("period:%d, timeout:%d\r\n", period, timeout);
    spisync_type_test(at_spisync, period, timeout);
}
SHELL_CMD_EXPORT_ALIAS(spisync_type_cmd, spisync_type, spisync_type <period> <timeout>);
static void at_debug_cmd(int argc, char **argv)
{
    uint8_t *buf;
    uint32_t type;
    uint32_t buf_len;
    int i;
    int ret;

    if (argc != 3) {
        type = 0;
        buf_len = 100;
        printf("arg null, use def, type:%d, len:%d\r\n", type, buf_len);
    } else {
        type = atoi(argv[1]);
        buf_len = atoi(argv[2]);
        if ((type < 0) || (type >= 3) || (buf_len < 1)) {
            printf("arg error, type:%d, len:%d\r\n", type, buf_len);
            return;
        }
        printf("arg type:%d, len:%d\r\n", type, buf_len);
    }
    buf = pvPortMalloc(buf_len);
    for (i = 0; i < buf_len; i++) {
        buf[i] = (uint8_t)(i&0xFF);
    }

    ret = spisync_write(at_spisync, type, buf, buf_len, 1000);
    if (ret != buf_len) {
        printf("write err ret:%d, buf_len:%d\r\n", ret, buf_len);
    }
    vPortFree(buf);
}
SHELL_CMD_EXPORT_ALIAS(at_debug_cmd, at_debug, at_debug [0] [100]);

static void spisync_wake_cmd(int argc, char **argv)
{
    spisync_wakeup(at_spisync);
}
SHELL_CMD_EXPORT_ALIAS(spisync_wake_cmd, spisync_wakeup, spisync_wakeup);

void app_atmoudle_init(void)
{
    at_spisync = (spisync_t *)pvPortMalloc(sizeof(spisync_t));
    if (NULL == at_spisync) {
        return -1;
    }
    spisync_init(at_spisync, &spisync_config);

    at_module_init();
}

