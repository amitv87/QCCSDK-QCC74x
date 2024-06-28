/**
  ******************************************************************************
  * @file    at_http_cmd.c
  * @version V1.0
  * @date
  * @brief   This file is part of AT command framework
  ******************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "at_main.h"
#include "at_core.h"

#ifdef LP_APP
static int at_pwr_cmd_pwrmode(int argc, const char **argv)
{
    int pwr_mode;
    uint32_t timeout_ms = 0xffffffff;
    int pwr_valid;
    int timeout_valid;
    int level;
    int level_valid = 0;


    AT_CMD_PARSE_NUMBER(0, &pwr_mode);
    AT_CMD_PARSE_OPT_NUMBER(1, &timeout_ms, timeout_valid);
    AT_CMD_PARSE_OPT_NUMBER(2, &level, level_valid);

    if (pwr_mode  == 0) {
    } else if (pwr_mode == 1) {
        void app_pm_enter_hbn(uint32_t ms, int level);
        app_pm_enter_hbn(timeout_ms, level);
    } else if (pwr_mode == 2) {
        int app_pm_wakeup_by_timer(uint32_t ms);
        app_pm_wakeup_by_timer(timeout_ms);
    } else {
        return AT_RESULT_CODE_ERROR;
    }

    return AT_RESULT_CODE_OK;
}

static int at_dtim_cmd(int argc, const char **argv)
{
    int dtim;

    AT_CMD_PARSE_NUMBER(0, &dtim);

    void set_dtim_config(int dtim);
    set_dtim_config(dtim);

    return AT_RESULT_CODE_OK;
}

static const at_cmd_struct at_pwr_cmd[] = {
    {"+PWR", NULL, NULL, at_pwr_cmd_pwrmode, NULL, 1, 3},
    {"+SLWKDTIM", NULL, NULL, at_dtim_cmd, NULL, 1, 1},
};

bool at_pwr_cmd_regist(void)
{
    if (at_cmd_register(at_pwr_cmd, sizeof(at_pwr_cmd) / sizeof(at_pwr_cmd[0])) == 0)
        return true;
    else
        return false;
}
#endif
