#include "qcc74x_adc.h"
#include "qcc74x_mtimer.h"
#include "board.h"

struct qcc74x_device_s *adc;

int main(void)
{
    board_init();

    adc = qcc74x_device_get_by_name("adc");

    /* adc clock = XCLK / 2 / 32 */
    struct qcc74x_adc_config_s cfg;
    cfg.clk_div = ADC_CLK_DIV_32;
    cfg.scan_conv_mode = false;
    cfg.continuous_conv_mode = false;
    cfg.differential_mode = false;
    cfg.resolution = ADC_RESOLUTION_16B;
    cfg.vref = ADC_VREF_3P2V;

    struct qcc74x_adc_channel_s chan;

    chan.pos_chan = ADC_CHANNEL_VABT_HALF;
    chan.neg_chan = ADC_CHANNEL_GND;

    qcc74x_adc_init(adc, &cfg);
    qcc74x_adc_channel_config(adc, &chan, 1);
    qcc74x_adc_vbat_enable(adc);

    struct qcc74x_adc_result_s result;

    for (uint16_t i = 0; i < 10; i++) {
        qcc74x_adc_start_conversion(adc);
        while (qcc74x_adc_get_count(adc) == 0) {
            qcc74x_mtimer_delay_ms(1);
        }
        uint32_t raw_data = qcc74x_adc_read_raw(adc);

        qcc74x_adc_parse_result(adc, &raw_data, &result, 1);
        printf("vBat = %d mV\r\n", (uint32_t)(result.millivolt * 2));
        qcc74x_adc_stop_conversion(adc);

        qcc74x_mtimer_delay_ms(500);
    }

    qcc74x_adc_deinit(adc);
    while (1) {
    }
}
