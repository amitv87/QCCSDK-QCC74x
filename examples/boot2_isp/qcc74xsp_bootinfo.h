#ifndef __QCC74xSP_BOOTINFO_H__
#define __QCC74xSP_BOOTINFO_H__

#include "stdint.h"
#include "qcc74xsp_port.h"

#define QCC74x_BOOT2_FLASH_CFG_MAGIC   "FCFG"
#define QCC74x_BOOT2_PLL_CFG_MAGICCODE "PCFG"
#define QCC74x_BOOT2_FLASH_TZC_MAGIC   "TCFG"

#define QCC74x_BOOT2_READBUF_SIZE         (5 * 1024)
#define QCC74x_FW_IMG_OFFSET_AFTER_HEADER HAL_BOOT2_FW_IMG_OFFSET_AFTER_HEADER

/* Image owner type */
#define QCC74x_BOOT2_CPU_0 0
#define QCC74x_BOOT2_CPU_1 1

/* Public key hash size */
#define QCC74x_BOOT2_PK_HASH_SIZE HAL_BOOT2_PK_HASH_SIZE

/* Power save define */
#define QCC74x_PSM_ACTIVE 0
#define QCC74x_PSM_HBN    1

typedef enum {
    QCC74x_BOOT2_SUCCESS = 0x00,

    /* Flash*/
    QCC74x_BOOT2_FLASH_INIT_ERROR = 0x0001,
    QCC74x_BOOT2_FLASH_ERASE_PARA_ERROR = 0x0002,
    QCC74x_BOOT2_FLASH_ERASE_ERROR = 0x0003,
    QCC74x_BOOT2_FLASH_WRITE_PARA_ERROR = 0x0004,
    QCC74x_BOOT2_FLASH_WRITE_ADDR_ERROR = 0x0005,
    QCC74x_BOOT2_FLASH_WRITE_ERROR = 0x0006,
    QCC74x_BOOT2_FLASH_BOOT_PARA = 0x0007,
    QCC74x_BOOT2_FLASH_READ_ERROR = 0x0008,

    /* Image*/
    QCC74x_BOOT2_IMG_BOOTHEADER_LEN_ERROR = 0x0201,
    QCC74x_BOOT2_IMG_BOOTHEADER_NOT_LOAD_ERROR = 0x0202,
    QCC74x_BOOT2_IMG_BOOTHEADER_MAGIC_ERROR = 0x0203,
    QCC74x_BOOT2_IMG_BOOTHEADER_CRC_ERROR = 0x0204,
    QCC74x_BOOT2_IMG_BOOTHEADER_ENCRYPT_NOTFIT = 0x0205,
    QCC74x_BOOT2_IMG_BOOTHEADER_SIGN_NOTFIT = 0x0206,
    QCC74x_BOOT2_IMG_SEGMENT_CNT_ERROR = 0x0207,
    QCC74x_BOOT2_IMG_AES_IV_LEN_ERROR = 0x0208,
    QCC74x_BOOT2_IMG_AES_IV_CRC_ERROR = 0x0209,
    QCC74x_BOOT2_IMG_PK_LEN_ERROR = 0x020a,
    QCC74x_BOOT2_IMG_PK_CRC_ERROR = 0x020b,
    QCC74x_BOOT2_IMG_PK_HASH_ERROR = 0x020c,
    QCC74x_BOOT2_IMG_SIGNATURE_LEN_ERROR = 0x020d,
    QCC74x_BOOT2_IMG_SIGNATURE_CRC_ERROR = 0x020e,
    QCC74x_BOOT2_IMG_SECTIONHEADER_LEN_ERROR = 0x020f,
    QCC74x_BOOT2_IMG_SECTIONHEADER_CRC_ERROR = 0x0210,
    QCC74x_BOOT2_IMG_SECTIONHEADER_DST_ERROR = 0x0211,
    QCC74x_BOOT2_IMG_SECTIONDATA_LEN_ERROR = 0x0212,
    QCC74x_BOOT2_IMG_SECTIONDATA_DEC_ERROR = 0x0213,
    QCC74x_BOOT2_IMG_SECTIONDATA_TLEN_ERROR = 0x0214,
    QCC74x_BOOT2_IMG_SECTIONDATA_CRC_ERROR = 0x0215,
    QCC74x_BOOT2_IMG_HALFBAKED_ERROR = 0x0216,
    QCC74x_BOOT2_IMG_HASH_ERROR = 0x0217,
    QCC74x_BOOT2_IMG_SIGN_PARSE_ERROR = 0x0218,
    QCC74x_BOOT2_IMG_SIGN_ERROR = 0x0219,
    QCC74x_BOOT2_IMG_DEC_ERROR = 0x021a,
    QCC74x_BOOT2_IMG_ALL_INVALID_ERROR = 0x021b,
    QCC74x_BOOT2_IMG_Roll_Back = 0x021c,

    /* MISC*/
    QCC74x_BOOT2_MEM_ERROR = 0xfffb,
    QCC74x_BOOT2_PLL_ERROR = 0xfffc,
    QCC74x_BOOT2_INVASION_ERROR = 0xfffd,
    QCC74x_BOOT2_POLLING = 0xfffe,
    QCC74x_BOOT2_FAIL = 0xffff,

} boot_error_code;

typedef struct
{
    uint8_t aes_iv[16];
    uint32_t crc32;
} boot_aes_config;

typedef struct
{
    uint8_t eckye_x[HAL_BOOT2_ECC_KEYXSIZE]; //ec key in boot header
    uint8_t eckey_y[HAL_BOOT2_ECC_KEYYSIZE]; //ec key in boot header
    uint32_t crc32;
} boot_pk_config;

typedef struct
{
    uint32_t sig_len;
    uint8_t signature[HAL_BOOT2_SIGN_MAXSIZE];
    uint32_t crc32;
} boot_sign_config;

typedef struct
{
    uint32_t msp_store_addr;
    uint32_t pc_store_addr;
    uint32_t default_xip_addr;
} boot_cpu_config;

/**
 *  @brief ram image config type
 */
typedef struct
{
    uint8_t valid;      /*!< valid or not */
    uint8_t core;       /*!< which core to run */
    uint32_t boot_addr; /*!< boot addr */
} ram_img_config_t;

typedef void (*pentry_t)(void);

extern ram_img_config_t ram_img_config[QCC74xSP_BOOT2_RAM_IMG_COUNT_MAX];
extern boot2_image_config g_boot_img_cfg[QCC74xSP_BOOT2_CPU_GROUP_MAX];
extern boot2_efuse_hw_config g_efuse_cfg;
extern uint8_t g_ps_mode;
//extern uint8_t g_cpu_count;
extern uint8_t *g_boot2_read_buf;
extern uint32_t g_no_active_fw_age;

#endif /* __QCC74xSP_BOOTINFO_H__ */
