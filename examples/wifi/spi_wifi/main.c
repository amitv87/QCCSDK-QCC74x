/****************************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "mem.h"

#include <lwip/tcpip.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

#include "export/qcc74x_fw_api.h"
#include "wifi_mgmr_ext.h"
#include "wifi_mgmr.h"

#include "qcc74x_irq.h"
#include "qcc74x_uart.h"
#include "qcc74x_common.h"

#include "qcc743_glb.h"
#include "rfparam_adapter.h"

#include "app_spiwifi.h"
#include "app_pm.h"

#include "board.h"
#include "shell.h"

#define INIT_STACK_SIZE    (2048)
#define TASK_PRIORITY_INIT (16)

static struct qcc74x_device_s *uart0;
extern void shell_init_with_task(struct qcc74x_device_s *shell);

#ifdef CONFIG_ANTI_ROLLBACK
#define COMPILE_TIME __DATE__ " " __TIME__
const char ver_name[4] __attribute__ ((section(".verinfo"))) = "app";
const char git_commit[41] __attribute__ ((section(".verinfo"))) = "";
const char time_info[30] __attribute__ ((section(".verinfo"))) = COMPILE_TIME;

const qcc74xverinf_t app_ver __attribute__ ((section(".qcc74xverinf"))) = {
    .anti_rollback = 0,
    .x = 0,
    .y = 0,
    .z = 0,
    .name = (uint32_t)ver_name,
    .build_time = (uint32_t)time_info,
    .commit_id = (uint32_t)git_commit,
    .rsvd0 = 0,
    .rsvd1 = 0,
};

uint8_t version = 0xFF;
#endif

void app_init_entry(void *param)
{
    app_spiwifi_init();

    vTaskDelete(NULL);
}

int main(void)
{
    board_init();

#ifdef LP_APP
    app_pm_init();
#endif

#ifdef CONFIG_ANTI_ROLLBACK
    if(0 != qcc74x_get_app_version_from_efuse(&version)){
        printf("error! can't read app version\r\n");
        while(1){
        }
    }else{
        printf("app version in efuse is: %d\r\n", version);
    }

    if(app_ver.anti_rollback < version){
        printf("app version in application is: %d, less than app version in efuse, the application should not run up\r\n", app_ver.anti_rollback);
    }else{
        printf("app version in application is: %d, not less than app version in efuse, the application should run up\r\n", app_ver.anti_rollback);
    }

    /* change app version in efuse to app_ver.anti_rollback, default is 0 */
    qcc74x_set_app_version_to_efuse(app_ver.anti_rollback);//be attention! app version in efuse is incremental(from 0 to 128), and cannot be reduced forever

    if(0 != qcc74x_get_app_version_from_efuse(&version)){
        printf("error! can't read app version\r\n");
        while(1){
        }
    }else{
        printf("app version in efuse is: %d\r\n", version);
    }
#endif

    uart0 = qcc74x_device_get_by_name("uart0");
    shell_init_with_task(uart0);

    // UART1 GPIO conflicts with SPI pins, only one can be used
    //board_uartx_gpio_init();
#if CONFIG_SPI_3PIN_MODE_ENABLE
    board_spi0_gpio_3pin_init();
#else
    board_spi0_gpio_init();
#endif 

    xTaskCreate(app_init_entry, (char *)"init", INIT_STACK_SIZE, NULL, TASK_PRIORITY_INIT, NULL);

    vTaskStartScheduler();

    while (1) {
    }
}

