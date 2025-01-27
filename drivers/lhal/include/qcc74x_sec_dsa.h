#ifndef _QCC74x_SEC_DSA_H
#define _QCC74x_SEC_DSA_H

#include "qcc74x_core.h"

/** @addtogroup LHAL
  * @{
  */

/** @addtogroup SEC_DSA
  * @{
  */

struct qcc74x_dsa_crt_s {
    uint32_t *dP;
    uint32_t *dQ;
    uint32_t *qInv;
    uint32_t *p;
    uint32_t *invR_p;
    uint32_t *primeN_p;
    uint32_t *q;
    uint32_t *invR_q;
    uint32_t *primeN_q;
};

struct qcc74x_dsa_s {
    uint32_t size;
    uint32_t crtSize;
    uint32_t *n;
    uint32_t *e;
    uint32_t *d;
    struct qcc74x_dsa_crt_s crtCfg;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 *
 * @param [in] handle
 * @param [in] size
 * @return int
 */
int qcc74x_sec_dsa_init(struct qcc74x_dsa_s *handle, uint32_t size);

/**
 * @brief
 *
 * @param [in] handle
 * @param [in] hash
 * @param [in] hashLenInWord
 * @param [in] s
 * @return int
 */
int qcc74x_sec_dsa_sign(struct qcc74x_dsa_s *handle, const uint32_t *hash, uint32_t hashLenInWord, uint32_t *s);

/**
 * @brief
 *
 * @param [in] handle
 * @param [in] hash
 * @param [in] hashLenInWord
 * @param [in] s
 * @return int
 */
int qcc74x_sec_dsa_verify(struct qcc74x_dsa_s *handle, const uint32_t *hash, uint32_t hashLenInWord, const uint32_t *s);

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