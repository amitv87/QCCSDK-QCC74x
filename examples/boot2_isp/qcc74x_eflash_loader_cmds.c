#include "qcc74x_eflash_loader.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"

#include "partition.h"
#include "qcc74x_flash.h"
#include "qcc74x_sec_sha.h"
#include "qcc74xsp_media_boot.h"
#include "qcc74xsp_common.h"
#include "qcc74x_flash.h"
#include "xz_config.h"

#define QCC74x_EFLASH_LOADER_CHECK_LEN 2048
#define QCC74x_EFLASH_MAX_SIZE         2 * 1024 * 1024

// 4bytes、32bit数据大小端转化
#define L2B32(Little) (((Little & 0xff) << 24) | (((Little) & 0xff00) << 8) | (((Little) & 0xff0000) >> 8) | ((Little >> 24) & 0xff))

#if QCC74xSP_BOOT2_SUPPORT_EFLASH_LOADER_RAM
static struct image_cfg_t image_cfg;
static struct bootrom_img_ctrl_t img_ctrl;
static struct segment_header_t segment_hdr;
uint32_t eflash_loader_cmd_ack_buf[16];
/*for qcc74x_undef*/
static int32_t qcc74x_bootrom_cmd_get_bootinfo(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_bootrom_cmd_load_bootheader(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_bootrom_cmd_load_segheader(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_bootrom_cmd_load_segdata(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_bootrom_cmd_check_img(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_bootrom_cmd_run(uint16_t cmd, uint8_t *data, uint16_t len);
static void qcc74x_bootrom_cmd_ack(uint32_t result);
#endif

#if QCC74xSP_BOOT2_SUPPORT_EFLASH_LOADER_FLASH
static uint32_t g_eflash_loader_error = 0;
/* for qcc74x_undef */
static int32_t qcc74x_eflash_loader_cmd_read_jedec_id(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_eflash_loader_cmd_reset(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_eflash_loader_cmd_erase_flash(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_eflash_loader_cmd_write_flash(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_eflash_loader_cmd_read_flash(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_eflash_loader_cmd_readSha_flash(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_eflash_loader_cmd_xip_readSha_flash(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_eflash_loader_cmd_write_flash_check(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_eflash_loader_cmd_set_flash_para(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_eflash_loader_cmd_xip_read_flash_start(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_eflash_loader_cmd_xip_read_flash_finish(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_eflash_loader_cmd_clock_set(uint16_t cmd, uint8_t *data, uint16_t len);
static int32_t qcc74x_eflash_loader_cmd_read_mac_addr(uint16_t cmd, uint8_t *data, uint16_t len);
#endif

#if QCC74xSP_BOOT2_SUPPORT_EFLASH_LOADER_FLASH || QCC74xSP_BOOT2_SUPPORT_EFLASH_LOADER_RAM
static ATTR_TCM_CONST_SECTION const struct eflash_loader_cmd_cfg_t eflash_loader_cmds[] = {
#if QCC74xSP_BOOT2_SUPPORT_EFLASH_LOADER_RAM
    /*for qcc74x_undef*/
    { QCC74x_EFLASH_LOADER_CMD_GET_BOOTINFO, EFLASH_LOADER_CMD_ENABLE, qcc74x_bootrom_cmd_get_bootinfo },
    { QCC74x_EFLASH_LOADER_CMD_LOAD_BOOTHEADER, EFLASH_LOADER_CMD_ENABLE, qcc74x_bootrom_cmd_load_bootheader },
    { QCC74x_EFLASH_LOADER_CMD_LOAD_SEGHEADER, EFLASH_LOADER_CMD_ENABLE, qcc74x_bootrom_cmd_load_segheader },
    { QCC74x_EFLASH_LOADER_CMD_LOAD_SEGDATA, EFLASH_LOADER_CMD_ENABLE, qcc74x_bootrom_cmd_load_segdata },
    { QCC74x_EFLASH_LOADER_CMD_CHECK_IMG, EFLASH_LOADER_CMD_ENABLE, qcc74x_bootrom_cmd_check_img },
    { QCC74x_EFLASH_LOADER_CMD_RUN, EFLASH_LOADER_CMD_ENABLE, qcc74x_bootrom_cmd_run },
#endif

#if QCC74xSP_BOOT2_SUPPORT_EFLASH_LOADER_FLASH
    /* for qcc74x_undef,qcc74x_undef,qcc74x_undefp,qcc74x_undef,qcc74x_undef */
    { QCC74x_EFLASH_LOADER_CMD_RESET, EFLASH_LOADER_CMD_ENABLE, qcc74x_eflash_loader_cmd_reset },
    { QCC74x_EFLASH_LOADER_CMD_FLASH_ERASE, EFLASH_LOADER_CMD_ENABLE, qcc74x_eflash_loader_cmd_erase_flash },
    { QCC74x_EFLASH_LOADER_CMD_FLASH_WRITE, EFLASH_LOADER_CMD_ENABLE, qcc74x_eflash_loader_cmd_write_flash },
    { QCC74x_EFLASH_LOADER_CMD_FLASH_READ, EFLASH_LOADER_CMD_ENABLE, qcc74x_eflash_loader_cmd_read_flash },
    { QCC74x_EFLASH_LOADER_CMD_FLASH_WRITE_CHECK, EFLASH_LOADER_CMD_ENABLE, qcc74x_eflash_loader_cmd_write_flash_check },
    { QCC74x_EFLASH_LOADER_CMD_FLASH_SET_PARA, EFLASH_LOADER_CMD_ENABLE, qcc74x_eflash_loader_cmd_set_flash_para },
    { QCC74x_EFLASH_LOADER_CMD_FLASH_READSHA, EFLASH_LOADER_CMD_ENABLE, qcc74x_eflash_loader_cmd_readSha_flash },
    { QCC74x_EFLASH_LOADER_CMD_FLASH_XIP_READSHA, EFLASH_LOADER_CMD_ENABLE, qcc74x_eflash_loader_cmd_xip_readSha_flash },
    { QCC74x_EFLASH_LOADER_CMD_XIP_READ_START, EFLASH_LOADER_CMD_ENABLE, qcc74x_eflash_loader_cmd_xip_read_flash_start },
    { QCC74x_EFLASH_LOADER_CMD_XIP_READ_FINISH, EFLASH_LOADER_CMD_ENABLE, qcc74x_eflash_loader_cmd_xip_read_flash_finish },
    { QCC74x_EFLASH_LOADER_CMD_FLASH_READ_JEDECID, EFLASH_LOADER_CMD_ENABLE, qcc74x_eflash_loader_cmd_read_jedec_id },
    { QCC74x_EFLASH_LOADER_CMD_CLK_SET, EFLASH_LOADER_CMD_ENABLE, qcc74x_eflash_loader_cmd_clock_set },
    { QCC74x_EFLASH_LOADER_CMD_EFUSE_READ_MAC_ADDR, EFLASH_LOADER_CMD_ENABLE, qcc74x_eflash_loader_cmd_read_mac_addr },
#endif
};
#endif

#if QCC74xSP_BOOT2_SUPPORT_EFLASH_LOADER_RAM
/* ack host with command process result */
static void qcc74x_bootrom_cmd_ack(uint32_t result)
{
    if (result == 0) {
        /*OK*/
        eflash_loader_cmd_ack_buf[0] = QCC74x_BOOTROM_CMD_ACK;
        qcc74x_eflash_loader_if_write((uint32_t *)eflash_loader_cmd_ack_buf, 2);
        return;
    } else {
        /* FL+Error code(2bytes) */
        eflash_loader_cmd_ack_buf[0] = QCC74x_BOOTROM_CMD_NACK | ((result << 16) & 0xffff0000);
        qcc74x_eflash_loader_if_write(eflash_loader_cmd_ack_buf, 4);
    }
}

/* for qcc74x_undef eflash loader */
static int32_t qcc74x_bootrom_cmd_get_bootinfo(uint16_t cmd, uint8_t *data, uint16_t len)
{
    /*OK(2)+len(2)+bootrom version(4)+OTP(16)*/
    uint8_t *bootinfo = (uint8_t *)eflash_loader_cmd_ack_buf;
    uint8_t otp_cfg[20] = { 0x00, 0x00, 0x00, 0x00, 0x03, 0x10, 0xc1, 0x02, 0x0d, 0xd2, 0x1d, 0xcf, 0x0e, 0xb4, 0x18, 0x00, 0x2f, 0xf4, 0xfb, 0x08 };


    eflash_loader_cmd_ack_buf[0] = QCC74x_BOOTROM_CMD_ACK;
    bootinfo[2] = 0x18;
    bootinfo[3] = 00;
    *((uint32_t *)(bootinfo + 4)) = QCC74x_BOOTROM_VERSION;
    memcpy(bootinfo + 8, &otp_cfg, 20);

    qcc74x_eflash_loader_if_write(eflash_loader_cmd_ack_buf, bootinfo[2] + 4);

    return QCC74x_EFLASH_LOADER_SUCCESS;
}

int32_t qcc74x_bootrom_parse_bootheader(uint8_t *data)
{
    struct bootheader_t *header = (struct bootheader_t *)data;
    uint32_t crc;
    uint32_t crcpass = 0;

    if (header->bootcfg.bval.crc_ignore == 1 && header->crc32 == BOOTROM_DEADBEEF_VAL) {
        crcpass = 1;
    } else {
        crc = QCC74x_Soft_CRC32((uint8_t *)header, sizeof(struct bootheader_t) - sizeof(header->crc32));
        if (header->crc32 == crc) {
            crcpass = 1;
        }
    }

    if (crcpass) {
        if (header->bootcfg.bval.notload_in_bootrom) {
            return QCC74x_EFLASH_LOADER_IMG_BOOTHEADER_NOT_LOAD_ERROR;
        }

        /* for boot2, one CPU only */
        /*get which CPU's img it is*/

        if (0 == memcmp((void *)&header->magiccode, "BFNP", sizeof(header->magiccode))) {
            //current_cpu=i;
            img_ctrl.pkhash_src = 0; //boot_cpu_cfg[i].pkhash_src;
        } else {
            return QCC74x_EFLASH_LOADER_IMG_BOOTHEADER_MAGIC_ERROR;
        }

        image_cfg.entrypoint = 0;
        /* set image valid 0 as default */
        image_cfg.img_valid = 0;

        /* deal with pll config */
        //qcc74x_bootrom_clk_set_from_user(&header->clkCfg,1);

        /* encrypt and sign */
        image_cfg.encrypt_type = header->bootcfg.bval.encrypt_type;
        image_cfg.sign_type = header->bootcfg.bval.sign;
        image_cfg.key_sel = header->bootcfg.bval.key_sel;

        /* xip relative */
        image_cfg.no_segment = header->bootcfg.bval.no_segment;
        image_cfg.cache_select = header->bootcfg.bval.cache_select;
        image_cfg.aes_region_lock = header->bootcfg.bval.aes_region_lock;
        image_cfg.halt_ap = header->bootcfg.bval.halt_ap;
        image_cfg.cache_way_disable = header->bootcfg.bval.cache_way_disable;

        image_cfg.hash_ignore = header->bootcfg.bval.hash_ignore;
        /* firmware len*/
        image_cfg.img_segment_info.segment_cnt = header->img_segment_info.segment_cnt;

        /*boot entry and flash offset */
        image_cfg.entrypoint = header->bootentry;
        image_cfg.img_start.flashoffset = header->img_start.flashoffset;

        if (image_cfg.img_segment_info.segment_cnt == 0) {
            return QCC74x_EFLASH_LOADER_IMG_SEGMENT_CNT_ERROR;
        }
        /*clear segment_cnt*/
        img_ctrl.segment_cnt = 0;

    } else {
        return QCC74x_EFLASH_LOADER_IMG_BOOTHEADER_CRC_ERROR;
    }

    return QCC74x_EFLASH_LOADER_SUCCESS;
}

static int32_t qcc74x_bootrom_cmd_load_bootheader(uint16_t cmd, uint8_t *data, uint16_t len)
{
    int32_t ret = QCC74x_EFLASH_LOADER_SUCCESS;


    if (len != sizeof(struct bootheader_t)) {
        ret = QCC74x_EFLASH_LOADER_IMG_BOOTHEADER_LEN_ERROR;
    } else {
        ret = qcc74x_bootrom_parse_bootheader(data);

        if (QCC74x_EFLASH_LOADER_SUCCESS == ret) {
            if (image_cfg.sign_type) {
                img_ctrl.state = BOOTROM_IMG_PK;
            } else if (image_cfg.encrypt_type) {
                img_ctrl.state = BOOTROM_IMG_AESIV;
            } else {
                img_ctrl.state = BOOTROM_IMG_SEGHEADER;
            }
        }
    }

    qcc74x_bootrom_cmd_ack(ret);

    return ret;
}

static int32_t qcc74x_bootrom_is_boot_dst_valid(uint32_t addr, uint32_t len)
{
    return 1;
}

int32_t qcc74x_bootrom_parse_seg_header(uint8_t *data)
{
    struct segment_header_t *hdr;

    hdr = (struct segment_header_t *)data;

    if (hdr->crc32 == QCC74x_Soft_CRC32(hdr, sizeof(struct segment_header_t) - 4)) {
        memcpy(&segment_hdr, hdr, sizeof(struct segment_header_t));
        if (qcc74x_bootrom_is_boot_dst_valid(segment_hdr.destaddr, segment_hdr.len) == 1) {
            return QCC74x_EFLASH_LOADER_SUCCESS;
        } else {
            return QCC74x_EFLASH_LOADER_IMG_SECTIONHEADER_DST_ERROR;
        }
    } else {
        return QCC74x_EFLASH_LOADER_IMG_SECTIONHEADER_CRC_ERROR;
    }
}

static int32_t qcc74x_bootrom_cmd_load_segheader(uint16_t cmd, uint8_t *data, uint16_t len)
{
    int32_t ret = QCC74x_EFLASH_LOADER_SUCCESS;
    uint8_t *segdatainfo = (uint8_t *)eflash_loader_cmd_ack_buf;


    if (img_ctrl.state != BOOTROM_IMG_SEGHEADER) {
        ret = QCC74x_EFLASH_LOADER_CMD_SEQ_ERROR;
    } else {
        if (len != sizeof(struct segment_header_t)) {
            ret = QCC74x_EFLASH_LOADER_IMG_SECTIONHEADER_LEN_ERROR;
        } else {
            memset(&segment_hdr, 0, sizeof(struct segment_header_t));
            img_ctrl.segdata_recv_len = 0;
            ret = qcc74x_bootrom_parse_seg_header(data);
            if (ret == QCC74x_EFLASH_LOADER_SUCCESS) {
                //qcc74x_bootrom_printe("dest=%08x,len=%d\r\n",segment_hdr.destaddr,segment_hdr.len);
                img_ctrl.state = BOOTROM_IMG_SEGDATA;
            }
        }
    }

    /* if segheader is encrypted, then ack segment len*/
    if (ret == QCC74x_EFLASH_LOADER_SUCCESS) {
        /*ack segdata len*/
        eflash_loader_cmd_ack_buf[0] = QCC74x_BOOTROM_CMD_ACK;
        segdatainfo[2] = sizeof(segment_hdr);
        segdatainfo[3] = 0x00;
        memcpy(&segdatainfo[4], (void *)&segment_hdr, sizeof(segment_hdr));
        qcc74x_eflash_loader_if_write(eflash_loader_cmd_ack_buf, segdatainfo[2] + 4);
    } else {
        qcc74x_bootrom_cmd_ack(ret);
    }
    return ret;
}

static int32_t qcc74x_bootrom_cmd_load_segdata(uint16_t cmd, uint8_t *data, uint16_t len)
{
    int32_t ret = QCC74x_EFLASH_LOADER_SUCCESS;


    if (img_ctrl.state != BOOTROM_IMG_SEGDATA) {
        ret = QCC74x_EFLASH_LOADER_CMD_SEQ_ERROR;
        goto finished;
    }

    if (image_cfg.encrypt_type && len % 16 != 0) {
        ret = QCC74x_EFLASH_LOADER_IMG_SECTIONDATA_LEN_ERROR;
        //qcc74x_bootrom_printe("len error,len=%d\r\n",len);
        goto finished;
    }

    if (img_ctrl.segdata_recv_len + len > segment_hdr.len) {
        ret = QCC74x_EFLASH_LOADER_IMG_SECTIONDATA_TLEN_ERROR;
        //qcc74x_bootrom_printe("tlen error,receive=%d,indicator=%d\r\n",
        //img_ctrl.segdata_recv_len+len,segment_hdr.len);
        goto finished;
    }

    /*no encryption,copy directlly */
    arch_memcpy_fast((void *)segment_hdr.destaddr, data, len);

    img_ctrl.segdata_recv_len += len;
    segment_hdr.destaddr += len;

    if (img_ctrl.segdata_recv_len >= segment_hdr.len) {
        /*finish loading one segment*/
        img_ctrl.segment_cnt++;
        if (img_ctrl.segment_cnt == image_cfg.img_segment_info.segment_cnt) {
            img_ctrl.state = BOOTROM_IMG_CHECK;
        } else {
            img_ctrl.state = BOOTROM_IMG_SEGHEADER;
        }
    }

finished:
    qcc74x_bootrom_cmd_ack(ret);

    return ret;
}

static int32_t qcc74x_bootrom_cmd_check_img(uint16_t cmd, uint8_t *data, uint16_t len)
{
    int32_t ret = QCC74x_EFLASH_LOADER_SUCCESS;


    if (img_ctrl.state != BOOTROM_IMG_CHECK) {
        ret = QCC74x_EFLASH_LOADER_IMG_HALFBAKED_ERROR;
        qcc74x_bootrom_cmd_ack(ret);
        return ret;
    }

    /*default,set state to BOOTROM_IMG_BOOTHEADER*/
    img_ctrl.state = BOOTROM_IMG_BOOTHEADER;

    /*finally, check hash and signature*/
    ret = QCC74x_EFLASH_LOADER_SUCCESS; //qcc74x_bootrom_check_hash();
    if (ret == QCC74x_EFLASH_LOADER_SUCCESS) {
        ret = QCC74x_EFLASH_LOADER_SUCCESS; //qcc74x_bootrom_check_signature();
        qcc74x_bootrom_cmd_ack(ret);
        if (ret == QCC74x_EFLASH_LOADER_SUCCESS) {
            img_ctrl.state = BOOTROM_IMG_RUN;
            image_cfg.img_valid = 1;
        }
    } else {
        qcc74x_bootrom_cmd_ack(ret);
    }

    return ret;
}

static void qcc74x_eflash_loader_exit_delay_us(uint32_t cnt)
{
    volatile uint32_t i = (32 / 5) * cnt;
    while (i--)
        ;
}

static void qcc74x_eflash_loader_jump_entry(void)
{
    pentry_t pentry = 0;

    /* deal NP's entry point */
    if (image_cfg.img_valid) {
        pentry = (pentry_t)image_cfg.entrypoint;
        if (pentry != NULL) {
            pentry();
        }
    }

    /*if cann't jump(open jtag only?),stay here */
    while (1) {
        /*use soft delay only */
        qcc74x_eflash_loader_exit_delay_us(100);
    }
}

static int32_t qcc74x_bootrom_cmd_run(uint16_t cmd, uint8_t *data, uint16_t len)
{
    int32_t ret = QCC74x_EFLASH_LOADER_SUCCESS;


    if (img_ctrl.state != BOOTROM_IMG_RUN) {
        ret = QCC74x_EFLASH_LOADER_CMD_SEQ_ERROR;
        qcc74x_bootrom_cmd_ack(ret);
        return ret;
    }

    qcc74x_bootrom_cmd_ack(ret);
    //qcc74x_eflash_loader_usart_if_wait_tx_idle(QCC74x_BOOTROM_IF_TX_IDLE_TIMEOUT);
    qcc74x_mtimer_delay_ms(QCC74x_BOOTROM_IF_TX_IDLE_TIMEOUT);

    /* get msp and pc value */


    if (image_cfg.img_valid) {
        if (image_cfg.entrypoint == 0) {
            if (image_cfg.img_start.ramaddr != 0) {
                image_cfg.entrypoint = image_cfg.img_start.ramaddr;
            } else {
                image_cfg.entrypoint = QCC74xSP_BOOT2_TCM_BASE;
            }
        }
    }

    /*jump to entry */
    qcc74x_eflash_loader_jump_entry();
    return ret;
}

#endif

#if QCC74xSP_BOOT2_SUPPORT_EFLASH_LOADER_FLASH
/* ack host with command process result */
static void ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_ack(uint32_t result)
{
    if (result == 0) {
        /*OK*/
        g_eflash_loader_cmd_ack_buf[0] = QCC74x_EFLASH_LOADER_CMD_ACK;
        qcc74x_eflash_loader_if_write((uint32_t *)g_eflash_loader_cmd_ack_buf, 2);
        return;
    } else {
        /* FL+Error code(2bytes) */
        g_eflash_loader_cmd_ack_buf[0] = QCC74x_EFLASH_LOADER_CMD_NACK | ((result << 16) & 0xffff0000);
        qcc74x_eflash_loader_if_write(g_eflash_loader_cmd_ack_buf, 4);
    }
}

static void ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_response(uint32_t *data, uint32_t len)
{
    qcc74x_eflash_loader_if_write(data, len);
}

static int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_read_jedec_id(uint16_t cmd, uint8_t *data, uint16_t len)
{
    uint32_t ackdata[2];
    uint8_t *tmp_buf;

    ackdata[0] = QCC74x_EFLASH_LOADER_CMD_ACK;
    tmp_buf = (uint8_t *)ackdata;
    /*ack read jedec ID */
    tmp_buf[2] = 4;
    tmp_buf[3] = 0;
    ackdata[1] = qcc74x_flash_get_jedec_id();
    ackdata[1] &= 0x00ffffff;
    ackdata[1] |= 0x80000000;
    ackdata[1] = (ackdata[1]) << 8;
    ackdata[1] |= 0x80;
    ackdata[1] = L2B32(ackdata[1]);
    qcc74x_eflash_loader_if_write((uint32_t *)ackdata, 4 + 4);
    return QCC74x_EFLASH_LOADER_SUCCESS;
}

static int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_reset(uint16_t cmd, uint8_t *data, uint16_t len)
{
    int32_t ret = QCC74x_EFLASH_LOADER_SUCCESS;

    qcc74x_eflash_loader_cmd_ack(ret);
    qcc74x_eflash_loader_if_wait_tx_idle(QCC74x_EFLASH_LOADER_IF_TX_IDLE_TIMEOUT);

    /* add for qcc74x_undef, will impact on boot pin read */
    hal_boot2_set_psmode_status(0x594c440B);

    /* FPGA POR RST NOT work,so add system reset */
    arch_delay_us(10);
    hal_boot2_sw_system_reset();

    return ret;
}

 static int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_erase_flash(uint16_t cmd, uint8_t *data, uint16_t len)
{
    int32_t ret = QCC74x_EFLASH_LOADER_SUCCESS;
    uint32_t startaddr, endaddr;


    if (len != 8) {
        ret = QCC74x_EFLASH_LOADER_FLASH_ERASE_PARA_ERROR;
    } else {
        /*clean write error, since write usually behand erase*/
        g_eflash_loader_error = QCC74x_EFLASH_LOADER_SUCCESS;

        arch_memcpy(&startaddr, data, 4);
        arch_memcpy(&endaddr, data + 4, 4);


        if (SUCCESS != qcc74x_flash_erase(startaddr, endaddr - startaddr + 1)) {
            ret = QCC74x_EFLASH_LOADER_FLASH_ERASE_ERROR;
        }
    }

    qcc74x_eflash_loader_cmd_ack(ret);
    return ret;
}

static int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_write_flash(uint16_t cmd, uint8_t *data, uint16_t len)
{
    int32_t ret = QCC74x_EFLASH_LOADER_SUCCESS;
    uint32_t startaddr, write_len;


    if (len <= 4) {
        ret = QCC74x_EFLASH_LOADER_FLASH_WRITE_PARA_ERROR;
    } else {
        arch_memcpy(&startaddr, data, 4);
        write_len = len - 4;

        if (startaddr < 0xffffffff) {
            if (SUCCESS != qcc74x_flash_write(startaddr, data + 4, write_len)) {
                /*error , response again with error */
                ret = QCC74x_EFLASH_LOADER_FLASH_WRITE_ERROR;
                g_eflash_loader_error = ret;
            } else {
                qcc74x_eflash_loader_cmd_ack(ret);
                return QCC74x_EFLASH_LOADER_SUCCESS;
            }
        } else {
            ret = QCC74x_EFLASH_LOADER_FLASH_WRITE_ADDR_ERROR;
        }

    }

    qcc74x_eflash_loader_cmd_ack(ret);
    return ret;
}

static int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_read_flash(uint16_t cmd, uint8_t *data, uint16_t len)
{
    int32_t ret = QCC74x_EFLASH_LOADER_SUCCESS;
    uint32_t startaddr, read_len;
    uint8_t *ackdata = (uint8_t *)g_boot2_read_buf;

    if (len != 8) {
        ret = QCC74x_EFLASH_LOADER_FLASH_WRITE_PARA_ERROR;
        qcc74x_eflash_loader_cmd_ack(ret);
    } else {
        arch_memcpy(&startaddr, data, 4);
        arch_memcpy(&read_len, data + 4, 4);

        if (read_len > QCC74x_BOOT2_READBUF_SIZE - 4) {
            read_len = QCC74x_BOOT2_READBUF_SIZE - 4;
        }

        ackdata[0] = 'O';
        ackdata[1] = 'K';
        ackdata[2] = read_len & 0xff;
        ackdata[3] = (read_len >> 8) & 0xff;
        qcc74x_flash_read(startaddr, &ackdata[4], read_len);

        qcc74x_eflash_loader_cmd_response((uint32_t *)ackdata, read_len + 4);
    }
    return ret;
}

static int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_xip_read_flash_start(uint16_t cmd, uint8_t *data, uint16_t len)
{
    int32_t ret = QCC74x_EFLASH_LOADER_SUCCESS;

    qcc74x_eflash_loader_cmd_ack(ret);
    return ret;
}

static int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_xip_read_flash_finish(uint16_t cmd, uint8_t *data, uint16_t len)
{
    int32_t ret = QCC74x_EFLASH_LOADER_SUCCESS;

    qcc74x_eflash_loader_cmd_ack(ret);
    return ret;
}

static int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_readSha_flash(uint16_t cmd, uint8_t *data, uint16_t len)
{
    return QCC74x_EFLASH_LOADER_SUCCESS;
}

static int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_xip_readSha_flash(uint16_t cmd, uint8_t *data, uint16_t len)
{
    int32_t ret = QCC74x_EFLASH_LOADER_SUCCESS;
    uint32_t startaddr, read_len;
    //SEC_Eng_SHA256_Ctx sha_ctx;
    //SEC_ENG_SHA_ID_Type shaId = SEC_ENG_SHA_ID0;
    uint16_t sha_len = 32;
    uint8_t ackdata[32 + 4];
    simple_malloc_init(g_malloc_buf, sizeof(g_malloc_buf));
    uint8_t *g_sha_in_buf = vmalloc(QCC74x_EFLASH_LOADER_READBUF_SIZE);
    if (len != 8) {
        ret = QCC74x_EFLASH_LOADER_FLASH_WRITE_PARA_ERROR;
        qcc74x_eflash_loader_cmd_ack(ret);
    } else {
        arch_memcpy(&startaddr, data, 4);
        arch_memcpy(&read_len, data + 4, 4);

        qcc74x_sha_init(sha, SHA_MODE_SHA256);
        qcc74x_sha256_start(sha, &ctx_sha256);

        while (read_len > 0) {
            if (read_len > QCC74x_EFLASH_LOADER_READBUF_SIZE) {
                qcc74xsp_mediaboot_read(startaddr, g_sha_in_buf, QCC74x_EFLASH_LOADER_READBUF_SIZE);
                /*cal sha here*/
                qcc74x_sha256_update(sha, &ctx_sha256, g_sha_in_buf, QCC74x_EFLASH_LOADER_READBUF_SIZE);
                read_len -= QCC74x_EFLASH_LOADER_READBUF_SIZE;
                startaddr += QCC74x_EFLASH_LOADER_READBUF_SIZE;
            } else {
                qcc74xsp_mediaboot_read(startaddr, g_sha_in_buf, read_len);
                /*cal sha here*/
                qcc74x_sha256_update(sha, &ctx_sha256, g_sha_in_buf, read_len);
                read_len -= read_len;
                startaddr += read_len;
            }
        }

        qcc74x_sha256_finish(sha, &ctx_sha256, (uint8_t *)&ackdata[4]);


        sha_len = 32;
        /*ack read data */
        ackdata[0] = QCC74x_EFLASH_LOADER_CMD_ACK & 0xff;
        ackdata[1] = (QCC74x_EFLASH_LOADER_CMD_ACK >> 8) & 0xff;
        ackdata[2] = sha_len & 0xff;
        ackdata[3] = (sha_len >> 8) & 0xff;
        qcc74x_eflash_loader_if_write((uint32_t *)ackdata, sha_len + 4);
    }

    return ret;
}

static int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_write_flash_check(uint16_t cmd, uint8_t *data, uint16_t len)
{

    qcc74x_eflash_loader_cmd_ack(g_eflash_loader_error);

    return QCC74x_EFLASH_LOADER_SUCCESS;
}

static int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_set_flash_para(uint16_t cmd, uint8_t *data, uint16_t len)
{
    qcc74x_eflash_loader_cmd_ack(QCC74x_EFLASH_LOADER_SUCCESS);
    return QCC74x_EFLASH_LOADER_SUCCESS;
}

static int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_read_mac_addr(uint16_t cmd, uint8_t *data, uint16_t len)
{
    uint32_t crc = 0;
    uint8_t ackdata[14];

    ackdata[0] = QCC74x_EFLASH_LOADER_CMD_ACK & 0xff;
    ackdata[1] = (QCC74x_EFLASH_LOADER_CMD_ACK >> 8) & 0xff;
    ackdata[2] = 10;
    ackdata[3] = 0;
    ackdata[4] = ackdata[5] = ackdata[6] = ackdata[7] = ackdata[8] = ackdata[9] = 0x22; /* fake for download */

    crc = QCC74x_Soft_CRC32((uint8_t *)ackdata + 4, 6);
    arch_memcpy(ackdata + 10, (uint8_t *)&crc, 4);
    qcc74x_eflash_loader_if_write((uint32_t *)ackdata, 14);

    return QCC74x_EFLASH_LOADER_SUCCESS;
}

static int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_clock_set(uint16_t cmd, uint8_t *data, uint16_t len)
{
    qcc74x_eflash_loader_cmd_ack(QCC74x_EFLASH_LOADER_SUCCESS);
    return QCC74x_EFLASH_LOADER_SUCCESS;
}

#endif

int32_t ATTR_TCM_SECTION qcc74x_eflash_loader_cmd_process(uint8_t cmdid, uint8_t *data, uint16_t len)
{
    int32_t ret = QCC74x_EFLASH_LOADER_SUCCESS;
#if QCC74xSP_BOOT2_SUPPORT_EFLASH_LOADER_FLASH || QCC74xSP_BOOT2_SUPPORT_EFLASH_LOADER_RAM
    int i = 0;
    for (i = 0; i < sizeof(eflash_loader_cmds) / sizeof(eflash_loader_cmds[0]); i++) {
        if (eflash_loader_cmds[i].cmd == cmdid) {
            if (EFLASH_LOADER_CMD_ENABLE == eflash_loader_cmds[i].enabled && NULL != eflash_loader_cmds[i].cmd_process) {
                ret = eflash_loader_cmds[i].cmd_process(cmdid, data, len);
            } else {
                return QCC74x_EFLASH_LOADER_CMD_ID_ERROR;
            }

            break;
        }
    }
#endif

    return ret;
}
