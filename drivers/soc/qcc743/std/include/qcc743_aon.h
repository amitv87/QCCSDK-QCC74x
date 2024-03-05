#ifndef __QCC743_AON_H__
#define __QCC743_AON_H__

#include "aon_reg.h"
#include "glb_reg.h"
#include "hbn_reg.h"
#include "pds_reg.h"
#include "qcc743_ef_cfg.h"
#include "qcc743_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup  QCC743_Peripheral_Driver
 *  @{
 */

/** @addtogroup  AON
 *  @{
 */

/** @defgroup  AON_Public_Types
 *  @{
 */

/*@} end of group AON_Public_Types */

/** @defgroup  AON_Public_Constants
 *  @{
 */

/*@} end of group AON_Public_Constants */

/** @defgroup  AON_Public_Macros
 *  @{
 */

/*@} end of group AON_Public_Macros */

/** @defgroup  AON_Public_Functions
 *  @{
 */
/*----------*/
QCC74x_Err_Type AON_Power_On_MBG(void);
QCC74x_Err_Type AON_Power_Off_MBG(void);
/*----------*/
QCC74x_Err_Type AON_Power_On_XTAL(void);
QCC74x_Err_Type AON_Set_Xtal_CapCode(uint8_t capIn, uint8_t capOut);
uint8_t AON_Get_Xtal_CapCode(void);
QCC74x_Err_Type AON_Power_Off_XTAL(void);
/*----------*/
QCC74x_Err_Type AON_Power_On_BG(void);
QCC74x_Err_Type AON_Power_Off_BG(void);
/*----------*/
QCC74x_Err_Type AON_Power_On_LDO15_RF(void);
QCC74x_Err_Type AON_Power_Off_LDO15_RF(void);
QCC74x_Err_Type AON_Output_Float_LDO15_RF(void);
QCC74x_Err_Type AON_Output_Pulldown_LDO15_RF(void);
/*----------*/
QCC74x_Err_Type AON_Power_On_SFReg(void);
QCC74x_Err_Type AON_Power_Off_SFReg(void);
/*----------*/
QCC74x_Err_Type AON_Power_On_MicBias(void);
QCC74x_Err_Type AON_Power_Off_MicBias(void);
/*----------*/
QCC74x_Err_Type AON_LowPower_Enter_PDS0(void);
QCC74x_Err_Type AON_LowPower_Exit_PDS0(void);
/*----------*/
QCC74x_Err_Type AON_Trim_DcdcDis(void);
QCC74x_Err_Type AON_Trim_DcdcVoutSel(void);
QCC74x_Err_Type AON_Trim_DcdcVoutTrim(void);
QCC74x_Err_Type AON_Trim_Ldo11socVoutTrim(void);
QCC74x_Err_Type AON_Trim_Usb20RcalCode(void);
/*----------*/
QCC74x_Err_Type AON_Output_Pulldown_DCDC18(void);
QCC74x_Err_Type AON_Output_Float_DCDC18(void);

/*@} end of group AON_Public_Functions */

/*@} end of group AON */

/*@} end of group QCC743_Peripheral_Driver */

#ifdef __cplusplus
}
#endif

#endif /* __QCC743_AON_H__ */
