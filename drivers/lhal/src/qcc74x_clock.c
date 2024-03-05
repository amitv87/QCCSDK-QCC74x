#include "qcc74x_clock.h"
#if defined(QCC74x_undef)
#include "qcc74x_undef_clock.h"
#elif defined(QCC74x_undefL)
#include "qcc74x_undefl_clock.h"
#elif defined(QCC74x_undefP)
#include "qcc74x_undefp_clock.h"
#elif defined(QCC74x_undef)
#include "qcc74x_undef_clock.h"
#elif defined(QCC743)
#include "qcc743_clock.h"
#elif defined(QCC74x_undef)
#include "qcc74x_undef_clock.h"
#elif defined(QCC74x_undef)
#include "qcc74x_undef_clock.h"
#endif

/****************************************************************************/ /**
 * @brief  get system clock
 *
 * @param  type: QCC74x_SYSTEM_XXX
 *
 * @return NONE
 *
*******************************************************************************/
uint32_t ATTR_CLOCK_SECTION qcc74x_clk_get_system_clock(uint8_t type)
{
    switch (type) {
        case QCC74x_SYSTEM_ROOT_CLOCK:
            return qcc74x_clock_get_root();
        case QCC74x_SYSTEM_CPU_CLK:
            return qcc74x_clock_get_fclk();
        case QCC74x_SYSTEM_PBCLK:
            return qcc74x_clock_get_bclk();
        case QCC74x_SYSTEM_XCLK:
            return qcc74x_clock_get_xclk();
        case QCC74x_SYSTEM_32K_CLK:
            return qcc74x_clock_get_f32k();
        default:
            return 0;
    }
    return 0;
}

/****************************************************************************/ /**
 * @brief  get peripheral clock
 *
 * @param  type: QCC74x_DEVICE_TYPE_XXX
 * @param  idx: peripheral index
 *
 * @return NONE
 *
*******************************************************************************/
uint32_t ATTR_CLOCK_SECTION qcc74x_clk_get_peripheral_clock(uint8_t type, uint8_t idx)
{
    switch (type) {
        case QCC74x_DEVICE_TYPE_ADC:
            return qcc74x_clock_get_adc();
        case QCC74x_DEVICE_TYPE_DAC:
            return qcc74x_clock_get_dac();
        case QCC74x_DEVICE_TYPE_UART:
            return qcc74x_clock_get_uart();
        case QCC74x_DEVICE_TYPE_SPI:
            return qcc74x_clock_get_spi();
        case QCC74x_DEVICE_TYPE_I2C:
            return qcc74x_clock_get_i2c();
        case QCC74x_DEVICE_TYPE_PWM:
            return qcc74x_clock_get_pwm();
        case QCC74x_DEVICE_TYPE_TIMER:
            return qcc74x_clock_get_timer(idx);
        case QCC74x_DEVICE_TYPE_WDT:
            return qcc74x_clock_get_wdt();
        case QCC74x_DEVICE_TYPE_FLASH:
            return qcc74x_clock_get_flash();
        case QCC74x_DEVICE_TYPE_IR:
            return qcc74x_clock_get_ir();
        case QCC74x_DEVICE_TYPE_PKA:
            return qcc74x_clock_get_pka();
        case QCC74x_DEVICE_TYPE_SDH:
            return qcc74x_clock_get_sdh();
        case QCC74x_DEVICE_TYPE_CAMERA:
            return qcc74x_clock_get_cam();
        case QCC74x_DEVICE_TYPE_DBI:
            return qcc74x_clock_get_dbi();
        case QCC74x_DEVICE_TYPE_I2S:
            return qcc74x_clock_get_i2s();
        default:
            return 0;
    }
    return 0;
}
