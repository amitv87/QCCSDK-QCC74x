#ifdef CONFIG_CONSOLE_WO
#include "qcc74x_wo.h"
#else
#include "qcc74x_uart.h"
#endif
#include "qcc74x_gpio.h"
#include "qcc74x_clock.h"
#include "qcc74x_rtc.h"
#include "qcc74x_flash.h"
#include "qcc74x_acomp.h"
#include "qcc74x_efuse.h"
#include "board.h"
#include "qcc743_tzc_sec.h"
#include "qcc743_psram.h"
#include "qcc743_glb.h"

#include "mem.h"

#ifdef CONFIG_BSP_SDH_SDCARD
#include "sdh_sdcard.h"
#endif

extern void log_start(void);

extern uint32_t __HeapBase;
extern uint32_t __HeapLimit;
extern uint32_t __psram_heap_base;
extern uint32_t __psram_limit;

#ifdef CONFIG_CONSOLE_WO
static struct qcc74x_device_s *wo;
#else
static struct qcc74x_device_s *uart0;
#endif

#if (defined(CONFIG_LUA) || defined(CONFIG_QCC74xLOG) || defined(CONFIG_FATFS))
static struct qcc74x_device_s *rtc;
#endif

static void system_clock_init(void)
{
#if 1
    /* wifipll/audiopll */
    GLB_Power_On_XTAL_And_PLL_CLK(GLB_XTAL_40M, GLB_PLL_WIFIPLL | GLB_PLL_AUPLL);
    GLB_Set_MCU_System_CLK(GLB_MCU_SYS_CLK_TOP_WIFIPLL_320M);
#else
    GLB_Set_MCU_System_CLK(GLB_MCU_SYS_CLK_RC32M);
    GLB_Power_On_XTAL_And_PLL_CLK(GLB_XTAL_40M, GLB_PLL_WIFIPLL);
    GLB_Config_AUDIO_PLL_To_384M();
    GLB_Set_MCU_System_CLK(GLB_MCU_SYS_CLK_TOP_AUPLL_DIV1);
    GLB_Set_MCU_System_CLK_Div(0, 3);
#endif
    CPU_Set_MTimer_CLK(ENABLE, QCC74x_MTIMER_SOURCE_CLOCK_MCU_XCLK, Clock_System_Clock_Get(QCC74x_SYSTEM_CLOCK_XCLK) / 1000000 - 1);
}

static void peripheral_clock_init(void)
{
    PERIPHERAL_CLOCK_ADC_DAC_ENABLE();
    PERIPHERAL_CLOCK_SEC_ENABLE();
    PERIPHERAL_CLOCK_DMA0_ENABLE();
    PERIPHERAL_CLOCK_UART0_ENABLE();
    PERIPHERAL_CLOCK_UART1_ENABLE();
    PERIPHERAL_CLOCK_SPI0_ENABLE();
    PERIPHERAL_CLOCK_I2C0_ENABLE();
    PERIPHERAL_CLOCK_PWM0_ENABLE();
    PERIPHERAL_CLOCK_TIMER0_1_WDG_ENABLE();
    PERIPHERAL_CLOCK_IR_ENABLE();
    PERIPHERAL_CLOCK_I2S_ENABLE();
    PERIPHERAL_CLOCK_USB_ENABLE();
    PERIPHERAL_CLOCK_CAN_ENABLE();
    PERIPHERAL_CLOCK_AUDIO_ENABLE();
    PERIPHERAL_CLOCK_CKS_ENABLE();

    GLB_Set_UART_CLK(ENABLE, HBN_UART_CLK_XCLK, 0);
    GLB_Set_SPI_CLK(ENABLE, GLB_SPI_CLK_MCU_MUXPLL_160M, 0);
    GLB_Set_DBI_CLK(ENABLE, GLB_SPI_CLK_MCU_MUXPLL_160M, 0);
    GLB_Set_I2C_CLK(ENABLE, GLB_I2C_CLK_XCLK, 0);
    GLB_Set_ADC_CLK(ENABLE, GLB_ADC_CLK_XCLK, 1);
    GLB_Set_DIG_CLK_Sel(GLB_DIG_CLK_XCLK);
    GLB_Set_DIG_512K_CLK(ENABLE, ENABLE, 0x4E);
    GLB_Set_PWM1_IO_Sel(GLB_PWM1_IO_SINGLE_END);
    GLB_Set_IR_CLK(ENABLE, GLB_IR_CLK_SRC_XCLK, 19);
    GLB_Set_CAM_CLK(ENABLE, GLB_CAM_CLK_WIFIPLL_96M, 3);

    GLB_Set_PKA_CLK_Sel(GLB_PKA_CLK_MCU_MUXPLL_160M);
#ifdef CONFIG_BSP_SDH_SDCARD
    PERIPHERAL_CLOCK_SDH_ENABLE();
    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_EXT_SDH);
#endif

    GLB_Set_USB_CLK_From_WIFIPLL(1);
    GLB_Swap_MCU_SPI_0_MOSI_With_MISO(0);
}

static void qcc74x_init_psram_gpio(void)
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    for (uint8_t i = 0; i < 12; i++) {
        qcc74x_gpio_init(gpio, (41 + i), GPIO_INPUT | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_0);
    }
}

static void psram_winbond_default_init(void)
{
    PSRAM_Ctrl_Cfg_Type default_psram_ctrl_cfg = {
        .vendor = PSRAM_CTRL_VENDOR_WINBOND,
        .ioMode = PSRAM_CTRL_X8_MODE,
        .size = PSRAM_SIZE_4MB,
        .dqs_delay = 0xfff0,
    };

    PSRAM_Winbond_Cfg_Type default_winbond_cfg = {
        .rst = DISABLE,
        .clockType = PSRAM_CLOCK_DIFF,
        .inputPowerDownMode = DISABLE,
        .hybridSleepMode = DISABLE,
        .linear_dis = ENABLE,
        .PASR = PSRAM_PARTIAL_REFRESH_FULL,
        .disDeepPowerDownMode = ENABLE,
        .fixedLatency = DISABLE,
        .brustLen = PSRAM_WINBOND_BURST_LENGTH_64_BYTES,
        .brustType = PSRAM_WRAPPED_BURST,
        .latency = PSRAM_WINBOND_6_CLOCKS_LATENCY,
        .driveStrength = PSRAM_WINBOND_DRIVE_STRENGTH_35_OHMS_FOR_4M_115_OHMS_FOR_8M,
    };

    PSram_Ctrl_Init(PSRAM0_ID, &default_psram_ctrl_cfg);
    // PSram_Ctrl_Winbond_Reset(PSRAM0_ID);
    PSram_Ctrl_Winbond_Write_Reg(PSRAM0_ID, PSRAM_WINBOND_REG_CR0, &default_winbond_cfg);
}

uint32_t board_psram_x8_init(void)
{
    uint16_t reg_read = 0;

    GLB_Set_PSRAMB_CLK_Sel(ENABLE, GLB_PSRAMB_EMI_WIFIPLL_320M, 0);

    qcc74x_init_psram_gpio();

    /* psram init*/
    psram_winbond_default_init();
    /* check psram work or not */
    PSram_Ctrl_Winbond_Read_Reg(PSRAM0_ID, PSRAM_WINBOND_REG_ID0, &reg_read);
    return reg_read;
}

void qcc74x_show_log(void)
{
    printf("\r\n");
    printf("\r\n");
    printf("Build:%s,%s\r\n", __TIME__, __DATE__);
}

static const char* qcc74x_sys_version(const char ***ctx)
{
    extern uint8_t _version_info_section_start;
    extern uint8_t _version_info_section_end;
    const char ** const version_section_start = (const char**)&_version_info_section_start;
    const char ** const version_section_end = (const char**)&_version_info_section_end;
    const char *version_str;

    //init
    if (NULL == (*ctx)) {
        (*ctx) = version_section_start;
    }
    //check the end
    if (version_section_end == (*ctx)) {
        return NULL;
    }
    version_str = (**ctx);
    *ctx = (*ctx) + 1;
    return version_str;
}

void qcc74x_show_component_version(void)
{
    const char **ctx = NULL;
    const char *version_str;

    puts("Version of used components:\r\n");
    while ((version_str = qcc74x_sys_version(&ctx))) {
        puts("\tVersion: ");
        puts(version_str);
        puts("\r\n");
    }
}

void qcc74x_show_flashinfo(void)
{
    spi_flash_cfg_type flashCfg;
    uint8_t *pFlashCfg = NULL;
    uint32_t flashSize = 0;
    uint32_t flashCfgLen = 0;
    uint32_t flashJedecId = 0;

    flashJedecId = qcc74x_flash_get_jedec_id();
    flashSize = qcc74x_flash_get_size();
    qcc74x_flash_get_cfg(&pFlashCfg, &flashCfgLen);
    arch_memcpy((void *)&flashCfg, pFlashCfg, flashCfgLen);
    printf("======== flash cfg ========\r\n");
    printf("flash size 0x%08X\r\n", flashSize);
    printf("jedec id     0x%06X\r\n", flashJedecId);
    printf("mid              0x%02X\r\n", flashCfg.mid);
    printf("iomode           0x%02X\r\n", flashCfg.io_mode);
    printf("clk delay        0x%02X\r\n", flashCfg.clk_delay);
    printf("clk invert       0x%02X\r\n", flashCfg.clk_invert);
    printf("read reg cmd0    0x%02X\r\n", flashCfg.read_reg_cmd[0]);
    printf("read reg cmd1    0x%02X\r\n", flashCfg.read_reg_cmd[1]);
    printf("write reg cmd0   0x%02X\r\n", flashCfg.write_reg_cmd[0]);
    printf("write reg cmd1   0x%02X\r\n", flashCfg.write_reg_cmd[1]);
    printf("qe write len     0x%02X\r\n", flashCfg.qe_write_reg_len);
    printf("cread support    0x%02X\r\n", flashCfg.c_read_support);
    printf("cread code       0x%02X\r\n", flashCfg.c_read_mode);
    printf("burst wrap cmd   0x%02X\r\n", flashCfg.burst_wrap_cmd);
    printf("===========================\r\n");
}

#ifdef CONFIG_CONSOLE_WO
extern void qcc74x_wo_set_console(struct qcc74x_device_s *dev);
#else
extern void qcc74x_uart_set_console(struct qcc74x_device_s *dev);
#endif

static void console_init()
{
#ifdef CONFIG_CONSOLE_WO
    wo = qcc74x_device_get_by_name("wo");
    qcc74x_wo_uart_init(wo, 2000000, GPIO_PIN_21);
    qcc74x_wo_set_console(wo);
#else
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    qcc74x_gpio_uart_init(gpio, GPIO_PIN_21, GPIO_UART_FUNC_UART0_TX);
    qcc74x_gpio_uart_init(gpio, GPIO_PIN_22, GPIO_UART_FUNC_UART0_RX);

    struct qcc74x_uart_config_s cfg = {0};
    cfg.baudrate = 2000000;
    cfg.data_bits = UART_DATA_BITS_8;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.parity = UART_PARITY_NONE;
    cfg.flow_ctrl = 0;
    cfg.tx_fifo_threshold = 7;
    cfg.rx_fifo_threshold = 7;
    cfg.bit_order = UART_LSB_FIRST;

    uart0 = qcc74x_device_get_by_name("uart0");

    qcc74x_uart_init(uart0, &cfg);
    qcc74x_uart_set_console(uart0);
#endif
}

void board_init(void)
{
    int ret = -1;
    uintptr_t flag;
    size_t heap_len;

    flag = qcc74x_irq_save();
#ifndef CONFIG_BOARD_FLASH_INIT_SKIP
    ret = qcc74x_flash_init();
#endif
    system_clock_init();
    peripheral_clock_init();
    qcc74x_irq_initialize();

    console_init();

    qcc74x_show_log();
    qcc74x_show_component_version();

#ifdef CONFIG_PSRAM
    static qcc74x_efuse_device_info_type device_info;

    qcc74x_efuse_get_device_info(&device_info);
    if (device_info.psram_info == 0) {
        printf("This chip has no psram, please disable CONFIG_PSRAM\r\n");
        while (1) {}
    }

    if (QCC743_PSRAM_INIT_DONE == 0) {
        board_psram_x8_init();
        Tzc_Sec_PSRAMB_Access_Release();
    }

    heap_len = ((size_t)&__psram_limit - (size_t)&__psram_heap_base);
    pmem_init((void *)&__psram_heap_base, heap_len);
#endif

    heap_len = ((size_t)&__HeapLimit - (size_t)&__HeapBase);
    kmem_init((void *)&__HeapBase, heap_len);

    if (ret != 0) {
        printf("flash init fail!!!\r\n");
    }
    qcc74x_show_flashinfo();
#ifdef CONFIG_PSRAM
    printf("dynamic memory init success, ocram heap size = %d Kbyte, psram heap size = %d Kbyte\r\n",
           ((size_t)&__HeapLimit - (size_t)&__HeapBase) / 1024,
           ((size_t)&__psram_limit - (size_t)&__psram_heap_base) / 1024);
#else
    printf("dynamic memory init success, ocram heap size = %d Kbyte \r\n", ((size_t)&__HeapLimit - (size_t)&__HeapBase) / 1024);
#endif

    printf("sig1:%08x\r\n", QCC74x_RD_REG(GLB_BASE, GLB_UART_CFG1));
    printf("sig2:%08x\r\n", QCC74x_RD_REG(GLB_BASE, GLB_UART_CFG2));
    printf("cgen1:%08x\r\n", getreg32(QCC74x_GLB_CGEN1_BASE));

    log_start();

#if (defined(CONFIG_LUA) || defined(CONFIG_QCC74xLOG) || defined(CONFIG_FATFS))
    rtc = qcc74x_device_get_by_name("rtc");
#endif
#ifdef CONFIG_MBEDTLS
    extern void qcc74x_sec_mutex_init(void);
    qcc74x_sec_mutex_init();
#endif
    qcc74x_irq_restore(flag);
}

void board_uartx_gpio_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");

    qcc74x_gpio_uart_init(gpio, GPIO_PIN_23, GPIO_UART_FUNC_UART1_TX);
    qcc74x_gpio_uart_init(gpio, GPIO_PIN_24, GPIO_UART_FUNC_UART1_RX);
    qcc74x_gpio_uart_init(gpio, GPIO_PIN_25, GPIO_UART_FUNC_UART1_CTS);
    qcc74x_gpio_uart_init(gpio, GPIO_PIN_26, GPIO_UART_FUNC_UART1_RTS);
}

void board_i2c0_gpio_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    /* I2C0_SDA */
    qcc74x_gpio_init(gpio, GPIO_PIN_11, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* I2C0_SCL */
    qcc74x_gpio_init(gpio, GPIO_PIN_14, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
}

void board_spi0_gpio_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    /* spi cs */
    qcc74x_gpio_init(gpio, GPIO_PIN_12, GPIO_FUNC_SPI0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* spi clk */
    qcc74x_gpio_init(gpio, GPIO_PIN_13, GPIO_FUNC_SPI0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* spi miso */
    qcc74x_gpio_init(gpio, GPIO_PIN_18, GPIO_FUNC_SPI0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* spi mosi */
    qcc74x_gpio_init(gpio, GPIO_PIN_19, GPIO_FUNC_SPI0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
}

void board_pwm_gpio_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    qcc74x_gpio_init(gpio, GPIO_PIN_24, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_25, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_26, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_27, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_28, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_29, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_30, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_31, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
}

void board_adc_gpio_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    /* ADC_CH0 */
    qcc74x_gpio_init(gpio, GPIO_PIN_20, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
    /* ADC_CH1 */
    qcc74x_gpio_init(gpio, GPIO_PIN_19, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
    /* ADC_CH2 */
    qcc74x_gpio_init(gpio, GPIO_PIN_2, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
    /* ADC_CH3 */
    qcc74x_gpio_init(gpio, GPIO_PIN_3, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
    /* ADC_CH4 */
    qcc74x_gpio_init(gpio, GPIO_PIN_14, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
    /* ADC_CH5 */
    qcc74x_gpio_init(gpio, GPIO_PIN_13, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
    /* ADC_CH6 */
    qcc74x_gpio_init(gpio, GPIO_PIN_12, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
    /* ADC_CH7 */
    qcc74x_gpio_init(gpio, GPIO_PIN_10, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
    /* ADC_CH8 */
    qcc74x_gpio_init(gpio, GPIO_PIN_1, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
    /* ADC_CH9 */
    qcc74x_gpio_init(gpio, GPIO_PIN_0, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
    /* ADC_CH10 */
    qcc74x_gpio_init(gpio, GPIO_PIN_27, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
    /* ADC_CH11 */
    qcc74x_gpio_init(gpio, GPIO_PIN_28, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
}

void board_dac_gpio_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    /* DAC_CHA */
    qcc74x_gpio_init(gpio, GPIO_PIN_3, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
    /* DAC_CHB */
    qcc74x_gpio_init(gpio, GPIO_PIN_2, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
}

void board_emac_gpio_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    qcc74x_gpio_init(gpio, GPIO_PIN_25, GPIO_FUNC_EMAC | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_26, GPIO_FUNC_EMAC | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_27, GPIO_FUNC_EMAC | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_28, GPIO_FUNC_EMAC | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_29, GPIO_FUNC_EMAC | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_30, GPIO_FUNC_EMAC | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_31, GPIO_FUNC_EMAC | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_32, GPIO_FUNC_EMAC | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_33, GPIO_FUNC_EMAC | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_34, GPIO_FUNC_EMAC | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
#if defined(QCC743)
    GLB_Config_AUDIO_PLL_To_400M();
    // GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_IP_EMAC);
#endif
}

void board_sdh_gpio_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    qcc74x_gpio_init(gpio, GPIO_PIN_10, GPIO_FUNC_SDH | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
    qcc74x_gpio_init(gpio, GPIO_PIN_11, GPIO_FUNC_SDH | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
    qcc74x_gpio_init(gpio, GPIO_PIN_12, GPIO_FUNC_SDH | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
    qcc74x_gpio_init(gpio, GPIO_PIN_13, GPIO_FUNC_SDH | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
    qcc74x_gpio_init(gpio, GPIO_PIN_14, GPIO_FUNC_SDH | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
    qcc74x_gpio_init(gpio, GPIO_PIN_15, GPIO_FUNC_SDH | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
}

void board_ir_gpio_init(void)
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    qcc74x_gpio_init(gpio, GPIO_PIN_10, GPIO_INPUT | GPIO_SMT_EN | GPIO_DRV_0);
    GLB_IR_RX_GPIO_Sel(GLB_GPIO_PIN_10);
}

void board_dvp_gpio_init(void)
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");

#if 1
    /* DVP0 GPIO init */
    /* I2C GPIO */
    qcc74x_gpio_init(gpio, GPIO_PIN_0, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_1, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);

    /* Power down GPIO */
    qcc74x_gpio_init(gpio, GPIO_PIN_2, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_reset(gpio, GPIO_PIN_2);

    /* Reset GPIO */
    qcc74x_gpio_init(gpio, GPIO_PIN_3, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_reset(gpio, GPIO_PIN_3);
    qcc74x_mtimer_delay_ms(10);
    qcc74x_gpio_set(gpio, GPIO_PIN_3);
    qcc74x_mtimer_delay_ms(10);

    /* MCLK GPIO */
    qcc74x_gpio_init(gpio, GPIO_PIN_23, GPIO_FUNC_CLKOUT | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);

    /* DVP0 GPIO */
    qcc74x_gpio_init(gpio, GPIO_PIN_24, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_25, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_26, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_27, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_28, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_29, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_30, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_31, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_32, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_33, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_34, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
#else
    /* DVP1 GPIO init */
    /* I2C GPIO */
    qcc74x_gpio_init(gpio, GPIO_PIN_28, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_27, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);

    /* Power down GPIO */
    qcc74x_gpio_init(gpio, GPIO_PIN_29, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_reset(gpio, GPIO_PIN_29);

    /* Reset GPIO */
    qcc74x_gpio_init(gpio, GPIO_PIN_30, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_reset(gpio, GPIO_PIN_3);
    qcc74x_mtimer_delay_ms(10);
    qcc74x_gpio_set(gpio, GPIO_PIN_3);
    qcc74x_mtimer_delay_ms(10);

    /* MCLK GPIO */
    qcc74x_gpio_init(gpio, GPIO_PIN_20, GPIO_FUNC_CLKOUT | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);

    /* DVP1 GPIO */
    qcc74x_gpio_init(gpio, GPIO_PIN_0, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_1, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_3, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_10, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_11, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_12, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_13, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_14, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_15, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_16, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_17, GPIO_FUNC_CAM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
#endif
}

void board_i2s_gpio_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    qcc74x_gpio_init(gpio, GPIO_PIN_16, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_17, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_18, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_init(gpio, GPIO_PIN_19, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
}

void board_acomp_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");

    qcc74x_gpio_init(gpio, GPIO_PIN_13, GPIO_ANALOG | GPIO_PULL_NONE | GPIO_DRV_0);
    qcc74x_gpio_init(gpio, GPIO_PIN_14, GPIO_ANALOG | GPIO_PULL_NONE | GPIO_DRV_0);

    struct qcc74x_acomp_config_s acomp_cfg = {
        .mux_en = ENABLE,
        .pos_chan_sel = AON_ACOMP_CHAN_ADC0,
        .neg_chan_sel = AON_ACOMP_CHAN_VIO_X_SCALING_FACTOR_1,
        .vio_sel = DEFAULT_ACOMP_VREF_1V65,
        .scaling_factor = AON_ACOMP_SCALING_FACTOR_1,
        .bias_prog = AON_ACOMP_BIAS_POWER_MODE1,
        .hysteresis_pos_volt = AON_ACOMP_HYSTERESIS_VOLT_NONE,
        .hysteresis_neg_volt = AON_ACOMP_HYSTERESIS_VOLT_NONE,
    };

    acomp_cfg.pos_chan_sel = AON_ACOMP_CHAN_ADC5;
    qcc74x_acomp_init(AON_ACOMP0_ID, &acomp_cfg);
    acomp_cfg.pos_chan_sel = AON_ACOMP_CHAN_ADC4;
    qcc74x_acomp_init(AON_ACOMP1_ID, &acomp_cfg);
}

#ifdef CONFIG_QCC74xLOG
__attribute__((weak)) uint64_t qcc74xlog_clock(void)
{
    return qcc74x_mtimer_get_time_us();
}

__attribute__((weak)) uint32_t qcc74xlog_time(void)
{
    return QCC74x_RTC_TIME2SEC(qcc74x_rtc_get_time(rtc));
}

__attribute__((weak)) char *qcc74xlog_thread(void)
{
    return "";
}
#endif

#ifdef CONFIG_LUA
__attribute__((weak)) clock_t luaport_clock(void)
{
    return (clock_t)qcc74x_mtimer_get_time_us();
}

__attribute__((weak)) time_t luaport_time(time_t *seconds)
{
    time_t t = (time_t)QCC74x_RTC_TIME2SEC(qcc74x_rtc_get_time(rtc));
    if (seconds != NULL) {
        *seconds = t;
    }

    return t;
}
#endif

#ifdef CONFIG_FATFS
#include "qcc74x_timestamp.h"
__attribute__((weak)) uint32_t get_fattime(void)
{
    qcc74x_timestamp_t tm;

    qcc74x_timestamp_utc2time(QCC74x_RTC_TIME2SEC(qcc74x_rtc_get_time(rtc)), &tm);

    return ((uint32_t)(tm.year - 1980) << 25) /* Year 2015 */
           | ((uint32_t)tm.mon << 21)         /* Month 1 */
           | ((uint32_t)tm.mday << 16)        /* Mday 1 */
           | ((uint32_t)tm.hour << 11)        /* Hour 0 */
           | ((uint32_t)tm.min << 5)          /* Min 0 */
           | ((uint32_t)tm.sec >> 1);         /* Sec 0 */
}
#endif

#ifdef CONFIG_SHELL
#include "shell.h"

static void reboot_cmd(int argc, char **argv)
{
    GLB_SW_POR_Reset();
}

static void show_sys_versoin_cmd(int argc, char **argv)
{
    qcc74x_show_component_version();
}
SHELL_CMD_EXPORT_ALIAS(reboot_cmd, reboot, reboot);
SHELL_CMD_EXPORT_ALIAS(show_sys_versoin_cmd, sysver, show sys version);

#define MFG_CONFIG_REG (0x2000F100)
#define MFG_CONFIG_VAL ("0mfg")

void mfg_config(void)
{
    union _reg_t {
        uint8_t byte[4];
        uint32_t word;
    } mfg = {
        .byte = MFG_CONFIG_VAL,
    };

    *(volatile uint32_t *)(MFG_CONFIG_REG) = mfg.word;
}

static void mfg_cmd(int argc, char **argv)
{
    mfg_config();
    GLB_SW_POR_Reset();
}
SHELL_CMD_EXPORT_ALIAS(mfg_cmd, mfg, mfg);

#endif
