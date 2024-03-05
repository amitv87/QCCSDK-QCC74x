#include "qcc74x_mtimer.h"
#include "board.h"
#include "log.h"
#include "qcc743_glb.h"
#include "qcc743_pds.h"
#include "qcc743_hbn.h"
#include "qcc743_aon.h"
#include "qcc743_pm.h"

int main(void)
{
    uint32_t tmpVal = 0;
    board_init();

    /* rf808_usb20_psw_rref output off */
    tmpVal = QCC74x_RD_REG(PDS_BASE, PDS_USB_PHY_CTRL);
    tmpVal = QCC74x_CLR_REG_BIT(tmpVal, PDS_REG_PU_USB20_PSW);
    QCC74x_WR_REG(PDS_BASE, PDS_USB_PHY_CTRL, tmpVal);

    pm_pds_mask_all_wakeup_src();
    AON_Output_Float_LDO15_RF();
    HBN_Pin_WakeUp_Mask(0xF);

    printf("enter pds mode\r\n");
    qcc74x_mtimer_delay_ms(100);

    /* Wake up every 1 seconds by pds15 */
    pm_pds_mode_enter(PM_PDS_LEVEL_15, 32768 * 1);

    while (1) {
    }
}
