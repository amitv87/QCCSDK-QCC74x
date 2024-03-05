#include "qcc74x_adc.h"
#include "qcc74x_mtimer.h"
#include "board.h"

struct qcc74x_device_s *adc;

#define TEST_ADC_CHANNELS 2

#define TEST_COUNT 10

struct qcc74x_adc_channel_s chan[] = {
    { .pos_chan = ADC_CHANNEL_2,
      .neg_chan = ADC_CHANNEL_GND },
    { .pos_chan = ADC_CHANNEL_GND,
      .neg_chan = ADC_CHANNEL_3 },
};

int main(void)
{
    board_init();
    board_adc_gpio_init();

    adc = qcc74x_device_get_by_name("adc");

    /* adc clock = XCLK / 2 / 32 */
    struct qcc74x_adc_config_s cfg;
    cfg.clk_div = ADC_CLK_DIV_32;
    cfg.scan_conv_mode = true;
    cfg.continuous_conv_mode = false;
    cfg.differential_mode = true;
    cfg.resolution = ADC_RESOLUTION_16B;
    cfg.vref = ADC_VREF_3P2V;

    qcc74x_adc_init(adc, &cfg);
    qcc74x_adc_channel_config(adc, chan, TEST_ADC_CHANNELS);

    for (uint32_t i = 0; i < TEST_COUNT; i++) {
        qcc74x_adc_start_conversion(adc);

        while (qcc74x_adc_get_count(adc) < TEST_ADC_CHANNELS) {
            qcc74x_mtimer_delay_ms(1);
        }

        for (size_t j = 0; j < TEST_ADC_CHANNELS; j++) {
            struct qcc74x_adc_result_s result;
            uint32_t raw_data = qcc74x_adc_read_raw(adc);
            printf("raw data:%08x\r\n", raw_data);
            qcc74x_adc_parse_result(adc, &raw_data, &result, 1);
            printf("pos chan %d,neg chan %d,%d mv \r\n", result.pos_chan, result.neg_chan, result.millivolt);
        }

        qcc74x_adc_stop_conversion(adc);
        qcc74x_mtimer_delay_ms(100);
    }

    qcc74x_adc_deinit(adc);
    
    while (1) {
    }
}
