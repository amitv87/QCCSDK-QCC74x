#ifndef _QCC74x_CLOCK_H
#define _QCC74x_CLOCK_H

#include "qcc74x_core.h"

/** @addtogroup LHAL
  * @{
  */

/** @addtogroup CLOCK
  * @{
  */

/** @defgroup QCC74x_SYSTEM_CLOCK system clock definition
  * @{
  */
#define QCC74x_SYSTEM_ROOT_CLOCK 0
#define QCC74x_SYSTEM_CPU_CLK    1
#define QCC74x_SYSTEM_PBCLK      2
#define QCC74x_SYSTEM_XCLK       3
#define QCC74x_SYSTEM_32K_CLK    4
/**
  * @}
  */

#if defined(QCC74x_undef) || defined(QCC74x_undef) || defined(QCC74x_undefL)
#define QCC74x_GLB_CGEN1_BASE (0x40000000 + 0x24)
#elif defined(QCC743) || defined(QCC74x_undefP) || defined(QCC74x_undef) || defined(QCC74x_undef)
#define QCC74x_GLB_CGEN1_BASE (0x20000000 + 0x584)
#define QCC74x_GLB_CGEN2_BASE (0x20000000 + 0x588)
#endif

#define PERIPHERAL_CLOCK_ADC_DAC_ENABLE()                         \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 2);                                       \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#define PERIPHERAL_CLOCK_SEC_ENABLE()                             \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 4);                                       \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#define PERIPHERAL_CLOCK_DMA0_ENABLE()                            \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 12);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#if defined(QCC74x_undefP) || defined(QCC74x_undef)
#define PERIPHERAL_CLOCK_DMA1_ENABLE()
#define PERIPHERAL_CLOCK_DMA2_ENABLE()
#endif

#define PERIPHERAL_CLOCK_UART0_ENABLE()                           \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 16);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#define PERIPHERAL_CLOCK_UART1_ENABLE()                           \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 17);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#if defined(QCC74x_undefP) || defined(QCC74x_undef) || defined(QCC74x_undef)
#define PERIPHERAL_CLOCK_UART2_ENABLE()
#endif

#if defined(QCC74x_undefP) || defined(QCC74x_undef)
#define PERIPHERAL_CLOCK_SPI0_1_ENABLE()                          \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 18);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)
#else
#define PERIPHERAL_CLOCK_SPI0_ENABLE()                            \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 18);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)
#endif

#define PERIPHERAL_CLOCK_I2C0_ENABLE()                            \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 19);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#if defined(QCC74x_undefP) || defined(QCC74x_undef) || defined(QCC743) || defined(QCC74x_undef)
#define PERIPHERAL_CLOCK_I2C1_ENABLE()                            \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 25);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#endif

#define PERIPHERAL_CLOCK_PWM0_ENABLE()                            \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 20);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#define PERIPHERAL_CLOCK_TIMER0_1_WDG_ENABLE()                    \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 21);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#define PERIPHERAL_CLOCK_IR_ENABLE()                              \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 22);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#define PERIPHERAL_CLOCK_CKS_ENABLE()                             \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 23);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#if defined(QCC74x_undefP) || defined(QCC74x_undef)
#define PERIPHERAL_CLOCK_CAN_UART2_ENABLE()                       \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 26);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)
#elif defined(QCC743) || defined(QCC74x_undef)
#define PERIPHERAL_CLOCK_CAN_ENABLE()                             \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 26);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)
#endif

#if defined(QCC74x_undef)
#define PERIPHERAL_CLOCK_USB_ENABLE()                             \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 28);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#define PERIPHERAL_CLOCK_I2S_ENABLE()                             \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 26);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#elif defined(QCC743) || defined(QCC74x_undefP) || defined(QCC74x_undef)
#define PERIPHERAL_CLOCK_USB_ENABLE()                             \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 13);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#define PERIPHERAL_CLOCK_I2S_ENABLE()                             \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 27);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)

#endif

#if defined(QCC743) || defined(QCC74x_undefP) || defined(QCC74x_undef) || defined(QCC74x_undef)
#define PERIPHERAL_CLOCK_SDH_ENABLE()                             \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN2_BASE); \
        regval |= (1 << 22);                                      \
        putreg32(regval, QCC74x_GLB_CGEN2_BASE);                    \
    } while (0)
#endif

#if defined(QCC74x_undef)
#define PERIPHERAL_CLOCK_EMAC_ENABLE()                            \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN1_BASE); \
        regval |= (1 << 13);                                      \
        putreg32(regval, QCC74x_GLB_CGEN1_BASE);                    \
    } while (0)
#elif defined(QCC743) || defined(QCC74x_undefP) || defined(QCC74x_undef)
#define PERIPHERAL_CLOCK_EMAC_ENABLE()                            \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN2_BASE); \
        regval |= (1 << 23);                                      \
        putreg32(regval, QCC74x_GLB_CGEN2_BASE);                    \
    } while (0)
#endif

#if defined(QCC743)
#define PERIPHERAL_CLOCK_AUDIO_ENABLE()                           \
    do {                                                          \
        volatile uint32_t regval = getreg32(QCC74x_GLB_CGEN2_BASE); \
        regval |= (1 << 21);                                      \
        putreg32(regval, QCC74x_GLB_CGEN2_BASE);                    \
    } while (0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get system clock frequence
 *
 * @param [in] type system clock type
 * @return frequence
 */
uint32_t qcc74x_clk_get_system_clock(uint8_t type);

/**
 * @brief Get peripheral clock frequence
 *
 * @param [in] type peripheral type
 * @param [in] idx peripheral index
 * @return frequence
 */
uint32_t qcc74x_clk_get_peripheral_clock(uint8_t type, uint8_t idx);

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

/**
  * @}
  */

#endif