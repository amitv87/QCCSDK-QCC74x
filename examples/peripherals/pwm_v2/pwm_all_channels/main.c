#include "qcc74x_mtimer.h"
#include "qcc74x_pwm_v2.h"
#include "qcc74x_clock.h"
#include "board.h"

struct qcc74x_device_s *pwm;

int main(void)
{
    board_init();
    board_pwm_gpio_init();

    pwm = qcc74x_device_get_by_name("pwm_v2_0");

    /* period = .PBCLK / .clk_div / .period = 80MHz( 64MHZ for qcc74x_undefl) / 80 / 1000 = 1KHz */
    struct qcc74x_pwm_v2_config_s cfg = {
        .clk_source = QCC74x_SYSTEM_PBCLK,
#if defined(QCC74x_undefL)
        .clk_div = 64,
#else
        .clk_div = 80,
#endif
        .period = 1000,
    };

    qcc74x_pwm_v2_init(pwm, &cfg);
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_CH0, 100, 500); /* duty = (500-100)/1000 = 40% */
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_CH1, 200, 400); /* duty = (400-200)/1000 = 20% */
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_CH2, 100, 999); /* duty = (999-100)/1000 = 89.9% */
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_CH3, 0, 500); /* duty = (500-0)/1000 = 50% */
    qcc74x_pwm_v2_channel_positive_start(pwm, PWM_CH0);
    qcc74x_pwm_v2_channel_negative_start(pwm, PWM_CH0);
    qcc74x_pwm_v2_channel_positive_start(pwm, PWM_CH1);
    qcc74x_pwm_v2_channel_negative_start(pwm, PWM_CH1);
    qcc74x_pwm_v2_channel_positive_start(pwm, PWM_CH2);
    qcc74x_pwm_v2_channel_negative_start(pwm, PWM_CH2);
    qcc74x_pwm_v2_channel_positive_start(pwm, PWM_CH3);
    qcc74x_pwm_v2_channel_negative_start(pwm, PWM_CH3);
    qcc74x_pwm_v2_start(pwm);

    while (1) {
        printf("pwm all_channel running\r\n");
        qcc74x_mtimer_delay_ms(2000);
    }
}
