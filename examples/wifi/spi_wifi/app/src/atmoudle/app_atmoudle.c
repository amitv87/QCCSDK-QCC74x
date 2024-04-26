#include <stdio.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <mem.h>
#include <shell.h>
#include <spisync.h>

#include "qcc74x_dma.h"

const struct spisync_config spisync_config = {
    .spi_name = "spi0",
    .spi_tx_dmach = "dma0_ch0",
    .spi_rx_dmach = "dma0_ch1",

    .spi_port = 0,
    .spi_mode = 0,
    .spi_speed = 80000000,

    .spi_dma_req_tx = DMA_REQUEST_SPI0_TX,
    .spi_dma_req_rx = DMA_REQUEST_SPI0_RX,
    .spi_dma_tdr = DMA_ADDR_SPI0_TDR,
    .spi_dma_rdr = DMA_ADDR_SPI0_RDR,
};
spisync_t    *at_spisync = NULL;

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
    spisync_dump(at_spisync);
}
SHELL_CMD_EXPORT_ALIAS(at_dump_cmd, at, at dump spisync dump);

static void at_iperfrx_cmd(int argc, char **argv)
{
    int enable = 1;

    if (argc == 2) {
        enable = atoi(argv[1]);
    }

    spisync_iperfrx(at_spisync, enable);
}
SHELL_CMD_EXPORT_ALIAS(at_iperfrx_cmd, at_iperfrx, at_iperfrx 1);

static void at_iperftx_cmd(int argc, char **argv)
{
    uint32_t time_sec;

    if (argc < 2) {
        printf("Usage: at_iperf <time_sec>\r\n");
        return;
    }
    time_sec = atoi(argv[1]);
    spisync_iperftx(at_spisync, time_sec);
}
SHELL_CMD_EXPORT_ALIAS(at_iperftx_cmd, at_iperftx, at_iperf 10);

void app_atmoudle_init(void)
{
    at_spisync = (spisync_t *)pvPortMalloc(sizeof(spisync_t));
    if (NULL == at_spisync) {
        return -1;
    }
    spisync_init(at_spisync, &spisync_config);

    at_module_init();
}

