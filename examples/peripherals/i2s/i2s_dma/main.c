#include "qcc74x_mtimer.h"
#include "qcc74x_i2s.h"
#include "qcc74x_dma.h"
#include "qcc74x_gpio.h"
#include "board.h"

#include "qcc74x_clock.h"

#define I2S_CHANMEL_MERGE_ENABLE 1

struct qcc74x_device_s *i2s0;
struct qcc74x_device_s *dma0_ch0;
struct qcc74x_device_s *dma0_ch1;

static ATTR_NOCACHE_NOINIT_RAM_SECTION uint16_t tx_buffer[256] __ALIGNED(4);
static ATTR_NOCACHE_NOINIT_RAM_SECTION uint16_t rx_buffer[256] __ALIGNED(4);

static volatile uint8_t dma_tc_flag0 = 0;
static volatile uint8_t dma_tc_flag1 = 0;
struct qcc74x_dma_channel_lli_pool_s tx_llipool[1];
struct qcc74x_dma_channel_lli_pool_s rx_llipool[1];

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
    uint32_t i;

    for (i = 0; i < 256; i++) {
        tx_buffer[i] = i;
        rx_buffer[i] = 0;
    }
}

int main(void)
{
    struct qcc74x_dma_channel_lli_transfer_s tx_transfers[1];
    struct qcc74x_dma_channel_lli_transfer_s rx_transfers[1];

    uint32_t num;

    struct qcc74x_i2s_config_s i2s_cfg = {
        .bclk_freq_hz = 32000 * 16 * 2, /* bclk = Sampling_rate * frame_width * channel_num */
        .role = I2S_ROLE_MASTER,
        .format_mode = I2S_MODE_LEFT_JUSTIFIED,
        .channel_mode = I2S_CHANNEL_MODE_NUM_2,
        .frame_width = I2S_SLOT_WIDTH_16,
        .data_width = I2S_SLOT_WIDTH_16,
        .fs_offset_cycle = 0,

        .tx_fifo_threshold = 0,
        .rx_fifo_threshold = 0,
    };

    struct qcc74x_dma_channel_config_s tx_config = {
        .direction = DMA_MEMORY_TO_PERIPH,
        .src_req = DMA_REQUEST_NONE,
        .dst_req = DMA_REQUEST_I2S_TX,
        .src_addr_inc = DMA_ADDR_INCREMENT_ENABLE,
        .dst_addr_inc = DMA_ADDR_INCREMENT_DISABLE,
        .src_burst_count = DMA_BURST_INCR1,
        .dst_burst_count = DMA_BURST_INCR1,
#if I2S_CHANMEL_MERGE_ENABLE
        .src_width = DMA_DATA_WIDTH_32BIT,
        .dst_width = DMA_DATA_WIDTH_32BIT,
#else
        .src_width = DMA_DATA_WIDTH_16BIT,
        .dst_width = DMA_DATA_WIDTH_16BIT,
#endif
    };

    struct qcc74x_dma_channel_config_s rx_config = {
        .direction = DMA_PERIPH_TO_MEMORY,
        .src_req = DMA_REQUEST_I2S_RX,
        .dst_req = DMA_REQUEST_NONE,
        .src_addr_inc = DMA_ADDR_INCREMENT_DISABLE,
        .dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE,
        .src_burst_count = DMA_BURST_INCR1,
        .dst_burst_count = DMA_BURST_INCR1,
#if I2S_CHANMEL_MERGE_ENABLE
        .src_width = DMA_DATA_WIDTH_32BIT,
        .dst_width = DMA_DATA_WIDTH_32BIT,
#else
        .src_width = DMA_DATA_WIDTH_16BIT,
        .dst_width = DMA_DATA_WIDTH_16BIT,
#endif
    };

    board_init();
    sram_init();
    /* gpio init */
    board_i2s_gpio_init();

    i2s0 = qcc74x_device_get_by_name("i2s0");
    /* i2s init */
    qcc74x_i2s_init(i2s0, &i2s_cfg);
    /* enable dma */
    qcc74x_i2s_link_txdma(i2s0, true);
    qcc74x_i2s_link_rxdma(i2s0, true);

    printf("\n\ri2s dma test\n\r");

    dma0_ch0 = qcc74x_device_get_by_name("dma0_ch0");
    dma0_ch1 = qcc74x_device_get_by_name("dma0_ch1");

    qcc74x_dma_channel_init(dma0_ch0, &tx_config);
    qcc74x_dma_channel_init(dma0_ch1, &rx_config);

    qcc74x_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL);
    qcc74x_dma_channel_irq_attach(dma0_ch1, dma0_ch1_isr, NULL);

    tx_transfers[0].src_addr = (uint32_t)tx_buffer;
    tx_transfers[0].dst_addr = (uint32_t)DMA_ADDR_I2S_TDR;
    tx_transfers[0].nbytes = sizeof(tx_buffer);

    rx_transfers[0].src_addr = (uint32_t)DMA_ADDR_I2S_RDR;
    rx_transfers[0].dst_addr = (uint32_t)rx_buffer;
    rx_transfers[0].nbytes = sizeof(rx_buffer);

    num = qcc74x_dma_channel_lli_reload(dma0_ch0, tx_llipool, 1, tx_transfers, 1);
    qcc74x_dma_channel_lli_link_head(dma0_ch0, tx_llipool, num);
    num = qcc74x_dma_channel_lli_reload(dma0_ch1, rx_llipool, 1, rx_transfers, 1);
    qcc74x_dma_channel_lli_link_head(dma0_ch1, tx_llipool, num);
    qcc74x_dma_channel_start(dma0_ch0);
    qcc74x_dma_channel_start(dma0_ch1);

#if I2S_CHANMEL_MERGE_ENABLE
    /* enable channel merge */
    qcc74x_i2s_feature_control(i2s0, I2S_CMD_CHANNEL_LR_MERGE, true);
#endif

    /* enable i2s tx and rx */
    qcc74x_i2s_feature_control(i2s0, I2S_CMD_DATA_ENABLE, I2S_CMD_DATA_ENABLE_TX | I2S_CMD_DATA_ENABLE_RX);

    printf("\n\rtest end\n\r");

    while (1) {
    }
}
