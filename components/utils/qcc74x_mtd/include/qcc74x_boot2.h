#ifndef __QCC74x_BOOT2_H__
#define __QCC74x_BOOT2_H__

#include "qcc74x_core.h"
#include "partition.h"

typedef struct {
    uint8_t type;                           /*!< Partition entry type */
    uint8_t device;                         /*!< Partition entry device */
    uint8_t activeIndex;                    /*!< Partition entry active index */
    uint8_t name[9];                        /*!< Partition entry name */
    uint32_t Address[2];                    /*!< Partition entry start address */
    uint32_t maxLen[2];                     /*!< Partition entry max length */
    uint32_t len;                           /*!< Partition entry length */
    uint32_t age;                           /*!< Partition entry age */
} qcc74x_partition_config_t;

#define BOOT2_PARTITION_TYPE_FW     (0)

int qcc74x_boot2_partition_bus_addr_active(const char *name, uint32_t *addr, uint32_t *size);
int qcc74x_boot2_partition_bus_addr_inactive(const char *name, uint32_t *addr, uint32_t *size);
int qcc74x_boot2_partition_bus_addr(const char *name, uint32_t *addr0, uint32_t *addr1, uint32_t *size0, uint32_t *size1, int *active);
int qcc74x_boot2_partition_addr_active(const char *name, uint32_t *addr, uint32_t *size);
int qcc74x_boot2_partition_addr_inactive(const char *name, uint32_t *addr, uint32_t *size);
int qcc74x_boot2_partition_addr(const char *name, uint32_t *addr0, uint32_t *addr1, uint32_t *size0, uint32_t *size1, int *active);
uint8_t qcc74x_boot2_get_active_partition(void);
int qcc74x_boot2_get_active_entries(int type, qcc74x_partition_config_t *ptEntry);
int qcc74x_boot2_update_ptable(qcc74x_partition_config_t *ptEntry);
int qcc74x_boot2_dump(void);
int qcc74x_boot2_init(void);
void qcc74x_update_mfg_ptable(void);
pt_table_stuff_config *qcc74x_boot2_get_pt_config(void);

#endif
