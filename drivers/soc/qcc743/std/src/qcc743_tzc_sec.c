#include "string.h"
#include "qcc743_tzc_sec.h"
#include "qcc743_glb.h"

/** @addtogroup  QCC743_Peripheral_Driver
 *  @{
 */

/** @addtogroup  TZC_ENG
 *  @{
 */

/** @defgroup  TZC_ENG_Private_Macros
 *  @{
 */

/*@} end of group TZC_ENG_Private_Macros */

/** @defgroup  TZC_ENG_Private_Types
 *  @{
 */

/*@} end of group TZC_ENG_Private_Types */

/** @defgroup  TZC_ENG_Private_Variables
 *  @{
 */

/*@} end of group TZC_ENG_Private_Variables */

/** @defgroup  TZC_ENG_Global_Variables
 *  @{
 */

/*@} end of group TZC_ENG_Global_Variables */

/** @defgroup  TZC_ENG_Private_Fun_Declaration
 *  @{
 */

/*@} end of group TZC_ENG_Private_Fun_Declaration */

/** @defgroup  TZC_ENG_Private_Functions
 *  @{
 */

/*@} end of group TZC_ENG_Private_Functions */

/** @defgroup  TZC_ENG_Public_Functions
 *  @{
 */

/****************************************************************************/ /**
 * @brief  None
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_Set_Sboot_Done(void)
{
    uint32_t tmpVal;
    /* Set Sboot done */
    tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, TZC_SEC_TZC_SBOOT_DONE, 0xf);
    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL, tmpVal);
}

void Tzc_Sec_Set_Bus_Remap(uint8_t busRmpEn)
{
    uint32_t tmpVal;

    /* Set bus remap */
    tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL);
    if (busRmpEn) {
        tmpVal = QCC74x_SET_REG_BIT(tmpVal, TZC_SEC_TZC_BUS_RMP_EN);
    } else {
        tmpVal = QCC74x_CLR_REG_BIT(tmpVal, TZC_SEC_TZC_BUS_RMP_EN);
    }
    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL, tmpVal);

    /* Set Lock */
    tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL);
    tmpVal = QCC74x_SET_REG_BIT(tmpVal, TZC_SEC_TZC_BUS_RMP_EN_LOCK);
    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL, tmpVal);
}

void Tzc_Sec_Set_Master_Group(TZC_SEC_Master_Type masterType, uint8_t group)
{
    uint32_t tmpVal;
    uint32_t tmpVal2;

    tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_BMX_TZMID);
    tmpVal2 = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_BMX_TZMID_LOCK);

    if (group == 0) {
        tmpVal &= (~(1 << masterType));
    } else {
        tmpVal |= (1 << masterType);
    }
    tmpVal |= (1 << (masterType + 16));
    tmpVal2 |= (1 << masterType);

    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_BMX_TZMID, tmpVal);
    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_BMX_TZMID_LOCK, tmpVal2);
}

void Tzc_Sec_Set_Slave_Group(TZC_SEC_Slave_Type slaveType, uint8_t group)
{
    uint32_t tmpVal;

    if (group > TZC_SEC_MAX_AUTH_GRP || slaveType >= TZC_SEC_SLAVE_MAX) {
        return;
    }
    group = 1 << (group);

    if (slaveType < TZC_SEC_SLAVE_S1_GLB) {
        tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S0);
        /* set group */
        tmpVal &= (~(3 << (slaveType * 2)));
        tmpVal |= (group << (slaveType * 2));
        /* set lock*/
        tmpVal |= (1 << (slaveType + 16));
        QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S0, tmpVal);
    } else if ((slaveType >= TZC_SEC_SLAVE_S1_GLB) && (slaveType < TZC_SEC_SLAVE_S2_EMI_MISC)) {
        slaveType -= TZC_SEC_SLAVE_S1_GLB;
        tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S1);
        /* set group */
        tmpVal &= (~(3 << (slaveType * 2)));
        tmpVal |= (group << (slaveType * 2));
        QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S1, tmpVal);

        tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S_LOCK);
        /* set lock */
        tmpVal |= (1 << slaveType);
        QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S_LOCK, tmpVal);
    } else if ((slaveType >= TZC_SEC_SLAVE_S2_EMI_MISC) && (slaveType < TZC_SEC_SLAVE_S1A_UART0)) {
        slaveType -= TZC_SEC_SLAVE_S2_EMI_MISC;

        tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S2);
        /* set group */
        tmpVal &= (~(3 << (slaveType * 2)));
        tmpVal |= (group << (slaveType * 2));
        QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S2, tmpVal);

        tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S_LOCK);
        /* set lock */
        tmpVal |= (1 << (slaveType + (TZC_SEC_SLAVE_S2_EMI_MISC - TZC_SEC_SLAVE_S1_GLB)));
        QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S_LOCK, tmpVal);
    } else {
        slaveType -= TZC_SEC_SLAVE_S1A_UART0;

        tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S1A);
        /* set group */
        tmpVal &= (~(3 << (slaveType * 2)));
        tmpVal |= (group << (slaveType * 2));
        QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S1A, tmpVal);

        tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S1A_LOCK);
        /* set lock */
        tmpVal |= (1 << slaveType);
        QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_BMX_S1A_LOCK, tmpVal);
    }
}

void Tzc_Sec_Set_Codec_Group(uint8_t group)
{
    uint32_t tmpVal;
    uint32_t tmpVal2;

    tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_MM_BMX_TZMID);
    tmpVal2 = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_MM_BMX_TZMID_LOCK);

    if (group == 0) {
        tmpVal &= (~(1 << 2));
    } else {
        tmpVal |= (1 << 2);
    }
    tmpVal |= (1 << (2 + 16));
    tmpVal2 |= (1 << 2);

    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_MM_BMX_TZMID, tmpVal);
    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_MM_BMX_TZMID_LOCK, tmpVal2);
}

void Tzc_Sec_Set_Glb_Ctrl_Group(TZC_SEC_GLB_Ctrl_Type slaveType, uint8_t group)
{
    uint32_t tmpVal;

    if (slaveType >= TZC_SEC_GLB_CTRL_MAX || group > TZC_SEC_MAX_AUTH_GRP) {
        return;
    }
    group = 1 << (group);

    tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_GLB_CTRL_0);
    /* set group */
    tmpVal &= (~(3 << (slaveType * 2)));
    tmpVal |= (group << (slaveType * 2));
    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_GLB_CTRL_0, tmpVal);

    tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_GLB_CTRL_2);
    /* set lock */
    tmpVal |= (1 << slaveType);
    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_GLB_CTRL_2, tmpVal);
}

void Tzc_Sec_Set_CPU_Group(uint8_t cpu, uint8_t group)
{
    uint32_t tmpVal;
    uint32_t tmpVal2;

    if (cpu == GLB_CORE_ID_M0) {
        tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_BMX_TZMID);
        tmpVal2 = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_BMX_TZMID_LOCK);

        tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, TZC_SEC_TZC_CPU_TZMID, group);
        tmpVal2 = QCC74x_SET_REG_BITS_VAL(tmpVal2, TZC_SEC_TZC_CPU_TZMID_LOCK, 1);

        QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_BMX_TZMID, tmpVal);
        QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_BMX_TZMID_LOCK, tmpVal2);
    }
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set ROM region access configuration
 *
 * @param  region: ROM region index 0-2
 * @param  startAddr: ROM region start address
 * @param  length: ROM region end length
 * @param  group: ROM region auth group type
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_ROM_Access_Set(uint8_t region, uint32_t startAddr, uint32_t length, uint8_t group)
{
    uint32_t tmpVal = 0;
    uint32_t alignEnd = (startAddr + length + 1023) & ~0x3FF;

    /* check the parameter */
    CHECK_PARAM(IS_TZC_SEC_GROUP_TYPE(group));
    if (region >= 3 || group > TZC_SEC_MAX_AUTH_GRP) {
        return;
    }
    group = 1 << (group);

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL);
    tmpVal &= (~(0xf << (region * 4)));
    tmpVal |= (group << (region * 4));
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL, tmpVal);

    tmpVal = (((alignEnd >> 10) & 0xffff) - 1) | (((startAddr >> 10) & 0xffff) << 16);
    QCC74x_WR_WORD(TZC_SEC_BASE + TZC_SEC_TZC_ROM_TZSRG_R0_OFFSET + region * 4, tmpVal);

    /* set enable and lock */
    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL);
    tmpVal |= 1 << (region + 16);
    tmpVal |= 1 << (region + 24);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set ROM region access configuration
 *
 * @param  region: ROM region index 0-2
 * @param  startAddr: ROM region start address
 * @param  length: ROM region end length
 * @param  group: ROM region auth group type
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_ROM_Access_Set_Advance(uint8_t region, uint32_t startAddr, uint32_t length, uint8_t group)
{
    uint32_t tmpVal = 0;
    uint32_t alignEnd = (startAddr + length + 1023) & ~0x3FF;

    /* check the parameter */
    CHECK_PARAM(IS_TZC_SEC_GROUP_TYPE(group));
    if (region >= 3) {
        return;
    }
    group = group & 0xf;

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL);
    tmpVal &= (~(0xf << (region * 4)));
    tmpVal |= (group << (region * 4));
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL, tmpVal);

    tmpVal = (((alignEnd >> 10) & 0xffff) - 1) | (((startAddr >> 10) & 0xffff) << 16);
    QCC74x_WR_WORD(TZC_SEC_BASE + TZC_SEC_TZC_ROM_TZSRG_R0_OFFSET + region * 4, tmpVal);

    /* set enable and lock */
    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL);
    tmpVal |= 1 << (region + 16);
    tmpVal |= 1 << (region + 24);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set OCRAM region access configuration
 *
 * @param  region: OCRAM region index 0-2
 * @param  startAddr: OCRAM region start address
 * @param  length: OCRAM region end length
 * @param  group: OCRAM region auth group type
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_OCRAM_Access_Set_Advance(uint8_t region, uint32_t startAddr, uint32_t length, uint8_t group)
{
    uint32_t tmpVal = 0;
    uint32_t alignEnd = (startAddr + length + 1023) & ~0x3FF;

    /* check the parameter */
    CHECK_PARAM(IS_TZC_SEC_GROUP_TYPE(group));
    if (region >= 3) {
        return;
    }
    group = group & 0xf;

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_OCRAM_TZSRG_CTRL);
    tmpVal &= (~(0xf << (region * 4)));
    tmpVal |= (group << (region * 4));
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_OCRAM_TZSRG_CTRL, tmpVal);

    tmpVal = (((alignEnd >> 10) & 0xffff) - 1) | (((startAddr >> 10) & 0xffff) << 16);
    QCC74x_WR_WORD(TZC_SEC_BASE + TZC_SEC_TZC_OCRAM_TZSRG_R0_OFFSET + region * 4, tmpVal);

    /* set enable and lock */
    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_OCRAM_TZSRG_CTRL);
    tmpVal |= 1 << (region + 16);
    tmpVal |= 1 << (region + 20);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_OCRAM_TZSRG_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set OCRAM regionx access configuration
 *
 * @param  group: OCRAM region auth group type
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_OCRAM_Access_Set_Regionx(uint8_t group)
{
    uint32_t tmpVal = 0;
    uint8_t region = 3;

    /* check the parameter */
    CHECK_PARAM(IS_TZC_SEC_GROUP_TYPE(group));
    if (group > TZC_SEC_MAX_AUTH_GRP) {
        return;
    }
    group = 1 << (group);

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_OCRAM_TZSRG_CTRL);
    tmpVal &= (~(0xf << (region * 4)));
    tmpVal |= (group << (region * 4));
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_OCRAM_TZSRG_CTRL, tmpVal);

    /* set enable and lock */
    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_OCRAM_TZSRG_CTRL);
    tmpVal |= 1 << (region + 16);
    tmpVal |= 1 << (region + 20);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_OCRAM_TZSRG_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set WRAM region access configuration
 *
 * @param  region: WRAM region index 0-2
 * @param  startAddr: WRAM region start address
 * @param  length: WRAM region end length
 * @param  group: WRAM region auth group type
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_WRAM_Access_Set_Advance(uint8_t region, uint32_t startAddr, uint32_t length, uint8_t group)
{
    uint32_t tmpVal = 0;
    uint32_t alignEnd = (startAddr + length + 1023) & ~0x3FF;

    /* check the parameter */
    CHECK_PARAM(IS_TZC_SEC_GROUP_TYPE(group));
    if (region >= 3) {
        return;
    }
    group = group & 0xf;

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_WRAM_TZSRG_CTRL);
    tmpVal &= (~(0xf << (region * 4)));
    tmpVal |= (group << (region * 4));
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_WRAM_TZSRG_CTRL, tmpVal);

    tmpVal = (((alignEnd >> 10) & 0xffff) - 1) | (((startAddr >> 10) & 0xffff) << 16);
    QCC74x_WR_WORD(TZC_SEC_BASE + TZC_SEC_TZC_WRAM_TZSRG_R0_OFFSET + region * 4, tmpVal);

    /* set enable and lock */
    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_WRAM_TZSRG_CTRL);
    tmpVal |= 1 << (region + 16);
    tmpVal |= 1 << (region + 20);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_WRAM_TZSRG_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set WRAM regionx access configuration
 *
 * @param  group: WRAM region auth group type
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_WRAM_Access_Set_Regionx(uint8_t group)
{
    uint32_t tmpVal = 0;
    uint8_t region = 3;

    /* check the parameter */
    CHECK_PARAM(IS_TZC_SEC_GROUP_TYPE(group));
    if (group > TZC_SEC_MAX_AUTH_GRP) {
        return;
    }
    group = 1 << (group);

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_WRAM_TZSRG_CTRL);
    tmpVal &= (~(0xf << (region * 4)));
    tmpVal |= (group << (region * 4));
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_WRAM_TZSRG_CTRL, tmpVal);

    /* set enable and lock */
    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_WRAM_TZSRG_CTRL);
    tmpVal |= 1 << (region + 16);
    tmpVal |= 1 << (region + 20);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_WRAM_TZSRG_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set HBNRAM region access configuration
 *
 * @param  startAddr: HBNRAM region start address
 * @param  length: HBNRAM region end length
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_HBNRAM_Access_Set(uint32_t startAddr, uint32_t length)
{
    uint32_t tmpVal = 0;
    uint32_t alignEnd = (startAddr + length + 3) & ~0x3;

    tmpVal = QCC74x_RD_REG(AON_BASE, AON_TZC_HBNRAM_R0);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AON_TZC_HBNRAM_R0_START, ((startAddr >> 2) & 0xffff));
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AON_TZC_HBNRAM_R0_END, (((alignEnd >> 2) & 0xffff) - 1));
    QCC74x_WR_REG(AON_BASE, AON_TZC_HBNRAM_R0, tmpVal);

    /* set enable and lock */
    tmpVal = QCC74x_RD_REG(AON_BASE, AON_TZC_HBNRAM_CTRL);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AON_TZC_HBNRAM_R0_EN, 1);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AON_TZC_HBNRAM_R0_LOCK, 1);
    QCC74x_WR_REG(AON_BASE, AON_TZC_HBNRAM_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set Flash region access configuration
 *
 * @param  region: Flash region index 0-2
 * @param  startAddr: Flash region start address
 * @param  length: Flash region end length
 * @param  group: Flash region auth group type
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_Flash_Access_Set(uint8_t region, uint32_t startAddr, uint32_t length, uint8_t group)
{
    uint32_t tmpVal = 0;
    uint32_t tmpVal2 = 0;
    uint32_t alignEnd = (startAddr + length + 1023) & ~0x3FF;

    /* check the parameter */
    CHECK_PARAM(IS_TZC_SEC_GROUP_TYPE(group));
    if (region >= 3 || group > TZC_SEC_MAX_AUTH_GRP) {
        return;
    }
    group = 1 << (group);

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_SF_TZSRG_CTRL);
    tmpVal &= (~(0xf << (region * 4)));
    tmpVal |= (group << (region * 4));
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_SF_TZSRG_CTRL, tmpVal);

    /* Set range */
    tmpVal = (((alignEnd >> 10) & 0xffff) - 1) | (((startAddr >> 10) & 0xffff) << 16);
    QCC74x_WR_WORD(TZC_SEC_BASE + TZC_SEC_TZC_SF_TZSRG_R0_OFFSET + region * 4, tmpVal);

    /* Set range MSB */
    tmpVal = QCC74x_RD_WORD(TZC_SEC_BASE + TZC_SEC_TZC_SF_TZSRG_MSB_OFFSET);
    tmpVal = tmpVal & (0xff << (8 * region));
    tmpVal2 = ((alignEnd >> 26) & 0x7) | (((startAddr >> 26) & 0x7) << 3);
    tmpVal2 = tmpVal2 << (8 * region);
    QCC74x_WR_WORD(TZC_SEC_BASE + TZC_SEC_TZC_SF_TZSRG_MSB_OFFSET, tmpVal | tmpVal2);

    /* set enable and lock */
    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_SF_TZSRG_CTRL);
    tmpVal |= 1 << (region + 20);
    tmpVal |= 1 << (region + 25);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_SF_TZSRG_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set Flash regionx access configuration
 *
 * @param  group: Flash region auth group type
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_Flash_Access_Set_Regionx(uint8_t group)
{
    uint32_t tmpVal = 0;
    uint8_t region = 4;

    /* check the parameter */
    CHECK_PARAM(IS_TZC_SEC_GROUP_TYPE(group));
    if (group > TZC_SEC_MAX_AUTH_GRP) {
        return;
    }
    group = 1 << (group);

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_SF_TZSRG_CTRL);
    tmpVal &= (~(0xf << (region * 4)));
    tmpVal |= (group << (region * 4));
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_SF_TZSRG_CTRL, tmpVal);

    /* set enable and lock */
    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_SF_TZSRG_CTRL);
    tmpVal |= 1 << (region + 20);
    tmpVal |= 1 << (region + 25);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_SF_TZSRG_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set Flash region access advance configuration
 *
 * @param  region: Flash region index 0-2
 * @param  startAddr: Flash region start address
 * @param  length: Flash region end length
 * @param  group: Flash region auth group type
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_Flash_Access_Set_Advance(uint8_t region, uint32_t startAddr, uint32_t length, uint8_t group)
{
    uint32_t tmpVal = 0;
    uint32_t tmpVal2 = 0;
    uint32_t alignEnd = (startAddr + length + 1023) & ~0x3FF;

    /* check the parameter */
    CHECK_PARAM(IS_TZC_SEC_GROUP_TYPE(group));
    if (region >= 3) {
        return;
    }
    group = group & 0xf;

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_SF_TZSRG_CTRL);
    tmpVal &= (~(0xf << (region * 4)));
    tmpVal |= (group << (region * 4));
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_SF_TZSRG_CTRL, tmpVal);

    /* Set range */
    tmpVal = (((alignEnd >> 10) & 0xffff) - 1) | (((startAddr >> 10) & 0xffff) << 16);
    QCC74x_WR_WORD(TZC_SEC_BASE + TZC_SEC_TZC_SF_TZSRG_R0_OFFSET + region * 4, tmpVal);

    /* Set range MSB */
    tmpVal = QCC74x_RD_WORD(TZC_SEC_BASE + TZC_SEC_TZC_SF_TZSRG_MSB_OFFSET);
    tmpVal = tmpVal & (0xff << (8 * region));
    tmpVal2 = ((alignEnd >> 26) & 0x7) | (((startAddr >> 26) & 0x7) << 3);
    tmpVal2 = tmpVal2 << (8 * region);
    QCC74x_WR_WORD(TZC_SEC_BASE + TZC_SEC_TZC_SF_TZSRG_MSB_OFFSET, tmpVal | tmpVal2);

    /* set enable and lock */
    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_SF_TZSRG_CTRL);
    tmpVal |= 1 << (region + 20);
    tmpVal |= 1 << (region + 25);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_SF_TZSRG_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set PSRAMB region access configuration
 *
 * @param  region: PSRAMB region index 0-2
 * @param  startAddr: PSRAMB region start address
 * @param  length: PSRAMB region end length
 * @param  group: PSRAMB region auth group type
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_PSRAMB_Access_Set(uint8_t region, uint32_t startAddr, uint32_t length, uint8_t group)
{
    uint32_t tmpVal = 0;
    uint32_t alignEnd = (startAddr + length + 1023) & ~0x3FF;

    /* check the parameter */
    CHECK_PARAM(IS_TZC_SEC_GROUP_TYPE(group));
    if (region >= 3 || group > TZC_SEC_MAX_AUTH_GRP) {
        return;
    }
    group = 1 << (group);

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_PSRAMB_TZSRG_CTRL);
    tmpVal &= (~(0xf << (region * 4)));
    tmpVal |= (group << (region * 4));
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_PSRAMB_TZSRG_CTRL, tmpVal);

    tmpVal = (((alignEnd >> 10) & 0xffff) - 1) | (((startAddr >> 10) & 0xffff) << 16);
    QCC74x_WR_WORD(TZC_SEC_BASE + TZC_SEC_TZC_PSRAMB_TZSRG_R0_OFFSET + region * 4, tmpVal);

    /* set enable and lock */
    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_PSRAMB_TZSRG_CTRL);
    tmpVal |= 1 << (region + 16);
    tmpVal |= 1 << (region + 24);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_PSRAMB_TZSRG_CTRL, tmpVal);
}

/**
 * @brief TrustZone Security set Release PSRAMB region access
 *
 */
void Tzc_Sec_PSRAMB_Access_Release(void)
{
    uint32_t tmpVal = 0;
    uint32_t region = 0;
    /* set disable */
    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_PSRAMB_TZSRG_CTRL);
    tmpVal &= (~(1 << (region + 16)));
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_PSRAMB_TZSRG_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set PSRAMB region access configuration
 *
 * @param  region: PSRAMB region index 0-2
 * @param  startAddr: PSRAMB region start address
 * @param  length: PSRAMB region end length
 * @param  group: PSRAMB region auth group type
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_PSRAMB_Access_Set_Advance(uint8_t region, uint32_t startAddr, uint32_t length, uint8_t group)
{
    uint32_t tmpVal = 0;
    uint32_t alignEnd = (startAddr + length + 1023) & ~0x3FF;

    /* check the parameter */
    CHECK_PARAM(IS_TZC_SEC_GROUP_TYPE(group));
    if (region >= 3 || group > TZC_SEC_MAX_AUTH_GRP) {
        return;
    }
    group = 0xf & group;

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_PSRAMB_TZSRG_CTRL);
    tmpVal &= (~(0xf << (region * 4)));
    tmpVal |= (group << (region * 4));
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_PSRAMB_TZSRG_CTRL, tmpVal);

    tmpVal = (((alignEnd >> 10) & 0xffff) - 1) | (((startAddr >> 10) & 0xffff) << 16);
    QCC74x_WR_WORD(TZC_SEC_BASE + TZC_SEC_TZC_PSRAMB_TZSRG_R0_OFFSET + region * 4, tmpVal);

    /* set enable and lock */
    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_PSRAMB_TZSRG_CTRL);
    tmpVal |= 1 << (region + 16);
    tmpVal |= 1 << (region + 24);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_PSRAMB_TZSRG_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set sec_eng module config
 *
 * @param  mode: sec_eng control mode
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_Set_Se_Ctrl_Mode(TZC_SEC_SE_Ctrl_Mode mode)
{
    uint32_t tmpVal = 0;

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_SE_CTRL_0);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, TZC_SEC_TZC_SE_TZSID_CRMD, mode);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_SE_CTRL_0, tmpVal);

    tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_SE_CTRL_2);
    /* set lock */
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, TZC_SEC_TZC_SE_TZSID_CRMD_LOCK, 1);
    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_SE_CTRL_2, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set sf_ctrl module config
 *
 * @param  mode: sf_ctrl control mode
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_Set_Sf_Ctrl_Mode(TZC_SEC_SF_Ctrl_Mode mode)
{
    uint32_t tmpVal = 0;

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_SE_CTRL_1);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, TZC_SEC_TZC_SF_TZSID_CRMD, mode);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_SE_CTRL_1, tmpVal);

    tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_SE_CTRL_2);
    /* set lock */
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, TZC_SEC_TZC_SF_TZSID_CRMD_LOCK, 1);
    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_SE_CTRL_2, tmpVal);
}

void Tzc_Sec_Set_Se_Group(TZC_SEC_SE_Ctrl_Type slaveType, uint8_t group)
{
    uint32_t tmpVal;

    if (group > TZC_SEC_MAX_AUTH_GRP || slaveType >= TZC_SEC_SE_CTRL_MAX) {
        return;
    }
    group = 1 << (group);

    tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_SE_CTRL_0);
    /* set group */
    tmpVal &= (~(3 << (slaveType * 2)));
    tmpVal |= (group << (slaveType * 2));
    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_SE_CTRL_0, tmpVal);

    tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_SE_CTRL_2);
    /* set lock */
    tmpVal |= (1 << (slaveType));
    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_SE_CTRL_2, tmpVal);
}

void Tzc_Sec_Set_Sf_Group(TZC_SEC_SF_Ctrl_Type slaveType, uint8_t group)
{
    uint32_t tmpVal;

    if (group > TZC_SEC_MAX_AUTH_GRP || slaveType >= TZC_SEC_SF_CTRL_MAX) {
        return;
    }
    group = 1 << (group);

    tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_SE_CTRL_1);
    /* set group */
    tmpVal &= (~(3 << (slaveType * 2)));
    tmpVal |= (group << (slaveType * 2));
    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_SE_CTRL_1, tmpVal);

    tmpVal = QCC74x_RD_REG(TZ1_BASE, TZC_SEC_TZC_SE_CTRL_2);
    /* set lock */
    tmpVal |= (1 << (slaveType + 16));
    QCC74x_WR_REG(TZ1_BASE, TZC_SEC_TZC_SE_CTRL_2, tmpVal);
}

/****************************************************************************/ /**
 * @brief  TrustZone Security set watchdog reset delay value
 *
 * @param  mode: sec_eng control mode
 *
 * @return None
 *
*******************************************************************************/
void Tzc_Sec_Set_WTD_Rst_Delay(uint16_t delayValue)
{
    uint32_t tmpVal = 0;

    tmpVal = QCC74x_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_SE_CTRL_0);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, TZC_SEC_TZC_SE_WDT_DLY, delayValue);
    QCC74x_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_SE_CTRL_0, tmpVal);
}

/*@} end of group TZC_ENG_Public_Functions */

/*@} end of group TZC_ENG */

/*@} end of group QCC743_Peripheral_Driver */
