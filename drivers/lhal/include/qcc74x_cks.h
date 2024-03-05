#ifndef _QCC74x_CKS_H
#define _QCC74x_CKS_H

#include "qcc74x_core.h"

/** @addtogroup LHAL
  * @{
  */

/** @addtogroup CKS
  * @{
  */

/** @defgroup CKS_ENDIAN cks endian definition
  * @{
  */
#define CKS_LITTLE_ENDIAN 0
#define CKS_BIG_ENDIAN    1
/**
  * @}
  */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reset checksum module.
 *
 * @param [in] dev device handle
 */
void qcc74x_cks_reset(struct qcc74x_device_s *dev);

/**
 * @brief Set checksum bitorder.
 *
 * @param [in] dev device handle
 * @param [in] endian cks endian, use @ref CKS_ENDIAN
 */
void qcc74x_cks_set_endian(struct qcc74x_device_s *dev, uint8_t endian);

/**
 * @brief Compute data with checksum.
 *
 * @param [in] dev device handle
 * @param [in] data input data buffer
 * @param [in] length data length
 * @return checksum value
 */
uint16_t qcc74x_cks_compute(struct qcc74x_device_s *dev, uint8_t *data, uint32_t length);

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
