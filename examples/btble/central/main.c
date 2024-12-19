#include "shell.h"
#include <FreeRTOS.h>
#include "task.h"
#include "board.h"

#if defined(QCC743)
#include "qcc743_glb.h"
#elif defined(QCC74x_undef)
#include "qcc74x_undef_glb.h"
#endif
#include "qcc74x_mtd.h"
#include "easyflash.h"

#include "rfparam_adapter.h"
#include "ble_app.h"

static struct qcc74x_device_s *uart0;


int main(void)
{
    board_init();

    configASSERT((configMAX_PRIORITIES > 4));

    uart0 = qcc74x_device_get_by_name("uart0");
    shell_init_with_task(uart0);

    qcc74x_mtd_init();
    /* ble stack need easyflash kv */
    easyflash_init();

#if defined(QCC743)
    /* Init rf */
    if (0 != rfparam_init(0, NULL, 0)) {
        printf("PHY RF init failed!\r\n");
        return 0;
    }
#endif

    ble_stack_start();

    vTaskStartScheduler();

    while (1) {
    }
}
