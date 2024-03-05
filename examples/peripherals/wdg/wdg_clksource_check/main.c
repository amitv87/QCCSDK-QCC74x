#include "qcc74x_mtimer.h"
#include "qcc74x_wdg.h"
#include "board.h"

#if !defined(QCC74x_undefL)
uint8_t wdg_clk_src_type[] = {
    WDG_CLKSRC_BCLK,
    WDG_CLKSRC_32K,
    WDG_CLKSRC_1K,
    WDG_CLKSRC_XTAL,
};
#else
uint8_t wdg_clk_src_type[] = {
    WDG_CLKSRC_32K,
    WDG_CLKSRC_1K,
    WDG_CLKSRC_XTAL,
};
#endif

int main(void)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t cnt = 0;

    board_init();
    printf("Timer clksource check\n");

    struct qcc74x_device_s *wdt;
    wdt = qcc74x_device_get_by_name("watchdog");

    /* watchdog clk = clock_source/(div + 1)*/
    struct qcc74x_wdg_config_s cfg;
    cfg.clock_source = WDG_CLKSRC_32K;
    cfg.clock_div = 0;
    cfg.comp_val = 0xffff;
    cfg.mode = WDG_MODE_INTERRUPT;

    for (i = 0; i < sizeof(wdg_clk_src_type) / sizeof(wdg_clk_src_type[0]); i++) {
        cnt = 0;

        if (wdg_clk_src_type[i] == WDG_CLKSRC_XTAL) {
            printf("Watchdog Src Clk is XTAL\r\n");
            cfg.clock_div = 199;
        } else if (wdg_clk_src_type[i] == WDG_CLKSRC_32K) {
            printf("Watchdog Src Clk is 32K\r\n");
            cfg.clock_div = 1;
        } else if (wdg_clk_src_type[i] == WDG_CLKSRC_1K) {
            printf("Watchdog Src Clk is 1K\r\n");
            cfg.clock_div = 1;
        }
#if !defined(QCC74x_undefL)
        else if (wdg_clk_src_type[i] == WDG_CLKSRC_BCLK) {
            printf("Watchdog Src Clk is BCLK\r\n");
            cfg.clock_div = 249;
        }
#endif
        else {
            printf("Other clock, not test.\r\n");
            continue;
        }

        cfg.clock_source = wdg_clk_src_type[i];
        qcc74x_wdg_init(wdt, &cfg);

        for (j = 0; j < 10; j++) {
            qcc74x_wdg_reset_countervalue(wdt);
            qcc74x_wdg_start(wdt);
            qcc74x_mtimer_delay_ms(200);
            cnt = qcc74x_wdg_get_countervalue(wdt);
            qcc74x_wdg_stop(wdt);
            qcc74x_mtimer_delay_ms(10);
            printf("Delay 200ms, div = %lu, test %lu times, cnt: %lu\r\n", cfg.clock_div + 1, j, cnt);
        }
    }
    printf("case success.\r\n");
    while (1) {
        qcc74x_mtimer_delay_ms(1500);
    }
}
