
#if defined (QCC743)
#include <rfparam_adapter.h>
#endif

#include <qcc74x_mtd.h>
#if defined (CONFIG_EASYFLASH4)
#include <easyflash.h>
#endif

#include <openthread_port.h>
#include <openthread/cli.h>

#include "board.h"

static struct qcc74x_device_s *uart0;

extern void __libc_init_array(void);
#if defined(OT_SERIAL_SHELL)
extern void shell_init_with_task(struct qcc74x_device_s *shell);
#endif

void otrInitUser(otInstance * instance)
{
    otAppCliInit(instance);
}

int main(void)
{
    otRadio_opt_t opt;

    board_init();

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
#if defined(OT_SERIAL_SHELL)
    shell_init_with_task(uart0);
#elif defined (OT_SERIAL_UART)
    ot_uart_init(uart0);
#else
    #error "No serial interface specified."
#endif

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
