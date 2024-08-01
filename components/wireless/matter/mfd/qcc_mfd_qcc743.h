#ifndef __QCC74x_MFD_QCC743_H

#include <stdbool.h>
#include <stdlib.h>
#include <qcc743.h>
#include <qcc74x_boot2.h>
#include <qcc74x_flash.h>
#include <qcc74x_sec_aes.h>
#include <qcc743_romdriver_e907.h>

#define MFD_XIP_BASE QCC743_FLASH_XIP_BASE
#define MFD_XIP_END QCC743_FLASH_XIP_END
#define SF_CTRL_GET_FLASH_IMAGE_OFFSET() qcc74x_sf_ctrl_get_flash_image_offset(0, SF_CTRL_FLASH_BANK0)
#define BOOT2_PARTITION_ADDR_ACTIVE qcc74x_boot2_partition_addr_active
#define MFD_RUNNING_MEMORY_CHECK() 

bool qcc74x_mfd_decrypt(uint8_t *p, uint32_t len, uint8_t *pout, uint32_t *pIv);

#endif
