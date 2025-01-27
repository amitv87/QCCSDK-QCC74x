#ifndef __MSP_QCC743_GPIO_H__
#define __MSP_QCC743_GPIO_H__

typedef enum {
    GLB_GPIO_PIN_0 = 0,
    GLB_GPIO_PIN_1,
    GLB_GPIO_PIN_2,
    GLB_GPIO_PIN_3,
    GLB_GPIO_PIN_4,
    GLB_GPIO_PIN_5,
    GLB_GPIO_PIN_6,
    GLB_GPIO_PIN_7,
    GLB_GPIO_PIN_8,
    GLB_GPIO_PIN_9,
    GLB_GPIO_PIN_10,
    GLB_GPIO_PIN_11,
    GLB_GPIO_PIN_12,
    GLB_GPIO_PIN_13,
    GLB_GPIO_PIN_14,
    GLB_GPIO_PIN_15,
    GLB_GPIO_PIN_16,
    GLB_GPIO_PIN_17,
    GLB_GPIO_PIN_18,
    GLB_GPIO_PIN_19,
    GLB_GPIO_PIN_20,
    GLB_GPIO_PIN_21,
    GLB_GPIO_PIN_22,
    GLB_GPIO_PIN_23,
    GLB_GPIO_PIN_24,
    GLB_GPIO_PIN_25,
    GLB_GPIO_PIN_26,
    GLB_GPIO_PIN_27,
    GLB_GPIO_PIN_28,
    GLB_GPIO_PIN_29,
    GLB_GPIO_PIN_30,
    GLB_GPIO_PIN_31,
    GLB_GPIO_PIN_32,
    GLB_GPIO_PIN_33,
    GLB_GPIO_PIN_34,
    GLB_GPIO_PIN_MAX,
} GLB_GPIO_Type;

#define GPIO_MODE_INPUT            ((uint32_t)0x00000000U) /*!< Input Floating Mode                   */
#define GPIO_MODE_OUTPUT           ((uint32_t)0x00000001U) /*!< Output Push Pull Mode                 */
#define GPIO_MODE_AF               ((uint32_t)0x00000002U) /*!< Alternate function                    */
#define GPIO_MODE_ANALOG           ((uint32_t)0x00000003U) /*!< Analog function                       */
#define GPIO_PULL_UP               ((uint32_t)0x00000000U) /*!< GPIO pull up                          */
#define GPIO_PULL_DOWN             ((uint32_t)0x00000001U) /*!< GPIO pull down                        */
#define GPIO_PULL_NONE             ((uint32_t)0x00000002U) /*!< GPIO no pull up or down               */
#define GPIO_OUTPUT_VALUE_MODE     ((uint8_t)0x00U) /*!< GPIO Output by reg_gpio_x_o Value                             */
#define GPIO_SET_CLR_MODE          ((uint8_t)0x01U) /*!< GPIO Output set by reg_gpio_x_set and clear by reg_gpio_x_clr */
#define GPIO_DMA_OUTPUT_VALUE_MODE ((uint8_t)0x02U) /*!< GPIO Output value by gpio_dma_o                               */
#define GPIO_DMA_SET_CLR_MODE      ((uint8_t)0x03U) /*!< GPIO Outout value by gpio_dma_set/gpio_dma_clr                */

typedef enum {
    GPIO_FUN_SDH = 0,
    GPIO_FUN_SPI = 1,
    GPIO_FUN_FLASH = 2,
    GPIO_FUN_I2S = 3,
    GPIO_FUN_PDM = 4,
    GPIO_FUN_I2C0 = 5,
    GPIO_FUN_I2C1 = 6,
    GPIO_FUN_UART = 7,
    GPIO_FUN_ETHER_MAC = 8,
    GPIO_FUN_CAM = 9,
    GPIO_FUN_ANALOG = 10,
    GPIO_FUN_GPIO = 11,
    GPIO_FUN_SDIO = 12,
    GPIO_FUN_PWM0 = 16,
    GPIO_FUN_MD_JTAG = 17,
    GPIO_FUN_MD_UART = 18,
    GPIO_FUN_MD_PWM = 19,
    GPIO_FUN_MD_SPI = 20,
    GPIO_FUN_MD_I2S = 21,
    GPIO_FUN_DBI_B = 22,
    GPIO_FUN_DBI_C = 23,
    GPIO_FUN_DISP_QSPI = 24,
    GPIO_FUN_AUPWM = 25,
    GPIO_FUN_JTAG = 26,
    GPIO_FUN_PEC = 27,
    GPIO_FUN_CLOCK_OUT = 31,

    GPIO_FUN_CLOCK_OUT_X_CAM_REF_CLK = 0xE0,
    GPIO_FUN_CLOCK_OUT_X_I2S_REF_CLK = 0xE1,
    GPIO_FUN_CLOCK_OUT_0_1_3_NONE = 0xE2,
    GPIO_FUN_CLOCK_OUT_0_1_SOLO_128FS_CLK = 0xE3,
    GPIO_FUN_CLOCK_OUT_2_ANA_XTAL_CLK = 0xE3,
    GPIO_FUN_CLOCK_OUT_2_WIFI_32M_CLK = 0xE3,
    GPIO_FUN_CLOCK_OUT_3_WIFI_48M_CLK = 0xE3,

    GPIO_FUN_UART0_RTS = 0xF0,
    GPIO_FUN_UART0_CTS = 0xF1,
    GPIO_FUN_UART0_TX = 0xF2,
    GPIO_FUN_UART0_RX = 0xF3,
    GPIO_FUN_UART1_RTS = 0xF4,
    GPIO_FUN_UART1_CTS = 0xF5,
    GPIO_FUN_UART1_TX = 0xF6,
    GPIO_FUN_UART1_RX = 0xF7,

    GPIO_FUN_UNUSED = 0xFF,
} GLB_GPIO_FUNC_Type;

typedef struct
{
    uint8_t gpioPin;
    uint8_t gpioFun;
    uint8_t gpioMode;
    uint8_t pullType;
    uint8_t drive;
    uint8_t smtCtrl;
    uint8_t outputMode;
} GLB_GPIO_Cfg_Type;

#endif /*__QCC743_GPIO_H__ */

