#include "board.h"

#include "qcc74x_clock.h"
#include "qcc74x_mtimer.h"
#include "qcc74x_i2s.h"
#include "qcc74x_i2c.h"
#include "qcc74x_dma.h"
#include "qcc74x_gpio.h"

#include "qcc743_glb.h"

#include "fhm_onechannel_16k_20.h"
#include "bsp_es8388.h"

struct qcc74x_device_s *i2s0;
struct qcc74x_device_s *i2c0;
struct qcc74x_device_s *dma0_ch0;

volatile uint8_t dma_tc_flag0 = 0;
static struct qcc74x_dma_channel_lli_pool_s tx_llipool[100];

static ES8388_Cfg_Type ES8388Cfg = {
    .work_mode = ES8388_CODEC_MDOE,          /*!< ES8388 work mode */
    .role = ES8388_SLAVE,                    /*!< ES8388 role */
    .mic_input_mode = ES8388_DIFF_ENDED_MIC, /*!< ES8388 mic input mode */
    .mic_pga = ES8388_MIC_PGA_3DB,           /*!< ES8388 mic PGA */
    .i2s_frame = ES8388_LEFT_JUSTIFY_FRAME,  /*!< ES8388 I2S frame */
    .data_width = ES8388_DATA_LEN_16,        /*!< ES8388 I2S dataWitdh */
};

void dma0_ch0_isr(void *arg)
{
    dma_tc_flag0++;
    printf("tc done\r\n");
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

    /* MCLK CLKOUT */
    qcc74x_gpio_init(gpio, GPIO_PIN_23, GPIO_FUNC_CLKOUT | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);

    /* I2C0_SCL */
    qcc74x_gpio_init(gpio, GPIO_PIN_10, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
    /* I2C0_SDA */
    qcc74x_gpio_init(gpio, GPIO_PIN_11, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
}

void i2s_dma_init()
{
    static struct qcc74x_dma_channel_lli_transfer_s tx_transfers[1];

    struct qcc74x_i2s_config_s i2s_cfg = {
        .bclk_freq_hz = 16000 * 32 * 2, /* bclk = Sampling_rate * frame_width * channel_num */
        .role = I2S_ROLE_MASTER,
        .format_mode = I2S_MODE_LEFT_JUSTIFIED,
        .channel_mode = I2S_CHANNEL_MODE_NUM_1,
        .frame_width = I2S_SLOT_WIDTH_32,
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

    printf("i2s init\r\n");
    i2s0 = qcc74x_device_get_by_name("i2s0");
    /* i2s init */
    qcc74x_i2s_init(i2s0, &i2s_cfg);
    /* enable dma */
    qcc74x_i2s_link_txdma(i2s0, true);
    qcc74x_i2s_link_rxdma(i2s0, true);

    printf("dma init\r\n");
    dma0_ch0 = qcc74x_device_get_by_name("dma0_ch0");
    qcc74x_dma_channel_init(dma0_ch0, &tx_config);
    qcc74x_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL);

    tx_transfers[0].src_addr = (uint32_t)fhm_onechannel_16k_20;
    tx_transfers[0].dst_addr = (uint32_t)DMA_ADDR_I2S_TDR;
    tx_transfers[0].nbytes = sizeof(fhm_onechannel_16k_20);

    printf("dma lli init\r\n");
    uint32_t num = qcc74x_dma_channel_lli_reload(dma0_ch0, tx_llipool, 100, tx_transfers, 1);
    qcc74x_dma_channel_lli_link_head(dma0_ch0, tx_llipool, num);
    printf("dma lli num: %d \r\n", num);
    qcc74x_dma_channel_start(dma0_ch0);
}

void mclk_out_init()
{
#ifdef QCC743
    /* output MCLK,
    Will change the clock source of i2s,
    It needs to be called before i2s is initialized
    clock source 25M
    */
    GLB_Set_I2S_CLK(ENABLE, 2, GLB_I2S_DI_SEL_I2S_DI_INPUT, GLB_I2S_DO_SEL_I2S_DO_OUTPT);
    GLB_Set_Chip_Clock_Out3_Sel(GLB_CHIP_CLK_OUT_3_I2S_REF_CLK);
#endif
}

int main(void)
{
    board_init();

    printf("\n\ri2s dma test\n\r");

    /* gpio init */
    i2s_gpio_init();

    /* init ES8388 Codec */
    printf("es8388 init\n\r");
    ES8388_Init(&ES8388Cfg);
    ES8388_Set_Voice_Volume(90);

    /* mclk clkout init */
    mclk_out_init();

    /* i2s init */
    i2s_dma_init();

    /* enable i2s tx and rx */
    qcc74x_i2s_feature_control(i2s0, I2S_CMD_DATA_ENABLE, I2S_CMD_DATA_ENABLE_TX | I2S_CMD_DATA_ENABLE_RX);

    printf("test end\n\r");

    while (1) {
        qcc74x_mtimer_delay_ms(10);
    }
}
