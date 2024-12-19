#include "qcc74x_adc.h"
#include "qcc74x_efuse.h"
#include "qcc74x_mtimer.h"
#include "board.h"

struct qcc74x_device_s *adc;

int main(void)
{
    board_init();
    board_adc_gpio_init();

    adc = qcc74x_device_get_by_name("adc");

    /* adc clock = XCLK / 2 / 32 */
    struct qcc74x_adc_config_s cfg;
    cfg.clk_div = ADC_CLK_DIV_32;
    cfg.scan_conv_mode = false;
    cfg.continuous_conv_mode = true;
    cfg.differential_mode = false;
    cfg.resolution = ADC_RESOLUTION_16B;
    cfg.vref = ADC_VREF_2P0V;

    struct qcc74x_adc_channel_s chan;

    chan.pos_chan = ADC_CHANNEL_TSEN_P;
    chan.neg_chan = ADC_CHANNEL_GND;

    qcc74x_adc_init(adc, &cfg);
    qcc74x_adc_channel_config(adc, &chan, 1);
    qcc74x_adc_tsen_init(adc, ADC_TSEN_MOD_INTERNAL_DIODE);

    for (uint32_t cnt = 0; cnt < 5; cnt++) {
        qcc74x_mtimer_delay_ms(100);
        printf("temp = %d\r\n", (uint32_t)(qcc74x_adc_tsen_get_temp(adc)));
    }

    return 0;
}
