

#ifndef __HARDWARE_ACOMP_H__
#define __HARDWARE_ACOMP_H__

/****************************************************************************
 * Pre-processor Definitions
****************************************************************************/

/* Register offsets *********************************************************/
/* aon base */
#define AON_ACOMP_REG_ACOMP0_CTRL_OFFSET        (0x900) /* acomp0_ctrl */
#define AON_ACOMP_REG_ACOMP1_CTRL_OFFSET        (0x904) /* acomp0_ctrl */
#define AON_ACOMP_REG_ACOMP_CFG_OFFSET          (0x908) /* acomp_ctrl */


/* Register Bitfield definitions *****************************************************/
/* 0x900 : acomp0_ctrl */
/* 0x904 : acomp1_ctrl */
#define AON_ACOMP_MUX_EN           (1 << 26U)
#define AON_ACOMP_POS_SEL_SHIFT    (22U)
#define AON_ACOMP_POS_SEL_MASK     (0xf << AON_ACOMP_POS_SEL_SHIFT)
#define AON_ACOMP_NEG_SEL_SHIFT    (18U)
#define AON_ACOMP_NEG_SEL_MASK     (0xf << AON_ACOMP_NEG_SEL_SHIFT)
#define AON_ACOMP_LEVEL_SEL_SHIFT  (12U)
#define AON_ACOMP_LEVEL_SEL_MASK   (0x3f << AON_ACOMP_LEVEL_SEL_SHIFT)
#define AON_ACOMP_BIAS_PROG_SHIFT  (10U)
#define AON_ACOMP_BIAS_PROG_MASK   (0x3 << AON_ACOMP_BIAS_PROG_SHIFT)
#define AON_ACOMP_HYST_SELP_SHIFT  (7U)
#define AON_ACOMP_HYST_SELP_MASK   (0x7 << AON_ACOMP_HYST_SELP_SHIFT)
#define AON_ACOMP_HYST_SELN_SHIFT  (4U)
#define AON_ACOMP_HYST_SELN_MASK   (0x7 << AON_ACOMP_HYST_SELN_SHIFT)
#define AON_ACOMP_EN               (1 << 0U)

/* 0x908 : acomp_ctrl */
#define AON_ACOMP_VREF_SEL_SHIFT        (24U)
#define AON_ACOMP_VREF_SEL_MASK         (0x3f << AON_ACOMP_VREF_SEL_SHIFT)
#define AON_ACOMP0_OUT_RAW_DATA_SHIFT   (19U)
#define AON_ACOMP0_OUT_RAW_DATA_MASK    (0x1 << AON_ACOMP0_OUT_RAW_DATA_SHIFT)
#define AON_ACOMP1_OUT_RAW_DATA_SHIFT   (17U)
#define AON_ACOMP1_OUT_RAW_DATA_MASK    (0x1 << AON_ACOMP1_OUT_RAW_DATA_SHIFT)
#define AON_ACOMP0_TEST_SEL_SHIFT       (12U)
#define AON_ACOMP0_TEST_SEL_MASK        (0x3 << AON_ACOMP0_TEST_SEL_SHIFT)
#define AON_ACOMP1_TEST_SEL_SHIFT       (10U)
#define AON_ACOMP1_TEST_SEL_MASK        (0x3 << AON_ACOMP1_TEST_SEL_SHIFT)
#define AON_ACOMP0_TEST_EN              (1 << 9U)
#define AON_ACOMP1_TEST_EN              (1 << 8U)
#define AON_ACOMP0_RSTN_ANA             (1 << 1U)
#define AON_ACOMP1_RSTN_ANA             (1 << 0U)
#endif
