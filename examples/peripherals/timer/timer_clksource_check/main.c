#include "qcc74x_mtimer.h"
#include "qcc74x_timer.h"
#include "board.h"

#if !defined(QCC74x_undefL)
uint8_t timer_clk_src_type[] = {
    TIMER_CLKSRC_BCLK,
    TIMER_CLKSRC_32K,
    TIMER_CLKSRC_1K,
    TIMER_CLKSRC_XTAL,
};
#else
uint8_t timer_clk_src_type[] = {
    TIMER_CLKSRC_32K,
    TIMER_CLKSRC_1K,
    TIMER_CLKSRC_XTAL,
};
#endif

int main(void)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t cnt = 0;

    board_init();
    printf("Timer clksource check\n");

    struct qcc74x_device_s *timer0;
    timer0 = qcc74x_device_get_by_name("timer0");

    /* timer clk = clock_source/(div + 1)*/
    struct qcc74x_timer_config_s cfg;
    cfg.counter_mode = TIMER_COUNTER_MODE_PROLOAD;
    cfg.clock_source = TIMER_CLKSRC_NO;
    cfg.clock_div = 9;
    cfg.trigger_comp_id = TIMER_COMP_ID_0;
    cfg.comp0_val = 0xFFFFFFFF;
    cfg.comp1_val = 0xFFFFFFFF;
    cfg.comp2_val = 0xFFFFFFFF;
    cfg.preload_val = 0;

    for (i = 0; i < sizeof(timer_clk_src_type) / sizeof(timer_clk_src_type[0]); i++) {
        cnt = 0;

        if (timer_clk_src_type[i] == TIMER_CLKSRC_XTAL) {
            printf("Timer Src Clk is XTAL\r\n");
        } else if (timer_clk_src_type[i] == TIMER_CLKSRC_32K) {
            printf("Timer Src Clk is 32K\r\n");
        } else if (timer_clk_src_type[i] == TIMER_CLKSRC_1K) {
            printf("Timer Src Clk is 1K\r\n");
        }
#if !defined(QCC74x_undefL)
        else if (timer_clk_src_type[i] == TIMER_CLKSRC_BCLK) {
            printf("Timer Src Clk is BCLK\r\n");
        }
#endif
        else {
            printf("Other clock, not test.\r\n");
            continue;
        }

        cfg.clock_source = timer_clk_src_type[i];
        qcc74x_timer_init(timer0, &cfg);

        for (j = 0; j < 10; j++) {
            qcc74x_timer_start(timer0);
            qcc74x_mtimer_delay_ms(1000);
            cnt = qcc74x_timer_get_countervalue(timer0);
            qcc74x_timer_stop(timer0);
            qcc74x_mtimer_delay_ms(10);
            printf("delay 1000ms, Test %lu times, cnt: %lu\r\n", j, cnt);
        }
        qcc74x_timer_deinit(timer0);
    }
    printf("case success.\r\n");
    while (1) {
        qcc74x_mtimer_delay_ms(1500);
    }
}
