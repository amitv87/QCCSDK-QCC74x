#include "usbd_core.h"
#include "qcc74x_mtimer.h"
#include "board.h"

extern void cdc_acm_init(void);
extern void cdc_acm_data_send_with_dtr_test(void);

int main(void)
{
    board_init();

    cdc_acm_init();
    while (1) {
        cdc_acm_data_send_with_dtr_test();
        qcc74x_mtimer_delay_ms(500);
    }
}
