#include "qcc74x_mtimer.h"
#include "qcc74x_wdg.h"
#include "board.h"

#define TEST_ADDR (0x80000000)

struct qcc74x_device_s *wdg;

int main(void)
{
    board_init();
    printf("Watchdog reset test\r\n");

    struct qcc74x_wdg_config_s wdg_cfg;
    wdg_cfg.clock_source = WDG_CLKSRC_32K;
    wdg_cfg.clock_div = 31;
    wdg_cfg.comp_val = 2000;
    wdg_cfg.mode = WDG_MODE_RESET;

    wdg = qcc74x_device_get_by_name("watchdog");
    qcc74x_wdg_init(wdg, &wdg_cfg);

    qcc74x_wdg_start(wdg);

    /* delay 1s and wdg reset should not trigger. */
    qcc74x_mtimer_delay_ms(1000);
    qcc74x_wdg_reset_countervalue(wdg);
    printf("Delay 1s, triggle set 2s, wdg should not reset, pass.\r\n");

    printf("Exception test addr: 0x%08x\r\n", TEST_ADDR);
    getreg32(TEST_ADDR);

    printf("Error! Can't run to here, delay 2s, wdg should reset, current count = %d\r\n",
           qcc74x_wdg_get_countervalue(wdg));

    while (1) {
        qcc74x_mtimer_delay_ms(1500);
    }
}