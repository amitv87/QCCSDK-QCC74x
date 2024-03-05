#ifndef _QCC74x_SEC_TRNG_H
#define _QCC74x_SEC_TRNG_H

#include "qcc74x_core.h"

/** @addtogroup LHAL
  * @{
  */

/** @addtogroup TRNG
  * @{
  */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read trng data.
 *
 * @param [in] dev device handle
 * @param [in] data pointer to trng data
 * @return A negated errno value on failure.
 */
int qcc74x_trng_read(struct qcc74x_device_s *dev, uint8_t data[32]);

/**
 * @brief Read trng data with custom length.
 *
 * @param [in] data pointer to trng data
 * @param [in] len length to read
 * @return A negated errno value on failure.
 */
int qcc74x_trng_readlen(uint8_t *data, uint32_t len);

/**
 * @brief Get trng data.
 *
 * @return trng data with word
 */
long random(void);

/**
 * @brief Enable trng in group0.
 *
 * @param [in] dev device handle
 */
void qcc74x_group0_request_trng_access(struct qcc74x_device_s *dev);

/**
 * @brief Disable trng in group0.
 *
 * @param [in] dev device handle
 */
void qcc74x_group0_release_trng_access(struct qcc74x_device_s *dev);

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