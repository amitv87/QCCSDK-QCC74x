#ifndef __QCC74xSP_COMMON_H__
#define __QCC74xSP_COMMON_H__

#include "stdint.h"

#define QCC74x_BOOT2_XZ_MALLOC_BUF_SIZE 80 * 1024

void qcc74xsp_dump_data(void *datain, int len);
void qcc74xsp_boot2_jump_entry(void);
int32_t qcc74xsp_mediaboot_pre_jump(void);
uint8_t qcc74xsp_boot2_get_feature_flag(void);

extern uint32_t g_anti_rollback_flag[3];
extern uint32_t g_anti_ef_en,g_anti_ef_app_ver;
extern uint8_t g_malloc_buf[QCC74x_BOOT2_XZ_MALLOC_BUF_SIZE];

#endif /* __QCC74xSP_COMMON_H__ */
