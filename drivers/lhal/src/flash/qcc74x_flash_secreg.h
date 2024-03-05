#ifndef _QCC74x_FLASH_SECREG_H
#define _QCC74x_FLASH_SECREG_H

#include <stdint.h>
#include <stdbool.h>

typedef union {
    uint8_t raw[8];
    struct __attribute__((packed)) {
        uint32_t region_count  : 6;  /*!< flash security register region count */
        uint32_t region_size   : 6;  /*!< flash security register region size, unit 256B */
        uint32_t secreg_size   : 6;  /*!< flash security register valid size, unit 256B */
        uint32_t api_type      : 6;  /*!< flash security register api type */
        uint32_t region_offset : 3;  /*!< flash security register address offset, unit region_size */
        uint32_t lb_offset     : 6;  /*!< lock bit offset in all status register */
        uint32_t lb_write_len  : 3;  /*!< lock bit write status register write len */
        uint32_t lb_read_len   : 3;  /*!< lock bit read status register read len */
        uint32_t lb_share      : 1;  /*!< lock bit only one bit, effect for all region */
        uint32_t rsvd          : 24; /*!< reserved 24bit */
    };
} qcc74x_flash_secreg_param_t;

typedef struct {
    uint32_t address;
    uint32_t physical_address;
    uint16_t region_size;
    uint16_t secreg_size;
    uint8_t index;
    uint8_t locked;
    uint8_t lockbit_share;
} qcc74x_flash_secreg_region_info_t;

/*****************************************************************************
* @brief        callback type for iteration over flash security register 
*               regions present on a device
* @param[in]    info        information for current region
* @param[in]    data        private data for callback
* @return       true to continue iteration, false to exit iteration.
* @see          qcc74x_flash_secreg_region_foreach()
* 
*****************************************************************************/
typedef bool (*region_cb)(const qcc74x_flash_secreg_region_info_t *info, void *data);

int qcc74x_flash_secreg_oplock(void);
int qcc74x_flash_secreg_opunlock(void);
int qcc74x_flash_secreg_get_param(const qcc74x_flash_secreg_param_t **param);
int qcc74x_flash_secreg_region_foreach(region_cb cb, void *data);

int qcc74x_flash_secreg_get_lockbits(uint8_t *lockbits);
int qcc74x_flash_secreg_set_lockbits(uint8_t lockbits);
int qcc74x_flash_secreg_get_locked(uint8_t index);
int qcc74x_flash_secreg_set_locked(uint8_t index);

int qcc74x_flash_secreg_get_info(uint32_t addr, qcc74x_flash_secreg_region_info_t *info);
int qcc74x_flash_secreg_valid(uint32_t addr, uint32_t len);
int qcc74x_flash_secreg_read(uint32_t addr, void *data, uint32_t len);
int qcc74x_flash_secreg_write(uint32_t addr, const void *data, uint32_t len);
int qcc74x_flash_secreg_erase(uint32_t addr, uint32_t len);

int qcc74x_flash_secreg_get_info_by_idx(uint8_t index, qcc74x_flash_secreg_region_info_t *info);
int qcc74x_flash_secreg_valid_by_idx(uint8_t index, uint32_t offset, uint32_t len);
int qcc74x_flash_secreg_read_by_idx(uint8_t index, uint32_t offset, void *data, uint32_t len);
int qcc74x_flash_secreg_write_by_idx(uint8_t index, uint32_t offset, const void *data, uint32_t len);
int qcc74x_flash_secreg_erase_by_idx(uint8_t index);

#endif
