#include "xz.h"
#include "qcc74xsp_bootinfo.h"
#include "qcc74xsp_common.h"
#include "qcc74xsp_media_boot.h"
#include "xz_config.h"
#include "partition.h"
#include "qcc74x_flash.h"

#define QCC74x_BOOT2_XZ_WRITE_BUF_SIZE 4 * 1024
#define QCC74x_BOOT2_XZ_READ_BUF_SIZE  4 * 1024

/****************************************************************************/ /**
 * @brief  Decompress XZ Firmware
 *
 * @param  srcAddress: Source address on flash
 * @param  destAddress: Destination address on flash
 * @param  destMaxSize: Destination flash region size for erase
 * @param  pDestSize: Pointer for output destination firmware size
 *
 * @return Decompress result status
 *
*******************************************************************************/
static int32_t qcc74xsp_boot2_fw_decompress(uint32_t src_address, uint32_t dest_address, uint32_t dest_max_size, uint32_t *p_dest_size)
{
    struct xz_buf b;
    struct xz_dec *s;
    enum xz_ret ret;

    int32_t parse_ret;
    boot2_image_config boot_img_cfg;
    g_boot2_parse_xz_image_status = 0;

    *p_dest_size = 0;

    if (dest_max_size > 0) {
        qcc74x_flash_erase(dest_address, dest_max_size);
    }

    xz_crc32_init();
    simple_malloc_init(g_malloc_buf, sizeof(g_malloc_buf));

    /*
    * Support up to 32k dictionary. The actually needed memory
    * is allocated once the headers have been parsed.
    */
    s = xz_dec_init(XZ_PREALLOC, 1 << 15);

    if (s == NULL) {
        return QCC74x_BOOT2_MEM_ERROR;
    }

    b.in = vmalloc(QCC74x_BOOT2_XZ_READ_BUF_SIZE);
    b.in_pos = 0;
    b.in_size = 0;
    b.out = vmalloc(QCC74x_BOOT2_XZ_WRITE_BUF_SIZE);
    b.out_pos = 0;
    b.out_size = QCC74x_BOOT2_XZ_WRITE_BUF_SIZE;

    while (1) {
        if (b.in_pos == b.in_size) {
            BOOT2_MSG_DBG("XZ Feeding\r\n");

            if (QCC74x_BOOT2_SUCCESS != qcc74xsp_mediaboot_read(src_address, (uint8_t *)b.in, QCC74x_BOOT2_XZ_READ_BUF_SIZE)) {
                return QCC74x_BOOT2_FLASH_READ_ERROR;
            }

            b.in_size = QCC74x_BOOT2_XZ_READ_BUF_SIZE;
            b.in_pos = 0;
            src_address += QCC74x_BOOT2_XZ_READ_BUF_SIZE;
        }

        ret = xz_dec_run(s, &b);

        if (b.out_pos == QCC74x_BOOT2_XZ_WRITE_BUF_SIZE) {
            //if (fwrite(out, 1, b.out_pos, stdout) != b.out_pos) {
            //  msg = "Write error\n";
            //  goto error;
            //}'
            BOOT2_MSG_DBG("XZ outputing\r\n");

            boot2_wdt_feed();

            if (dest_max_size > 0) {
                qcc74x_flash_write(dest_address, b.out, QCC74x_BOOT2_XZ_WRITE_BUF_SIZE);
            }

            if (dest_max_size == 0) {
                parse_ret = qcc74xsp_mediaboot_parse_one_group_xz(&boot_img_cfg, 0, 0, b.out, QCC74x_BOOT2_XZ_WRITE_BUF_SIZE, 0);
                if(parse_ret){
                    BOOT2_MSG_DBG("parse err\r\n");
                    goto error;
                }
            }

            dest_address += QCC74x_BOOT2_XZ_WRITE_BUF_SIZE;
            *p_dest_size += QCC74x_BOOT2_XZ_WRITE_BUF_SIZE;
            b.out_pos = 0;
        }

        if (ret == XZ_OK) {
            continue;
        }
        if (b.out_pos == 0) {
            if (dest_max_size == 0) {
                parse_ret = qcc74xsp_mediaboot_parse_one_group_xz(&boot_img_cfg, 0, 0, NULL, 0, 1);
                if(parse_ret){
                    BOOT2_MSG_DBG("parse err\r\n");
                    goto error;
                }
            }
        }
        //if (fwrite(out, 1, b.out_pos, stdout) != b.out_pos
        //  || fclose(stdout)) {
        //  msg = "Write error\n";
        //  goto error;
        //}
        if (b.out_pos > 0) {
            if (dest_max_size > 0) {
                qcc74x_flash_write(dest_address, b.out, b.out_pos);
            }
            if (dest_max_size == 0) {
                parse_ret = qcc74xsp_mediaboot_parse_one_group_xz(&boot_img_cfg, 0, 0, b.out, b.out_pos, 1);
                if (parse_ret) {
                    BOOT2_MSG_DBG("parse err\r\n");
                    goto error;
                }
            }

            dest_address += b.out_pos;
            *p_dest_size += b.out_pos;
        }

        switch (ret) {
            case XZ_STREAM_END:
                xz_dec_end(s);
                return 0;

            case XZ_MEM_ERROR:
                goto error;

            case XZ_MEMLIMIT_ERROR:
                goto error;

            case XZ_FORMAT_ERROR:
                BOOT2_MSG("Not a .xz file\r\n");
                goto error;

            case XZ_OPTIONS_ERROR:
                goto error;

            case XZ_DATA_ERROR:
            case XZ_BUF_ERROR:
                BOOT2_MSG("File is corrupt\r\n");
                goto error;

            default:
                BOOT2_MSG("XZ Bug!\r\n");
                goto error;
        }
    }

error:
    xz_dec_end(s);
    return QCC74x_BOOT2_FAIL;
}

/****************************************************************************/ /**
 * @brief  Update decompressed firmware to flash according to XZ firmware
 *
 * @param  activeID: Active partition table ID
 * @param  ptStuff: Pointer of partition table stuff
 * @param  ptEntry: Pointer of active entry
 *
 * @return XZ firmware update result status
 *
*******************************************************************************/
int32_t qcc74xsp_boot2_update_fw(pt_table_id_type active_id, pt_table_stuff_config *pt_stuff, pt_table_entry_config *pt_entry)
{
    uint8_t active_index = pt_entry->active_index;
    uint32_t new_fw_len;
    int32_t ret;

    BOOT2_MSG_DBG("try to decomp,xz s addr %x,d addr %x\r\n",
          pt_entry->start_address[active_index],
          pt_entry->start_address[!(active_index & 0x01)]);

    /* Try to check Image integrity: try to decompress */
    if (QCC74x_BOOT2_SUCCESS != qcc74xsp_boot2_fw_decompress(pt_entry->start_address[active_index],
                                                       pt_entry->start_address[!(active_index & 0x01)],
                                                       0, &new_fw_len)) {
#ifdef QCC74xSP_BOOT2_ROLLBACK
        /* Decompress fail, try to rollback to old one */
        pt_entry->active_index = !(active_index & 0x01);
        pt_entry->age++;
        ret = pt_table_update_entry((pt_table_id_type)(!active_id), pt_stuff, pt_entry);

        if (ret != PT_ERROR_SUCCESS) {
            return QCC74x_BOOT2_FAIL;
        }
#endif
        return QCC74x_BOOT2_SUCCESS;
    }

    BOOT2_MSG_DBG("get new fw len %d\r\n", new_fw_len);

    if (new_fw_len > pt_entry->max_len[!(active_index & 0x01)]) {
        BOOT2_MSG_DBG("Img is too large,%d->%d\r\n", new_fw_len, pt_entry->max_len[!(active_index & 0x01)]);
#ifdef QCC74xSP_BOOT2_ROLLBACK
        /* Decompress fail, try to rollback to old one */
        pt_entry->active_index = !(active_index & 0x01);
        pt_entry->age++;
        ret = pt_table_update_entry((pt_table_id_type)(!active_id), pt_stuff, pt_entry);

        if (ret != PT_ERROR_SUCCESS) {
            return QCC74x_BOOT2_FAIL;
        }
#endif
        return QCC74x_BOOT2_SUCCESS;
    }

    /* Do decompress */
    if (QCC74x_BOOT2_SUCCESS == qcc74xsp_boot2_fw_decompress(pt_entry->start_address[active_index],
                                                       pt_entry->start_address[!(active_index & 0x01)],
                                                       pt_entry->max_len[!(active_index & 0x01)], &new_fw_len)) {
        pt_entry->active_index = !(active_index & 0x01);
        pt_entry->len = new_fw_len;
        pt_entry->age++;
        ret = pt_table_update_entry((pt_table_id_type)(!active_id), pt_stuff, pt_entry);

        if (ret != PT_ERROR_SUCCESS) {
            return QCC74x_BOOT2_FAIL;
        }
    } else {
        BOOT2_MSG_DBG("XZ Decomp fail\r\n");
        return QCC74x_BOOT2_FAIL;
    }

    BOOT2_MSG_DBG("get new fw len %d\r\n", new_fw_len);
    return QCC74x_BOOT2_SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Check if buffer is XZ header
 *
 * @param  buffer: Buffer of firmware
 *
 * @return 1 for XZ firmware and 0 for not
 *
*******************************************************************************/
int qcc74xsp_boot2_verify_xz_header(uint8_t *buffer)
{
    const uint8_t xz_header[6] = { 0xFD, '7', 'z', 'X', 'Z', 0x00 };

    //const uint8_t xz_header[6] = { 0x42, 0x46, 0x4e, 0x50, 0x01, 0x00 };
    if (buffer) {
        if (!memcmp(buffer, xz_header, 6)) {
            return 1;
        }
    }

    return 0;
}
