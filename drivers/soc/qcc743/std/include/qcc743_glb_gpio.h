#ifndef __QCC743_GLB_GPIO_H__
#define __QCC743_GLB_GPIO_H__

#include "glb_reg.h"
#include "mm_glb_reg.h"
#include "pds_reg.h"
#include "qcc743_gpio.h"
#include "qcc743_hbn.h"
#include "qcc743_aon.h"
#include "qcc743_pds.h"
#include "qcc743_common.h"
#include "qcc74x_sf_ctrl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup  QCC743_Peripheral_Driver
 *  @{
 */

/** @addtogroup  GLB_GPIO
 *  @{
 */

/** @defgroup  GLB_GPIO_Public_Types
 *  @{
 */

/**
 *  @brief GLB GPIO interrupt control mode type definition
 */
#define GLB_GPIO_INT_TRIG_SYNC_FALLING_EDGE        (0)  /*!< GPIO interrupt sync mode, GPIO falling edge trigger interrupt */
#define GLB_GPIO_INT_TRIG_SYNC_RISING_EDGE         (1)  /*!< GPIO interrupt sync mode, GPIO rising edge trigger interrupt */
#define GLB_GPIO_INT_TRIG_SYNC_LOW_LEVEL           (2)  /*!< GPIO interrupt sync mode, GPIO low level trigger interrupt (32k 3T) */
#define GLB_GPIO_INT_TRIG_SYNC_HIGH_LEVEL          (3)  /*!< GPIO interrupt sync mode, GPIO high level trigger interrupt (32k 3T) */
#define GLB_GPIO_INT_TRIG_SYNC_FALLING_RISING_EDGE (4)  /*!< GPIO interrupt sync mode, GPIO falling and rising edge trigger interrupt */
#define GLB_GPIO_INT_TRIG_ASYNC_FALLING_EDGE       (8)  /*!< GPIO interrupt async mode, GPIO falling edge trigger interrupt */
#define GLB_GPIO_INT_TRIG_ASYNC_RISING_EDGE        (9)  /*!< GPIO interrupt async mode, GPIO rising edge trigger interrupt */
#define GLB_GPIO_INT_TRIG_ASYNC_LOW_LEVEL          (10) /*!< GPIO interrupt async mode, GPIO low level trigger interrupt (32k 3T) */
#define GLB_GPIO_INT_TRIG_ASYNC_HIGH_LEVEL         (11) /*!< GPIO interrupt async mode, GPIO high level trigger interrupt (32k 3T) */

/**
 *  @brief GLB GPIO FIFO interrupt type definition
 */
#define GLB_GPIO_FIFO_INT_FER  (0) /*!< GLB GPIO FIFO Underflow or Overflow interrupt */
#define GLB_GPIO_FIFO_INT_FIFO (1) /*!< GLB GPIO FIFO ready (tx_fifo_cnt > tx_fifo_th) interrupt */
#define GLB_GPIO_FIFO_INT_END  (2) /*!< GLB GPIO FIFO Empty interrupt */
#define GLB_GPIO_FIFO_INT_ALL  (3) /*!< All the interrupt */

/**
 *  @brief GLB GPIO FIFO Timing Phase type definition
 */
#define GPIO_FIFO_PHASE_FIRST_HIGH (0) /*!< GPIO first send high level */
#define GPIO_FIFO_PHASE_FIRST_LOW  (1)  /*!< GPIO first send low level */

/**
 *  @brief GLB GPIO FIFO Idle State type definition
 */
#define GPIO_FIFO_IDLE_LOW  (0)
#define GPIO_FIFO_IDLE_HIGH (1)

/**
 *  @brief GLB GPIO FIFO Latch Mode type definition
 */
#define GPIO_FIFO_LATCH_WRITE    (0)    /*!< GPIO FIFO direct write I/O */
#define GPIO_FIFO_LATCH_SETCLEAR (1) /*!< GPIO FIFO set/clr I/O */

/**
 *  @brief GPIO interrupt configuration structure type definition
 */
typedef struct
{
    uint8_t gpioPin; /*!< GPIO pin num */
    uint8_t trig;    /*!< GPIO interrupt trig mode */
    uint8_t intMask; /*!< GPIO interrupt mask config */
} GLB_GPIO_INT_Cfg_Type;

/**
 *  @brief UART configuration structure type definition
 */
typedef struct
{
    uint8_t code0FirstTime;   /*!< The clock num of code0 first send */
    uint8_t code1FirstTime;   /*!< The clock num of code1 first send */
    uint16_t codeTotalTime;   /*!< The total clock num of code0/1(high + low */
    uint8_t code0Phase;       /*!< low or high level of code0 first send */
    uint8_t code1Phase;       /*!< low or high level of code1 first send */
    uint8_t idle;             /*!< the I/O idle level */
    uint8_t fifoDmaThreshold; /*!< FIFO threshold */
    uint8_t fifoDmaEnable;    /*!< Enable or disable DMA of GPIO */
    uint8_t latch;            /*!< Write or set/clr GPIO level */
} GLB_GPIO_FIFO_CFG_Type;

/*@} end of group GLB_GPIO_Public_Types */

/** @defgroup  GLB_GPIO_Public_Constants
 *  @{
 */

/** @defgroup  GLB_GPIO_INT_TRIG_TYPE
 *  @{
 */
#define IS_GLB_GPIO_INT_TRIG_TYPE(type) (((type) == GLB_GPIO_INT_TRIG_SYNC_FALLING_EDGE) ||         \
                                         ((type) == GLB_GPIO_INT_TRIG_SYNC_RISING_EDGE) ||          \
                                         ((type) == GLB_GPIO_INT_TRIG_SYNC_LOW_LEVEL) ||            \
                                         ((type) == GLB_GPIO_INT_TRIG_SYNC_HIGH_LEVEL) ||           \
                                         ((type) == GLB_GPIO_INT_TRIG_SYNC_FALLING_RISING_EDGE) ||  \
                                         ((type) == GLB_GPIO_INT_TRIG_ASYNC_FALLING_EDGE) ||        \
                                         ((type) == GLB_GPIO_INT_TRIG_ASYNC_RISING_EDGE) ||         \
                                         ((type) == GLB_GPIO_INT_TRIG_ASYNC_LOW_LEVEL) ||           \
                                         ((type) == GLB_GPIO_INT_TRIG_ASYNC_HIGH_LEVEL))

/** @defgroup  GLB_GPIO_INT_TRIG_TYPE
 *  @{
 */
#define IS_GLB_GPIO_FIFO_INT_TYPE(type) (((type) == GLB_GPIO_FIFO_INT_FER) ||  \
                                         ((type) == GLB_GPIO_FIFO_INT_FIFO) || \
                                         ((type) == GLB_GPIO_FIFO_INT_END) ||  \
                                         ((type) == GLB_GPIO_FIFO_INT_ALL))

/** @defgroup  GLB_GPIO_FIFO_PHASE_TYPE
 *  @{
 */
#define IS_GLB_GPIO_FIFO_PHASE_TYPE(type) (((type) == GPIO_FIFO_PHASE_FIRST_HIGH) || \
                                           ((type) == GPIO_FIFO_PHASE_FIRST_LOW))

/** @defgroup  GLB_GPIO_FIFO_PHASE_TYPE
 *  @{
 */
#define IS_GLB_GPIO_FIFO_LATCH_TYPE(type) (((type) == GPIO_FIFO_LATCH_WRITE) || \
                                           ((type) == GPIO_FIFO_LATCH_SETCLEAR))

/*@} end of group GLB_GPIO_Public_Constants */

/** @defgroup  GLB_GPIO_Public_Macros
 *  @{
 */

/*@} end of group GLB_GPIO_Public_Macros */

/** @defgroup  GLB_GPIO_Public_Functions
 *  @{
 */
/*----------*/
#ifndef QCC74x_USE_HAL_DRIVER
void GPIO_INT0_IRQHandler(void);
#endif
/*----------*/
QCC74x_Sts_Type GLB_GPIO_Pad_LeadOut_Sts(uint8_t gpioPin);
/*----------*/
QCC74x_Err_Type GLB_GPIO_Init(GLB_GPIO_Cfg_Type *cfg);
QCC74x_Err_Type GLB_GPIO_Func_Init(uint8_t gpioFun, uint8_t *pinList, uint8_t cnt);
QCC74x_Err_Type GLB_GPIO_Input_Enable(uint8_t gpioPin);
QCC74x_Err_Type GLB_Embedded_Flash_Pad_Enable(uint8_t swapIo2Cs);
QCC74x_Err_Type GLB_GPIO_Input_Disable(uint8_t gpioPin);
QCC74x_Err_Type GLB_GPIO_Output_Enable(uint8_t gpioPin);
QCC74x_Err_Type GLB_GPIO_Output_Disable(uint8_t gpioPin);
QCC74x_Err_Type GLB_GPIO_Set_HZ(uint8_t gpioPin);
uint8_t GLB_GPIO_Get_Fun(uint8_t gpioPin);
uint32_t GLB_GPIO_Read(uint8_t gpioPin);
QCC74x_Err_Type GLB_GPIO_Write(uint8_t gpioPin, uint32_t val);
QCC74x_Err_Type GLB_GPIO_Set(uint8_t gpioPin);
QCC74x_Err_Type GLB_GPIO_Clr(uint8_t gpioPin);
QCC74x_Err_Type GLB_GPIO_Int_Init(GLB_GPIO_INT_Cfg_Type *intCfg);
QCC74x_Err_Type GLB_GPIO_IntMask(uint8_t gpioPin, QCC74x_Mask_Type intMask);
QCC74x_Sts_Type GLB_Get_GPIO_IntStatus(uint8_t gpioPin);
QCC74x_Err_Type GLB_Clr_GPIO_IntStatus(uint8_t gpioPin);
QCC74x_Err_Type GLB_GPIO_INT0_IRQHandler_Install(void);
QCC74x_Err_Type GLB_GPIO_INT0_Callback_Install(uint8_t gpioPin, intCallback_Type *cbFun);

#ifndef QCC74x_USE_HAL_DRIVER
void GPIO_FIFO_IRQHandler(void);
#endif
QCC74x_Err_Type GLB_GPIO_Fifo_Callback_Install(uint8_t intType, intCallback_Type *cbFun);
QCC74x_Err_Type GLB_GPIO_Fifo_IRQHandler_Install(void);
QCC74x_Err_Type GLB_GPIO_Fifo_Init(GLB_GPIO_FIFO_CFG_Type *cfg);
QCC74x_Err_Type GLB_GPIO_Fifo_Push(uint16_t *data, uint16_t len);
uint32_t GLB_GPIO_Fifo_GetCount(void);
QCC74x_Err_Type GLB_GPIO_Fifo_Clear(void);
QCC74x_Err_Type GLB_GPIO_Fifo_IntMask(uint8_t intType, QCC74x_Mask_Type intMask);
QCC74x_Err_Type GLB_GPIO_Fifo_IntClear(uint8_t intType);
QCC74x_Sts_Type GLB_GPIO_Fifo_GetIntStatus(uint8_t intType);
QCC74x_Err_Type GLB_GPIO_Fifo_Enable(void);
QCC74x_Err_Type GLB_GPIO_Fifo_Disable(void);

/*@} end of group GLB_GPIO_Public_Functions */

/*@} end of group GLB_GPIO */

/*@} end of group QCC743_Peripheral_Driver */

#ifdef __cplusplus
}
#endif

#endif /* __QCC743_GLB_GPIO_H__ */
