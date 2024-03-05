#ifndef __QCC743_IRQ_H
#define __QCC743_IRQ_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define QCC743_IRQ_NUM_BASE       16

#define QCC743_IRQ_SSOFT          1
#define QCC743_IRQ_MSOFT          3
#define QCC743_IRQ_STIME          5
#define QCC743_IRQ_MTIME          7
#define QCC743_IRQ_SEXT           9
#define QCC743_IRQ_MEXT           11
#define QCC743_IRQ_CLIC_SOFT_PEND 12
#if (__riscv_xlen == 64)
#define QCC743_IRQ_HPM_OVF 17
#endif

#define QCC743_IRQ_BMX_MCU_BUS_ERR                   (QCC743_IRQ_NUM_BASE + 0)
#define QCC743_IRQ_BMX_MCU_TO                        (QCC743_IRQ_NUM_BASE + 1)
#define QCC743_IRQ_DBI                               (QCC743_IRQ_NUM_BASE + 2)
#define QCC743_IRQ_SDU_SOFT_RST                      (QCC743_IRQ_NUM_BASE + 3)
#define QCC743_IRQ_AUDAC                             (QCC743_IRQ_NUM_BASE + 4)
#define QCC743_IRQ_RF_TOP_INT0                       (QCC743_IRQ_NUM_BASE + 5)
#define QCC743_IRQ_RF_TOP_INT1                       (QCC743_IRQ_NUM_BASE + 6)
#define QCC743_IRQ_SDIO                              (QCC743_IRQ_NUM_BASE + 7)
#define QCC743_IRQ_WIFI_TBTT_SLEEP                   (QCC743_IRQ_NUM_BASE + 8)
#define QCC743_IRQ_SEC_ENG_ID1_SHA_AES_TRNG_PKA_GMAC (QCC743_IRQ_NUM_BASE + 9)
#define QCC743_IRQ_SEC_ENG_ID0_SHA_AES_TRNG_PKA_GMAC (QCC743_IRQ_NUM_BASE + 10)
#define QCC743_IRQ_SEC_ENG_ID1_CDET                  (QCC743_IRQ_NUM_BASE + 11)
#define QCC743_IRQ_SEC_ENG_ID0_CDET                  (QCC743_IRQ_NUM_BASE + 12)
#define QCC743_IRQ_SF_CTRL_ID1                       (QCC743_IRQ_NUM_BASE + 13)
#define QCC743_IRQ_SF_CTRL_ID0                       (QCC743_IRQ_NUM_BASE + 14)
#define QCC743_IRQ_DMA0_ALL                          (QCC743_IRQ_NUM_BASE + 15)
#define QCC743_IRQ_DVP2BUS_INT0                      (QCC743_IRQ_NUM_BASE + 16)
#define QCC743_IRQ_SDH                               (QCC743_IRQ_NUM_BASE + 17)
#define QCC743_IRQ_DVP2BUS_INT1                      (QCC743_IRQ_NUM_BASE + 18)
#define QCC743_IRQ_WIFI_TBTT_WAKEUP                  (QCC743_IRQ_NUM_BASE + 19)
#define QCC743_IRQ_IRRX                              (QCC743_IRQ_NUM_BASE + 20)
#define QCC743_IRQ_USB                               (QCC743_IRQ_NUM_BASE + 21)
#define QCC743_IRQ_AUADC                             (QCC743_IRQ_NUM_BASE + 22)
#define QCC743_IRQ_MJPEG                             (QCC743_IRQ_NUM_BASE + 23)
#define QCC743_IRQ_EMAC                              (QCC743_IRQ_NUM_BASE + 24)
#define QCC743_IRQ_GPADC_DMA                         (QCC743_IRQ_NUM_BASE + 25)
#define QCC743_IRQ_EFUSE                             (QCC743_IRQ_NUM_BASE + 26)
#define QCC743_IRQ_SPI0                              (QCC743_IRQ_NUM_BASE + 27)
#define QCC743_IRQ_UART0                             (QCC743_IRQ_NUM_BASE + 28)
#define QCC743_IRQ_UART1                             (QCC743_IRQ_NUM_BASE + 29)
#define QCC743_IRQ_GPIO_DMA                          (QCC743_IRQ_NUM_BASE + 31)
#define QCC743_IRQ_I2C0                              (QCC743_IRQ_NUM_BASE + 32)
#define QCC743_IRQ_PWM                               (QCC743_IRQ_NUM_BASE + 33)
#define QCC743_IRQ_RESERVED0                         (QCC743_IRQ_NUM_BASE + 34)
#define QCC743_IRQ_RESERVED1                         (QCC743_IRQ_NUM_BASE + 35)
#define QCC743_IRQ_TIMER0                            (QCC743_IRQ_NUM_BASE + 36)
#define QCC743_IRQ_TIMER1                            (QCC743_IRQ_NUM_BASE + 37)
#define QCC743_IRQ_WDG                               (QCC743_IRQ_NUM_BASE + 38)
#define QCC743_IRQ_I2C1                              (QCC743_IRQ_NUM_BASE + 39)
#define QCC743_IRQ_I2S                               (QCC743_IRQ_NUM_BASE + 40)
#define QCC743_IRQ_ANA_OCP_OUT_TO_CPU_0              (QCC743_IRQ_NUM_BASE + 41)
#define QCC743_IRQ_ANA_OCP_OUT_TO_CPU_1              (QCC743_IRQ_NUM_BASE + 42)
#define QCC743_IRQ_XTAL_RDY_SCAN                     (QCC743_IRQ_NUM_BASE + 43)
#define QCC743_IRQ_GPIO_INT0                         (QCC743_IRQ_NUM_BASE + 44)
#define QCC743_IRQ_DM                                (QCC743_IRQ_NUM_BASE + 45)
#define QCC743_IRQ_BT                                (QCC743_IRQ_NUM_BASE + 46)
#define QCC743_IRQ_M154_REQ_ACK                      (QCC743_IRQ_NUM_BASE + 47)
#define QCC743_IRQ_M154                              (QCC743_IRQ_NUM_BASE + 48)
#define QCC743_IRQ_M154_AES                          (QCC743_IRQ_NUM_BASE + 49)
#define QCC743_IRQ_PDS_WAKEUP                        (QCC743_IRQ_NUM_BASE + 50)
#define QCC743_IRQ_HBN_OUT0                          (QCC743_IRQ_NUM_BASE + 51)
#define QCC743_IRQ_HBN_OUT1                          (QCC743_IRQ_NUM_BASE + 52)
#define QCC743_IRQ_BOD                               (QCC743_IRQ_NUM_BASE + 53)
#define QCC743_IRQ_WIFI                              (QCC743_IRQ_NUM_BASE + 54)
#define QCC743_IRQ_BZ_PHY_INT                        (QCC743_IRQ_NUM_BASE + 55)
#define QCC743_IRQ_BLE                               (QCC743_IRQ_NUM_BASE + 56)
#define QCC743_IRQ_MAC_INT_TIMER                     (QCC743_IRQ_NUM_BASE + 57)
#define QCC743_IRQ_MAC_INT_MISC                      (QCC743_IRQ_NUM_BASE + 58)
#define QCC743_IRQ_MAC_INT_RX_TRIGGER                (QCC743_IRQ_NUM_BASE + 59)
#define QCC743_IRQ_MAC_INT_TX_TRIGGER                (QCC743_IRQ_NUM_BASE + 60)
#define QCC743_IRQ_MAC_INT_GEN                       (QCC743_IRQ_NUM_BASE + 61)
#define QCC743_IRQ_MAC_INT_PROT_TRIGGER              (QCC743_IRQ_NUM_BASE + 62)
#define QCC743_IRQ_WIFI_IPC                          (QCC743_IRQ_NUM_BASE + 63)

#endif
