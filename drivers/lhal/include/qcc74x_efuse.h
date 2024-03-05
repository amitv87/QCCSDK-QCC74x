#ifndef _QCC74x_EFUSE_H
#define _QCC74x_EFUSE_H

#include "qcc74x_core.h"

/**
 *  @brief Efuse device information type definition
 */
typedef struct
{
    uint8_t package;
    uint8_t flash_info;
#if defined(QCC74x_undef)
    uint8_t ext_info;
    uint8_t mcu_info;
#else
    uint8_t psram_info;
#endif
#if defined(QCC74x_undef) || defined(QCC74x_undefL)
    uint8_t sf_swap_cfg;
#else
    uint8_t version;
#endif
    const char *package_name;
    const char *flash_info_name;
#if defined(QCC74x_undef)
#else
    const char *psram_info_name;
#endif
} qcc74x_efuse_device_info_type;

#ifdef __cplusplus
extern "C" {
#endif

void qcc74x_efuse_get_chipid(uint8_t chipid[8]);

uint8_t qcc74x_efuse_is_mac_address_slot_empty(uint8_t slot, uint8_t reload);
int qcc74x_efuse_write_mac_address_opt(uint8_t slot, uint8_t mac[6], uint8_t program);
int qcc74x_efuse_read_mac_address_opt(uint8_t slot, uint8_t mac[6], uint8_t reload);

float qcc74x_efuse_get_adc_trim(void);
uint32_t qcc74x_efuse_get_adc_tsen_trim(void);

void qcc74x_efuse_get_device_info(qcc74x_efuse_device_info_type *device_info);
void qcc74x_efuse_read_secure_boot(uint8_t *sign, uint8_t *aes);

#ifdef __cplusplus
}
#endif

#endif
