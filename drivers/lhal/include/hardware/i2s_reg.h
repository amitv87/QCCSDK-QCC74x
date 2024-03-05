
#ifndef __I2S_REG_H__
#define __I2S_REG_H__

/****************************************************************************
 * Pre-processor Definitions
****************************************************************************/

/* Register offsets *********************************************************/

#define I2S_CONFIG_OFFSET        (0x0)  /* i2s_config */
#define I2S_INT_STS_OFFSET       (0x4)  /* i2s_int_sts */
#define I2S_BCLK_CONFIG_OFFSET   (0x10) /* i2s_bclk_config */
#define I2S_FIFO_CONFIG_0_OFFSET (0x80) /* i2s_fifo_config_0 */
#define I2S_FIFO_CONFIG_1_OFFSET (0x84) /* i2s_fifo_config_1 */
#define I2S_FIFO_WDATA_OFFSET    (0x88) /* i2s_fifo_wdata */
#define I2S_FIFO_RDATA_OFFSET    (0x8C) /* i2s_fifo_rdata */
#define I2S_IO_CONFIG_OFFSET     (0xFC) /* i2s_io_config */

/* Register Bitfield definitions *****************************************************/

/* 0x0 : i2s_config */
#define I2S_CR_I2S_M_EN         (1 << 0U)
#define I2S_CR_I2S_S_EN         (1 << 1U)
#define I2S_CR_I2S_TXD_EN       (1 << 2U)
#define I2S_CR_I2S_RXD_EN       (1 << 3U)
#define I2S_CR_MONO_MODE        (1 << 4U)
#define I2S_CR_MUTE_MODE        (1 << 5U)
#define I2S_CR_FS_1T_MODE       (1 << 6U)
#define I2S_CR_FS_CH_CNT_SHIFT  (7U)
#define I2S_CR_FS_CH_CNT_MASK   (0x3 << I2S_CR_FS_CH_CNT_SHIFT)
#define I2S_CR_FRAME_SIZE_SHIFT (12U)
#define I2S_CR_FRAME_SIZE_MASK  (0x3 << I2S_CR_FRAME_SIZE_SHIFT)
#define I2S_CR_DATA_SIZE_SHIFT  (14U)
#define I2S_CR_DATA_SIZE_MASK   (0x3 << I2S_CR_DATA_SIZE_SHIFT)
#define I2S_CR_I2S_MODE_SHIFT   (16U)
#define I2S_CR_I2S_MODE_MASK    (0x3 << I2S_CR_I2S_MODE_SHIFT)
#define I2S_CR_ENDIAN           (1 << 18U)
#define I2S_CR_MONO_RX_CH       (1 << 19U)
#define I2S_CR_OFS_CNT_SHIFT    (20U)
#define I2S_CR_OFS_CNT_MASK     (0x1f << I2S_CR_OFS_CNT_SHIFT)
#define I2S_CR_OFS_EN           (1 << 25U)

/* 0x4 : i2s_int_sts */
#define I2S_TXF_INT         (1 << 0U)
#define I2S_RXF_INT         (1 << 1U)
#define I2S_FER_INT         (1 << 2U)
#define I2S_CR_I2S_TXF_MASK (1 << 8U)
#define I2S_CR_I2S_RXF_MASK (1 << 9U)
#define I2S_CR_I2S_FER_MASK (1 << 10U)
#define I2S_CR_I2S_TXF_EN   (1 << 24U)
#define I2S_CR_I2S_RXF_EN   (1 << 25U)
#define I2S_CR_I2S_FER_EN   (1 << 26U)

/* 0x10 : i2s_bclk_config */
#define I2S_CR_BCLK_DIV_L_SHIFT (0U)
#define I2S_CR_BCLK_DIV_L_MASK  (0xfff << I2S_CR_BCLK_DIV_L_SHIFT)
#define I2S_CR_BCLK_DIV_H_SHIFT (16U)
#define I2S_CR_BCLK_DIV_H_MASK  (0xfff << I2S_CR_BCLK_DIV_H_SHIFT)

/* 0x80 : i2s_fifo_config_0 */
#define I2S_DMA_TX_EN         (1 << 0U)
#define I2S_DMA_RX_EN         (1 << 1U)
#define I2S_TX_FIFO_CLR       (1 << 2U)
#define I2S_RX_FIFO_CLR       (1 << 3U)
#define I2S_TX_FIFO_OVERFLOW  (1 << 4U)
#define I2S_TX_FIFO_UNDERFLOW (1 << 5U)
#define I2S_RX_FIFO_OVERFLOW  (1 << 6U)
#define I2S_RX_FIFO_UNDERFLOW (1 << 7U)
#define I2S_CR_FIFO_LR_MERGE  (1 << 8U)
#define I2S_CR_FIFO_LR_EXCHG  (1 << 9U)
#define I2S_CR_FIFO_24B_LJ    (1 << 10U)

/* 0x84 : i2s_fifo_config_1 */
#define I2S_TX_FIFO_CNT_SHIFT (0U)
#define I2S_TX_FIFO_CNT_MASK  (0x1f << I2S_TX_FIFO_CNT_SHIFT)
#define I2S_RX_FIFO_CNT_SHIFT (8U)
#define I2S_RX_FIFO_CNT_MASK  (0x1f << I2S_RX_FIFO_CNT_SHIFT)
#define I2S_TX_FIFO_TH_SHIFT  (16U)
#define I2S_TX_FIFO_TH_MASK   (0xf << I2S_TX_FIFO_TH_SHIFT)
#define I2S_RX_FIFO_TH_SHIFT  (24U)
#define I2S_RX_FIFO_TH_MASK   (0xf << I2S_RX_FIFO_TH_SHIFT)

/* 0x88 : i2s_fifo_wdata */
#define I2S_FIFO_WDATA_SHIFT (0U)
#define I2S_FIFO_WDATA_MASK  (0xffffffff << I2S_FIFO_WDATA_SHIFT)

/* 0x8C : i2s_fifo_rdata */
#define I2S_FIFO_RDATA_SHIFT (0U)
#define I2S_FIFO_RDATA_MASK  (0xffffffff << I2S_FIFO_RDATA_SHIFT)

/* 0xFC : i2s_io_config */
#define I2S_CR_I2S_TXD_INV   (1 << 0U)
#define I2S_CR_I2S_RXD_INV   (1 << 1U)
#define I2S_CR_I2S_FS_INV    (1 << 2U)
#define I2S_CR_I2S_BCLK_INV  (1 << 3U)
#define I2S_CR_DEG_CNT_SHIFT (4U)
#define I2S_CR_DEG_CNT_MASK  (0x7 << I2S_CR_DEG_CNT_SHIFT)
#define I2S_CR_DEG_EN        (1 << 7U)

#endif /* __I2S_REG_H__ */
