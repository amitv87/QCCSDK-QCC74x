#ifndef MBEDTLS_SHA256_ALT_H
#define MBEDTLS_SHA256_ALT_H

#include "qcc74x_sec_sha.h"

/**
 * \brief          The SHA-1 context structure.
 *
 * \warning        SHA-1 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
typedef struct mbedtls_sha256_context
{
    struct qcc74x_device_s *sha;
    struct qcc74x_sha256_link_ctx_s link_ctx;
}
mbedtls_sha256_context __attribute__((aligned(32)));

#endif /* sha1_alt.h */
