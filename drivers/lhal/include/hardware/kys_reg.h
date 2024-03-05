
#ifndef __HARDWARE_KYS_H__
#define __HARDWARE_KYS_H__

/****************************************************************************
 * Pre-processor Definitions
****************************************************************************/

/* Register offsets *********************************************************/
#define KYS_KS_CTRL_OFFSET     (0x0)
#define KYS_KS_INT_EN_OFFSET   (0x10)
#define KYS_KS_INT_STS_OFFSET  (0x14)
#define KYS_KEYCODE_CLR_OFFSET (0x18)
#if defined(qcc74x_undef)
#define KYS_KEYFIFO_VALUE_OFFSET (0x1C)
#else
#define KYS_KEYFIFO_IDX_OFFSET   (0x30)
#define KYS_KEYFIFO_VALUE_OFFSET (0x34)
#endif

/* 0x0 : ks_ctrl */
#define KYS_KS_EN_SHIFT    (0U)
#define KYS_KS_EN_MASK     (0x1 << KYS_KS_EN_SHIFT)
#define KYS_GHOST_EN_SHIFT (2U)
#define KYS_GHOST_EN_MASK  (0x1 << KYS_GHOST_EN_SHIFT)
#define KYS_DEG_EN_SHIFT   (3U)
#define KYS_DEG_EN_MASK    (0x1 << KYS_DEG_EN_SHIFT)
#define KYS_DEG_CNT_SHIFT  (4U)
#define KYS_DEG_CNT_MASK   (0xf << KYS_DEG_CNT_SHIFT)
#define KYS_RC_EXT_SHIFT   (8U)
#define KYS_RC_EXT_MASK    (0x3 << KYS_RC_EXT_SHIFT)
#define KYS_ROW_NUM_SHIFT  (16U)
#define KYS_ROW_NUM_MASK   (0x7 << KYS_ROW_NUM_SHIFT)
#if defined(qcc74x_undef)
#define KYS_COL_NUM_SHIFT (20U)
#define KYS_COL_NUM_MASK  (0x7 << KYS_COL_NUM_SHIFT)
#else
#define KYS_FIFO_MODE_SHIFT (1U)
#define KYS_FIFO_MODE_MASK  (0x1 << KYS_FIFO_MODE_SHIFT)
#define KYS_COL_NUM_SHIFT   (20U)
#define KYS_COL_NUM_MASK    (0x1f << KYS_COL_NUM_SHIFT)
#endif

/* 0x10 : ks_int_en */
#if defined(qcc74x_undef)
#define KYS_KS_INT_EN_SHIFT (0U)
#define KYS_KS_INT_EN_MASK  (0x1 << KYS_KS_INT_EN_SHIFT)
#else
#define KYS_KS_DONE_INT_EN_SHIFT          (7U)
#define KYS_KS_DONE_INT_EN_MASK           (0x1 << KYS_KS_DONE_INT_EN_SHIFT)
#define KYS_KEYFIFO_FULL_INT_EN_SHIFT     (8U)
#define KYS_KEYFIFO_FULL_INT_EN_MASK      (0x1 << KYS_KEYFIFO_FULL_INT_EN_SHIFT)
#define KYS_KEYFIFO_HALF_INT_EN_SHIFT     (9U)
#define KYS_KEYFIFO_HALF_INT_EN_MASK      (0x1 << KYS_KEYFIFO_HALF_INT_EN_SHIFT)
#define KYS_KEYFIFO_QUARTER_INT_EN_SHIFT  (10U)
#define KYS_KEYFIFO_QUARTER_INT_EN_MASK   (0x1 << KYS_KEYFIFO_QUARTER_INT_EN_SHIFT)
#define KYS_KEYFIFO_NONEMPTY_INT_EN_SHIFT (11U)
#define KYS_KEYFIFO_NONEMPTY_INT_EN_MASK  (0x1 << KYS_KEYFIFO_NONEMPTY_INT_EN_SHIFT)
#define KYS_GHOST_INT_EN_SHIFT            (12U)
#define KYS_GHOST_INT_EN_MASK             (0x1 << KYS_GHOST_INT_EN_SHIFT)
#endif

/* 0x14 : ks_int_sts */
#if defined(qcc74x_undef)
#define KYS_KEYCODE_VALID_SHIFT (0U)
#define KYS_KEYCODE_VALID_MASK  (0xf << KYS_KEYCODE_VALID_SHIFT)
#else
#define KYS_KEYCODE_DONE_SHIFT     (7U)
#define KYS_KEYCODE_DONE_MASK      (0x1 << KYS_KEYCODE_DONE_SHIFT)
#define KYS_KEYFIFO_FULL_SHIFT     (8U)
#define KYS_KEYFIFO_FULL_MASK      (0x1 << KYS_KEYFIFO_FULL_SHIFT)
#define KYS_KEYFIFO_HALF_SHIFT     (9U)
#define KYS_KEYFIFO_HALF_MSK       (0x1 << KYS_KEYFIFO_HALF_SHIFT)
#define KYS_KEYFIFO_QUARTER_SHIFT  (10U)
#define KYS_KEYFIFO_QUARTER_MSK    (0x1 << KYS_KEYFIFO_QUARTER_SHIFT)
#define KYS_KEYFIFO_NONEMPTY_SHIFT (11U)
#define KYS_KEYFIFO_NONEMPTY_MSK   (0x1 << KYS_KEYFIFO_NONEMPTY_SHIFT)
#define KYS_GHOST_DET_SHIFT        (12U)
#define KYS_GHOST_DET_MASK         (0x1 << KYS_GHOST_DET_SHIFT)
#endif

/* 0x18 : keycode_clr */
#if defined(qcc74x_undef)
#define KYS_KEYCODE_CLR_SHIFT (0U)
#define KYS_KEYCODE_CLR_MASK  (0xf << KYS_KEYCODE_CLR_SHIFT)
#else
#define KYS_KS_DONE_CLR_SHIFT      (7U)
#define KYS_KS_DONE_CLR_MASK       (0x1 << KYS_KS_DONE_CLR_SHIFT)
#define KYS_KEYFIFO_FULL_CLR_SHIFT (8U)
#define KYS_KEYFIFO_FULL_CLR_MASK  (0x1 << KYS_KEYFIFO_FULL_CLR_SHIFT)
#define KYS_GHOST_CLR_SHIFT        (12U)
#define KYS_GHOST_CLR_MASK         (0x1 << KYS_GHOST_CLR_SHIFT)
#endif

#if defined(qcc74x_undef)
/* 0x1c : keycode value */
#define KYS_KEYCODE0_SHIFT (0U)
#define KYS_KEYCODE0_MASK  (0xff << KYS_KEYCODE0_SHIFT)
#define KYS_KEYCODE1_SHIFT (8U)
#define KYS_KEYCODE1_MASK  (0xff << KYS_KEYCODE1_SHIFT)
#define KYS_KEYCODE2_SHIFT (16U)
#define KYS_KEYCODE2_MASK  (0xff << KYS_KEYCODE2_SHIFT)
#define KYS_KEYCODE3_SHIFT (24U)
#define KYS_KEYCODE3_MASK  (0xff << KYS_KEYCODE3_SHIFT)
#endif

#if defined(qcc74x_undefl)
/* 0x30 : keyfifo_idx */
#define KYS_KEYFIFO_HEAD_SHIFT (0U)
#define KYS_KEYFIFO_HEAD_MASK  (0x7 << KYS_KEYFIFO_HEAD_SHIFT)
#define KYS_KEYFIFO_TAIL_SHIFT (8U)
#define KYS_KEYFIFO_TAIL_MASK  (0x7 << KYS_KEYFIFO_TAIL_SHIFT)

/* 0x34 : keyfifo_value */
#define KYS_KEYFIFO_VALUE_SHIFT (0U)
#define KYS_KEYFIFO_VALUE_MASK  (0xff << KYS_KEYFIFO_VALUE_SHIFT)
#endif

#endif /* __HARDWARE_KYS_H__ */
