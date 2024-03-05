#include "qcc74x_mtimer.h"
#include "qcc74x_timer.h"
#include "board.h"

#define TEST_TIMER_COMP_ID TIMER_COMP_ID_2

struct qcc74x_device_s *timer0;
struct qcc74x_device_s *timer1;

void timer0_isr(int irq, void *arg)
{
    bool status = qcc74x_timer_get_compint_status(timer0, TIMER_COMP_ID_0);
    if (status) {
        qcc74x_timer_compint_clear(timer0, TIMER_COMP_ID_0);
        printf("timer0 comp0 trigger\r\n");
    }
    status = qcc74x_timer_get_compint_status(timer0, TIMER_COMP_ID_1);
    if (status) {
        qcc74x_timer_compint_clear(timer0, TIMER_COMP_ID_1);
        printf("timer0 comp1 trigger\r\n");
    }
    status = qcc74x_timer_get_compint_status(timer0, TIMER_COMP_ID_2);
    if (status) {
        qcc74x_timer_compint_clear(timer0, TIMER_COMP_ID_2);
        printf("timer0 comp2 trigger\r\n");
    }
}

void timer1_isr(int irq, void *arg)
{
    bool status = qcc74x_timer_get_compint_status(timer1, TIMER_COMP_ID_0);
    if (status) {
        qcc74x_timer_compint_clear(timer1, TIMER_COMP_ID_0);
        printf("timer1 comp0 trigger\r\n");
    }
    status = qcc74x_timer_get_compint_status(timer1, TIMER_COMP_ID_1);
    if (status) {
        qcc74x_timer_compint_clear(timer1, TIMER_COMP_ID_1);
        printf("timer1 comp1 trigger\r\n");
    }
    status = qcc74x_timer_get_compint_status(timer1, TIMER_COMP_ID_2);
    if (status) {
        qcc74x_timer_compint_clear(timer1, TIMER_COMP_ID_2);
        printf("timer1 comp2 trigger\r\n");
    }
}

int main(void)
{
    board_init();
    printf("Timer basic test\n");

    /* timer clk = XCLK/(div + 1 )*/
    struct qcc74x_timer_config_s cfg0;
    cfg0.counter_mode = TIMER_COUNTER_MODE_PROLOAD; /* preload when match occur */
    cfg0.clock_source = TIMER_CLKSRC_XTAL;
    cfg0.clock_div = 39; /* for qcc743/qcc74x_undef/qcc74x_undefp is 39, for qcc74x_undef is 31 */
    cfg0.trigger_comp_id = TEST_TIMER_COMP_ID;
    cfg0.comp0_val = 1000000; /* match value 0  */
    cfg0.comp1_val = 1500000; /* match value 1 */
    cfg0.comp2_val = 2500000; /* match value 2 */
    cfg0.preload_val = 0;    /* preload value */

    struct qcc74x_timer_config_s cfg1;
    cfg1.counter_mode = TIMER_COUNTER_MODE_PROLOAD;
    cfg1.clock_source = TIMER_CLKSRC_XTAL;
    cfg1.clock_div = 39; /* for qcc743/qcc74x_undef/qcc74x_undefp is 39, for qcc74x_undef is 31 */
    cfg1.trigger_comp_id = TEST_TIMER_COMP_ID;
    cfg1.comp0_val = 1000000; /* match value 0  */
    cfg1.comp1_val = 1500000; /* match value 1 */
    cfg1.comp2_val = 2500000; /* match value 2 */
    cfg1.preload_val = 0;    /* preload value */

    timer0 = qcc74x_device_get_by_name("timer0");
    timer1 = qcc74x_device_get_by_name("timer1");

    /* Timer init with default configuration */
    qcc74x_timer_init(timer0, &cfg0);
    qcc74x_timer_init(timer1, &cfg1);

    qcc74x_irq_attach(timer0->irq_num, timer0_isr, NULL);
    qcc74x_irq_attach(timer1->irq_num, timer1_isr, NULL);
    qcc74x_irq_enable(timer0->irq_num);
    qcc74x_irq_enable(timer1->irq_num);

    /* Enable timer */
    qcc74x_timer_start(timer0);
    qcc74x_timer_start(timer1);

    printf("case success.\r\n");
    while (1) {
        qcc74x_mtimer_delay_ms(1500);
    }
}
