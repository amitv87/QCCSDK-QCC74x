#include "qcc74x_mtimer.h"
#include "board.h"
#include "log.h"
#include "qcc743_glb.h"
#include "qcc743_glb_gpio.h"
#include "qcc743_pds.h"
#include "qcc743_hbn.h"
#include "qcc743_aon.h"
#include "qcc743_pm.h"
#include "qcc743_lp.h"

qcc74x_lp_hbn_fw_cfg_t qcc74x_lp_hbn_fw_cfg = {
    .hbn_sleep_cnt = 0,
    .hbn_level = PM_HBN_LEVEL_0,
};

int main(void)
{
    board_init();

    static qcc74x_lp_io_cfg_t lp_wake_io_cfg = {
        /* input enable, use @ref QCC74x_LP_IO_INPUT_EN */
        .io_0_15_ie = QCC74x_LP_IO_INPUT_DISABLE,
        .io_16_ie = QCC74x_LP_IO_INPUT_ENABLE,
        .io_17_ie = QCC74x_LP_IO_INPUT_ENABLE,
        .io_18_ie = QCC74x_LP_IO_INPUT_DISABLE,
        .io_19_ie = QCC74x_LP_IO_INPUT_DISABLE,
        .io_20_34_ie = QCC74x_LP_IO_INPUT_DISABLE,
        /* trigger mode */
        .io_0_7_pds_trig_mode = QCC74x_LP_PDS_IO_TRIG_SYNC_FALLING_EDGE,          /* use @ref QCC74x_LP_PDS_IO_TRIG */
        .io_8_15_pds_trig_mode = QCC74x_LP_PDS_IO_TRIG_SYNC_FALLING_EDGE,         /* use @ref QCC74x_LP_PDS_IO_TRIG */
        .io_16_19_aon_trig_mode = QCC74x_LP_AON_IO_TRIG_SYNC_RISING_FALLING_EDGE, /* aon io, use @ref QCC74x_LP_AON_IO_TRIG, full mode support */
        .io_20_27_pds_trig_mode = QCC74x_LP_PDS_IO_TRIG_SYNC_FALLING_EDGE,        /* use @ref QCC74x_LP_PDS_IO_TRIG */
        .io_28_34_pds_trig_mode = QCC74x_LP_PDS_IO_TRIG_SYNC_FALLING_EDGE,        /* use @ref QCC74x_LP_PDS_IO_TRIG */
        /* resistors */
        .io_0_15_res = QCC74x_LP_IO_RES_NONE,
        .io_16_res = QCC74x_LP_IO_RES_PULL_UP,
        .io_17_res = QCC74x_LP_IO_RES_PULL_UP,
        .io_18_res = QCC74x_LP_IO_RES_NONE,
        .io_19_res = QCC74x_LP_IO_RES_NONE,
        .io_20_34_res = QCC74x_LP_IO_RES_NONE,
        /* wake up unmask */
        .io_wakeup_unmask = 0,
    };

    /* wake up unmask */
    lp_wake_io_cfg.io_wakeup_unmask |= ((uint64_t)1 << 16); /* gpio 16 */
    lp_wake_io_cfg.io_wakeup_unmask |= ((uint64_t)1 << 17); /* gpio 17 */

    qcc74x_lp_io_wakeup_cfg(&lp_wake_io_cfg);

    /* disable feed external watchdog during boot2 */
    qcc74x_lp_hbn_init(0, 0, 0, 0);

    printf("enter hbn mode\r\n");
    qcc74x_mtimer_delay_ms(100);

    qcc74x_lp_hbn_enter(&qcc74x_lp_hbn_fw_cfg);

    while (1) {
    }
}
