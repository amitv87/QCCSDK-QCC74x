#include "qcc74x_sf_ctrl.h"
#include "qcc74x_sf_cfg.h"
#include "qcc74x_sflash.h"
#include "qcc74x_xip_sflash.h"
#include "qcc74x_flash_secreg_port.h"

/** @addtogroup general_flash_secreg_port General
-----------------------------------------------------------------------------
* @{
----------------------------------------------------------------------------*/
static int ATTR_TCM_SECTION sflash_general_read(const spi_flash_cfg_type *flash_cfg, uint32_t address, void *data, uint32_t len)
{
    uint8_t *const flash_ctrl_buf = (uint8_t *)QCC74x_SF_CTRL_BUF_BASE;
    struct __ALIGNED(4) sf_ctrl_cmd_cfg_type flash_cmd;
    uint32_t timeout;
    uint32_t curlen;

    arch_memset4((void *)&flash_cmd, 0, sizeof(flash_cmd) / 4);

    flash_cmd.rw_flag = SF_CTRL_READ;
    flash_cmd.addr_size = 3;
    flash_cmd.dummy_clks = 1;

    while (len) {
        flash_cmd.cmd_buf[0] = (0x48 << 24) | address;

        if (len >= NOR_FLASH_CTRL_BUF_SIZE) {
            curlen = NOR_FLASH_CTRL_BUF_SIZE;
            flash_cmd.nb_data = NOR_FLASH_CTRL_BUF_SIZE;
        } else {
            curlen = len;
            flash_cmd.nb_data = (len + 3) & ~3;
        }

        qcc74x_sf_ctrl_sendcmd(&flash_cmd);

        timeout = SF_CTRL_BUSY_STATE_TIMEOUT;

        while (qcc74x_sf_ctrl_get_busy_state()) {
            timeout--;

            if (!timeout) {
                return -1;
            }
        }

        arch_memcpy_fast(data, flash_ctrl_buf, curlen);

        len -= curlen;
        data += curlen;
        address += curlen;
    };

    return 0;
}

static int ATTR_TCM_SECTION sflash_general_write(const spi_flash_cfg_type *flash_cfg, uint32_t address, const void *data, uint32_t len)
{
    uint8_t *const flash_ctrl_buf = (uint8_t *)QCC74x_SF_CTRL_BUF_BASE;
    struct __ALIGNED(4) sf_ctrl_cmd_cfg_type flash_cmd;
    uint32_t timeout;
    uint32_t curlen;
    int ret;

    arch_memset4((void *)&flash_cmd, 0, sizeof(flash_cmd) / 4);

    flash_cmd.rw_flag = SF_CTRL_WRITE;
    flash_cmd.addr_size = 3;

    while (len) {
        if ((ret = qcc74x_sflash_write_enable((void *)flash_cfg)) != 0) {
            return ret;
        }

        curlen = 256 - address % 256;

        if (curlen > len) {
            curlen = len;
        }

        arch_memcpy_fast(flash_ctrl_buf, data, curlen);
        flash_cmd.cmd_buf[0] = (0x42 << 24) | address;
        flash_cmd.nb_data = curlen;

        qcc74x_sf_ctrl_sendcmd(&flash_cmd);

        len -= curlen;
        data += curlen;
        address += curlen;

        timeout = 0;
        while (qcc74x_sflash_busy((void *)flash_cfg)) {
            arch_delay_us(100);
            timeout++;

            if (timeout > flash_cfg->time_page_pgm * 20) {
                return -1;
            }
        }
    }

    return 0;
}

static int ATTR_TCM_SECTION sflash_general_erase(const spi_flash_cfg_type *flash_cfg, uint32_t address)
{
    struct __ALIGNED(4) sf_ctrl_cmd_cfg_type flash_cmd;
    uint32_t timeout;
    int ret;

    if ((ret = qcc74x_sflash_write_enable((void *)flash_cfg)) != 0) {
        return ret;
    }

    arch_memset4((void *)&flash_cmd, 0, sizeof(flash_cmd) / 4);

    flash_cmd.rw_flag = SF_CTRL_READ;
    flash_cmd.addr_size = 3;
    flash_cmd.cmd_buf[0] = (0x44 << 24) | address;

    qcc74x_sf_ctrl_sendcmd(&flash_cmd);

    timeout = 0;
    while (qcc74x_sflash_busy((void *)flash_cfg)) {
        arch_delay_us(500);
        timeout++;

        if (timeout > flash_cfg->time_e_sector * 3) {
            return -1;
        }
    }

    return 0;
}

static int ATTR_TCM_SECTION sflash_general_get_lock(const spi_flash_cfg_type *flash_cfg, uint8_t lb_count, uint8_t lb_offset, uint8_t *lb)
{
    int ret;
    uint8_t reg_index = lb_offset / 8;
    uint8_t reg_value;
    uint8_t lb_mask;

    if ((ret = qcc74x_sflash_read_reg((void *)flash_cfg, reg_index, &reg_value, 1)) != 0) {
        return ret;
    }

    lb_mask = ~(0xff << lb_count);
    *lb = (reg_value >> (lb_offset % 8)) & lb_mask;

    return 0;
}

static int ATTR_TCM_SECTION sflash_general_set_lock(const spi_flash_cfg_type *flash_cfg, uint8_t lb_count, uint8_t lb_offset, uint8_t lb, uint8_t lb_write_len)
{
    int ret;
    uint8_t lb_cur;
    uint8_t lb_diff;
    uint16_t reg = 0;

    if ((ret = sflash_general_get_lock(flash_cfg, lb_count, lb_offset, &lb_cur)) != 0) {
        return ret;
    }

    lb_diff = lb_cur ^ lb;

    /*!< if no bit to set */
    if (!(lb_diff & ~lb_cur)) {
        return 0;
    }

    /*!< lb_write_len only can be 1 or 2 */
    if (lb_write_len != 1) {
        if ((ret = qcc74x_sflash_read_reg((void *)flash_cfg, 0, ((uint8_t *)&reg) + 0, 1)) != 0) {
            return ret;
        }
        if ((ret = qcc74x_sflash_read_reg((void *)flash_cfg, 1, ((uint8_t *)&reg) + 1, 1)) != 0) {
            return ret;
        }

        reg |= lb << lb_offset;

        if ((ret = qcc74x_sflash_write_enable((void *)flash_cfg)) != 0) {
            return ret;
        }

        if ((ret = qcc74x_sflash_write_reg((void *)flash_cfg, 0, (uint8_t *)&reg, 2))) {
            return ret;
        }
    } else {
        if ((ret = qcc74x_sflash_read_reg((void *)flash_cfg, lb_offset / 8, ((uint8_t *)&reg) + 0, 1)) != 0) {
            return ret;
        }

        reg |= lb << (lb_offset % 8);

        if ((ret = qcc74x_sflash_write_enable((void *)flash_cfg)) != 0) {
            return ret;
        }

        if ((ret = qcc74x_sflash_write_reg((void *)flash_cfg, lb_offset / 8, (uint8_t *)&reg, 1))) {
            return ret;
        }
    }

    if ((ret = sflash_general_get_lock(flash_cfg, lb_count, lb_offset, &lb_cur)) != 0) {
        return ret;
    }

    lb_diff = lb_cur ^ lb;

    /*!< if no bit to set */
    if (!(lb_diff & ~lb_cur)) {
        return 0;
    }

    /*!< set lb failed */
    return -1;
}

/*---------------------------------------------------------------------------
* @}            general_flash_secreg_port Normal
----------------------------------------------------------------------------*/

struct flash_secreg_api flash_secreg_apis[] = {
    [QCC74x_FLASH_SECREG_API_TYPE_GENERAL] = {
        .read = sflash_general_read,
        .write = sflash_general_write,
        .erase = sflash_general_erase,
        .get_lock = sflash_general_get_lock,
        .set_lock = sflash_general_set_lock,
    },
    [QCC74x_FLASH_SECREG_API_TYPE_ISSI] = {
        .read = NULL,
        .write = NULL,
        .erase = NULL,
        .get_lock = NULL,
        .set_lock = NULL,
    },
    [QCC74x_FLASH_SECREG_API_TYPE_MXIC] = {
        .read = NULL,
        .write = NULL,
        .erase = NULL,
        .get_lock = NULL,
        .set_lock = NULL,
    },
};
