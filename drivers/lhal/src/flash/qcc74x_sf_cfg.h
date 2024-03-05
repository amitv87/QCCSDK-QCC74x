#ifndef _QCC74x_SF_CFG_H
#define _QCC74x_SF_CFG_H

#include "qcc74x_gpio.h"
#include "qcc74x_sflash.h"

/** @addtogroup  SF_CFG
 *  @{
 */

/** @defgroup  SF_CFG_Public_Types
 *  @{
 */

/*@} end of group SF_CFG_Public_Types */

/** @defgroup  SF_CFG_Public_Constants
 *  @{
 */

/*@} end of group SF_CFG_Public_Constants */

/** @defgroup  SF_CFG_Public_Macros
 *  @{
 */
#define QCC74x_GPIO_FUNC_SF 2
#if defined(QCC74x_undef) || defined(QCC743)
/* Flash option sf2 */
/* Flash CLK */
#define QCC74x_EXTFLASH_CLK0_GPIO   GPIO_PIN_8
/* FLASH CS */
#define QCC74x_EXTFLASH_CS0_GPIO    GPIO_PIN_4
/* FLASH DATA */
#define QCC74x_EXTFLASH_DATA00_GPIO GPIO_PIN_7
#define QCC74x_EXTFLASH_DATA10_GPIO GPIO_PIN_5
#define QCC74x_EXTFLASH_DATA20_GPIO GPIO_PIN_6
#define QCC74x_EXTFLASH_DATA30_GPIO GPIO_PIN_9
/* Flash option sf3 */
/* Flash CLK */
#define QCC74x_EXTFLASH_CLK1_GPIO   GPIO_PIN_15
/* FLASH CS */
#define QCC74x_EXTFLASH_CS1_GPIO    GPIO_PIN_20
/* FLASH DATA */
#define QCC74x_EXTFLASH_DATA01_GPIO GPIO_PIN_16
#define QCC74x_EXTFLASH_DATA11_GPIO GPIO_PIN_19
#define QCC74x_EXTFLASH_DATA21_GPIO GPIO_PIN_18
#define QCC74x_EXTFLASH_DATA31_GPIO GPIO_PIN_14
#elif defined(QCC74x_undef) || defined(QCC74x_undefP)
/* Flash option sf2 */
/* Flash CLK */
#define QCC74x_EXTFLASH_CLK0_GPIO   GPIO_PIN_34
/* FLASH CS */
#define QCC74x_EXTFLASH_CS0_GPIO    GPIO_PIN_35
/* FLASH DATA */
#define QCC74x_EXTFLASH_DATA00_GPIO GPIO_PIN_36
#define QCC74x_EXTFLASH_DATA10_GPIO GPIO_PIN_37
#define QCC74x_EXTFLASH_DATA20_GPIO GPIO_PIN_38
#define QCC74x_EXTFLASH_DATA30_GPIO GPIO_PIN_39
#elif defined(QCC74x_undefL)
/* Flash option */
/* Flash CLK */
#define QCC74x_EXTFLASH_CLK0_GPIO   GPIO_PIN_27
/* FLASH CS */
#define QCC74x_EXTFLASH_CS0_GPIO    GPIO_PIN_26
/* FLASH DATA */
#define QCC74x_EXTFLASH_DATA00_GPIO GPIO_PIN_23
#define QCC74x_EXTFLASH_DATA10_GPIO GPIO_PIN_25
#define QCC74x_EXTFLASH_DATA20_GPIO GPIO_PIN_24
#define QCC74x_EXTFLASH_DATA30_GPIO GPIO_PIN_28
#elif defined(QCC74x_undef)
/* Flash option 0 */
/* Flash CLK */
#define QCC74x_EXTFLASH_CLK0_GPIO      GPIO_PIN_21
/* FLASH CS */
#define QCC74x_EXTFLASH_CS0_GPIO       GPIO_PIN_19
/* FLASH DATA */
#define QCC74x_EXTFLASH_DATA00_GPIO    GPIO_PIN_17
#define QCC74x_EXTFLASH_DATA10_GPIO    GPIO_PIN_18
#define QCC74x_EXTFLASH_DATA20_GPIO    GPIO_PIN_22
#define QCC74x_EXTFLASH_DATA30_GPIO    GPIO_PIN_20
/* Flash option 1 */
/* Flash CLK */
#define QCC74x_EXTFLASH_CLK1_GPIO      GPIO_PIN_27
/* FLASH CS */
#define QCC74x_EXTFLASH_CS1_GPIO       GPIO_PIN_25
/* FLASH DATA */
#define QCC74x_EXTFLASH_DATA01_GPIO    GPIO_PIN_28
#define QCC74x_EXTFLASH_DATA11_GPIO    GPIO_PIN_24
#define QCC74x_EXTFLASH_DATA21_GPIO    GPIO_PIN_23
#define QCC74x_EXTFLASH_DATA31_GPIO    GPIO_PIN_26
/* Flash option 2 */
/* Flash CLK */
#define QCC74x_EXTFLASH_CLK2_GPIO      GPIO_PIN_36
/* FLASH CS */
#define QCC74x_EXTFLASH_CS2_GPIO       GPIO_PIN_35
/* FLASH DATA */
#define QCC74x_EXTFLASH_DATA02_GPIO    GPIO_PIN_32
#define QCC74x_EXTFLASH_DATA12_GPIO    GPIO_PIN_34
#define QCC74x_EXTFLASH_DATA22_GPIO    GPIO_PIN_33
#define QCC74x_EXTFLASH_DATA32_GPIO    GPIO_PIN_37
#define QCC74x_FLASH_CFG_SF2_EXT_23_28 0
#define QCC74x_FLASH_CFG_SF2_INT_512K  1
#define QCC74x_FLASH_CFG_SF2_INT_1M    2
#define QCC74x_FLASH_CFG_SF1_EXT_17_22 3
#define QCC74x_SF2_SWAP_NONE           0
#define QCC74x_SF2_SWAP_CS_IO2         1
#define QCC74x_SF2_SWAP_IO0_IO3        2
#define QCC74x_SF2_SWAP_BOTH           3
#elif defined(QCC74x_undef)
/*Flash option 0*/
/*Flash CLK*/
#define QCC74x_EXTFLASH_CLK0_GPIO       GPIO_PIN_22
/*FLASH CS*/
#define QCC74x_EXTFLASH_CS0_GPIO        GPIO_PIN_21
/*FLASH DATA*/
#define QCC74x_EXTFLASH_DATA00_GPIO     GPIO_PIN_20
#define QCC74x_EXTFLASH_DATA10_GPIO     GPIO_PIN_19
#define QCC74x_EXTFLASH_DATA20_GPIO     GPIO_PIN_18
#define QCC74x_EXTFLASH_DATA30_GPIO     GPIO_PIN_17
/*Flash option 1*/
/*Flash CLK*/
#define QCC74x_EXTFLASH_CLK1_GPIO       GPIO_PIN_22
/*FLASH CS*/
#define QCC74x_EXTFLASH_CS1_GPIO        GPIO_PIN_21
/*FLASH DATA*/
#define QCC74x_EXTFLASH_DATA01_GPIO     GPIO_PIN_20
#define QCC74x_EXTFLASH_DATA11_GPIO     GPIO_PIN_0
#define QCC74x_EXTFLASH_DATA21_GPIO     GPIO_PIN_1
#define QCC74x_EXTFLASH_DATA31_GPIO     GPIO_PIN_2
#define QCC74x_FLASH_CFG_DESWAP         1
#define QCC74x_FLASH_CFG_EXT0_17_22     2
#define QCC74x_FLASH_CFG_EXT1_0_2_20_22 3
#endif
#define QCC74x_FLASH_ID_VALID_FLAG 0x80000000
#define QCC74x_FLASH_ID_VALID_MASK 0x7FFFFFFF

/*@} end of group SF_CFG_Public_Macros */

/** @defgroup  SF_CFG_Public_Functions
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

int qcc74x_sf_cfg_get_flash_cfg_need_lock(uint32_t flash_id, spi_flash_cfg_type *p_flash_cfg,
                                        uint8_t group, uint8_t bank);
int qcc74x_sf_cfg_get_flash_cfg_need_lock_ext(uint32_t flash_id, spi_flash_cfg_type *p_flash_cfg,
                                            uint8_t group, uint8_t bank);
#if defined(QCC74x_undefL) || defined(QCC74x_undef)
void qcc74x_sf_cfg_init_internal_flash_gpio(void);
#endif
#if defined(QCC74x_undef)
void qcc74x_sf_cfg_restore_gpio17_fun(uint8_t fun);
#endif
int qcc74x_sf_cfg_init_flash_gpio(uint8_t flash_pin_cfg, uint8_t restore_default);
#ifdef QCC74x_SF_CTRL_SBUS2_ENABLE
int qcc74x_sf_cfg_init_flash2_gpio(uint8_t swap);
#endif
int qcc74x_sf_cfg_init_ext_flash_gpio(uint8_t ext_flash_pin);
int qcc74x_sf_cfg_deinit_ext_flash_gpio(uint8_t ext_flash_pin);
uint32_t qcc74x_sf_cfg_flash_identify(uint8_t call_from_flash, uint8_t flash_pin_cfg, uint8_t restore_default,
                                    spi_flash_cfg_type *p_flash_cfg, uint8_t group, uint8_t bank);
uint32_t qcc74x_sf_cfg_flash_identify_ext(uint8_t callfromflash, uint8_t flash_pin_cfg, uint8_t restore_default,
                                        spi_flash_cfg_type *p_flash_cfg, uint8_t group, uint8_t bank);
#if defined(QCC74x_undef) || defined(QCC743) || defined(QCC74x_undef) || defined(QCC74x_undefP)
int qcc74x_sf_cfg_flash_init(uint8_t sel, const struct sf_ctrl_cfg_type *p_sfctrl_cfg,
                           const struct sf_ctrl_bank2_cfg *p_bank2_cfg);
#ifdef QCC74x_SF_CTRL_SBUS2_ENABLE
int qcc74x_sf_cfg_sbus2_flash_init(uint8_t sel, const struct sf_ctrl_bank2_cfg *p_bank2_cfg);
#endif
#else
int qcc74x_sf_cfg_flash_init(uint8_t sel, const struct sf_ctrl_cfg_type *p_sfctrl_cfg);
#endif

#ifdef __cplusplus
}
#endif

/*@} end of group SF_CFG_Public_Functions */

/*@} end of group SF_CFG */

#endif /* _QCC74x_SF_CFG_H */
