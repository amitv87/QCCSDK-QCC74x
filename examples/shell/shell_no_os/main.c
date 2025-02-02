#include "qcc74x_mtimer.h"
#include "qcc74x_uart.h"
#include "shell.h"
#include "board.h"

static struct qcc74x_device_s *uart0;

int main(void)
{
    int ch;
    board_init();
    uart0 = qcc74x_device_get_by_name("uart0");
    shell_init();
    while (1) {
        if((ch = qcc74x_uart_getchar(uart0)) != -1)
        {
            shell_handler(ch);
        }
    }
}

int shell_test(int argc, char **argv)
{
    printf("shell test\r\n");
    return 0;
}
SHELL_CMD_EXPORT_ALIAS(shell_test, test, shell test.);
