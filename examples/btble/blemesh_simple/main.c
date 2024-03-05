#include "shell.h"
#include <FreeRTOS.h>
#include "task.h"
#include "board.h"

#include "bluetooth.h"
#include "conn.h"
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

#include "qcc74x_mtd.h"
#include "easyflash.h"

#include "hci_driver.h"
#include "hci_core.h"

static struct qcc74x_device_s *uart0;

extern void shell_init_with_task(struct qcc74x_device_s *shell);

void bt_enable_cb(int err)
{
    if (!err) {
        bt_addr_le_t bt_addr;
        bt_get_local_public_address(&bt_addr);
        printf("BD_ADDR:(MSB)%02x:%02x:%02x:%02x:%02x:%02x(LSB) \r\n",
            bt_addr.a.val[5], bt_addr.a.val[4], bt_addr.a.val[3], bt_addr.a.val[2], bt_addr.a.val[1], bt_addr.a.val[0]);
        extern int mesh_start(void);
        mesh_start();
    }
}

static TaskHandle_t app_start_handle;

static void app_start_task(void *pvParameters)
{
    // Initialize BLE controller
    #if defined(QCC74x_undef) || defined(QCC74x_undef)
    ble_controller_init(configMAX_PRIORITIES - 1);
    #else
    btble_controller_init(configMAX_PRIORITIES - 1);
    #endif
    // Initialize BLE Host stack
    hci_driver_init();
    bt_enable(bt_enable_cb);

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

extern void cmd_gen_cli_send(int argc, char **argv);
SHELL_CMD_EXPORT_ALIAS(cmd_gen_cli_send, gen_cli_send, Gerneric onoff send);
