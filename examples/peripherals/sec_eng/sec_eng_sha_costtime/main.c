#include "qcc74x_mtimer.h"
#include "qcc74x_sec_sha.h"
#include "board.h"

ATTR_NOCACHE_NOINIT_RAM_SECTION __attribute__((aligned(32))) uint8_t sha_input_buf[16 * 1024]; /* input addr must be align 32 */

uint8_t sha_output_buf[128];

ATTR_NOCACHE_NOINIT_RAM_SECTION struct qcc74x_sha1_ctx_s ctx_sha1;
ATTR_NOCACHE_NOINIT_RAM_SECTION struct qcc74x_sha256_ctx_s ctx_sha256;
ATTR_NOCACHE_NOINIT_RAM_SECTION struct qcc74x_sha512_ctx_s ctx_sha512;

int main(void)
{
    uint64_t start_time;

    board_init();

    struct qcc74x_device_s *sha;

    sha = qcc74x_device_get_by_name("sha");

    qcc74x_group0_request_sha_access(sha);

    for (uint32_t i = 0; i < 16 * 1024; i++) {
        sha_input_buf[i] = i & 0xff;
    }

    qcc74x_sha_init(sha, SHA_MODE_SHA1);

    for (uint32_t i = 1; i <= 1024; i++) {
        start_time = qcc74x_mtimer_get_time_us();
        qcc74x_sha1_start(sha, &ctx_sha1);
        qcc74x_sha1_update(sha, &ctx_sha1, sha_input_buf, 16 * i);
        qcc74x_sha1_finish(sha, &ctx_sha1, sha_output_buf);
        printf("sha1 block:%d cost time:%d us\r\n", i, (uint32_t)(qcc74x_mtimer_get_time_us() - start_time));
    }

    printf("sha1 success\r\n");

    qcc74x_sha_init(sha, SHA_MODE_SHA224);
    for (uint32_t i = 1; i <= 1024; i++) {
        start_time = qcc74x_mtimer_get_time_us();
        qcc74x_sha256_start(sha, &ctx_sha256);
        qcc74x_sha256_update(sha, &ctx_sha256, sha_input_buf, 16 * i);
        qcc74x_sha256_finish(sha, &ctx_sha256, sha_output_buf);
        printf("sha224 block:%d cost time:%d us\r\n", i, (uint32_t)(qcc74x_mtimer_get_time_us() - start_time));
    }

    qcc74x_sha_init(sha, SHA_MODE_SHA256);
    for (uint32_t i = 1; i <= 1024; i++) {
        start_time = qcc74x_mtimer_get_time_us();
        qcc74x_sha256_start(sha, &ctx_sha256);
        qcc74x_sha256_update(sha, &ctx_sha256, sha_input_buf, 16 * i);
        qcc74x_sha256_finish(sha, &ctx_sha256, sha_output_buf);
        printf("sha256 block:%d cost time:%d us\r\n", i, (uint32_t)(qcc74x_mtimer_get_time_us() - start_time));
    }

#if !defined(QCC74x_undefL)
    qcc74x_sha_init(sha, SHA_MODE_SHA384);
    for (uint32_t i = 1; i <= 1024; i++) {
        start_time = qcc74x_mtimer_get_time_us();
        qcc74x_sha512_start(sha, &ctx_sha512);
        qcc74x_sha512_update(sha, &ctx_sha512, sha_input_buf, 16 * i);
        qcc74x_sha512_finish(sha, &ctx_sha512, sha_output_buf);
        printf("sha384 block:%d cost time:%d us\r\n", i, (uint32_t)(qcc74x_mtimer_get_time_us() - start_time));
    }

    qcc74x_sha_init(sha, SHA_MODE_SHA512);
    for (uint32_t i = 1; i <= 1024; i++) {
        start_time = qcc74x_mtimer_get_time_us();
        qcc74x_sha512_start(sha, &ctx_sha512);
        qcc74x_sha512_update(sha, &ctx_sha512, sha_input_buf, 16 * i);
        qcc74x_sha512_finish(sha, &ctx_sha512, sha_output_buf);
        printf("sha512 block:%d cost time:%d us\r\n", i, (uint32_t)(qcc74x_mtimer_get_time_us() - start_time));
    }
#endif

    qcc74x_group0_release_sha_access(sha);
    while (1) {
        qcc74x_mtimer_delay_ms(2000);
    }
}