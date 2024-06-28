/**
  ******************************************************************************
  * @file    at_base_cmd.c
  * @version V1.0
  * @date
  * @brief   This file is part of AT command framework
  ******************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>

#include "at_main.h"
#include "at_core.h"
#include "at_port.h"
#include "at_base_config.h"
#include "qcc74x_adc.h"
#include "qcc74x_boot2.h"
#include "partition.h"
#include "qcc743_glb.h"
#include "at_ota.h"
#include "mem.h"
#include "at_through.h"

#define AT_CMD_GET_VERSION(v) (int)((v>>24)&0xFF),(int)((v>>16)&0xFF),(int)((v>>8)&0xFF),(int)(v&0xFF)

static int at_exe_cmd_rst(int argc, const char **argv)
{
    int i;

    at_response_string(AT_CMD_MSG_OK);

    /* stop all service */
    for (i = 0; i < AT_CMD_MAX_FUNC; i++) {
        if (at->function_ops[i].stop_func)
            at->function_ops[i].stop_func();
    }

    vTaskDelay(pdMS_TO_TICKS(100));
    GLB_SW_POR_Reset();
    return AT_RESULT_CODE_OK;
}

static int at_exe_cmd_gmr(int argc, const char **argv)
{
    char *outbuf = NULL;
    uint32_t core_version;
    char core_compile_time[32];

    size_t outbuf_len = 1024;
    outbuf = (char *)pvPortMalloc(outbuf_len);
    if (!outbuf)
        return AT_RESULT_CODE_FAIL;

    core_version = at_cmd_get_version();
    at_cmd_get_compile_time(core_compile_time, sizeof(core_compile_time));

    snprintf(outbuf, outbuf_len, "AT version:%d.%d.%d.%d(%s)\r\n", AT_CMD_GET_VERSION(core_version), core_compile_time);
    snprintf(outbuf+strlen(outbuf), outbuf_len-strlen(outbuf), "SDK version:%s\r\n", "sdk_2.0.5");
    snprintf(outbuf+strlen(outbuf), outbuf_len-strlen(outbuf), "compile time:%s %s\r\n",  __DATE__, __TIME__);
    snprintf(outbuf+strlen(outbuf), outbuf_len-strlen(outbuf), "Bin version:%s\r\n", "1.1.0");
    AT_CMD_RESPONSE(outbuf);
    vPortFree(outbuf);

    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_cmd(int argc, const char **argv)
{
    int i, n = 0;
    int t, q, s, e;
    char *name;
    char outbuf[64];

    memset(outbuf, 0, sizeof(outbuf));
    snprintf(outbuf, sizeof(outbuf), "+CMD:0,\"AT\",0,0,0,1\r\n");
    AT_CMD_RESPONSE(outbuf);
    for (i = 0, n = 0; i < AT_CMD_MAX_NUM && n < at->num_commands; i++) {
        name = at->commands[i]->at_name;
        if (name) {
            t = at->commands[i]->at_test_cmd ? 1 : 0;
            q = at->commands[i]->at_query_cmd ? 1 : 0;
            s = at->commands[i]->at_setup_cmd ? 1 : 0;
            e = at->commands[i]->at_exe_cmd ? 1 : 0;
            memset(outbuf, 0, sizeof(outbuf));
            snprintf(outbuf, sizeof(outbuf), "+CMD:%d,\"AT%s\",%d,%d,%d,%d\r\n", ++n, name, t, q, s, e);
            AT_CMD_RESPONSE(outbuf);
        }
    }

    return AT_RESULT_CODE_OK;
}

#if 0
static int at_setup_cmd_gslp(int argc, const char **argv)
{
    int sleep_time = 0;
    uint8_t weakup_pin = 0xFF;

    AT_CMD_PARSE_NUMBER(0, &sleep_time);

    at_response_string(AT_CMD_MSG_OK);
    vTaskDelay(pdMS_TO_TICKS(100));

    if (sleep_time <= 0) {
        hal_reboot();
    }
    else {
        hal_hbn_init(&weakup_pin, 1);
        hal_hbn_enter((uint32_t)sleep_time);
    }
    return AT_RESULT_CODE_OK;
}
#endif

static int at_exe_cmd_close_echo(int argc, const char **argv)
{
    at->echo = 0;
    return AT_RESULT_CODE_OK;
}

static int at_exe_cmd_open_echo(int argc, const char **argv)
{
    at->echo = 1;
    return AT_RESULT_CODE_OK;
}

static int at_exe_cmd_restore(int argc, const char **argv)
{
    int i;

    /* stop all service */
    for (i = 0; i < AT_CMD_MAX_FUNC; i++) {
        if (at->function_ops[i].restore_func)
            at->function_ops[i].restore_func();
    }

    at_response_string(AT_CMD_MSG_OK);
    vTaskDelay(pdMS_TO_TICKS(100));
    GLB_SW_POR_Reset();

    return AT_RESULT_CODE_OK;
}

static int at_query_fakeout(int argc, const char **argv)
{
    at_response_string("+FAKEOUTPUT:%d\r\n", at->fakeoutput);
    at_response_string(AT_CMD_MSG_OK);
    return AT_RESULT_CODE_OK;
}

static int at_exe_fakeout(int argc, const char **argv)
{
    int enable = 0;

    AT_CMD_PARSE_NUMBER(0, &enable);

    if (enable == 0 || enable == 1) {
        at->fakeoutput = enable;
    } else {
        return AT_RESULT_CODE_ERROR;
    }
    return AT_RESULT_CODE_OK;
}

#if 0
static int at_query_cmd_uart_cur(int argc, const char **argv)
{
    int baudrate;
    uint8_t databits, stopbits, parity, flow_control;
    if (at_port_para_get(&baudrate, &databits, &stopbits, &parity, &flow_control) != 0)
        return AT_RESULT_CODE_FAIL;

    at_response_string("+UART_CUR:%d,%d,%d,%d,%d\r\n", baudrate, databits, stopbits, parity, flow_control);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_uart_cur(int argc, const char **argv)
{
    int baudrate, databits, stopbits, parity, flow_control;
    int ret;

    AT_CMD_PARSE_NUMBER(0, &baudrate);
    AT_CMD_PARSE_NUMBER(1, &databits);
    AT_CMD_PARSE_NUMBER(2, &stopbits);
    AT_CMD_PARSE_NUMBER(3, &parity);
    AT_CMD_PARSE_NUMBER(4, &flow_control);

    if (baudrate < 80 || baudrate > 5000000)
        return AT_RESULT_CODE_ERROR;
    if (databits < 5 || databits > 8)
        return AT_RESULT_CODE_ERROR;
    if (stopbits < 1 || stopbits > 3)
        return AT_RESULT_CODE_ERROR;
    if (parity < 0 || parity > 2)
        return AT_RESULT_CODE_ERROR;
    if (flow_control < 0 || flow_control > 3)
        return AT_RESULT_CODE_ERROR;

    at_response_string(AT_CMD_MSG_OK);
    vTaskDelay(pdMS_TO_TICKS(100));
    ret = at_port_para_set(baudrate, (uint8_t)databits, (uint8_t)stopbits, (uint8_t)parity, (uint8_t)flow_control);
    if (ret != 0)
        return AT_RESULT_CODE_FAIL;
    else
        return AT_RESULT_CODE_MAX;
}

static int at_query_cmd_uart_def(int argc, const char **argv)
{
    at_response_string("+UART_DEF:%d,%d,%d,%d,%d\r\n", at_base_config->uart_cfg.baudrate,
            at_base_config->uart_cfg.databits,
            at_base_config->uart_cfg.stopbits,
            at_base_config->uart_cfg.parity,
            at_base_config->uart_cfg.flow_control);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_uart_def(int argc, const char **argv)
{
    int baudrate, databits, stopbits, parity, flow_control;

    AT_CMD_PARSE_NUMBER(0, &baudrate);
    AT_CMD_PARSE_NUMBER(1, &databits);
    AT_CMD_PARSE_NUMBER(2, &stopbits);
    AT_CMD_PARSE_NUMBER(3, &parity);
    AT_CMD_PARSE_NUMBER(4, &flow_control);

    if (baudrate < 80 || baudrate > 5000000)
        return AT_RESULT_CODE_ERROR;
    if (databits < 5 || databits > 8)
        return AT_RESULT_CODE_ERROR;
    if (stopbits < 1 || stopbits > 3)
        return AT_RESULT_CODE_ERROR;
    if (parity < 0 || parity > 2)
        return AT_RESULT_CODE_ERROR;
    if (flow_control < 0 || flow_control > 3)
        return AT_RESULT_CODE_ERROR;

    at_base_config->uart_cfg.baudrate = baudrate;
    at_base_config->uart_cfg.databits = (uint8_t)databits;
    at_base_config->uart_cfg.stopbits = (uint8_t)stopbits;
    at_base_config->uart_cfg.parity = (uint8_t)parity;
    at_base_config->uart_cfg.flow_control = (uint8_t)flow_control;
    at_base_config_save(AT_CONFIG_KEY_UART_CFG);
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_sleep(int argc, const char **argv)
{
    at_response_string("+SLEEP:%d\r\n", at_base_config->sleep_mode);
    return 0;
}

static int at_setup_cmd_sleep(int argc, const char **argv)
{
    int sleep_mode;

    AT_CMD_PARSE_NUMBER(0, &sleep_mode);

    if (sleep_mode == BASE_SLEEP_MODE_DISABLE) {
        if (at_base_config->sleep_mode != BASE_SLEEP_MODE_DISABLE) {
            at_base_config->sleep_mode = BASE_SLEEP_MODE_DISABLE;
        }
    }
    else if (sleep_mode == BASE_SLEEP_MODE_MODEM) {
        if (at_base_config->sleep_mode != BASE_SLEEP_MODE_MODEM) {
            at_base_config->sleep_mode = BASE_SLEEP_MODE_MODEM;
        }
    }
    else if (sleep_mode == BASE_SLEEP_MODE_LIGHT) {
        if (!at_base_config->sleepwk_cfg.wakeup_valid)
            return AT_RESULT_CODE_ERROR;

        if (at_base_config->sleep_mode != BASE_SLEEP_MODE_LIGHT) {
            at_base_config->sleep_mode = BASE_SLEEP_MODE_LIGHT;

            uint32_t sleep_time = 0;
            uint8_t weakup_pin = 0xFF;

            if (at_base_config->sleepwk_cfg.wakeup_source == 0) {
                sleep_time = at_base_config->sleepwk_cfg.wakeup_sleep_time;
            }
            else if (at_base_config->sleepwk_cfg.wakeup_source == 2) {
                weakup_pin = at_base_config->sleepwk_cfg.wakeup_gpio;
                if (at_base_config->sleepwk_cfg.wakeup_level)
                    weakup_pin |= 0x80;
            }

            at_response_string(AT_CMD_MSG_OK);
            vTaskDelay(pdMS_TO_TICKS(100));

            hal_hbn_init(&weakup_pin, 1);
            hal_hbn_enter((uint32_t)sleep_time);
        }
    }
    else
        return AT_RESULT_CODE_ERROR;

    return AT_RESULT_CODE_OK;
}
#endif
static int at_query_cmd_sysram(int argc, const char **argv)
{
    //at_response_string("+SYSRAM:%d,%d", info.free_size, info.total_size);
    at_response_string("+SYSRAM:%d", kfree_size());
    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_sysmsg(int argc, const char **argv)
{
    at_response_string("+SYSMSG:%d", at_base_config->sysmsg_cfg.byte);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_sysmsg(int argc, const char **argv)
{
    int sysmsg = 0;

    AT_CMD_PARSE_NUMBER(0, &sysmsg);

    if (sysmsg >= 0 && sysmsg <= 7) {
        at_base_config->sysmsg_cfg.byte = (uint8_t)sysmsg;
        if (at->store) {
            at_base_config_save(AT_CONFIG_KEY_SYS_MSG);
        }
    } else {
        return AT_RESULT_CODE_ERROR;
    }

    return AT_RESULT_CODE_OK;
}

#if 0
static int at_query_cmd_sysflash(int argc, const char **argv)
{
    return 0;
}

static int at_setup_cmd_sysflash(int argc, const char **argv)
{
    return 0;
}

static int at_query_cmd_rfpower(int argc, const char **argv)
{
    return 0;
}

static int at_setup_cmd_rfpower(int argc, const char **argv)
{
    return 0;
}

static int at_exe_cmd_sysrollback(int argc, const char **argv)
{
    return 0;
}

static int at_query_cmd_systimestamp(int argc, const char **argv)
{
    uint32_t systimestamp = (at_base_config->systime_stamp + rtc_get_timestamp_ms())/1000;
    at_response_string("+SYSTIMESTAMP:%d", systimestamp);
    return 0;
}

static int at_setup_cmd_systimestamp(int argc, const char **argv)
{
    int systimestamp;
    uint64_t time_stamp;

    AT_CMD_PARSE_NUMBER(0, &systimestamp);

    time_stamp = systimestamp;
    at_base_config->systime_stamp = time_stamp*1000 - rtc_get_timestamp_ms();
    return AT_RESULT_CODE_OK;
}
#endif

static int at_query_cmd_syslog(int argc, const char **argv)
{
    at_response_string("+SYSLOG:%d\r\n", at->syslog);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_syslog(int argc, const char **argv)
{
    int enable = 0;

    AT_CMD_PARSE_NUMBER(0, &enable);

    if (enable == 0 || enable == 1) {
        at->syslog = enable;
    } else {
        return AT_RESULT_CODE_ERROR;
    }
    return AT_RESULT_CODE_OK;
}

#if 0
static int at_setup_cmd_sleepwkcfg(int argc, const char **argv)
{
    int wakeup_source;
    int wakeup_para1;
    int wakeup_para2;

    AT_CMD_PARSE_NUMBER(0, &wakeup_source);
    if (wakeup_source == 0) {
        if (argc != 2)
            return AT_RESULT_CODE_ERROR;
        AT_CMD_PARSE_NUMBER(1, &wakeup_para1);
    }
    else if (wakeup_source == 2) {
        if (argc != 3)
            return AT_RESULT_CODE_ERROR;
        AT_CMD_PARSE_NUMBER(1, &wakeup_para1);
        AT_CMD_PARSE_NUMBER(2, &wakeup_para2);
        if (wakeup_para1 != 7 && wakeup_para1 != 8)
            return AT_RESULT_CODE_ERROR;
        if (wakeup_para2 != 0 && wakeup_para2 != 1)
            return AT_RESULT_CODE_ERROR;
    }
    else
        return AT_RESULT_CODE_ERROR;

    at_base_config->sleepwk_cfg.wakeup_valid = 1;
    at_base_config->sleepwk_cfg.wakeup_source = (uint8_t)wakeup_source;
    if (wakeup_source == 0) {
        at_base_config->sleepwk_cfg.wakeup_sleep_time = (uint32_t)wakeup_para1;
    }
    else {
        at_base_config->sleepwk_cfg.wakeup_gpio = (uint8_t)wakeup_para1;
        at_base_config->sleepwk_cfg.wakeup_level = (uint8_t)wakeup_para2;
    }

    return AT_RESULT_CODE_OK;
}
#endif

static int at_query_cmd_sysstore(int argc, const char **argv)
{
    at_response_string("+SYSSTORE:%d\r\n", at->store);
    return AT_RESULT_CODE_OK;
}

static int at_setup_cmd_sysstore(int argc, const char **argv)
{
    int store = 0;

    AT_CMD_PARSE_NUMBER(0, &store);

    if (store == 0 || store == 1) {
        at->store = (uint8_t)store;
    } else {
        return AT_RESULT_CODE_ERROR;
    }

    return AT_RESULT_CODE_OK;
}

#define AVERAGE_COUNT 50
static int at_query_temp(int argc, const char **argv)
{
    struct qcc74x_device_s *adc;
    float average_filter = 0.0;
   
    adc = qcc74x_device_get_by_name("adc");
    
    for (int i = 0; i < AVERAGE_COUNT; i++) {
        average_filter += qcc74x_adc_tsen_get_temp(adc);
    }
    average_filter = average_filter / AVERAGE_COUNT;
    printf("temp = %f\r\n", average_filter);

    at_response_string("+TEMP:%f\r\n", average_filter);

    return AT_RESULT_CODE_OK;
}

static int at_setup_efuse_write(int argc, const char **argv)
{
    int nbytes = 0, word = 0;
    char addr[12] = {0};
    char *endptr;
    uint32_t address = 0;
    int recv_num = 0;
    struct qcc74x_device_s *efuse_dev;

    AT_CMD_PARSE_NUMBER(0, &nbytes);
    AT_CMD_PARSE_STRING(1, addr, sizeof(addr));
    address = strtoul(addr, &endptr, 16);
    
    if (nbytes <= 0 || nbytes > 8192) {
        return AT_RESULT_CODE_ERROR;
    }

    word = ((nbytes + 3) & ~3) >> 2;
    char *buffer = (char *)pvPortMalloc(word * 4);
    if (!buffer) {
        return AT_RESULT_CODE_FAIL;
    }
    memset(buffer, 0, word * 4);
    
    at_response_result(AT_RESULT_CODE_OK);

    while(recv_num < nbytes) {
        recv_num += AT_CMD_DATA_RECV(buffer + recv_num, nbytes - recv_num);
    }
    at_response_string("Recv %d bytes\r\n", recv_num);

    efuse_dev = qcc74x_device_get_by_name("ef_ctrl");

    printf("efuse write 0x%x %d \r\n", address, word);
    qcc74x_ef_ctrl_write_direct(efuse_dev, address, (uint32_t *)buffer, word, 0);
    vPortFree(buffer);

    return AT_RESULT_CODE_SEND_OK;
}

static int at_setup_efuse_read(int argc, const char **argv)
{
    int nbytes = 0, word = 0;
    char addr[12] = {0};
    char *endptr;
    uint32_t address = 0;
    int send_num = 0;
    int reload = 0, reload_valid = 0;
    struct qcc74x_device_s *efuse_dev;

    AT_CMD_PARSE_NUMBER(0, &nbytes);
    AT_CMD_PARSE_STRING(1, addr, sizeof(addr));
    AT_CMD_PARSE_OPT_NUMBER(2, &reload, reload_valid);
    address = strtoul(addr, &endptr, 16);
    
    if (nbytes <= 0 || nbytes > 8192) {
        return AT_RESULT_CODE_ERROR;
    }
    
    word = ((nbytes + 3) & ~3) >> 2;
    char *buffer = (char *)pvPortMalloc(word * 4);
    if (!buffer) {
        return AT_RESULT_CODE_FAIL;
    }
    memset(buffer, 0, word * 4);

    efuse_dev = qcc74x_device_get_by_name("ef_ctrl");

    printf("efuse read 0x%x %d \r\n", address, word);
    qcc74x_ef_ctrl_read_direct(efuse_dev, address, (uint32_t *)buffer, word, reload_valid ? reload : 0);
    
    at_response_string("+EFUSE-R:");
    send_num = AT_CMD_DATA_SEND(buffer, nbytes);

    vPortFree(buffer);

    if (send_num == nbytes) {
        return AT_RESULT_CODE_OK;
    } else {
        return AT_RESULT_CODE_ERROR;
    }
}

static int at_setup_efuse_write_cfm(int argc, const char **argv)
{
    struct qcc74x_device_s *efuse_dev;

    efuse_dev = qcc74x_device_get_by_name("ef_ctrl");

    qcc74x_ef_ctrl_write_direct(efuse_dev, 0, NULL, 0, 1);

    return AT_RESULT_CODE_OK;
}

static int at_setup_flash_write(int argc, const char **argv)
{
    int nbytes = 0;
    char addr[12] = {0};
    char *endptr;
    uint32_t address = 0;
    int recv_num = 0;

    AT_CMD_PARSE_NUMBER(0, &nbytes);
    AT_CMD_PARSE_STRING(1, addr, sizeof(addr));
    address = strtoul(addr, &endptr, 16);

    if (nbytes <= 0 || nbytes > 8192) {
        return AT_RESULT_CODE_ERROR;
    }

    char *buffer = (char *)pvPortMalloc(nbytes);
    if (!buffer) {
        return AT_RESULT_CODE_FAIL;
    }
    memset(buffer, 0, nbytes);

    at_response_result(AT_RESULT_CODE_OK);

    while(recv_num < nbytes) {
        recv_num += AT_CMD_DATA_RECV(buffer + recv_num, nbytes - recv_num);
        printf("xxxx recv_num:%d nbytes:%d\r\n", recv_num, nbytes);
    }
    at_response_string("Recv %d bytes\r\n", recv_num);

    printf("flash write 0x%x %d \r\n", address, nbytes);
    qcc74x_flash_write(address, buffer, nbytes);
    vPortFree(buffer);

    return AT_RESULT_CODE_SEND_OK;
}

static int at_setup_flash_read(int argc, const char **argv)
{
    int nbytes = 0;
    char addr[12] = {0};
    char *endptr;
    uint32_t address = 0;
    int send_num = 0;

    AT_CMD_PARSE_NUMBER(0, &nbytes);
    AT_CMD_PARSE_STRING(1, addr, sizeof(addr));
    address = strtoul(addr, &endptr, 16);

    if (nbytes <= 0 || nbytes > 8192) {
        return AT_RESULT_CODE_ERROR;
    }

    char *buffer = (char *)pvPortMalloc(nbytes);
    if (!buffer) {
        return AT_RESULT_CODE_FAIL;
    }
    memset(buffer, 0, nbytes);

    printf("flash read 0x%x %d \r\n", address, nbytes);
    qcc74x_flash_read(address, buffer, nbytes);

    at_response_string("+FLASH-R:");
    send_num = AT_CMD_DATA_SEND(buffer, nbytes);

    vPortFree(buffer);

    if (send_num == nbytes) {
        return AT_RESULT_CODE_OK;
    } else {
        return AT_RESULT_CODE_ERROR;
    }
}

static int at_setup_flash_erase(int argc, const char **argv)
{
    int nbytes = 0;
    char addr[12] = {0};
    char *endptr;
    uint32_t address = 0;
    int send_num = 0;

    AT_CMD_PARSE_NUMBER(0, &nbytes);
    AT_CMD_PARSE_STRING(1, addr, sizeof(addr));
    address = strtoul(addr, &endptr, 16);

    printf("flash erase 0x%x %d \r\n", address, nbytes);
    qcc74x_flash_erase(address, nbytes);

    return AT_RESULT_CODE_OK;
}

static int at_setup_gpio_output(int argc, const char **argv)
{
    int pin, pupd;
    struct qcc74x_device_s *gpio = qcc74x_device_get_by_name("gpio");

    AT_CMD_PARSE_NUMBER(0, &pin);
    AT_CMD_PARSE_NUMBER(1, &pupd);

    if (pupd) {
        qcc74x_gpio_init(gpio, pin, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);
    } else {
        qcc74x_gpio_init(gpio, pin, GPIO_OUTPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_0);
    }

    return AT_RESULT_CODE_OK;
}

static int at_setup_gpio_set(int argc, const char **argv)
{
    int pin, state;
    struct qcc74x_device_s *gpio = qcc74x_device_get_by_name("gpio");

    AT_CMD_PARSE_NUMBER(0, &pin);
    AT_CMD_PARSE_NUMBER(1, &state);

    if (state) {
        qcc74x_gpio_set(gpio, pin);    
    } else {
        qcc74x_gpio_reset(gpio, pin);    
    }

    return AT_RESULT_CODE_OK;
}

static int at_setup_gpio_input(int argc, const char **argv)
{
    int pin, state;
    struct qcc74x_device_s *gpio = qcc74x_device_get_by_name("gpio");

    AT_CMD_PARSE_NUMBER(0, &pin);

    qcc74x_gpio_init(gpio, pin, GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);

    return AT_RESULT_CODE_OK;
}

static int at_query_gpio_input(int argc, const char **argv)
{
    int pin, state;
    struct qcc74x_device_s *gpio = qcc74x_device_get_by_name("gpio");

    AT_CMD_PARSE_NUMBER(0, &pin);
    at_response_string("+IOIN=%d:%d\r\n", pin, qcc74x_gpio_read(gpio, pin));

    return AT_RESULT_CODE_OK;
}

static int at_setup_gpio_analog_input(int argc, const char **argv)
{
    int pin, state;
    struct qcc74x_device_s *gpio = qcc74x_device_get_by_name("gpio");

    AT_CMD_PARSE_NUMBER(0, &pin);

    qcc74x_gpio_init(gpio, pin, GPIO_ANALOG | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_0);

    return AT_RESULT_CODE_OK;
}

static int at_query_part(int argc, const char **argv)
{
    int i;
    pt_table_stuff_config *part = qcc74x_boot2_get_pt_config();

    for (i = 0; i < part->pt_table.entryCnt; i++) {
        at_response_string("+PART=%d,%d,\"%8s\",0x%08lx,0x%08lx,%lu,%lu\r\n",
                           part->pt_entries[i].active_index,
                           part->pt_entries[i].age, 
                           part->pt_entries[i].name,
                           part->pt_entries[i].start_address[0],
                           part->pt_entries[i].start_address[1],
                           part->pt_entries[i].max_len[0],
                           part->pt_entries[i].max_len[1]);
    }
    return AT_RESULT_CODE_OK;
}

static int at_query_ota_start(int argc, const char **argv)
{
    return AT_RESULT_CODE_OK;
}

#define OTA_BUFFER_LEN (4096)
static int at_setup_ota_start(int argc, const char **argv)
{
    int ota_size;
    int recv_size;
    int recv_total = 0;
    uint32_t bin_size = 0;
    at_ota_header_t *ota_head;
    uint8_t *buffer;

    AT_CMD_PARSE_NUMBER(0, &ota_size);

    at_set_work_mode(AT_WORK_MODE_THROUGHPUT);

    buffer = pvPortMalloc(OTA_BUFFER_LEN);
    if (!buffer) {
        return AT_RESULT_CODE_FAIL;
    }
    memset(buffer, 0, OTA_BUFFER_LEN);

    recv_total = AT_CMD_DATA_RECV(buffer, sizeof(at_ota_header_t));

    if (recv_total != sizeof(at_ota_header_t)) {
        vPortFree(buffer);
        return AT_RESULT_CODE_FAIL;
    }
    ota_head = (at_ota_header_t *)buffer;
    at_base_config->ota_handle = at_ota_start(ota_head);

    if (!at_base_config->ota_handle) {
        vPortFree(buffer);
        return AT_RESULT_CODE_ERROR;
    }
    bin_size = ota_head->u.s.len;

    while(recv_total < ota_size) {
        
        recv_size = AT_CMD_DATA_RECV(buffer, (ota_size - recv_total > OTA_BUFFER_LEN)?OTA_BUFFER_LEN:(ota_size - recv_total));
        
        if (recv_size == strlen(AT_THROUGH_EXIT_CMD) && memcmp(buffer, AT_THROUGH_EXIT_CMD, strlen(AT_THROUGH_EXIT_CMD)) == 0) {
        	at_ota_abort(at_base_config->ota_handle);
        	at_base_config->ota_handle = NULL;
        	printf("OTA exit !!!\r\n");
        	break;
        }
        printf("xxxx recv_total:%d recv_size:%d total_size:%d ota_size:%d\r\n", recv_total, recv_size, bin_size, ota_size);
       
        if (at_ota_update(at_base_config->ota_handle, recv_total - sizeof(at_ota_header_t), buffer, recv_size) != 0) {
        	at_ota_abort(at_base_config->ota_handle);
        	at_base_config->ota_handle = NULL;
        	vPortFree(buffer);
            return AT_RESULT_CODE_ERROR;
        }
        recv_total += recv_size;

    }
    at_set_work_mode(AT_WORK_MODE_CMD);

    vPortFree(buffer);
    //at_response_string("OTA receive %d bytes\r\n", recv_total);
    if (recv_total == ota_size) {
        return AT_RESULT_CODE_SEND_OK;
    }

    return AT_RESULT_CODE_ERROR;
}

static int at_setup_ota_finish_reset(int argc, const char **argv)
{
    if (!at_base_config->ota_handle) {
        return AT_RESULT_CODE_ERROR;
    }
    if (at_ota_finish(at_base_config->ota_handle, 1, 0) != 0) {
        return AT_RESULT_CODE_ERROR;
    }
    at_response_result(AT_RESULT_CODE_OK);
    vTaskDelay(pdMS_TO_TICKS(100));
    GLB_SW_POR_Reset();
    return AT_RESULT_CODE_OK;
}

#if 0
static int at_setup_cmd_sysreg(int argc, const char **argv)
{
    int direct = 0;
    uint32_t regAddr, regValue;
    volatile uint32_t *reg = NULL;

    AT_CMD_PARSE_NUMBER(0, &direct);

    get_uint32_from_string((char **)&argv[1], &regAddr);
    reg = (volatile uint32_t *)regAddr;

    if (direct == 0 && argc == 2) {
        at_response_string("+SYSREG:0x%lx\r\n", *reg);
    }
    else if (direct == 1 && argc == 3) {
        get_uint32_from_string((char **)&argv[2], &regValue);
        *reg = regValue;
    }
    else
        return AT_RESULT_CODE_ERROR;

    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_systemp(int argc, const char **argv)
{
    int temp = 0;

#ifdef CONF_ADC_ENABLE_TSEN
    hosal_adc_dev_t *adc;
    adc = wifi_hosal_adc_device_get();
    if (NULL == adc) {
        return AT_RESULT_CODE_FAIL;
    }
    temp = wifi_hosal_adc_tsen_value_get(adc);

    printf("temp is %u\r\n", temp);
    
#endif
    at_response_string("+SYSTEMP:%d.00", temp);

    return AT_RESULT_CODE_OK;
}

static int at_query_cmd_flash(int argc, const char **argv)
{
    int flash_size = 0;
    char flash_type[16] = {0};

    uint32_t usage;
    EF_Ctrl_Read_Sw_Usage(0, &usage);
    if(((usage>>16)&0x03) == 0) {
        Efuse_Device_Info_Type deviceInfo;
        EF_Ctrl_Read_Device_Info(&deviceInfo);

        if(deviceInfo.memoryInfo == 1)
            flash_size = 1;
        else if(deviceInfo.memoryInfo == 2)
            flash_size = 2;
        else if(deviceInfo.memoryInfo == 3)
            flash_size = 4;
        else
            flash_size = 0;
        strlcpy(flash_type, "IntFlash");
    }
    else {
        uint8_t jedecID[3] = {0, 0, 0};
        uint32_t flags = bx_irq_save();
        SPI_Flash_Cfg_Type *flash_info = (SPI_Flash_Cfg_Type *)bx_flash_get_flashCfg();
        XIP_SFlash_GetJedecId_Need_Lock(flash_info, jedecID);
        bx_irq_restore(flags);

        printf("flash jedec id: 0x%02X 0x%02X 0x%02X\r\n", jedecID[0], jedecID[1], jedecID[2]);
        if (jedecID[2] == 0x17 && jedecID[0] == 0xC8 && jedecID[1] == 0x40) {
            flash_size = 8;
            strlcpy(flash_type, "GD25Q64C");
        }
        else if(jedecID[2] == 0x17 && jedecID[0] == 0x5E && jedecID[1] == 0x40) {
            flash_size = 2;
            strlcpy(flash_type, "ZB25VQ64B");
        }
        else if(jedecID[2] == 0x15 && jedecID[0] == 0x5E && jedecID[1] == 0x60) {
            flash_size = 2;
            strlcpy(flash_type, "ZB25VQ16A");
        }
        else
            flash_size = 0;
    }

    if (flash_size == 0)
        at_response_string("+FLASH:%s\r\n", "unknown flash info");
    else
        at_response_string("+FLASH:%dMB, %s\r\n", flash_size, flash_type);
    return AT_RESULT_CODE_OK;
}
#endif

static const at_cmd_struct at_base_cmd[] = {
    {"+RST", NULL, NULL, NULL, at_exe_cmd_rst, 0, 0},
    {"+GMR", NULL, NULL, NULL, at_exe_cmd_gmr, 0, 0},
    {"+CMD", NULL, at_query_cmd_cmd, NULL, NULL, 0, 0},
    //{"+GSLP", NULL, NULL, at_setup_cmd_gslp, NULL, 1, 1},
    {"E0", NULL, NULL, NULL, at_exe_cmd_close_echo, 0, 0},
    {"E1", NULL, NULL, NULL, at_exe_cmd_open_echo, 0, 0},
    {"+RESTORE", NULL, NULL, NULL, at_exe_cmd_restore, 0, 0},
    {"+FAKEOUTPUT", NULL, at_query_fakeout, at_exe_fakeout, NULL, 1, 1},
#if 0
    {"+UART_CUR", NULL, at_query_cmd_uart_cur, at_setup_cmd_uart_cur, NULL, 5, 5},
    {"+UART_DEF", NULL, at_query_cmd_uart_def, at_setup_cmd_uart_def, NULL, 5, 5},
    {"+SLEEP", NULL, at_query_cmd_sleep, at_setup_cmd_sleep, NULL, 1, 1},
#endif
    {"+SYSRAM", NULL, at_query_cmd_sysram, NULL, NULL, 0, 0},
    {"+SYSMSG", NULL, at_query_cmd_sysmsg, at_setup_cmd_sysmsg, NULL, 1, 1},
#if 0
    {"+SYSFLASH", NULL, at_query_cmd_sysflash, at_setup_cmd_sysflash, NULL, 5, 5},
    {"+RFPOWER", NULL, at_query_cmd_rfpower, at_setup_cmd_rfpower, NULL, 1, 4},
    {"+SYSROLLBACK", NULL, NULL, NULL, at_exe_cmd_sysrollback, 0, 0},
    {"+SYSTIMESTAMP", NULL, at_query_cmd_systimestamp, at_setup_cmd_systimestamp, NULL, 1, 1},
#endif
    {"+SYSLOG", NULL, at_query_cmd_syslog, at_setup_cmd_syslog, NULL, 1, 1},
    //{"+SLEEPWKCFG", NULL, NULL, at_setup_cmd_sleepwkcfg, NULL, 2, 3},
    {"+SYSSTORE", NULL, at_query_cmd_sysstore, at_setup_cmd_sysstore, NULL, 1, 1},
#if 0
    {"+SYSREG", NULL, NULL, at_setup_cmd_sysreg, NULL, 2, 3},
    {"+SYSTEMP", NULL, at_query_cmd_systemp, NULL, NULL, 0, 0},
    {"+FLASH", NULL, at_query_cmd_flash, NULL, NULL, 0, 0},
#endif
    {"+TEMP", NULL, at_query_temp, NULL, NULL, 0, 0},
    {"+EFUSE-W", NULL, NULL, at_setup_efuse_write, NULL, 2, 3},
    {"+EFUSE-R", NULL, NULL, at_setup_efuse_read, NULL, 2, 2},
    {"+EFUSE-CFM", NULL, NULL, NULL, at_setup_efuse_write_cfm, 0, 0},
    {"+FLASH-W", NULL, NULL, at_setup_flash_write, NULL, 2, 2},
    {"+FLASH-R", NULL, NULL, at_setup_flash_read, NULL, 2, 2},
    {"+FLASH-E", NULL, NULL, at_setup_flash_erase, NULL, 2, 2},
    {"+IOPUPD", NULL, NULL, at_setup_gpio_output, NULL, 2, 2},
    {"+IOOUT", NULL, NULL, at_setup_gpio_set, NULL, 2, 2},
    {"+IOIN", NULL, at_query_gpio_input, at_setup_gpio_input, NULL, 1, 1},
    {"+IORST", NULL, NULL, at_setup_gpio_analog_input, NULL, 1, 1},
    {"+PART", NULL, at_query_part, NULL, NULL, 0, 0},
    {"+OTA-S", NULL, at_query_ota_start, at_setup_ota_start, NULL, 1, 1},
    {"+OTA-R", NULL, NULL, NULL, at_setup_ota_finish_reset, 0, 0},
};

bool at_base_cmd_regist(void)
{
    at_base_config_init();

    at_port_para_set(at_base_config->uart_cfg.baudrate,
            at_base_config->uart_cfg.databits,
            at_base_config->uart_cfg.stopbits,
            at_base_config->uart_cfg.parity,
            at_base_config->uart_cfg.flow_control);

    at_register_function(at_base_config_default, NULL);

    if (at_cmd_register(at_base_cmd, sizeof(at_base_cmd) / sizeof(at_base_cmd[0])) == 0)
        return true;
    else
        return false;
}

