#ifndef _QCC74x_CORE_H
#define _QCC74x_CORE_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <risc-v/csr.h>
#include <risc-v/riscv_arch.h>
#include <compiler/compiler_gcc.h>
#include <compiler/compiler_ld.h>
#include "qcc74x_name.h"
#include "qcc74x_common.h"
#include "qcc74x_mtimer.h"
#include "qcc74x_irq.h"
#include "qcc74x_l1c.h"

/** @addtogroup LHAL
  * @{
  */

/** @addtogroup CORE
  * @{
  */

#if !defined(QCC74x_undef) && !defined(QCC74x_undef) && !defined(QCC74x_undefL) && \
    !defined(QCC743) && !defined(QCC74x_undefP) && !defined(QCC74x_undef) && !defined(QCC74x_undef)
#error please define a supported chip
#endif

#ifdef CONFIG_LHAL_PARAM_ASSERT
#define LHAL_PARAM_ASSERT(expr) ((expr) ? (void)0 : qcc74x_lhal_assert_func(__FILE__, __LINE__, __FUNCTION__, #expr))
void qcc74x_lhal_assert_func(const char *file, uint32_t line, const char *function, const char *string);
#else
#define LHAL_PARAM_ASSERT(expr) ((void)0U)
#endif

#if defined(QCC74x_undef)
#define QCC74x_PSRAM_BASE 0x26000000
#elif defined(QCC743)
#define QCC74x_PSRAM_BASE 0xA8000000
#elif defined(QCC74x_undef)
#define QCC74x_PSRAM_BASE 0x50000000
#elif defined(QCC74x_undefP)
#define QCC74x_PSRAM_BASE 0x54000000
#endif

#define QCC74x_DEVICE_TYPE_ADC      0
#define QCC74x_DEVICE_TYPE_DAC      1
#define QCC74x_DEVICE_TYPE_AUDIOADC 2
#define QCC74x_DEVICE_TYPE_AUDIODAC 3
#define QCC74x_DEVICE_TYPE_GPIO     4
#define QCC74x_DEVICE_TYPE_UART     5
#define QCC74x_DEVICE_TYPE_SPI      6
#define QCC74x_DEVICE_TYPE_I2C      7
#define QCC74x_DEVICE_TYPE_DMA      8
#define QCC74x_DEVICE_TYPE_I2S      9
#define QCC74x_DEVICE_TYPE_IR       10
#define QCC74x_DEVICE_TYPE_TIMER    11
#define QCC74x_DEVICE_TYPE_PWM      12
#define QCC74x_DEVICE_TYPE_CAMERA   14
#define QCC74x_DEVICE_TYPE_FLASH    15
#define QCC74x_DEVICE_TYPE_QSPI     16
#define QCC74x_DEVICE_TYPE_SDH      17
#define QCC74x_DEVICE_TYPE_SDU      18
#define QCC74x_DEVICE_TYPE_ETH      19
#define QCC74x_DEVICE_TYPE_RTC      20
#define QCC74x_DEVICE_TYPE_CRC      21
#define QCC74x_DEVICE_TYPE_RNG      22
#define QCC74x_DEVICE_TYPE_MIPI     23
#define QCC74x_DEVICE_TYPE_DPI      24
#define QCC74x_DEVICE_TYPE_DSI      25
#define QCC74x_DEVICE_TYPE_CSI      26
#define QCC74x_DEVICE_TYPE_USB      27
#define QCC74x_DEVICE_TYPE_AES      28
#define QCC74x_DEVICE_TYPE_SHA      29
#define QCC74x_DEVICE_TYPE_MD5      30
#define QCC74x_DEVICE_TYPE_TRNG     31
#define QCC74x_DEVICE_TYPE_PKA      32
#define QCC74x_DEVICE_TYPE_CKS      33
#define QCC74x_DEVICE_TYPE_MJPEG    34
#define QCC74x_DEVICE_TYPE_KYS      35
#define QCC74x_DEVICE_TYPE_DBI      36
#define QCC74x_DEVICE_TYPE_WDT      37
#define QCC74x_DEVICE_TYPE_EF_CTRL  38
#define QCC74x_DEVICE_TYPE_SDIO2    39
#define QCC74x_DEVICE_TYPE_SDIO3    40
#define QCC74x_DEVICE_TYPE_PLFMDMA  41
#define QCC74x_DEVICE_TYPE_WO       42

struct qcc74x_device_s {
    const char *name;
    uint32_t reg_base;
    uint8_t irq_num;
    uint8_t idx;
    uint8_t sub_idx;
    uint8_t dev_type;
    void *user_data;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get device handle by name.
 *
 * @param [in] name device name
 * @return device handle
 */
struct qcc74x_device_s *qcc74x_device_get_by_name(const char *name);

/**
 * @brief Set user data into device handle.
 *
 * @param [in] device device handle
 * @param [in] user_data pointer to user data
 */
void qcc74x_device_set_userdata(struct qcc74x_device_s *device, void *user_data);

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
