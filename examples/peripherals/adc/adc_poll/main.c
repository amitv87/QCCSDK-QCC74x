#include "qcc74x_adc.h"
#include "qcc74x_mtimer.h"
#include "board.h"

struct qcc74x_device_s *adc;

#define TEST_ADC_CHANNEL_0  1
#define TEST_ADC_CHANNEL_1  1
#define TEST_ADC_CHANNEL_2  1
#define TEST_ADC_CHANNEL_3  1
#define TEST_ADC_CHANNEL_4  1
#define TEST_ADC_CHANNEL_5  1
#define TEST_ADC_CHANNEL_6  1
#define TEST_ADC_CHANNEL_7  1
#define TEST_ADC_CHANNEL_8  1
#define TEST_ADC_CHANNEL_9  1
#define TEST_ADC_CHANNEL_10 1

#define TEST_ADC_CHANNELS   (TEST_ADC_CHANNEL_0 + \
                           TEST_ADC_CHANNEL_1 +   \
                           TEST_ADC_CHANNEL_2 +   \
                           TEST_ADC_CHANNEL_3 +   \
                           TEST_ADC_CHANNEL_4 +   \
                           TEST_ADC_CHANNEL_5 +   \
                           TEST_ADC_CHANNEL_6 +   \
                           TEST_ADC_CHANNEL_7 +   \
                           TEST_ADC_CHANNEL_8 +   \
                           TEST_ADC_CHANNEL_9 +   \
                           TEST_ADC_CHANNEL_10)

struct qcc74x_adc_channel_s chan[] = {
#if TEST_ADC_CHANNEL_0
    { .pos_chan = ADC_CHANNEL_0,
      .neg_chan = ADC_CHANNEL_GND },
#endif
#if TEST_ADC_CHANNEL_1
    { .pos_chan = ADC_CHANNEL_1,
      .neg_chan = ADC_CHANNEL_GND },
#endif
#if TEST_ADC_CHANNEL_2
    { .pos_chan = ADC_CHANNEL_2,
      .neg_chan = ADC_CHANNEL_GND },
#endif
#if TEST_ADC_CHANNEL_3
    { .pos_chan = ADC_CHANNEL_3,
      .neg_chan = ADC_CHANNEL_GND },
#endif
#if TEST_ADC_CHANNEL_4
    { .pos_chan = ADC_CHANNEL_4,
      .neg_chan = ADC_CHANNEL_GND },
#endif
#if TEST_ADC_CHANNEL_5
    { .pos_chan = ADC_CHANNEL_5,
      .neg_chan = ADC_CHANNEL_GND },
#endif
#if TEST_ADC_CHANNEL_6
    { .pos_chan = ADC_CHANNEL_6,
      .neg_chan = ADC_CHANNEL_GND },
#endif
#if TEST_ADC_CHANNEL_7
    { .pos_chan = ADC_CHANNEL_7,
      .neg_chan = ADC_CHANNEL_GND },
#endif
#if TEST_ADC_CHANNEL_8
    { .pos_chan = ADC_CHANNEL_8,
      .neg_chan = ADC_CHANNEL_GND },
#endif
#if TEST_ADC_CHANNEL_9
    { .pos_chan = ADC_CHANNEL_9,
      .neg_chan = ADC_CHANNEL_GND },
#endif
#if TEST_ADC_CHANNEL_10
    { .pos_chan = ADC_CHANNEL_10,
      .neg_chan = ADC_CHANNEL_GND },
#endif
};

#define TEST_COUNT 10

uint32_t raw_data[TEST_ADC_CHANNELS * TEST_COUNT];
struct qcc74x_adc_result_s result[TEST_ADC_CHANNELS * TEST_COUNT];

int main(void)
{
    board_init();
    board_adc_gpio_init();

    adc = qcc74x_device_get_by_name("adc");

    /* adc clock = XCLK / 2 / 32 */
    struct qcc74x_adc_config_s cfg;
    cfg.clk_div = ADC_CLK_DIV_32;
    cfg.scan_conv_mode = true;
    cfg.continuous_conv_mode = true; /* do not support single mode */
    cfg.differential_mode = false;
    cfg.resolution = ADC_RESOLUTION_16B;
    cfg.vref = ADC_VREF_3P2V;

    qcc74x_adc_init(adc, &cfg);
    qcc74x_adc_channel_config(adc, chan, TEST_ADC_CHANNELS);

    qcc74x_adc_start_conversion(adc);

    for (uint16_t i = 0; i < TEST_ADC_CHANNELS * TEST_COUNT; i++) {
        while (qcc74x_adc_get_count(adc) == 0) {
            qcc74x_mtimer_delay_ms(1);
        }

        raw_data[i] = qcc74x_adc_read_raw(adc);
    }

    qcc74x_adc_stop_conversion(adc);

    qcc74x_adc_parse_result(adc, raw_data, result, TEST_ADC_CHANNELS * TEST_COUNT);

    for (uint16_t j = 0; j < TEST_ADC_CHANNELS * TEST_COUNT; j++) {
        printf("raw data:%08x\r\n", raw_data[j]);
        printf("pos chan %d,%d mv \r\n", result[j].pos_chan, result[j].millivolt);
    }

    qcc74x_adc_deinit(adc);

    while (1) {
    }
}
