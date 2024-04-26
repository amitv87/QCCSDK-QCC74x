#ifndef __QCC74x_SDU_H__
#define __QCC74x_SDU_H__

#include <stdint.h>

int qcc74x_sdu_init(void);
int32_t qcc74x_sdio_handshake(void);
typedef void (*qcc74x_sdio_read_cb_t)(void *cb_arg, const void *data_ptr, const uint16_t data_len);
int qcc74x_sdio_read_cb_register(void *env, qcc74x_sdio_read_cb_t cb, const void *cb_arg);
int qcc74x_hal_sdio_write(void *env, const void *data_ptr, const uint16_t data_len);
/* data_len is len(V), excluding len(TL) */
int qcc74x_hal_sdio_write_tlv(void *env, const uint16_t type, const void *data_ptr, const uint16_t data_len);
/* data_len is len(V), excluding len(TL) */

#endif
