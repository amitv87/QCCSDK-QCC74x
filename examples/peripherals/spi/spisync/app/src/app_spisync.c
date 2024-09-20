
#include <stdio.h>

#include "qcc74x_irq.h"
#include "qcc74x_uart.h"
#include "qcc743_glb.h"
#include "spisync.h"
#include "log.h"
#include "qcc74x_dma.h"
#include "shell.h"

spisync_t    g_spisync;
static void __spisync_gpio_init(void *arg);

const spisync_hw_t spisync_hw = {
    .spi_name = "spi0",
    .spi_tx_dmach = "dma0_ch0",
    .spi_rx_dmach = "dma0_ch1",
    .spi_3pin_mode = 1,
    .spi_port = 0,
    .spi_mode = 0,
    .spi_speed = 80000000,
    .data_rdy_pin = GPIO_PIN_20,

    .spi_dma_req_tx = DMA_REQUEST_SPI0_TX,
    .spi_dma_req_rx = DMA_REQUEST_SPI0_RX,
    .spi_dma_tdr = DMA_ADDR_SPI0_TDR,
    .spi_dma_rdr = DMA_ADDR_SPI0_RDR,
};

const spisync_config_t spisync_config = {
    .hw = &spisync_hw,
    .ops = {
        {
            .cb     = NULL,
            .cb_pri = NULL,
            .type   = 0
        },
    },
    .reset_cb = __spisync_gpio_init,
    .reset_arg = NULL,
};

static void __spisync_gpio_init(void *arg)
{
    board_spi0_gpio_3pin_init();
}

int app_spisync_init(void)
{
    spisync_init(&g_spisync, &spisync_config);
    return 0;
}

