#include "qcc74x_mtimer.h"
#include "qcc74x_dma.h"
#include "qcc74x_uart.h"
#include "board.h"

struct qcc74x_device_s *uartx;
struct qcc74x_device_s *dma0_ch0;

#define UART_RX_DMA_BUF_SIZE 4096
static ATTR_NOCACHE_NOINIT_RAM_SECTION uint8_t uart_rx_dma_buf[UART_RX_DMA_BUF_SIZE];

struct qcc74x_rx_cycle_dma g_uart_rx_dma;
volatile uint8_t count = 0;
struct qcc74x_dma_channel_lli_pool_s rx_llipool[20];

void uart_isr(int irq, void *arg)
{
    uint32_t intstatus = qcc74x_uart_get_intstatus(uartx);

    if (intstatus & UART_INTSTS_RTO) {
        //printf("rto\r\n");
        qcc74x_uart_int_clear(uartx, UART_INTCLR_RTO);
        qcc74x_rx_cycle_dma_process(&g_uart_rx_dma, 0);
    }
}

void dma0_ch0_isr(void *arg)
{
    //printf("dma\r\n");
    qcc74x_rx_cycle_dma_process(&g_uart_rx_dma, 1);
}

void qcc74x_rx_cycle_dma_copy(uint8_t *data, uint32_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        if (data[i] != count) {
            printf("rx error\r\n");
            while (1) {}
        }
        count++;
        count &= 0xff;
    }
}

int main(void)
{
    board_init();
    board_uartx_gpio_init();

    uartx = qcc74x_device_get_by_name(DEFAULT_TEST_UART);

    struct qcc74x_uart_config_s cfg;

    cfg.baudrate = 2000000;
    cfg.data_bits = UART_DATA_BITS_8;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.parity = UART_PARITY_NONE;
    cfg.flow_ctrl = 0;
    cfg.tx_fifo_threshold = 7;
    cfg.rx_fifo_threshold = 0;
    cfg.bit_order = UART_LSB_FIRST;
    qcc74x_uart_init(uartx, &cfg);

    qcc74x_uart_feature_control(uartx, UART_CMD_SET_RTO_VALUE, 0x80);
    qcc74x_irq_attach(uartx->irq_num, uart_isr, NULL);

    qcc74x_uart_link_rxdma(uartx, true);

    dma0_ch0 = qcc74x_device_get_by_name("dma0_ch0");

    struct qcc74x_dma_channel_config_s rxconfig;

    rxconfig.direction = DMA_PERIPH_TO_MEMORY;
    rxconfig.src_req = DEFAULT_TEST_UART_DMA_RX_REQUEST;
    rxconfig.dst_req = DMA_REQUEST_NONE;
    rxconfig.src_addr_inc = DMA_ADDR_INCREMENT_DISABLE;
    rxconfig.dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE;
    rxconfig.src_burst_count = DMA_BURST_INCR1;
    rxconfig.dst_burst_count = DMA_BURST_INCR1;
    rxconfig.src_width = DMA_DATA_WIDTH_8BIT;
    rxconfig.dst_width = DMA_DATA_WIDTH_8BIT;
    qcc74x_dma_channel_init(dma0_ch0, &rxconfig);

    qcc74x_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL);

    qcc74x_rx_cycle_dma_init(&g_uart_rx_dma,
                           dma0_ch0,
                           rx_llipool,
                           20,
                           DEFAULT_TEST_UART_DMA_RDR,
                           uart_rx_dma_buf,
                           UART_RX_DMA_BUF_SIZE,
                           qcc74x_rx_cycle_dma_copy);

    qcc74x_dma_channel_start(dma0_ch0);
    qcc74x_irq_enable(uartx->irq_num);

    /* simulate flash operation */
    uintptr_t flag = qcc74x_irq_save();
    qcc74x_mtimer_delay_ms(5000);
    qcc74x_irq_restore(flag);

    while (1) {
        qcc74x_mtimer_delay_ms(2000);
    }
}
