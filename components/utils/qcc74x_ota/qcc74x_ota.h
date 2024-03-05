#ifndef __HOSAL_OTA_H__
#define __HOSAL_OTA_H__
#include <stdint.h>

#define HOSAL_OTA_FILE_SIZE_MAX    0x100000  // 1M

#define QCC74x_MTD_PARTITION_NAME_FW_DEFAULT    "FW"

int qcc74x_ota_start(uint32_t file_size);
int qcc74x_ota_update(uint32_t filesize, uint32_t offset, uint8_t *buf, uint32_t buf_len);
int qcc74x_ota_finish(uint8_t check_hash);
int qcc74x_ota_read(uint32_t offset, uint8_t *buf, uint32_t buf_len);
int qcc74x_ota_check(void);
int qcc74x_ota_apply(void);
void qcc74x_ota_abort(void);
#endif
