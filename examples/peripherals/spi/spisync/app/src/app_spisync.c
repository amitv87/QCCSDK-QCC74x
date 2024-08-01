
#include <stdio.h>

#include "qcc74x_irq.h"
#include "qcc74x_uart.h"
#include "qcc743_glb.h"
#include "spisync.h"
#include "log.h"
#include "qcc74x_dma.h"
#include "shell.h"

const struct spisync_config spisync_config = {
    .spi_name = "spi0",
    .spi_tx_dmach = "dma0_ch0",
    .spi_rx_dmach = "dma0_ch1",

    .spi_port = 0,
    .spi_3pin_mode = 1,
    .spi_mode = 0,
    .spi_speed = 80000000,

    .spi_dma_req_tx = DMA_REQUEST_SPI0_TX,
    .spi_dma_req_rx = DMA_REQUEST_SPI0_RX,
    .spi_dma_tdr = DMA_ADDR_SPI0_TDR,
    .spi_dma_rdr = DMA_ADDR_SPI0_RDR,
};

spisync_t    g_spisync;

static void spisync_dump_cmd(int argc, char **argv)
{
    spisync_dump_internal(&g_spisync);
}

static void spisync_iperf_cmd(int argc, char **argv)
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

    spisync_iperf_test(&g_spisync, tx_en, type, 1, 30);
}

static void spisync_type_cmd(int argc, char **argv)
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
    spisync_type_test(&g_spisync, period, timeout);
}

static void spisync_wake_cmd(int argc, char **argv)
{
    spisync_wakeup(&g_spisync);
}

SHELL_CMD_EXPORT_ALIAS(spisync_dump_cmd, spisync, spisync dump);
SHELL_CMD_EXPORT_ALIAS(spisync_iperf_cmd, spisync_iperf, spisync_iperf <tx_en> <type>);
SHELL_CMD_EXPORT_ALIAS(spisync_type_cmd, spisync_type, spisync_type <period> <timeout>);
SHELL_CMD_EXPORT_ALIAS(spisync_wake_cmd, spisync_wakeup, spisync_wakeup);

int app_spisync_init(void)
{
    spisync_init(&g_spisync, &spisync_config);
    return 0;
}

