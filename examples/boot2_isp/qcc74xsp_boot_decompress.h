#ifndef __QCC74xSP_BOOT_DECOMPRESS_H__
#define __QCC74xSP_BOOT_DECOMPRESS_H__

#include "stdint.h"
#include "partition.h"

int32_t qcc74xsp_boot2_update_fw(pt_table_id_type activeID, pt_table_stuff_config *ptStuff, pt_table_entry_config *ptEntry);
int qcc74xsp_boot2_verify_xz_header(uint8_t *buffer);

#endif /* __QCC74xSP_BOOT_DECOMPRESS_H__ */
