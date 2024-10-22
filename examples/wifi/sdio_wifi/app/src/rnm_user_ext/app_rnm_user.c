#include <stdio.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <mem.h>

#include <shell.h>

#define DBG_TAG "APP_RNM"
#include <log.h>

int cmd_rnm_sdio_no_rsp_test(int argc, char **argv)
{
    LOG_I("Starting cmd_rnm_test ...\r\n");
extern void rnms_user_send_extension(uint8_t is_response);
   
    printf("send no need reponse\r\n");
    rnms_user_send_extension(0);

    return 0;
}

int cmd_rnm_sdio_rsp_test(int argc, char **argv)
{
    LOG_I("Starting cmd_rnm_test ...\r\n");
extern void rnms_user_send_extension(uint8_t is_response);
    
    printf("send and need reponse\r\n");
    rnms_user_send_extension(1);
    
    return 0;
}

int hostrouter_stats_display(void);
int cmd_hr(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: hr [status|test]\r\n");
        hostrouter_stats_display();
        return -1;
    }

    if (strcmp(argv[1], "status") == 0) {
        hostrouter_stats_display();
    } else if (strcmp(argv[1], "test") == 0) {
        hostrouter_stats_display();
    }

    return 0;
}
SHELL_CMD_EXPORT_ALIAS(cmd_rnm_sdio_rsp_test, rnm_sdio_rsp, rnm rsp test.);
SHELL_CMD_EXPORT_ALIAS(cmd_rnm_sdio_no_rsp_test, rnm_sdio_no_rsp, rnm no rsp test.);
SHELL_CMD_EXPORT_ALIAS(cmd_hr, hr, hr [status|test].);

void app_rnm_sdio_init(void)
{
}

