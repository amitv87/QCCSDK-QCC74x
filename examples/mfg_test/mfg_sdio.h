#ifndef QCC74x_SDIO_H_NAWSQHJB
#define QCC74x_SDIO_H_NAWSQHJB

#include "stdint.h"
#include <qcc743_sdu.h>

int32_t qcc74x_sdio_init(void *cfg);

int32_t qcc74x_sdio_handshake_poll(void *data);

void qcc74x_sdio_cmd_process(void);

int32_t qcc74x_sdio_write(uint32_t *data,uint32_t len);

int32_t qcc74x_sdio_wait_tx_idle(uint32_t timeout);

int32_t qcc74x_sdio_deinit(void);

#endif /* end of include guard: QCC74x_SDIO_H_NAWSQHJB */
