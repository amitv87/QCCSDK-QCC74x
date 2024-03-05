#include "qcc74x_mtimer.h"
#include "qcc74x_dma.h"
#include "qcc74x_uart.h"
#include "qcc74x_l1c.h"
#include "board.h"

struct qcc74x_device_s *uartx;
struct qcc74x_device_s *dma0_ch0;
struct qcc74x_device_s *dma0_ch1;

static ATTR_NOCACHE_NOINIT_RAM_SECTION uint8_t src_buffer[4100];
static ATTR_NOCACHE_NOINIT_RAM_SECTION uint8_t src2_buffer[4100];
static ATTR_NOCACHE_NOINIT_RAM_SECTION uint8_t src3_buffer[4100];
static ATTR_NOCACHE_NOINIT_RAM_SECTION uint8_t receive_buffer[50] = { 0 };

static volatile uint8_t dma_tc_flag0 = 0;
static volatile uint8_t dma_tc_flag1 = 0;
struct qcc74x_dma_channel_lli_pool_s tx_llipool[20]; /* max trasnfer size 4064 * 20 */
struct qcc74x_dma_channel_lli_pool_s rx_llipool[20];

void dma0_ch0_isr(void *arg)
{
    dma_tc_flag0++;
    printf("tc done\r\n");
}

void dma0_ch1_isr(void *arg)
{
    dma_tc_flag1++;
    printf("rx done\r\n");
}

void sram_init()
{
    memset(src_buffer, 'a', 4100);
    src_buffer[3999] = 'B';
    src_buffer[4095] = 'A';
    src_buffer[4096] = 'B';
    src_buffer[4097] = 'C';
    src_buffer[4098] = 'D';
    src_buffer[4099] = 'E';

    memset(src2_buffer, 'c', 4100);
    memset(src3_buffer, 'd', 4100);
}

int main(void)
{
    board_init();
    board_uartx_gpio_init();
    sram_init();

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
    qcc74x_uart_link_txdma(uartx, true);
    qcc74x_uart_link_rxdma(uartx, true);

    dma0_ch0 = qcc74x_device_get_by_name("dma0_ch0");
    dma0_ch1 = qcc74x_device_get_by_name("dma0_ch1");

    struct qcc74x_dma_channel_config_s config;

    config.direction = DMA_MEMORY_TO_PERIPH;
    config.src_req = DMA_REQUEST_NONE;
    config.dst_req = DEFAULT_TEST_UART_DMA_TX_REQUEST;
    config.src_addr_inc = DMA_ADDR_INCREMENT_ENABLE;
    config.dst_addr_inc = DMA_ADDR_INCREMENT_DISABLE;
    config.src_burst_count = DMA_BURST_INCR1;
    config.dst_burst_count = DMA_BURST_INCR1;
    config.src_width = DMA_DATA_WIDTH_8BIT;
    config.dst_width = DMA_DATA_WIDTH_8BIT;
    qcc74x_dma_channel_init(dma0_ch0, &config);

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
    qcc74x_dma_channel_init(dma0_ch1, &rxconfig);

    qcc74x_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL);
    qcc74x_dma_channel_irq_attach(dma0_ch1, dma0_ch1_isr, NULL);

    struct qcc74x_dma_channel_lli_transfer_s tx_transfers[3];

    tx_transfers[0].src_addr = (uint32_t)src_buffer;
    tx_transfers[0].dst_addr = (uint32_t)DEFAULT_TEST_UART_DMA_TDR;
    tx_transfers[0].nbytes = 4100;

    tx_transfers[1].src_addr = (uint32_t)src2_buffer;
    tx_transfers[1].dst_addr = (uint32_t)DEFAULT_TEST_UART_DMA_TDR;
    tx_transfers[1].nbytes = 4100;

    tx_transfers[2].src_addr = (uint32_t)src3_buffer;
    tx_transfers[2].dst_addr = (uint32_t)DEFAULT_TEST_UART_DMA_TDR;
    tx_transfers[2].nbytes = 4100;

    struct qcc74x_dma_channel_lli_transfer_s rx_transfers[1];
    rx_transfers[0].src_addr = (uint32_t)DEFAULT_TEST_UART_DMA_RDR;
    rx_transfers[0].dst_addr = (uint32_t)receive_buffer;
    rx_transfers[0].nbytes = 50;

    qcc74x_dma_channel_lli_reload(dma0_ch0, tx_llipool, 20, tx_transfers, 3);
    qcc74x_dma_channel_lli_reload(dma0_ch1, rx_llipool, 20, rx_transfers, 1);
    qcc74x_dma_channel_start(dma0_ch0);
    qcc74x_dma_channel_start(dma0_ch1);

    while (dma_tc_flag0 != 3) {
        qcc74x_mtimer_delay_ms(1);
    }
    while (dma_tc_flag1 == 0) {
    }
    for (uint8_t i = 0; i < 50; i++) {
        printf("receive data:%02x\r\n", receive_buffer[i]);
    }
    while (1) {
        qcc74x_mtimer_delay_ms(2000);
    }
}
