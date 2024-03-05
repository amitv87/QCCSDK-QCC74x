#include "qcc74x_mtimer.h"
#include "qcc74x_pwm_v2.h"
#include "qcc74x_clock.h"
#include "board.h"
#include "math.h"
#include "qcc743_glb.h"
#include "qcc743_glb_gpio.h"
#include "qcc743_gpio.h"

#define TEST_EN                     (1)

#define PWM_PERIOD_WIDTH            (12)
#define PWM_PERIOD                  (1 << PWM_PERIOD_WIDTH)
#define PWM_TRI_CH_U                (PWM_CH0)
#define PWM_TRI_CH_V                (PWM_CH1)
#define PWM_TRI_CH_W                (PWM_CH2)
#define PWM_PIN_U_H                 (GPIO_PIN_24)
#define PWM_PIN_U_L                 (GPIO_PIN_25)
#define PWM_PIN_V_H                 (GPIO_PIN_26)
#define PWM_PIN_V_L                 (GPIO_PIN_27)
#define PWM_PIN_W_H                 (GPIO_PIN_28)
#define PWM_PIN_W_L                 (GPIO_PIN_29)

#define PWM_MATH_ROUND_DEGREE_WIDTH (16)
#define PWM_MATH_SIN_TAB_CNT_WIDTH  (6)
#define PWM_MATH_SIN_TAB_PER_WIDTH  (PWM_MATH_ROUND_DEGREE_WIDTH - 2 - PWM_MATH_SIN_TAB_CNT_WIDTH)
#define PWM_MATH_ROUND_DEGREE       (1 << PWM_MATH_ROUND_DEGREE_WIDTH)
#define PWM_MATH_SIN_TAB_CNT        (1 << PWM_MATH_SIN_TAB_CNT_WIDTH) /* table only 1/4 round degree */
#define PWM_MATH_SIN_TAB_PER        (1 << PWM_MATH_SIN_TAB_PER_WIDTH)

uint16_t sinArr[PWM_MATH_SIN_TAB_CNT + 1];
uint16_t angle_u, angle_v, angle_w;

struct qcc74x_device_s *pwm;
struct qcc74x_device_s *gpio;

void global_data_init(void)
{
    float val;

    for (uint32_t i = 0; i <= PWM_MATH_SIN_TAB_CNT; i++) {
        val = (float)i / PWM_MATH_SIN_TAB_CNT / 4.0f * 2.0f * M_PI;
        val = sinf(val);
        val = val * PWM_PERIOD / 2.0f + PWM_PERIOD / 2;
        val += 0.5f;
        sinArr[i] = (uint16_t)val;
    }

    angle_u = 0;
    angle_v = PWM_MATH_ROUND_DEGREE / 3;
    angle_w = PWM_MATH_ROUND_DEGREE / 3 * 2;
}

void pwm_calc_threshold(uint16_t *threL, uint16_t *threH, uint16_t angle)
{
    uint16_t quadrant, area, idx, val;

    quadrant = angle >> (PWM_MATH_ROUND_DEGREE_WIDTH - 2);
    angle = angle & ((1 << (PWM_MATH_SIN_TAB_CNT_WIDTH + PWM_MATH_SIN_TAB_PER_WIDTH)) - 1);
    if (quadrant == 1 || quadrant == 3) {
        angle = (1 << (PWM_MATH_SIN_TAB_CNT_WIDTH + PWM_MATH_SIN_TAB_PER_WIDTH)) - angle;
    }
    area = angle >> PWM_MATH_SIN_TAB_PER_WIDTH;
    idx = angle & (PWM_MATH_SIN_TAB_PER - 1);
    val = (((sinArr[area + 1] - sinArr[area]) * idx) >> PWM_MATH_SIN_TAB_PER_WIDTH) + sinArr[area];
    if (quadrant == 2 || quadrant == 3) {
        val = PWM_PERIOD - val;
    }
    if (val > PWM_PERIOD - 600) {
        val = PWM_PERIOD - 600;
    } else if (val < 600) {
        val = 600;
    }
    *threL = (PWM_PERIOD >> 1) - (val >> 1);
    *threH = (PWM_PERIOD >> 1) + (val >> 1);
}

void pwm_isr(int irq, void *arg)
{
#if TEST_EN
    *(volatile uint32_t *)(GLB_BASE + GLB_GPIO_CFG17_OFFSET) = 0x00400B6A | (1 << 24);
#endif
#if TEST_EN
    static int flag = 0;
    if (flag & 1) {
        *(volatile uint32_t *)(GLB_BASE + GLB_GPIO_CFG16_OFFSET) = 0x00400B6A;
    } else {
        *(volatile uint32_t *)(GLB_BASE + GLB_GPIO_CFG16_OFFSET) = 0x00400B6A | (1 << 24);
    }
    flag ^= 1;

#endif
    uint16_t threL, threH;

    qcc74x_pwm_v2_int_clear(pwm, PWM_INTSTS_PERIOD);
    // angle_u++;
    // angle_v++;
    // angle_w++;
    angle_u += 150;
    angle_v += 150;
    angle_w += 150;
    pwm_calc_threshold(&threL, &threH, angle_u);
    // printf("u:%d,%d,%d\n", angle_u, threL, threH);
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_TRI_CH_U, threL, threH);
    pwm_calc_threshold(&threL, &threH, angle_v);
    // printf("v:%d,%d,%d\n", angle_v, threL, threH);
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_TRI_CH_V, threL, threH);
    pwm_calc_threshold(&threL, &threH, angle_w);
    // printf("w:%d,%d,%d\n\n", angle_w, threL, threH);
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_TRI_CH_W, threL, threH);
#if TEST_EN
    *(volatile uint32_t *)(GLB_BASE + GLB_GPIO_CFG17_OFFSET) = 0x00400B6A;
#endif
}

void peri_pwm_init(void)
{
    struct qcc74x_pwm_v2_config_s cfg = {
        .clk_source = QCC74x_SYSTEM_PBCLK,
        .clk_div = 1,
        .period = PWM_PERIOD,
    };
    struct qcc74x_pwm_v2_channel_config_s ch_cfg = {
        .positive_polarity = PWM_POLARITY_ACTIVE_HIGH,
        .negative_polarity = PWM_POLARITY_ACTIVE_HIGH,
        .positive_stop_state = PWM_STATE_INACTIVE,
        .negative_stop_state = PWM_STATE_ACTIVE,
        .positive_brake_state = PWM_STATE_INACTIVE,
        .negative_brake_state = PWM_STATE_ACTIVE,
        .dead_time = 0xB8, //240,
    };

    pwm = qcc74x_device_get_by_name("pwm_v2_0");
    gpio = qcc74x_device_get_by_name("gpio");

    /* config pin */
    qcc74x_gpio_init(gpio, PWM_PIN_U_H, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, PWM_PIN_U_L, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, PWM_PIN_V_H, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, PWM_PIN_V_L, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, PWM_PIN_W_H, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, PWM_PIN_W_L, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    GLB_Set_PWM1_IO_Sel(GLB_PWM1_IO_DIFF_END);
    qcc74x_pwm_v2_init(pwm, &cfg);
    qcc74x_pwm_v2_channel_init(pwm, PWM_TRI_CH_U, &ch_cfg);
    qcc74x_pwm_v2_channel_init(pwm, PWM_TRI_CH_V, &ch_cfg);
    qcc74x_pwm_v2_channel_init(pwm, PWM_TRI_CH_W, &ch_cfg);
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_TRI_CH_U, 0, 0);
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_TRI_CH_V, 0, 0);
    qcc74x_pwm_v2_channel_set_threshold(pwm, PWM_TRI_CH_W, 0, 0);
    qcc74x_pwm_v2_channel_positive_start(pwm, PWM_TRI_CH_U);
    qcc74x_pwm_v2_channel_negative_start(pwm, PWM_TRI_CH_U);
    qcc74x_pwm_v2_channel_positive_start(pwm, PWM_TRI_CH_V);
    qcc74x_pwm_v2_channel_negative_start(pwm, PWM_TRI_CH_V);
    qcc74x_pwm_v2_channel_positive_start(pwm, PWM_TRI_CH_W);
    qcc74x_pwm_v2_channel_negative_start(pwm, PWM_TRI_CH_W);
    /* config interrupt */
    qcc74x_pwm_v2_int_enable(pwm, PWM_INTEN_PERIOD, true);
    qcc74x_irq_attach(pwm->irq_num, pwm_isr, NULL);
    // qcc74x_irq_enable(pwm->irq_num);
    /* start pwm */
    qcc74x_pwm_v2_feature_control(pwm, PWM_CMD_SET_SW_BRAKE_ENABLE, true);
    qcc74x_pwm_v2_start(pwm);
    /* delay for precharg */
    arch_delay_ms(1000);
    qcc74x_pwm_v2_feature_control(pwm, PWM_CMD_SET_SW_BRAKE_ENABLE, false);
    qcc74x_irq_enable(pwm->irq_num);
}

int main(void)
{
    board_init();
    global_data_init();
    for (uint32_t i = 0; i <= PWM_MATH_SIN_TAB_CNT; i++) {
        if (i % 16 == 0) {
            printf("\n");
        }
        printf("%4d ", sinArr[i]);
    }
    peri_pwm_init();
}
