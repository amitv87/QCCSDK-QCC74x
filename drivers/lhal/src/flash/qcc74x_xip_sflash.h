#ifndef _QCC74x_XIP_SFLASH_H
#define _QCC74x_XIP_SFLASH_H

#include "qcc74x_sflash.h"

/** @addtogroup  XIP_SFLASH
 *  @{
 */

/** @defgroup  XIP_SFLASH_Public_Types
 *  @{
 */

/*@} end of group XIP_SFLASH_Public_Types */

/** @defgroup  XIP_SFLASH_Public_Constants
 *  @{
 */

/*@} end of group XIP_SFLASH_Public_Constants */

/** @defgroup  XIP_SFLASH_Public_Macros
 *  @{
 */

/*@} end of group XIP_SFLASH_Public_Macros */

/** @defgroup  XIP_SFLASH_Public_Functions
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

int qcc74x_xip_sflash_state_save(spi_flash_cfg_type *p_flash_cfg, uint32_t *offset,
                               uint8_t group, uint8_t bank);
int qcc74x_xip_sflash_state_restore(spi_flash_cfg_type *p_flash_cfg, uint32_t offset,
                                  uint8_t group, uint8_t bank);
int qcc74x_xip_sflash_erase_need_lock(spi_flash_cfg_type *p_flash_cfg, uint32_t start_addr,
                                    int len, uint8_t group, uint8_t bank);
int qcc74x_xip_sflash_write_need_lock(spi_flash_cfg_type *p_flash_cfg, uint32_t addr,
                                    uint8_t *data, uint32_t len, uint8_t group, uint8_t bank);
int qcc74x_xip_sflash_read_need_lock(spi_flash_cfg_type *p_flash_cfg, uint32_t addr,
                                   uint8_t *data, uint32_t len, uint8_t group, uint8_t bank);
int qcc74x_xip_sflash_get_jedecid_need_lock(spi_flash_cfg_type *p_flash_cfg, uint8_t *data,
                                          uint8_t group, uint8_t bank);
int qcc74x_xip_sflash_get_deviceid_need_lock(spi_flash_cfg_type *p_flash_cfg, uint8_t is_32bits_addr,
                                           uint8_t *data, uint8_t group, uint8_t bank);
int qcc74x_xip_sflash_get_uniqueid_need_lock(spi_flash_cfg_type *p_flash_cfg, uint8_t *data,
                                           uint8_t idlen, uint8_t group, uint8_t bank);
int qcc74x_xip_sflash_clear_status_register_need_lock(spi_flash_cfg_type *p_Flash_Cfg,
                                                    uint8_t group, uint8_t bank);
int qcc74x_xip_sflash_read_via_cache_need_lock(uint32_t addr, uint8_t *data, uint32_t len,
                                             uint8_t group, uint8_t bank);
void qcc74x_xip_sflash_opt_enter(uint8_t *aes_enable);
void qcc74x_xip_sflash_opt_exit(uint8_t aes_enable);

#ifdef __cplusplus
}
#endif

/*@} end of group XIP_SFLASH_Public_Functions */

/*@} end of group XIP_SFLASH */

#endif /* _QCC74x_XIP_SFLASH_H */
