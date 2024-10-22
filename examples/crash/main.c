#include <assert.h>
#include "qcc74x_mtimer.h"
#include "qcc74x_uart.h"
#include "shell.h"
#include <FreeRTOS.h>
#include "semphr.h"
#include "board.h"

#include "qcc74x_mtd.h"
#include "coredump.h"

static struct qcc74x_device_s *uart0;

extern void shell_init_with_task(struct qcc74x_device_s *shell);

int main(void)
{
    qcc74x_mtd_info_t info;
    qcc74x_mtd_handle_t handle;
    int ret;

    board_init();

    qcc74x_mtd_init();
    ret = qcc74x_mtd_open("core", &handle, QCC74x_MTD_OPEN_FLAG_BUSADDR);
    if (ret < 0) {
        puts("No valid coredump partition found\r\n");
    }
    memset(&info, 0, sizeof(info));
    qcc74x_mtd_info(handle, &info);
    printf("Found Valid coredump partition, XIP Addr %08x, flash addr %08x, size %d\r\n",
           info.xip_addr,
           info.offset,
           info.size);
    core_partition_init(info.offset, info.size);

    uart0 = qcc74x_device_get_by_name("uart0");
    shell_init_with_task(uart0);

    vTaskStartScheduler();

    while (1) {
    }
}

int shell_crash(int argc, char **argv)
{
    printf("shell crash\r\n");
    //asm ("ebreak");
    assert(0);
    return 0;
}
SHELL_CMD_EXPORT_ALIAS(shell_crash, crash, trigger crash.);
