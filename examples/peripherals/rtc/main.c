#include "qcc74x_mtimer.h"
#include "qcc74x_rtc.h"
#include "board.h"

struct qcc74x_device_s *rtc;
struct qcc74x_tm g_time;

int main(void)
{
    board_init();

    rtc = qcc74x_device_get_by_name("rtc");

    qcc74x_rtc_set_time(rtc, 0);

    /* 2023-11-27, 10:2:1, Monday */
    g_time.tm_sec = 1;
    g_time.tm_min = 2;
    g_time.tm_hour = 10;
    g_time.tm_wday = 1;
    g_time.tm_mday = 27;
    g_time.tm_mon = 10;
    g_time.tm_year = 2023 - 1900;
    qcc74x_rtc_set_utc_time(&g_time);
    while (1) {
        printf("time:%lld\r\n", QCC74x_RTC_TIME2SEC(qcc74x_rtc_get_time(rtc)));
        qcc74x_rtc_get_utc_time(&g_time);
        printf("utc time:%u-%u-%u, %u:%u:%u, wday:%u\r\n",
               g_time.tm_year + 1900, g_time.tm_mon + 1, g_time.tm_mday,
               g_time.tm_hour, g_time.tm_min, g_time.tm_sec,
               g_time.tm_wday);
        qcc74x_mtimer_delay_ms(5000);
    }
}
