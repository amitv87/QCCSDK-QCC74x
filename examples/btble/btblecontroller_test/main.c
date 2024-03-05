#include "shell.h"
#include <FreeRTOS.h>
#include "task.h"
#include "board.h"

#if defined(QCC74x_undef) || defined(QCC74x_undef)
#include "ble_lib_api.h"
#elif defined(QCC743)
#include "btble_lib_api.h"
#include "qcc743_glb.h"
#include "rfparam_adapter.h"
#elif defined(QCC74x_undef)
#include "btble_lib_api.h"
#include "qcc74x_undef_glb.h"
#endif

//#include "btblecontroller_port_uart.h"

static TaskHandle_t app_start_handle;

static void app_start_task(void *pvParameters)
{
    printf("config hci uart\r\n");
    btble_uart_pin_config(1,27,28,29,30);
    btble_uart_init(1);

    // Initialize BLE controller
    #if defined(QCC74x_undef) || defined(QCC74x_undef)
    ble_controller_init(configMAX_PRIORITIES - 1);
    #else
    btble_controller_init(configMAX_PRIORITIES - 1);
    #endif

    vTaskDelete(NULL);
}

int main(void)
{
    board_init();

    configASSERT((configMAX_PRIORITIES > 4));

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
