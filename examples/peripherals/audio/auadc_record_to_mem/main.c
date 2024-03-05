#include "qcc74x_auadc.h"
#include "qcc74x_gpio.h"
#include "qcc74x_dma.h"

#include "qcc743_glb.h"
#include "board.h"

#define AUADC_SAMPLING_NUM (48 * 1024)

static __attribute((aligned(32))) uint16_t record_buff[AUADC_SAMPLING_NUM];
static struct qcc74x_dma_channel_lli_pool_s lli_pool[20];

/* audio adc config */
struct qcc74x_auadc_init_config_s auadc_init_cfg = {
    .sampling_rate = AUADC_SAMPLING_RATE_48K,
    .input_mode = AUADC_INPUT_MODE_ADC,
    .data_format = AUADC_DATA_FORMAT_16BIT,
    .fifo_threshold = 3,
};

/* audio adc analog config */
struct qcc74x_auadc_adc_init_config_s auadc_analog_init_cfg = {
    .auadc_analog_en = true,
    .adc_mode = AUADC_ADC_MODE_AUDIO,
    .adc_pga_mode = AUADC_ADC_PGA_MODE_AC_DIFFER,
    .adc_pga_posi_ch = AUADC_ADC_ANALOG_CH_6,
    .adc_pga_nega_ch = AUADC_ADC_ANALOG_CH_7,
    .adc_pga_gain = 30,
};

struct qcc74x_device_s *auadc_hd;
struct qcc74x_device_s *auadc_dma_hd;

void audio_dma_callback(void *arg)
{
    qcc74x_l1c_dcache_invalidate_range(record_buff, sizeof(record_buff));
    printf("auadc record end\r\n");
}

void auadc_init(void)
{
    /* clock config */
    GLB_Config_AUDIO_PLL_To_491P52M();
    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_AUDIO);

    /* auadc init */
    auadc_hd = qcc74x_device_get_by_name("auadc");
    qcc74x_auadc_init(auadc_hd, &auadc_init_cfg);
    qcc74x_auadc_adc_init(auadc_hd, &auadc_analog_init_cfg);
    /* set volume */
    qcc74x_auadc_feature_control(auadc_hd, AUADC_CMD_SET_VOLUME_VAL, (size_t)(0));
    /* auadc enable dma */
    qcc74x_auadc_link_rxdma(auadc_hd, true);
}

void auadc_dma_init(void)
{
    struct qcc74x_dma_channel_lli_transfer_s transfers[1];
    struct qcc74x_dma_channel_config_s auadc_dma_cfg;

    auadc_dma_cfg.direction = DMA_PERIPH_TO_MEMORY;
    auadc_dma_cfg.src_req = DMA_REQUEST_AUADC_RX;
    auadc_dma_cfg.dst_req = DMA_REQUEST_NONE;
    auadc_dma_cfg.src_addr_inc = DMA_ADDR_INCREMENT_DISABLE;
    auadc_dma_cfg.dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE;
    auadc_dma_cfg.src_burst_count = DMA_BURST_INCR4;
    auadc_dma_cfg.dst_burst_count = DMA_BURST_INCR4;
    auadc_dma_cfg.src_width = DMA_DATA_WIDTH_16BIT;
    auadc_dma_cfg.dst_width = DMA_DATA_WIDTH_16BIT;

    auadc_dma_hd = qcc74x_device_get_by_name("dma0_ch0");
    qcc74x_dma_channel_init(auadc_dma_hd, &auadc_dma_cfg);
    qcc74x_dma_channel_irq_attach(auadc_dma_hd, audio_dma_callback, NULL);

    transfers[0].src_addr = (uint32_t)DMA_ADDR_AUADC_RDR;
    transfers[0].dst_addr = (uint32_t)record_buff;
    transfers[0].nbytes = sizeof(record_buff);

    qcc74x_dma_channel_lli_reload(auadc_dma_hd, lli_pool, 20, transfers, 1);
}

/* gpio init */
void auadc_gpio_init(void)
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");

    /* auadc ch0 */
    //qcc74x_gpio_init(gpio, GPIO_PIN_20, GPIO_ANALOG | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_2);
    /* auadc ch1 */
    //qcc74x_gpio_init(gpio, GPIO_PIN_21, GPIO_ANALOG | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_2);

    /* auadc ch2 */
    //qcc74x_gpio_init(gpio, GPIO_PIN_22, GPIO_ANALOG | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_2);
    /* auadc ch3 */
    //qcc74x_gpio_init(gpio, GPIO_PIN_23, GPIO_ANALOG | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_2);

    /* auadc ch4 */
    //qcc74x_gpio_init(gpio, GPIO_PIN_27, GPIO_ANALOG | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_2);
    /* auadc ch5 */
    //qcc74x_gpio_init(gpio, GPIO_PIN_28, GPIO_ANALOG | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_2);

    /* auadc ch6 */
    qcc74x_gpio_init(gpio, GPIO_PIN_29, GPIO_ANALOG | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_2);
    /* auadc ch7 */
    qcc74x_gpio_init(gpio, GPIO_PIN_30, GPIO_ANALOG | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_2);
}

int main(void)
{
    /* board init */
    board_init();

    printf("auadc record to mem start \r\n");

    /* gpio init */
    auadc_gpio_init();

    /* auadc init */
    auadc_init();

    /* dma init and start */
    auadc_dma_init();
    qcc74x_dma_channel_start(auadc_dma_hd);

    /* auadc start */
    qcc74x_auadc_feature_control(auadc_hd, AUADC_CMD_RECORD_START, 0);

    printf("Wait for dma to complete... \r\n");

    qcc74x_mtimer_delay_ms(100);

    while (1) {
        qcc74x_mtimer_delay_ms(100);
    }
}
