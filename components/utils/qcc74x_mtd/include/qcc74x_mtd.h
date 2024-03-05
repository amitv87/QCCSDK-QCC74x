#ifndef __QCC74x_MTD_H__
#define __QCC74x_MTD_H__

#include "qcc74x_core.h"

typedef void *qcc74x_mtd_handle_t;
typedef struct {
    char name[16];
    unsigned int offset;
    unsigned int size;
    void *xip_addr;
} qcc74x_mtd_info_t;

#define QCC74x_MTD_OPEN_FLAG_NONE           (0)
/* open backup partition */
#define QCC74x_MTD_OPEN_FLAG_BACKUP        (1 << 0)
#define QCC74x_MTD_OPEN_FLAG_BUSADDR       (1 << 1)

void qcc74x_mtd_init(void);
int qcc74x_mtd_open(const char *name, qcc74x_mtd_handle_t *handle, unsigned int flags);
int qcc74x_mtd_close(qcc74x_mtd_handle_t handle);
int qcc74x_mtd_info(qcc74x_mtd_handle_t handle, qcc74x_mtd_info_t *info);
int qcc74x_mtd_erase(qcc74x_mtd_handle_t handle, unsigned int addr, unsigned int size);
int qcc74x_mtd_erase_all(qcc74x_mtd_handle_t handle);
int qcc74x_mtd_write(qcc74x_mtd_handle_t handle, unsigned int addr, unsigned int size, const uint8_t *data);
int qcc74x_mtd_read(qcc74x_mtd_handle_t handle,  unsigned int addr, unsigned int size, uint8_t *data);
int qcc74x_mtd_size(qcc74x_mtd_handle_t handle, unsigned int *size);

#define QCC74x_MTD_PARTITION_NAME_PSM               "PSM"
#define QCC74x_MTD_PARTITION_NAME_FW_DEFAULT        "FW"
#define QCC74x_MTD_PARTITION_NAME_ROMFS             "media"

#endif
