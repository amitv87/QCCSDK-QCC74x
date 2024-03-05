#include "qcc74x_lp.h"
#include <qcc743_glb.h>
#include "qcc743_hbn.h"
#include "qcc74x_sys.h"


#define REASON_SOFTWARE   (4) // software        soft
#define REASON_SWWDT      (3) // watchdog reboot wdog
#define REASON_EXCEPTION  (2) // software        soft
#define REASON_HWWDT      (1) // watchdog reboot wdog
#define REASON_POWEROFF   (0x0) // software        soft

#define RST_REASON          HBN_SYS_RESET_REASON      // use 4 Bytes
#define RST_REASON_CHK      HBN_SYS_RESET_REASON_CHK  // use 4 Bytes
#define RST_REASON_CHK_VAL  (0xBF1BA55A)

static QCC74x_RST_REASON_E s_rst_reason = QCC74x_RST_POWER_OFF;

static char *RST_REASON_ARRAY[] = {
    "QCC74x_RST_POWER_OFF",
    "QCC74x_RST_HARDWARE_WATCHDOG",
    "QCC74x_RST_FATAL_EXCEPTION",
    "QCC74x_RST_SOFTWARE_WATCHDOG",
    "QCC74x_RST_SOFTWARE",
    "QCC74x_RST_HBN_WAKEUP",
    "QCC74x_RST_BOD"
};

QCC74x_RST_REASON_E qcc74x_sys_rstinfo_get(void)
{
    return s_rst_reason;
}

char * qcc74x_sys_rstinfo_getstring(void)
{
    return RST_REASON_ARRAY[s_rst_reason];
}

int qcc74x_sys_rstinfo_set(QCC74x_RST_REASON_E val)
{
    RST_REASON = (uint32_t)val;
    RST_REASON_CHK = (uint32_t)val ^ RST_REASON_CHK_VAL;

    return 0;
}

void qcc74x_sys_rstinfo_init(void)
{
  uint32_t reason = RST_REASON;
  uint32_t reason_chk = RST_REASON_CHK;

  uint8_t reset_evt=0;

  HBN_Get_Reset_Event(&reset_evt);

  if( 0x1 & (reset_evt>>4) ) {
    s_rst_reason = QCC74x_RST_BOD;
  } else {
    if ((reason ^ RST_REASON_CHK_VAL) == reason_chk) {
        s_rst_reason = reason;
    }
  }

  HBN_Clr_Reset_Event();

  RST_REASON = REASON_HWWDT;
  RST_REASON_CHK = REASON_HWWDT ^ RST_REASON_CHK_VAL;
}

int qcc74x_sys_rstinfo_getsting(char *info)
{
    memcpy(info, (char *)RST_REASON_ARRAY[s_rst_reason], strlen(RST_REASON_ARRAY[s_rst_reason]));
    *(info + strlen(RST_REASON_ARRAY[s_rst_reason])) = '\0';
    return 0;
}

int qcc74x_sys_isxipaddr(uint32_t addr)
{
    if ( ((addr & 0xFF000000) == 0xA0000000) || ((addr & 0xFF000000) == 0xA1000000) ||
         ((addr & 0xFF000000) == 0xA2000000) || ((addr & 0xFF000000) == 0xA3000000) ||
         ((addr & 0xFF000000) == 0xA4000000) || ((addr & 0xFF000000) == 0xA5000000) ||
         ((addr & 0xFF000000) == 0xA6000000) || ((addr & 0xFF000000) == 0xA7000000) ) {
        return 1;
    }
    return 0;
}

int qcc74x_sys_em_config(void)
{
    extern uint8_t __LD_CONFIG_EM_SEL;
    volatile uint32_t em_size;

    em_size = (uint32_t)&__LD_CONFIG_EM_SEL;

    if (em_size == 0) {
        GLB_Set_EM_Sel(GLB_WRAM160KB_EM0KB);
    } else if (em_size == 32*1024) {
        GLB_Set_EM_Sel(GLB_WRAM128KB_EM32KB);
    } else if (em_size == 64*1024) {
        GLB_Set_EM_Sel(GLB_WRAM96KB_EM64KB);
    } else {
        GLB_Set_EM_Sel(GLB_WRAM96KB_EM64KB);
    }

    return 0;
}

int qcc74x_sys_reset_por(void)
{
    qcc74x_lp_rtc_use_rc32k();
    qcc74x_sys_rstinfo_set(QCC74x_RST_SOFTWARE);
    __disable_irq();

    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_WIFI);
    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_BTDM);
    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_ZIGBEE);
    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_BLE2);
    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_ZIGBEE2);

    arch_delay_ms(10);

    GLB_SW_POR_Reset();
    while (1) {
        /*empty dead loop*/
    }

    return 0;
}

void qcc74x_sys_reset_system(void)
{
    qcc74x_sys_rstinfo_set(QCC74x_RST_SOFTWARE);
    __disable_irq();
    GLB_SW_System_Reset();
    while (1) {
        /*empty dead loop*/
    }
}

