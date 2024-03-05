#ifndef __QCC74xSP_BOOT_PARSER_H__
#define __QCC74xSP_BOOT_PARSER_H__

#include "stdint.h"

int32_t qcc74xsp_boot_parse_pkey(boot2_image_config *g_boot_img_cfg, uint8_t *data, uint8_t own);
int32_t qcc74xsp_boot_parse_signature(boot2_image_config *g_boot_img_cfg, uint8_t *data, uint8_t own);
int32_t qcc74xsp_boot_parse_aesiv(boot2_image_config *g_boot_img_cfg, uint8_t *data);
int32_t qcc74xsp_boot_parser_check_signature(boot2_image_config *g_boot_img_cfg);
int32_t qcc74xsp_boot_parser_check_hash(boot2_image_config *g_boot_img_cfg);

#endif /* __QCC74xSP_BOOT_PARSER_H__ */
