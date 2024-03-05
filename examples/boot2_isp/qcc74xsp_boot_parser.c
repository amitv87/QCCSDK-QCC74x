#include "qcc74xsp_common.h"
#include "qcc74xsp_bootinfo.h"
#include "qcc74xsp_boot_parser.h"

#include "qcc74xsp_port.h"
#include "qcc74x_sec_sha.h"
#include "qcc74x_sec_ecdsa.h"

extern uint32_t g_user_hash_ignored;
/****************************************************************************/ /**
 * @brief  Check if the input public key is the same as  burned in the efuse
 *
 * @param  cpu_type: CPU Type
 * @param  pkhash: Public key hash pointer
 *
 * @return 1 for valid and 0 for invalid
 *
*******************************************************************************/
static uint32_t qcc74xsp_boot_parse_is_pkhash_valid(uint8_t pk_src, uint8_t *pkhash)
{
    if (pk_src == QCC74x_BOOT2_CPU_0) {
        if (0 == memcmp(g_efuse_cfg.pk_hash_cpu0, pkhash, QCC74x_BOOT2_PK_HASH_SIZE)) {
            return 1;
        }
    } else if (pk_src == QCC74x_BOOT2_CPU_1) {
        if (0 == memcmp(g_efuse_cfg.pk_hash_cpu1, pkhash, QCC74x_BOOT2_PK_HASH_SIZE)) {
            return 1;
        }
    }

    return 0;
}

/****************************************************************************/ /**
 * @brief  Parse public key
 *
 * @param  g_boot_img_cfg: Boot image config pointer
 * @param  data: Public key data pointer
 * @param  own: 1 for current CPU, 0 for other(s)
 *
 * @return boot_error_code type
 *
*******************************************************************************/
int32_t qcc74xsp_boot_parse_pkey(boot2_image_config *g_boot_img_cfg, uint8_t *data, uint8_t own)
{
    boot_pk_config *cfg = (boot_pk_config *)data;
    uint32_t pk_hash[QCC74x_BOOT2_PK_HASH_SIZE / 4];

    if (cfg->crc32 == QCC74x_Soft_CRC32((uint8_t *)cfg, sizeof(boot_pk_config) - 4)) {
        /* Check public key with data info in OTP*/
        qcc74x_sha256_update(sha, &ctx_sha256, data, HAL_BOOT2_ECC_KEYXSIZE + HAL_BOOT2_ECC_KEYYSIZE);
        qcc74x_sha256_finish(sha, &ctx_sha256, (uint8_t *)pk_hash);

        qcc74x_sha_init(sha, SHA_MODE_SHA256);
        qcc74x_sha256_start(sha, &ctx_sha256);
        /* Check pk is valid */
        if (own == 1) {
            if (1 != qcc74xsp_boot_parse_is_pkhash_valid(g_boot_img_cfg->pk_src,
                                                     (uint8_t *)pk_hash)) {
                BOOT2_MSG("PK sha error\r\n");
                return QCC74x_BOOT2_IMG_PK_HASH_ERROR;
            }
        }

        if (own == 1) {
            arch_memcpy_fast(g_boot_img_cfg->eckye_x, cfg->eckye_x, sizeof(cfg->eckye_x));
            arch_memcpy_fast(g_boot_img_cfg->eckey_y, cfg->eckey_y, sizeof(cfg->eckey_y));
        }
    } else {
        BOOT2_MSG("PK crc error\r\n");
        return QCC74x_BOOT2_IMG_PK_CRC_ERROR;
    }

    return QCC74x_BOOT2_SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Parse signature
 *
 * @param  g_boot_img_cfg: Boot image config pointer
 * @param  data: Signature data pointer
 * @param  own: 1 for current CPU, 0 for other(s)
 *
 * @return boot_error_code type
 *
*******************************************************************************/
int32_t qcc74xsp_boot_parse_signature(boot2_image_config *g_boot_img_cfg, uint8_t *data, uint8_t own)
{
    boot_sign_config *cfg = (boot_sign_config *)data;
    uint32_t crc;

    if (cfg->sig_len > sizeof(g_boot_img_cfg->signature)) {
        return QCC74x_BOOT2_IMG_SIGNATURE_LEN_ERROR;
    }

    /* CRC include sig_len*/
    crc = QCC74x_Soft_CRC32((uint8_t *)&cfg->sig_len, cfg->sig_len + sizeof(cfg->sig_len));

    if (memcmp(&crc, &cfg->signature[cfg->sig_len], 4) == 0) {
        if (own == 1) {
            arch_memcpy_fast(g_boot_img_cfg->signature, cfg->signature, cfg->sig_len);
            //g_boot_img_cfg->sig_len = cfg->sig_len;
        } else {
            arch_memcpy_fast(g_boot_img_cfg->signature2, cfg->signature, cfg->sig_len);
            //g_boot_img_cfg->sig_len2 = cfg->sig_len;
        }
    } else {
        BOOT2_MSG("SIG crc error\r\n");
        return QCC74x_BOOT2_IMG_SIGNATURE_CRC_ERROR;
    }

    return QCC74x_BOOT2_SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Parse ASE IV
 *
 * @param  g_boot_img_cfg: Boot image config pointer
 * @param  data: AES IV data pointer
 *
 * @return boot_error_code type
 *
*******************************************************************************/
int32_t qcc74xsp_boot_parse_aesiv(boot2_image_config *g_boot_img_cfg, uint8_t *data)
{
    boot_aes_config *cfg = (boot_aes_config *)data;

    if (cfg->crc32 == QCC74x_Soft_CRC32(cfg->aes_iv, sizeof(cfg->aes_iv))) {
        memcpy(g_boot_img_cfg->aes_iv, cfg->aes_iv, sizeof(boot_aes_config));

        /* Update image hash */
        if (!g_boot_img_cfg->basic_cfg.hash_ignore) {
            //Sec_Eng_SHA256_Update(&g_sha_ctx, SEC_ENG_SHA_ID0, data, sizeof(boot_aes_config));
#ifndef CHIP_QCC74x_undef
            qcc74x_sha256_update(sha, &ctx_sha256, data, sizeof(boot_aes_config));
#endif
        }
    } else {
        BOOT2_MSG("AES IV crc error\r\n");
        return QCC74x_BOOT2_IMG_AES_IV_CRC_ERROR;
    }

    return QCC74x_BOOT2_SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Check signature is valid
 *
 * @param  g_boot_img_cfg: Boot image config pointer
 *
 * @return boot_error_code type
 *
*******************************************************************************/
int32_t qcc74xsp_boot_parser_check_signature(boot2_image_config *g_boot_img_cfg)
{
    int32_t ret = 0;
    uint64_t startTime = 0;
    struct qcc74x_ecdsa_s ecdsa_handle;

    BOOT2_MSG_DBG("ps_mode %d,efuse hbn_check_sign %d\r\n", g_ps_mode, g_efuse_cfg.hbn_check_sign);

    if (g_ps_mode == QCC74x_PSM_HBN && (!g_efuse_cfg.hbn_check_sign)) {
        return QCC74x_BOOT2_SUCCESS;
    }

    if (g_boot_img_cfg->basic_cfg.sign_type) {
        BOOT2_MSG_DBG("Check sig1\r\n");
        startTime = qcc74x_mtimer_get_time_ms();
        qcc74x_sec_ecdsa_init(&ecdsa_handle, ECP_SECP256R1);

        ecdsa_handle.publicKeyx = (uint32_t *)g_boot_img_cfg->eckye_x;
        ecdsa_handle.publicKeyy = (uint32_t *)g_boot_img_cfg->eckey_y;

        ret = qcc74x_sec_ecdsa_verify(&ecdsa_handle, (uint32_t *)g_boot_img_cfg->basic_cfg.hash, 8,
                                    (uint32_t *)g_boot_img_cfg->signature, (uint32_t *)&g_boot_img_cfg->signature[32]);
        if (ret != 0) {
            BOOT2_MSG_DBG("verify failed\r\n");
            return QCC74x_BOOT2_IMG_SIGN_ERROR;
        }

        BOOT2_MSG_DBG("Sign suss,Time=%d ms\r\n", (unsigned int)(qcc74x_mtimer_get_time_ms() - startTime));
    }

    return QCC74x_BOOT2_SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Check hash is valid
 *
 * @param  g_boot_img_cfg: Boot image config pointer
 *
 * @return boot_error_code type
 *
*******************************************************************************/
int32_t qcc74xsp_boot_parser_check_hash(boot2_image_config *g_boot_img_cfg)
{
    uint32_t img_hash_cal[HAL_BOOT2_IMG_HASH_SIZE / 4];

    if (!g_boot_img_cfg->basic_cfg.hash_ignore) {
        //Sec_Eng_SHA256_Finish(&g_sha_ctx, SEC_ENG_SHA_ID0, (uint8_t *)img_hash_cal);
        qcc74x_sha256_finish(sha, &ctx_sha256, (uint8_t *)img_hash_cal);

        if (memcmp(img_hash_cal, g_boot_img_cfg->basic_cfg.hash, 32) != 0) {
            BOOT2_MSG_ERR("Hash error\r\n");
            return QCC74x_BOOT2_IMG_HASH_ERROR;
        } else {
            BOOT2_MSG_DBG("Hash Success\r\n");
        }
    }

    return QCC74x_BOOT2_SUCCESS;
}
