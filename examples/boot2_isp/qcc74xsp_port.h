#ifndef __QCC74xSP_PORT_H__
#define __QCC74xSP_PORT_H__

#include "stdint.h"
#include "qcc74x_flash.h"
#include "qcc74x_sec_sha.h"
#include "qcc74x_port_boot2.h"

#define MFG_START_REQUEST_OFFSET HAL_BOOT2_MFG_START_REQUEST_OFFSET
#define QCC74xSP_BOOT2_XIP_BASE      QCC74x_FLASH_XIP_BASE
#define QCC74xSP_BOOT2_ROLLBACK
#define QCC74xSP_BOOT2_SUPPORT_DECOMPRESS          HAL_BOOT2_SUPPORT_DECOMPRESS
#define QCC74xSP_BOOT2_SUPPORT_USB_IAP             HAL_BOOT2_SUPPORT_USB_IAP
#define QCC74xSP_BOOT2_SUPPORT_EFLASH_LOADER_RAM   HAL_BOOT2_SUPPORT_EFLASH_LOADER_RAM
#define QCC74xSP_BOOT2_SUPPORT_EFLASH_LOADER_FLASH HAL_BOOT2_SUPPORT_EFLASH_LOADER_FLASH
#define QCC74xSP_BOOT2_SUPPORT_SIGN_ENCRYPT        HAL_BOOT2_SUPPORT_SIGN_ENCRYPT
#define QCC74xSP_BOOT2_TCM_BASE                    0

#define BOOT2_MODE_RELEASE 0x01
#define BOOT2_MODE_DEBUG   0x02
#ifdef CONFIG_DEBUG
#define QCC74xSP_BOOT2_MODE BOOT2_MODE_DEBUG
#else
#define QCC74xSP_BOOT2_MODE BOOT2_MODE_RELEASE
#endif
#define QCC74xSP_BOOT2_CPU_GROUP_MAX     HAL_BOOT2_CPU_GROUP_MAX
#define QCC74xSP_BOOT2_CPU_MAX           HAL_BOOT2_CPU_MAX
#define QCC74xSP_BOOT2_RAM_IMG_COUNT_MAX HAL_BOOT2_RAM_IMG_COUNT_MAX

#define QCC74x_BOOT2_LOG_OUT_BUF_SIZE       (2 * 1024)

void qcc74x_boot2_release_dump_log(void);

uint32_t qcc74xsp_boot2_get_cpu_count(void);
uint8_t qcc74xsp_read_power_save_mode(void);
void qcc74xsp_boot2_pass_parameter(void *data, uint32_t len);
void qcc74xsp_boot2_releae_other_cpu(void);
void qcc74xsp_boot2_start_timer(void);
void qcc74xsp_boot2_show_timer(void);
void qcc74xsp_boot2_init_sec_eng_pka(void);
uint8_t qcc74xsp_get_anti_rollback_flag(uint32_t *value);
uint8_t qcc74xsp_set_anti_rollback_flag(uint32_t value);
uint8_t qcc74xsp_clean_anti_rollback_flag();

void boot2_wdt_init(void);
void boot2_wdt_set_value(uint16_t wdt_timeout);
void boot2_wdt_feed(void);
void boot2_wdt_disable(void);

void qcc74xsp_boot2_printf(const char *format, ...);

extern struct qcc74x_device_s *boot2_wdg;
extern struct qcc74x_device_s *sha;
extern struct qcc74x_device_s *uartx;
extern struct qcc74x_sha256_ctx_s ctx_sha256;
extern struct qcc74x_device_s *console;
extern void qcc74x_uart_set_console(struct qcc74x_device_s *dev);
extern const qcc74xverinf_t boot2_ver;

#define BOOT2_MSG(a, ...)            qcc74xsp_boot2_printf(a, ##__VA_ARGS__)
#define BOOT2_MSG_WAR(a, ...)        qcc74xsp_boot2_printf(a, ##__VA_ARGS__)
#define BOOT2_MSG_DBG(a, ...)        qcc74xsp_boot2_printf(a, ##__VA_ARGS__)
#define BOOT2_MSG_ERR(a, ...)        qcc74xsp_boot2_printf(a, ##__VA_ARGS__)
#define HBN_STATUS_ANTIROLLBACK_FLAG 0x42524E41

#endif /* __QCC74xSP_PORT_H__ */
