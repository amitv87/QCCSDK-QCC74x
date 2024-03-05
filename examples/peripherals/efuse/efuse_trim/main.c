#include "qcc74x_mtimer.h"
#include "board.h"
#include "qcc74x_ef_ctrl.h"

char *trim_list[] = {
    "rc32m",
    "rc32k",
    "tsen"
};

int main(void)
{
    struct qcc74x_device_s *efuse_dev;
    qcc74x_ef_ctrl_com_trim_t trim;
    int i = 0;

    board_init();
    printf("\r\neFuse read trim value case!\r\n");

    efuse_dev = qcc74x_device_get_by_name("ef_ctrl");
    if (NULL == efuse_dev) {
        printf("efuse device driver not found!\r\n");
        while (1)
            ;
    }

    for (i = 0; i < sizeof(trim_list) / sizeof(trim_list[0]); i++) {
        qcc74x_ef_ctrl_read_common_trim(NULL, trim_list[i], &trim, 1);
        if (trim.en) {
            if (trim.parity == qcc74x_ef_ctrl_get_trim_parity(trim.value, trim.len)) {
                printf("trim %s value=%d!\r\n", trim_list[i], trim.value);
            } else {
                printf("trim %s parity error!\r\n", trim_list[i]);
            }
        } else {
            printf("trim %s not found!\r\n", trim_list[i]);
        }
    }

    printf("case finished!\r\n");
    while (1) {
        qcc74x_mtimer_delay_ms(1000);
    }
}
