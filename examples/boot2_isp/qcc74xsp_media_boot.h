#ifndef __QCC74xSP_MEDIA_BOOT_H__
#define __QCC74xSP_MEDIA_BOOT_H__

#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "qcc74xsp_bootinfo.h"

int32_t qcc74xsp_mediaboot_read(uint32_t addr, uint8_t *data, uint32_t len);
int32_t qcc74xsp_mediaboot_main(uint32_t cpu_boot_header_addr[QCC74xSP_BOOT2_CPU_GROUP_MAX], uint8_t cpu_roll_back[QCC74xSP_BOOT2_CPU_GROUP_MAX], uint8_t roll_back);
void qcc74xsp_boot2_show_timer(void);

#define QCC74xSP_BOOT2_PSRAM_BASE 0x50000000

int32_t qcc74xsp_mediaboot_parse_one_group_xz(boot2_image_config *boot_img_cfg, uint32_t boot_header_addr, uint32_t img_addr, uint8_t *input, uint32_t len, uint8_t last_packet);
int32_t qcc74xsp_mediaboot_version_check(uint8_t *image_start, uint8_t group_roll_back[QCC74xSP_BOOT2_CPU_GROUP_MAX]);
extern uint32_t g_boot2_parse_xz_image_status;
extern uint32_t g_anti_rollback_flag[3];

#define QCC74xSP_APP_VERSION_LINK_OFFSET          (0xC00)
#define QCC74xSP_APP_QCC74x_FLAG_PRE                (0x42464c42)
#define QCC74xSP_APP_VERF_FLAG_PRE                (0x46524556)
#define QCC74xSP_APP_VERSION_MAX                  (128)

#endif /* __QCC74xSP_MEDIA_BOOT_H__ */
