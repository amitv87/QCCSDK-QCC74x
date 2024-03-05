
#include "qcc74x_core.h"
#include "qcc74x_kys.h"
#include "qcc74x_clock.h"
#include "qcc74x_l1c.h"
#include "hardware/kys_reg.h"

/**
 * @brief keyscan init
 *
 * @param dev
 * @param config
 */
void qcc74x_kys_init(struct qcc74x_device_s *dev, const struct qcc74x_kys_config_s *config)
{
    uint32_t reg_base;
    uint32_t reg_val;

    reg_base = dev->reg_base;
    /* disable kys */
    reg_val = getreg32(reg_base + KYS_KS_CTRL_OFFSET);
    reg_val &= ~(KYS_KS_EN_MASK);
    putreg32(reg_val, reg_base + KYS_KS_CTRL_OFFSET);

    /* mask all interrupt before init */
    reg_val = getreg32(reg_base + KYS_KS_INT_EN_OFFSET);
#if defined(QCC74x_undef)
    reg_val &= ~(KYS_KS_INT_EN_MASK);
#else
    reg_val &= ~(KYS_KS_DONE_INT_EN_MASK |
                 KYS_KEYFIFO_FULL_INT_EN_MASK |
                 KYS_KEYFIFO_HALF_INT_EN_MASK |
                 KYS_KEYFIFO_QUARTER_INT_EN_MASK |
                 KYS_KEYFIFO_NONEMPTY_INT_EN_MASK |
                 KYS_GHOST_INT_EN_MASK);
#endif

    /* kys init */
    reg_val = getreg32(reg_base + KYS_KS_CTRL_OFFSET);

    reg_val &= ~(KYS_COL_NUM_MASK |
                 KYS_ROW_NUM_MASK |
#if defined(QCC74x_undefL)
                 KYS_FIFO_MODE_MASK |
#endif
                 KYS_RC_EXT_MASK |
                 KYS_GHOST_EN_MASK |
                 KYS_DEG_EN_MASK |
                 KYS_DEG_CNT_MASK);
    reg_val |= ((config->col - 1) << KYS_COL_NUM_SHIFT);
    reg_val |= ((config->row - 1) << KYS_ROW_NUM_SHIFT);
    reg_val |= ((config->idle_duration) << KYS_RC_EXT_SHIFT);
    reg_val |= (config->ghost_en << KYS_GHOST_EN_SHIFT);
    reg_val |= (config->deglitch_en << KYS_DEG_EN_SHIFT);
    reg_val |= (config->deglitch_cnt << KYS_DEG_CNT_SHIFT);
#if defined(QCC74x_undefL)
    reg_val |= (0x1 << KYS_FIFO_MODE_SHIFT);
#endif
    putreg32(reg_val, reg_base + KYS_KS_CTRL_OFFSET);
}

/**
 * @brief keyscan enable
 *
 */
void qcc74x_kys_enable(struct qcc74x_device_s *dev)
{
    uint32_t reg_base;
    uint32_t reg_val;

    reg_base = dev->reg_base;
    reg_val = getreg32(reg_base + KYS_KS_CTRL_OFFSET);
    reg_val |= (0x1 << KYS_KS_EN_SHIFT);
    putreg32(reg_val, reg_base + KYS_KS_CTRL_OFFSET);
}

/**
 * @brief keyscan disable
 *
 */
void qcc74x_kys_disable(struct qcc74x_device_s *dev)
{
    uint32_t reg_base;
    uint32_t reg_val;

    reg_base = dev->reg_base;

    reg_val = getreg32(reg_base + KYS_KS_CTRL_OFFSET);
    reg_val &= ~(KYS_KS_EN_MASK);
    putreg32(reg_val, reg_base + KYS_KS_CTRL_OFFSET);
}

/**
 * @brief keyscan interrupt enable
 *
 * @param dev
 * @param flag interrupt flag @KEYSCAN_INT_EN define
 * @param enable enable or disable interrupt
 */
void qcc74x_kys_int_enable(struct qcc74x_device_s *dev, uint32_t flag, bool enable)
{
    uint32_t reg_base;
    uint32_t reg_val_en;

    reg_base = dev->reg_base;
    reg_val_en = getreg32(reg_base + KYS_KS_INT_EN_OFFSET);

    if (enable) {
        reg_val_en |= (flag);
    } else {
        reg_val_en &= ~(flag);
    }
    putreg32(reg_val_en, reg_base + KYS_KS_INT_EN_OFFSET);
}

/**
 * @brief keyscan interrupt clear flag
 *
 * @param dev
 * @param flag
 */
void qcc74x_kys_int_clear(struct qcc74x_device_s *dev, uint32_t flag)
{
    putreg32(flag, dev->reg_base + KYS_KEYCODE_CLR_OFFSET);
}
/**
 * @brief keyscan get interrupt status
 *
 * @param dev
 * @return uint32_t
 */
uint32_t qcc74x_kys_get_int_status(struct qcc74x_device_s *dev)
{
    uint32_t reg_base;
    uint32_t reg_sts_val, reg_mask_val;

    reg_base = dev->reg_base;
    reg_sts_val = getreg32(reg_base + KYS_KS_INT_STS_OFFSET);
    reg_mask_val = getreg32(reg_base + KYS_KS_INT_EN_OFFSET);

    return (reg_sts_val & (reg_mask_val));
}

#if defined(QCC74x_undefL)
/**
 * @brief get keyscan keycode index in fifo
 *
 * @param dev
 * @param fifo_head
 * @param fifo_tail
 * @param fifo_valid_cnt
 *
 */
void qcc74x_kys_get_fifo_info(struct qcc74x_device_s *dev, uint8_t *fifo_head, uint8_t *fifo_tail, uint8_t *fifo_valid_cnt)
{
    uint32_t reg_base;
    uint32_t reg_val;

    reg_base = dev->reg_base;
    reg_val = getreg32(reg_base + KYS_KEYFIFO_IDX_OFFSET);

    *fifo_head = (uint8_t)(reg_val & 0x7);
    *fifo_tail = (uint8_t)((reg_val & 0x700) >> 8);
    *fifo_valid_cnt = (uint8_t)((reg_val & 0xf0000) >> 16);
}

#endif
/**
 * @brief get keyscan keycode value
 *
 * @param dev
 * @param index
 * @return uint8_t
 */
uint8_t qcc74x_kys_read_keyvalue(struct qcc74x_device_s *dev, uint8_t index)
{
#if defined(QCC74x_undefL)
    return (uint8_t)(getreg32(dev->reg_base + KYS_KEYFIFO_VALUE_OFFSET) & 0xff);
#endif
#if defined(QCC74x_undef)
    return (uint8_t)((getreg32(dev->reg_base + KYS_KEYFIFO_VALUE_OFFSET) >> (index * 8)) & 0xff);
#endif
}
