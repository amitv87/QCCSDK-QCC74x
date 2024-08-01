#include <stdio.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <mem.h>
#include <shell.h>
#include <netbus_uart.h>
#include "at_main.h"
#include "qcc74x_dma.h"

const netbus_uart_config_t uart_config = {
    .name = "uart1",
    .speed = 2000000,
    .databits = 8,
    .stopbits = 1,
    .parity = 0,
    .flow_control =0,
};

netbus_uart_ctx_t at_uart;

void app_atmoudle_init(void)
{
    netbus_uart_init(&at_uart, &uart_config, 1024, 1024);

    at_module_init();
}

