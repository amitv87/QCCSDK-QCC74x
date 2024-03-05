#include "qcc743_l1c.h"
#include "qcc743_common.h"
// #include "qcc743_glb.h"

/** @addtogroup  QCC743_Peripheral_Driver
 *  @{
 */

/** @addtogroup  L1C
 *  @{
 */

/** @defgroup  L1C_Private_Macros
 *  @{
 */

/*@} end of group L1C_Private_Macros */

/** @defgroup  L1C_Private_Types
 *  @{
 */

/*@} end of group L1C_Private_Types */

/** @defgroup  L1C_Private_Variables
 *  @{
 */

/*@} end of group L1C_Private_Variables */

/** @defgroup  L1C_Global_Variables
 *  @{
 */

/*@} end of group L1C_Global_Variables */

/** @defgroup  L1C_Private_Fun_Declaration
 *  @{
 */

/*@} end of group L1C_Private_Fun_Declaration */

/** @defgroup  L1C_Private_Functions
 *  @{
 */

/*@} end of group L1C_Private_Functions */

/** @defgroup  L1C_Public_Functions
 *  @{
 */

/****************************************************************************/ /**
 * @brief  Enable I-Cache
 *
 * @param  wayDsiable: cache way disable config
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
QCC74x_Err_Type ATTR_TCM_SECTION L1C_ICache_Enable(uint8_t wayDsiable)
{
#ifndef __riscv_32e
    csi_icache_enable();
#endif
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Enable D-Cache
 *
 * @param  wayDsiable: cache way disable config
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
QCC74x_Err_Type ATTR_TCM_SECTION L1C_DCache_Enable(uint8_t wayDsiable)
{
#ifndef __riscv_32e
    csi_dcache_enable();
#endif
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Disable I-Cache
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
QCC74x_Err_Type ATTR_TCM_SECTION L1C_ICache_Disable(void)
{
#ifndef __riscv_32e
    csi_icache_disable();
#endif
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Disable D-Cache
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
QCC74x_Err_Type ATTR_TCM_SECTION L1C_DCache_Disable(void)
{
#ifndef __riscv_32e
    csi_dcache_disable();
#endif
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  L1C D-Cache write set
 *
 * @param  wtEn: L1C write through enable
 * @param  wbEn: L1C write back enable
 * @param  waEn: L1C write allocate enable
 *
 * @return None
 *
*******************************************************************************/
__WEAK
void ATTR_TCM_SECTION L1C_DCache_Write_Set(QCC74x_Fun_Type wtEn, QCC74x_Fun_Type wbEn, QCC74x_Fun_Type waEn)
{
    return;
}

/****************************************************************************/ /**
 * @brief  Clean all D_Cache
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
QCC74x_Err_Type ATTR_TCM_SECTION L1C_DCache_Clean_All(void)
{
#ifndef __riscv_32e
    csi_dcache_clean();
#endif
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Clean and invalid all D_Cache
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
QCC74x_Err_Type ATTR_TCM_SECTION L1C_DCache_Clean_Invalid_All(void)
{
#ifndef __riscv_32e
    csi_dcache_clean_invalid();
#endif
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Invalid all I-Cache
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
QCC74x_Err_Type ATTR_TCM_SECTION L1C_ICache_Invalid_All(void)
{
#ifndef __riscv_32e
    csi_icache_invalid();
#endif
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Invalid all D_Cache
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
QCC74x_Err_Type ATTR_TCM_SECTION L1C_DCache_Invalid_All(void)
{
#ifndef __riscv_32e
    csi_dcache_invalid();
#endif
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Clean D-Cache according to address
 *
 * @param  addr: Address to clean
 * @param  len: Length to clean
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
QCC74x_Err_Type ATTR_TCM_SECTION L1C_DCache_Clean_By_Addr(uintptr_t addr, uint32_t len)
{
#ifndef __riscv_32e
#ifdef CPU_D0
    csi_dcache_clean_range((uintptr_t *)addr, len);
#else
    csi_dcache_clean_range((uint32_t *)(uintptr_t *)addr, len);
#endif
#endif
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Clean and invalid D-Cache according to address
 *
 * @param  addr: Address to clean
 * @param  len: Length to clean
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
QCC74x_Err_Type ATTR_TCM_SECTION L1C_DCache_Clean_Invalid_By_Addr(uintptr_t addr, uint32_t len)
{
#ifndef __riscv_32e
#ifdef CPU_D0
    csi_dcache_clean_invalid_range((uintptr_t *)addr, len);
#else
    csi_dcache_clean_invalid_range((uint32_t *)(uintptr_t *)addr, len);
#endif
#endif
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Invalid I-Cache according to address
 *
 * @param  addr: Address to clean
 * @param  len: Length to clean
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
QCC74x_Err_Type ATTR_TCM_SECTION L1C_ICache_Invalid_By_Addr(uintptr_t addr, uint32_t len)
{
    L1C_ICache_Invalid_All();

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Invalid D-Cache according to address
 *
 * @param  addr: Address to clean
 * @param  len: Length to clean
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
QCC74x_Err_Type ATTR_TCM_SECTION L1C_DCache_Invalid_By_Addr(uintptr_t addr, uint32_t len)
{
#ifndef __riscv_32e
#ifdef CPU_D0
    csi_dcache_invalid_range((uintptr_t *)addr, len);
#else
    csi_dcache_invalid_range((uint32_t *)(uintptr_t *)addr, len);
#endif
#endif
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  cache wrap
 *
 * @param  en: wrap enable or disable
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
QCC74x_Err_Type ATTR_TCM_SECTION L1C_Set_Wrap(uint8_t en)
{
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  cache wrap
 *
 * @param  core: cpu core
 * @param  cacheSetting: cache setting
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
QCC74x_Err_Type ATTR_TCM_SECTION L1C_Set_Cache_Setting_By_ID(uint8_t core, L1C_CACHE_Cfg_Type *cacheSetting)
{
    (void)core;
    (void)cacheSetting;

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Judge addr is in cache range
 *
 * @param  addr: phyical addr
 *
 * @return 1 for addr is in cache range and 0 for not in cache range
 *
*******************************************************************************/
int ATTR_TCM_SECTION L1C_Is_DCache_Range(uintptr_t addr)
{
    if(((addr>>16)&0xffff)>=0x62FC){
        return 1;
    }else{
        return 0;
    }
}

/****************************************************************************/ /**
 * @brief  Get None Cache address according to Cache address
 *
 * @param  addr: cache addr
 *
 * @return none cache addr
 *
*******************************************************************************/
int ATTR_TCM_SECTION L1C_Get_None_Cache_Addr(uintptr_t addr)
{
    return (addr&0x0FFFFFFF)|0x20000000;
}

/*@} end of group L1C_Public_Functions */

/*@} end of group L1C */

/*@} end of group QCC743_Peripheral_Driver */
