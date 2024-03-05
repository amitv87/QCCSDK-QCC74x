
#include "msp_qcc743_platform.h"
#include "msp_qcc743_glb_gpio.h"
#include "msp_glb_reg.h"
#include "msp_qcc743_gpio.h"

/****************************************************************************/ /**
 * @brief  GPIO initialization
 *
 * @param  cfg: GPIO configuration
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
QCC74x_Err_Type ATTR_TCM_SECTION ms_GLB_GPIO_Init(GLB_GPIO_Cfg_Type *cfg)
{
    uint8_t gpioPin = cfg->gpioPin;
    uint32_t gpioCfgAddress;
    uint32_t tmpVal;

    /* drive strength(drive) = 0  <=>  8.0mA  @ 3.3V */
    /* drive strength(drive) = 1  <=>  9.6mA  @ 3.3V */
    /* drive strength(drive) = 2  <=>  11.2mA @ 3.3V */
    /* drive strength(drive) = 3  <=>  12.8mA @ 3.3V */

    gpioCfgAddress = GLB_BASE + GLB_GPIO_CFG0_OFFSET + (gpioPin << 2);

    /* Disable output anyway*/
    tmpVal = QCC74x_RD_WORD(gpioCfgAddress);
    tmpVal = QCC74x_CLR_REG_BIT(tmpVal, GLB_REG_GPIO_0_OE);
    QCC74x_WR_WORD(gpioCfgAddress, tmpVal);

    /* input/output, pull up/down, drive, smt, function */
    tmpVal = QCC74x_RD_WORD(gpioCfgAddress);

    if (cfg->gpioMode != GPIO_MODE_ANALOG) {
        /* not analog mode */

        if (cfg->gpioMode == GPIO_MODE_OUTPUT) {
            tmpVal = QCC74x_CLR_REG_BIT(tmpVal, GLB_REG_GPIO_0_IE);
            tmpVal = QCC74x_SET_REG_BIT(tmpVal, GLB_REG_GPIO_0_OE);
        } else {
            tmpVal = QCC74x_SET_REG_BIT(tmpVal, GLB_REG_GPIO_0_IE);
            tmpVal = QCC74x_CLR_REG_BIT(tmpVal, GLB_REG_GPIO_0_OE);
        }

        if (cfg->pullType == GPIO_PULL_UP) {
            tmpVal = QCC74x_SET_REG_BIT(tmpVal, GLB_REG_GPIO_0_PU);
            tmpVal = QCC74x_CLR_REG_BIT(tmpVal, GLB_REG_GPIO_0_PD);
        } else if (cfg->pullType == GPIO_PULL_DOWN) {
            tmpVal = QCC74x_CLR_REG_BIT(tmpVal, GLB_REG_GPIO_0_PU);
            tmpVal = QCC74x_SET_REG_BIT(tmpVal, GLB_REG_GPIO_0_PD);
        } else {
            tmpVal = QCC74x_CLR_REG_BIT(tmpVal, GLB_REG_GPIO_0_PU);
            tmpVal = QCC74x_CLR_REG_BIT(tmpVal, GLB_REG_GPIO_0_PD);
        }
    } else {
        /* analog mode */

        /* clear ie && oe */
        tmpVal = QCC74x_CLR_REG_BIT(tmpVal, GLB_REG_GPIO_0_IE);
        tmpVal = QCC74x_CLR_REG_BIT(tmpVal, GLB_REG_GPIO_0_OE);

        /* clear pu && pd */
        tmpVal = QCC74x_CLR_REG_BIT(tmpVal, GLB_REG_GPIO_0_PU);
        tmpVal = QCC74x_CLR_REG_BIT(tmpVal, GLB_REG_GPIO_0_PD);
    }

    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, GLB_REG_GPIO_0_DRV, cfg->drive);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, GLB_REG_GPIO_0_SMT, cfg->smtCtrl);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, GLB_REG_GPIO_0_FUNC_SEL, cfg->gpioFun);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, GLB_REG_GPIO_0_MODE, cfg->outputMode);
    QCC74x_WR_WORD(gpioCfgAddress, tmpVal);

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Write GPIO
 *
 * @param  gpioPin: GPIO type
 * @param  val: GPIO value
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
QCC74x_Err_Type msp_GLB_GPIO_Write(GLB_GPIO_Type gpioPin, uint32_t val)
{
    uint32_t gpioCfgAddress;
    uint32_t tmpVal;

    gpioCfgAddress = GLB_BASE + GLB_GPIO_CFG0_OFFSET + (gpioPin << 2);
    tmpVal = QCC74x_RD_WORD(gpioCfgAddress);

    if (val) {
        tmpVal = QCC74x_SET_REG_BIT(tmpVal, GLB_REG_GPIO_0_O);
    } else {
        tmpVal = QCC74x_CLR_REG_BIT(tmpVal, GLB_REG_GPIO_0_O);
    }

    QCC74x_WR_WORD(gpioCfgAddress, tmpVal);

    return SUCCESS;
}

