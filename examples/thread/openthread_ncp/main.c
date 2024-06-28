#include <qcc743_glb.h>
#include <qcc74x_clock.h>
#include <qcc74x_gpio.h>
#include <qcc74x_uart.h>
#include "qcc74x_flash.h"

#include <qcc74x_mtd.h>
#if defined (CONFIG_EASYFLASH4)
#include <easyflash.h>
#endif

#if defined(QCC743)
#include "rfparam_adapter.h"
#endif

#include <openthread_port.h>
#include <openthread/ncp.h>

#include "mem.h"
#include "board.h"

extern uint32_t __HeapBase;
extern uint32_t __HeapLimit;
static struct qcc74x_device_s *uart0;

extern void __libc_init_array(void);
extern void log_start(void);

static void system_clock_ncp_init(void)
{
    /* wifipll/audiopll */
    GLB_Power_On_XTAL_And_PLL_CLK(GLB_XTAL_40M, GLB_PLL_WIFIPLL | GLB_PLL_AUPLL);
    GLB_Set_MCU_System_CLK(GLB_MCU_SYS_CLK_TOP_WIFIPLL_320M);

    CPU_Set_MTimer_CLK(ENABLE, QCC74x_MTIMER_SOURCE_CLOCK_MCU_XCLK, Clock_System_Clock_Get(QCC74x_SYSTEM_CLOCK_XCLK) / 1000000 - 1);
}

static void peripheral_clock_ncp_init(void)
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

static void console_ncp_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    qcc74x_gpio_uart_init(gpio, GPIO_PIN_21, GPIO_UART_FUNC_UART0_TX);
    qcc74x_gpio_uart_init(gpio, GPIO_PIN_22, GPIO_UART_FUNC_UART0_RX);

    struct qcc74x_uart_config_s cfg;
    cfg.baudrate = 460800;
    cfg.data_bits = UART_DATA_BITS_8;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.parity = UART_PARITY_NONE;
    cfg.flow_ctrl = 0;
    cfg.tx_fifo_threshold = 7;
    cfg.rx_fifo_threshold = 7;

    uart0 = qcc74x_device_get_by_name("uart0");

    qcc74x_uart_init(uart0, &cfg);
}

static void board_ncp_init(void)
{
    int ret = -1;
    uintptr_t flag;
    size_t heap_len;

    flag = qcc74x_irq_save();
    ret = qcc74x_flash_init();
    configASSERT(ret == 0);

    system_clock_ncp_init();
    peripheral_clock_ncp_init();
    qcc74x_irq_initialize();

    console_ncp_init();

    heap_len = ((size_t)&__HeapLimit - (size_t)&__HeapBase);
    kmem_init((void *)&__HeapBase, heap_len);

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

void otrInitUser(otInstance * instance)
{
    otAppNcpInit((otInstance * )instance);
}

int main(void)
{
    otRadio_opt_t opt;

    board_ncp_init();

    qcc74x_mtd_init();
#if defined (CONFIG_EASYFLASH4)
    easyflash_init();
#endif

    configASSERT((configMAX_PRIORITIES > 4));

#if defined(QCC743)
    /* Init rf */
    if (0 != rfparam_init(0, NULL, 0)) {
        printf("PHY RF init failed!\r\n");
        return 0;
    }
#endif

    __libc_init_array();

    uart0 = qcc74x_device_get_by_name("uart0");
    ot_uart_init(uart0);

    opt.byte = 0;

    opt.bf.isCoexEnable = false;
#if OPENTHREAD_FTD
    opt.bf.isFtd = true;
#endif

#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
    opt.bf.isLinkMetricEnable = true;
#endif
#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    opt.bf.isCSLReceiverEnable = true;
#endif
#if OPENTHREAD_CONFIG_TIME_SYNC_ENABLE
    opt.bf.isTimeSyncEnable = true;
#endif

    otrStart(opt);

    puts("[OS] Starting OS Scheduler...\r\n");
    vTaskStartScheduler();

    while (1) {
    }
}
