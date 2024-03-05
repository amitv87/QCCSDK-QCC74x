#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "qcc74xsp_port.h"
#include "qcc74xsp_bootinfo.h"
#include "qcc74xsp_common.h"
#include "qcc74xsp_media_boot.h"
#include "qcc74x_flash.h"
#include "qcc74x_eflash_loader.h"
#include "qcc74x_uart.h"

extern uint32_t edata_address;
extern uint32_t edata_length;

ATTR_NOCACHE_NOINIT_RAM_SECTION uint8_t g_malloc_buf[QCC74x_BOOT2_XZ_MALLOC_BUF_SIZE];

int32_t qcc74xsp_boot2_set_encrypt(uint8_t index, boot2_image_config *g_boot_img_cfg);

/****************************************************************************/ /**
 * @brief  Dump data
 *
 * @param  datain: Data pointer to dump
 * @param  len: Data length to dump
 *
 * @return None
 *
*******************************************************************************/
void qcc74xsp_dump_data(void *datain, int len)
{
    int i = 0;
    uint8_t *data = (uint8_t *)datain;
    data = data;

    for (i = 0; i < len; i++) {
        if (i % 16 == 0 && i != 0) {
            BOOT2_MSG("\r\n");
        }

        BOOT2_MSG("%x ", data[i]);
    }

    BOOT2_MSG("\r\n");
}

/****************************************************************************/ /**
 * @brief  Media boot pre-jump
 *
 * @param  None
 *
 * @return QCC74x_Err_Type
 *
*******************************************************************************/
int32_t qcc74xsp_mediaboot_pre_jump(void)
{
    extern void system_mtimer_clock_reinit(void);

    /* reinit mtimer clock */
    //system_mtimer_clock_reinit();
    qcc74x_uart_deinit(uartx);
    /* deinit uart */
    hal_boot2_debug_uart_gpio_deinit();

    /* Sec eng deinit*/
    hal_boot2_reset_sec_eng();

    /* Platform deinit */
    //qcc74x_platform_deinit();

    /* Jump to entry */
    __disable_irq();
    boot2_wdt_feed();
    boot2_wdt_disable();
    qcc74xsp_boot2_jump_entry();
    boot2_wdt_init();
    return QCC74x_BOOT2_SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Boot2 exit with error and will stay forever
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void qcc74xsp_boot2_exit(void)
{
    hal_boot2_sboot_finish();

    /* Release other CPUs*/
    qcc74xsp_boot2_releae_other_cpu();

    /* Stay here */
    while (1) {
        /* Use soft delay only */
        arch_delay_ms(100);
    }
}

/****************************************************************************/ /**
 * @brief  customer board init post
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
__WEAK void ATTR_TCM_SECTION qcc74xsp_boot2_customer_init_on_pt_found_post(void)
{

}

/****************************************************************************/ /**
 * @brief  Boot2 jump to entry_point
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void ATTR_TCM_SECTION qcc74xsp_boot2_jump_entry(void)
{
    pentry_t pentry;
    int32_t ret;

    hal_boot2_clean_cache();
    hal_boot2_sboot_finish();

    /*Note:enable cache with flash offset, after this, should be no flash directl read,
      If need read, should take flash offset into consideration */
    if (0 != g_efuse_cfg.encrypted[0]) {
        /*for encrypted img, use none-continuos read*/
        ret = hal_boot2_set_cache(0, &g_boot_img_cfg[0]);
    } else {
        /*for unencrypted img, use continuos read*/
        ret = hal_boot2_set_cache(1, &g_boot_img_cfg[0]);
    }

    if (ret != QCC74x_BOOT2_SUCCESS) {
        return;
    }

#if QCC74xSP_BOOT2_SUPPORT_SIGN_ENCRYPT
    /* Set decryption before read MSP and PC*/
    if (0 != g_efuse_cfg.encrypted[0]) {
        qcc74xsp_boot2_set_encrypt(0, &g_boot_img_cfg[0]);
        qcc74xsp_boot2_set_encrypt(1, &g_boot_img_cfg[1]);
#if QCC74xSP_BOOT2_CPU_MAX > 1
        if (hal_boot2_get_feature_flag() == HAL_BOOT2_CP_FLAG) {
            /*co-processor*/
            g_boot_img_cfg[0].cpu_cfg[1].msp_val = g_boot_img_cfg[0].cpu_cfg[0].msp_val;
            g_boot_img_cfg[0].cpu_cfg[1].boot_entry = g_boot_img_cfg[0].cpu_cfg[0].boot_entry;
            g_boot_img_cfg[0].cpu_cfg[1].cache_enable = g_boot_img_cfg[0].cpu_cfg[0].cache_enable;
            g_boot_img_cfg[0].img_valid = 1;
            g_boot_img_cfg[0].cpu_cfg[1].cache_way_dis = 0xf;
        }
#endif
    }
#endif

    for (uint32_t i = 0; i < 3; i++) {
        volatile uint32_t *p = (volatile uint32_t *)(QCC74x_FLASH_XIP_BASE + QCC74xSP_APP_VERSION_LINK_OFFSET);
        g_anti_rollback_flag[i] = p[i];
    }
    /* check app version */
    if (g_anti_ef_en == 1) {
        if ((g_anti_rollback_flag[0] != QCC74xSP_APP_QCC74x_FLAG_PRE) ||
            (g_anti_rollback_flag[1] != QCC74xSP_APP_VERF_FLAG_PRE)) {
            qcc74xsp_set_anti_rollback_flag((1 << 24) | (g_anti_rollback_flag[2] & 0xff));
            GLB_SW_POR_Reset();
        }
        if (((g_anti_rollback_flag[2] & 0xff) < g_anti_ef_app_ver) ||
            ((g_anti_rollback_flag[2] & 0xff) > QCC74xSP_APP_VERSION_MAX)) {
            qcc74xsp_set_anti_rollback_flag(g_anti_rollback_flag[2] & 0xff);

            GLB_SW_POR_Reset();
        }
    }

    qcc74xsp_boot2_customer_init_on_pt_found_post();
    qcc74xsp_boot2_releae_other_cpu();

    /* Deal CPU0's entry point */
    for (uint32_t group = 0; group < QCC74xSP_BOOT2_CPU_GROUP_MAX; group++) {
        if (g_boot_img_cfg[group].img_valid && g_boot_img_cfg[group].cpu_cfg[0].config_enable) {
            if (g_boot_img_cfg[group].cpu_cfg[0].halt_cpu == 0) {
                pentry = (pentry_t)g_boot_img_cfg[group].cpu_cfg[0].boot_entry;
                if (pentry) {
                    pentry();
                }
            }
        }
    }

    /* If cann't jump stay here */
    while (1) {
        /*use soft delay only */
        arch_delay_ms(100);
    }
}
