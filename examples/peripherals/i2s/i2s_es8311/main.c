#include "board.h"
#include "qcc74x_gpio.h"
#include "qcc74x_l1c.h"
#include "qcc74x_mtimer.h"

#include "qcc74x_i2c.h"

#include "qcc743_glb.h"
#include "qcc74x_dma.h"
#include "bsp_es8311.h"
#include "qcc74x_i2s.h"

#include "fhm_onechannel_16k_20.h"

// 8311
struct qcc74x_device_s *i2s0;
struct qcc74x_device_s *dma0_ch0;
struct qcc74x_device_s *dma0_ch1;

ATTR_NOCACHE_RAM_SECTION uint8_t rx_buffer[16000];

void dma0_ch0_isr(void *arg)
{
    printf("tc done\r\n");
}

void dma0_ch1_isr(void *arg)
{
    printf("rx done\r\n");\
}

void i2s_gpio_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");

    /* I2S_FS */
    qcc74x_gpio_init(gpio, GPIO_PIN_13, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* I2S_DI */
    qcc74x_gpio_init(gpio, GPIO_PIN_14, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* I2S_DO */
    qcc74x_gpio_init(gpio, GPIO_PIN_15, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* I2S_BCLK */
    qcc74x_gpio_init(gpio, GPIO_PIN_20, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    // /* MCLK CLKOUT */
    // qcc74x_gpio_init(gpio, GPIO_PIN_23, GPIO_FUNC_CLKOUT | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);

    /* I2C0_SCL */
    qcc74x_gpio_init(gpio, GPIO_PIN_10, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
    /* I2C0_SDA */
    qcc74x_gpio_init(gpio, GPIO_PIN_11, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);

}

// void i2s_dma_init()
// {
//     static struct qcc74x_dma_channel_lli_pool_s tx_llipool[100];
//     static struct qcc74x_dma_channel_lli_transfer_s tx_transfers[1];

//     struct qcc74x_i2s_config_s i2s_cfg = {
//         .bclk_freq_hz = 16000 * 32 * 2, /* bclk = Sampling_rate * frame_width * channel_num */
//         .role = I2S_ROLE_MASTER,
//         .format_mode = I2S_MODE_LEFT_JUSTIFIED,
//         .channel_mode = I2S_CHANNEL_MODE_NUM_1,
//         .frame_width = I2S_SLOT_WIDTH_32,
//         .data_width = I2S_SLOT_WIDTH_16,
//         .fs_offset_cycle = 0,

//         .tx_fifo_threshold = 0,
//         .rx_fifo_threshold = 0,
//     };

//     struct qcc74x_dma_channel_config_s tx_config = {
//         .direction = DMA_MEMORY_TO_PERIPH,
//         .src_req = DMA_REQUEST_NONE,
//         .dst_req = DMA_REQUEST_I2S_TX,
//         .src_addr_inc = DMA_ADDR_INCREMENT_ENABLE,
//         .dst_addr_inc = DMA_ADDR_INCREMENT_DISABLE,
//         .src_burst_count = DMA_BURST_INCR1,
//         .dst_burst_count = DMA_BURST_INCR1,
//         .src_width = DMA_DATA_WIDTH_16BIT,
//         .dst_width = DMA_DATA_WIDTH_16BIT,
//     };

//     printf("i2s init\r\n");
//     i2s0 = qcc74x_device_get_by_name("i2s0");
//     /* i2s init */
//     qcc74x_i2s_init(i2s0, &i2s_cfg);
//     /* enable dma */
//     qcc74x_i2s_link_txdma(i2s0, true);
//     qcc74x_i2s_link_rxdma(i2s0, true);

//     printf("dma init\r\n");
//     dma0_ch0 = qcc74x_device_get_by_name("dma0_ch0");
//     qcc74x_dma_channel_init(dma0_ch0, &tx_config);
//     qcc74x_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL);

//     tx_transfers[0].src_addr = (uint32_t)fhm_onechannel_16k_20;
//     tx_transfers[0].dst_addr = (uint32_t)DMA_ADDR_I2S_TDR;
//     tx_transfers[0].nbytes = sizeof(fhm_onechannel_16k_20);

//     printf("dma lli init\r\n");
//     uint32_t num = qcc74x_dma_channel_lli_reload(dma0_ch0, tx_llipool, 100, tx_transfers, 1);
//     qcc74x_dma_channel_lli_link_head(dma0_ch0, tx_llipool, num);
//     printf("dma lli num: %d \r\n", num);
//     qcc74x_dma_channel_start(dma0_ch0);
// }

void i2s_dma_init()
{
    static struct qcc74x_dma_channel_lli_pool_s tx_llipool[100];
    static struct qcc74x_dma_channel_lli_transfer_s tx_transfers[1];
    static struct qcc74x_dma_channel_lli_pool_s rx_llipool[100];
    static struct qcc74x_dma_channel_lli_transfer_s rx_transfers[1];

    struct qcc74x_i2s_config_s i2s_cfg = {
        .bclk_freq_hz = 16000 * 16 * 2, /* bclk = Sampling_rate * frame_width * channel_num */
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
        .src_width = DMA_DATA_WIDTH_16BIT,
        .dst_width = DMA_DATA_WIDTH_16BIT,
    };

    struct qcc74x_dma_channel_config_s rx_config = {
        .direction = DMA_PERIPH_TO_MEMORY,
        .src_req = DMA_REQUEST_I2S_RX,
        .dst_req = DMA_REQUEST_NONE,
        .src_addr_inc = DMA_ADDR_INCREMENT_DISABLE,
        .dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE,
        .src_burst_count = DMA_BURST_INCR1,
        .dst_burst_count = DMA_BURST_INCR1,
        .src_width = DMA_DATA_WIDTH_16BIT,
        .dst_width = DMA_DATA_WIDTH_16BIT
    };
    printf("i2s init\r\n");
    i2s0 = qcc74x_device_get_by_name("i2s0");
    /* i2s init */
    qcc74x_i2s_init(i2s0, &i2s_cfg);
    /* enable dma */
    qcc74x_i2s_link_txdma(i2s0, true);
    qcc74x_i2s_link_rxdma(i2s0, true);

    printf("dma init\r\n");
    dma0_ch0 = qcc74x_device_get_by_name("dma0_ch0");
    dma0_ch1 = qcc74x_device_get_by_name("dma0_ch1");

    qcc74x_dma_channel_init(dma0_ch0, &tx_config);
    qcc74x_dma_channel_init(dma0_ch1, &rx_config);

    qcc74x_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL);
    qcc74x_dma_channel_irq_attach(dma0_ch1, dma0_ch1_isr, NULL);

    tx_transfers[0].src_addr = (uint32_t)rx_buffer;
    tx_transfers[0].dst_addr = (uint32_t)DMA_ADDR_I2S_TDR;
    tx_transfers[0].nbytes = sizeof(rx_buffer);

    rx_transfers[0].src_addr = (uint32_t)DMA_ADDR_I2S_RDR;
    rx_transfers[0].dst_addr = (uint32_t)rx_buffer;
    rx_transfers[0].nbytes = sizeof(rx_buffer);

    printf("dma lli init\r\n");
    uint32_t num = qcc74x_dma_channel_lli_reload(dma0_ch0, tx_llipool, 100, tx_transfers, 1);
    qcc74x_dma_channel_lli_link_head(dma0_ch0, tx_llipool, num);
    printf("tx dma lli num: %d \r\n", num);
    num = qcc74x_dma_channel_lli_reload(dma0_ch1, rx_llipool, 100, rx_transfers, 1);
    printf("rx dma lli num: %d \r\n", num);
    qcc74x_dma_channel_lli_link_head(dma0_ch1, rx_llipool, num);
    qcc74x_dma_channel_start(dma0_ch0);
    qcc74x_dma_channel_start(dma0_ch1);
}

void mclk_out_init()
{
    /* output MCLK,
    Will change the clock source of i2s,
    It needs to be called before i2s is initialized
    clock source 25M
    */
    GLB_Set_I2S_CLK(ENABLE, 0, GLB_I2S_DI_SEL_I2S_DI_INPUT, GLB_I2S_DO_SEL_I2S_DO_OUTPT);
    /* ES8311 do not mclk */
    // GLB_Set_Chip_Clock_Out3_Sel(GLB_CHIP_CLK_OUT_3_I2S_REF_CLK);
}

int main(void)
{
    board_init();

    /* gpio init */
    i2s_gpio_init();
    /* mclk clkout init */
    mclk_out_init();

    i2s_es8311_t es8311_cfg;
    es8311_cfg.mode = ES8311_MODE_SLAVE;
    es8311_cfg.fmt = ES8311_I2S_NORMAL;
    es8311_cfg.samples = ES8311_16K_SAMPLES;
    es8311_cfg.bits = ES8311_BIT_LENGTH_16BITS;
    /* init ES8311 Codec */
    printf("es8311 init\n\r");
    es8311_init(&es8311_cfg);
    es8311_codec_set_voice_volume(70);
    es8311_start(ES_MODULE_ADC_DAC);

    /* i2s init */
    i2s_dma_init();

    /* enable i2s tx and rx */
    qcc74x_i2s_feature_control(i2s0, I2S_CMD_DATA_ENABLE, I2S_CMD_DATA_ENABLE_TX | I2S_CMD_DATA_ENABLE_RX);

    while (1) {
        qcc74x_mtimer_delay_ms(10);
    }
}
