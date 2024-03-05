
#include "board.h"
#include "qcc74x_gpio.h"
#include "qcc74x_l1c.h"
#include "qcc74x_mtimer.h"

#include "lv_conf.h"
#include "lvgl.h"

#include "lv_port_disp.h"
#include "lv_port_indev.h"

#include "demos/lv_demos.h"

#include "lcd.h"

/* lvgl log cb */
void lv_log_print_g_cb(const char *buf)
{
    printf("[LVGL] %s", buf);
}

int main(void)
{
    board_init();

    printf("lvgl case\r\n");

    /* lvgl init */
    lv_log_register_print_cb(lv_log_print_g_cb);
    lv_init();
    lv_port_disp_init();
    // lv_port_indev_init();

    lv_demo_benchmark();
    // lv_demo_stress();

    lv_task_handler();

    printf("lvgl success\r\n");

    while (1) {
        lv_task_handler();
        qcc74x_mtimer_delay_ms(1);
    }
}
