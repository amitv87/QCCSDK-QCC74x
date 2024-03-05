#include "qcc74x_mtimer.h"
#include "qcc74x_pwm_v2.h"
#include "qcc74x_clock.h"
#include "board.h"

#define REPT_CNT 3

struct qcc74x_device_s *pwm;

void pwm_isr(int irq, void *arg)
{
    uint32_t intstatus = qcc74x_pwm_v2_get_intstatus(pwm);

    if (intstatus & PWM_INTSTS_CH0_L) {
        qcc74x_pwm_v2_int_clear(pwm, PWM_INTCLR_CH0_L);
        static int times = 0;
        printf("CH0_L interrupt, %d times\r\n", ++times);
    }
    if (intstatus & PWM_INTSTS_CH0_H) {
        qcc74x_pwm_v2_int_clear(pwm, PWM_INTSTS_CH0_H);
        static int times = 0;
        printf("CH0_H interrupt, %d times\r\n", ++times);
    }
    if (intstatus & PWM_INTSTS_CH1_L) {
        qcc74x_pwm_v2_int_clear(pwm, PWM_INTCLR_CH1_L);
        static int times = 0;
        printf("CH1_L interrupt, %d times\r\n", ++times);
    }
    if (intstatus & PWM_INTSTS_CH1_H) {
        qcc74x_pwm_v2_int_clear(pwm, PWM_INTCLR_CH1_H);
        static int times = 0;
        printf("CH1_H interrupt, %d times\r\n", ++times);
    }
    if (intstatus & PWM_INTSTS_CH2_L) {
        qcc74x_pwm_v2_int_clear(pwm, PWM_INTCLR_CH2_L);
        static int times = 0;
        printf("CH2_L interrupt, %d times\r\n", ++times);
    }
    if (intstatus & PWM_INTSTS_CH2_H) {
        qcc74x_pwm_v2_int_clear(pwm, PWM_INTCLR_CH2_H);
        static int times = 0;
        printf("CH2_H interrupt, %d times\r\n", ++times);
    }
    if (intstatus & PWM_INTSTS_CH3_L) {
        qcc74x_pwm_v2_int_clear(pwm, PWM_INTCLR_CH3_L);
        static int times = 0;
        printf("CH3_L interrupt, %d times\r\n", ++times);
    }
    if (intstatus & PWM_INTSTS_CH3_H) {
        qcc74x_pwm_v2_int_clear(pwm, PWM_INTCLR_CH3_H);
        static int times = 0;
        printf("CH3_H interrupt, %d times\r\n", ++times);
    }
    if (intstatus & PWM_INTSTS_PERIOD) {
        qcc74x_pwm_v2_int_clear(pwm, PWM_INTSTS_PERIOD);
        static int times = 0;
        printf("period interrupt, %d times\r\n", ++times);
    }
    if (intstatus & PWM_INTSTS_BRAKE) {
        qcc74x_pwm_v2_int_clear(pwm, PWM_INTSTS_BRAKE);
        static int times = 0;
        printf("brake interrupt, %d times\r\n", ++times);
    }
    if (intstatus & PWM_INTSTS_REPT) {
        qcc74x_pwm_v2_int_clear(pwm, PWM_INTSTS_REPT);
        static int times = 0;
        printf("rept interrupt, %d times\r\n", ++times);
        qcc74x_pwm_v2_stop(pwm);
    }
}

int main(void)
{
    board_init();
    board_pwm_gpio_init();

    pwm = qcc74x_device_get_by_name("pwm_v2_0");

    /* period = .PBCLK / .clk_div / .period = 80MHz( 64MHZ for qcc74x_undefl) / 80( 64 for qcc74x_undefl) / 1000 = 1kHz */
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
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_CH0, 100, 700);
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_CH1, 200, 300);
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_CH2, 600, 800);
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_CH3, 500, 900);
    qcc74x_pwm_v2_int_enable(pwm, PWM_INTEN_CH0_L | \
                              PWM_INTEN_CH0_H | \
                              PWM_INTEN_CH1_L | \
                              PWM_INTEN_CH1_H | \
                              PWM_INTEN_CH2_L | \
                              PWM_INTEN_CH2_H | \
                              PWM_INTEN_CH3_L | \
                              PWM_INTEN_CH3_H | \
                              PWM_INTEN_PERIOD | \
                              PWM_INTEN_BRAKE | \
                              PWM_INTEN_REPT, true);
    qcc74x_irq_attach(pwm->irq_num, pwm_isr, NULL);
    qcc74x_irq_enable(pwm->irq_num);
    qcc74x_pwm_v2_feature_control(pwm, PWM_CMD_SET_REPT_COUNT, REPT_CNT);
    qcc74x_pwm_v2_feature_control(pwm, PWM_CMD_SET_STOP_ON_REPT, true);
    qcc74x_pwm_v2_start(pwm);

    while (1) {
        qcc74x_mtimer_delay_ms(2000);
    }
}
