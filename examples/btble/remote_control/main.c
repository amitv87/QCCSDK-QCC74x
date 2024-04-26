#include "shell.h"
#include <FreeRTOS.h>
#include "task.h"
#include "board.h"
#include "ble_rc_app.h"
#include "qcc74x_mtd.h"
#include "easyflash.h"

static struct qcc74x_device_s *uart0;

extern void shell_init_with_task(struct qcc74x_device_s *shell);


static TaskHandle_t app_start_handle;

static void app_start_task(void *pvParameters)
{
    ble_stack_start();
    vTaskDelete(NULL);
}

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

    xTaskCreate(app_start_task, (char *)"app_start", 1024, NULL, configMAX_PRIORITIES - 2, &app_start_handle);

    vTaskStartScheduler();

    while (1) {
    }
}
