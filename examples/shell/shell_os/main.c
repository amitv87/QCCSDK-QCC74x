#include "qcc74x_mtimer.h"
#include "qcc74x_uart.h"
#include "shell.h"
#include <FreeRTOS.h>
#include "semphr.h"
#include "board.h"

static struct qcc74x_device_s *uart0;

extern void shell_init_with_task(struct qcc74x_device_s *shell);

int main(void)
{
    board_init();

    configASSERT((configMAX_PRIORITIES > 4));

    uart0 = qcc74x_device_get_by_name("uart0");
    shell_init_with_task(uart0);

    vTaskStartScheduler();

    while (1) {
    }
}

int shell_test(int argc, char **argv)
{
    printf("shell test\r\n");
    return 0;
}
SHELL_CMD_EXPORT_ALIAS(shell_test, test, shell test.);