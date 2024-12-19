#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "mfg_atcmd.h"
#include "mfg_atcmd_port.h"
#include "mfg_otp.h"
#include "qcc743_pm.h"
#include "qcc74x_gpio.h"

#define GPIO_LOOPBACK_TEST  (0)
#define QC_PN_LEN           (24)
#define mfg_atcmd_print                           printf

extern qcc74xverinf_t app_ver;
struct qcc74x_device_s *gpio;

static int get_FTFW(uint32_t *value)
{
    *value = (app_ver.x << 16) | (app_ver.y << 8) | (app_ver.z);
    return 0;
}

static int get_QCFW(uint32_t *value)
{
    *value = (app_ver.x << 16) | (app_ver.y << 8) | (app_ver.z); 
    return 0;
}

static int at_setup_cmd_tstwrite(int argc, const char **argv)
{
    char field[16];
    uint32_t value;
    uint32_t value_readback;
    int error;
    qcc74x_manufacture_otp_struct *otp;

    if (argc != 2) {
        AT_CMD_DEBUG_INFO("AT+TSTWRITE: wrong arg number\n");
        return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_ERROR, 0);
    }

    // Parse the field name
    AT_CMD_PARSE_UNQUOTED_STRING(0, field, sizeof(field));
    AT_CMD_DEBUG_INFO("AT+TSTWRITE: field = %s\n", field);

    // Parse the value
    AT_CMD_PARSE_HEXSTR_NUMBER(1, &value);
    AT_CMD_DEBUG_INFO("AT+TSTWRITE: value = 0x%x\n", value);

    // Determine which field to write to and perform the write operation
    otp = otp_find_by_name(field);
    if (otp != NULL) {
        error = otp_write(otp, value);
        AT_CMD_DEBUG_INFO("AT+TSTWRITE: error = %d\n", error);

        if (error) {
            return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_ERROR, 0);
        }

        error = otp_read(otp, &value_readback);
        if (error) {
            return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_ERROR, 0);
        }

        return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_OK, value_readback);
    } else {
        AT_CMD_DEBUG_INFO("AT+TSTWRITE: field not found in OTP\n");
        return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_ERROR, 0);
    }
}

static int at_setup_cmd_tstread(int argc, const char **argv)
{
    char field[16];
    uint32_t value = 0;
    int error = AT_RESULT_CODE_ERROR;
    qcc74x_manufacture_otp_struct *otp;

    if (argc != 1) {
        AT_CMD_DEBUG_INFO("AT+TSTREAD: wrong arg number\n");
        return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_ERROR, 0);
    }

    // Parse the field name
    AT_CMD_PARSE_UNQUOTED_STRING(0, field, sizeof(field));

    // Determine which field to read from and perform the read operation
    otp = otp_find_by_name(field);
    if (otp != NULL) {
        error = otp_read(otp, &value);
        
        if (error) {
            return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_ERROR, 0);
        }
        else {
            return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_OK, value);
        }
    } else {
        AT_CMD_DEBUG_INFO("AT+TSTREAD: field %s not found in OTP\n", field);
        return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_ERROR, 0);
    }
}

static int at_setup_cmd_get(int argc, const char **argv)
{
    char field[16];
    int result;
    uint32_t field_value = 0;
    
    if (argc != 1) {
        AT_CMD_DEBUG_INFO("AT+GET: wrong arg number\n");
        return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_ERROR, 0);
    }

    // Parse the field name
    AT_CMD_PARSE_UNQUOTED_STRING(0, field, sizeof(field));
    AT_CMD_DEBUG_INFO("AT+GET: arg[0]=%s\n", field);

    // Determine which field to read from and perform the read operation
    if (strcmp(field, "FTFW") == 0) {
        result = get_FTFW(&field_value);
    } else if (strcmp(field, "QCFW") == 0) {
        result = get_QCFW(&field_value);
    } else if (strcmp(field, "SOCSIG") == 0) {
        result = otp_get_soc_signature(&field_value);
    } else {
        AT_CMD_DEBUG_INFO("AT+GET: %s not supported\n", field);        
        result = AT_RESULT_CODE_ERROR;
    }

    // Send ATCMD Response
    // memset(outbuf, 0, sizeof(outbuf));
    // snprintf(outbuf, sizeof(outbuf), "+GET:%s=%s", field, field_value);
    // if (result == 0) {
    //     return AT_CMD_RESPOND(outbuf);
    // } else {
    //     return AT_CMD_RESPOND(result);
    // }
    return QC_AT_CMD_RESPONSE(result, field_value);
}

static int at_setup_cmd_tstshtdwn(int argc, const char **argv)
{
    return AT_CMD_RESPOND(AT_RESULT_CODE_ERROR);
}

static int at_setup_cmd_tsthib(int argc, const char **argv)
{
    uint32_t mode = PM_HBN_LEVEL_2;
    uint32_t rtc_time_ms = 1000;

    if (argc > 0) {
        AT_CMD_PARSE_NUMBER(0, &mode);
    }
    
    if (argc > 1) {
        AT_CMD_PARSE_NUMBER(1, &rtc_time_ms);
    }

    if (mode > PM_HBN_LEVEL_2) {
        return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_ERROR, 0);
    }

    pm_hbn_mode_enter(mode, rtc_time_ms << 5);

    return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_OK, 0);
}

static int at_setup_cmd_tststdby(int argc, const char **argv)
{
    uint32_t mode = PM_PDS_LEVEL_15;
    uint32_t rtc_time_ms = 1000;

    if (argc > 0) {
        AT_CMD_PARSE_NUMBER(0, &mode);
    }
    
    if (argc > 1) {
        AT_CMD_PARSE_NUMBER(1, &rtc_time_ms);
    }
    
    if (mode != PM_PDS_LEVEL_1 && mode != PM_PDS_LEVEL_2 &&
        mode != PM_PDS_LEVEL_3 && mode != PM_PDS_LEVEL_7 &&
        mode != PM_PDS_LEVEL_15) {
        return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_ERROR, 0);
    }

    L1C_DCache_Clean_Invalid_All();
    pm_pds_mask_all_wakeup_src();
    pm_pds_mode_enter(mode, rtc_time_ms << 5);

    return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_OK, 0);
}

static int at_setup_cmd_gpio(int argc, const char **argv)
{
    char io_type[2];
    uint32_t value;
    int at_result;

    #if GPIO_LOOPBACK_TEST
    uint8_t gpio_pin_list_in[] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};
    uint8_t gpio_pin_list_out[] = {GPIO_PIN_27, GPIO_PIN_28, GPIO_PIN_29, GPIO_PIN_30};
    uint32_t gpio_pin_read = 0;
    #else
    uint8_t gpio_pin_list_in[] = {
        GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, 
        GPIO_PIN_16, GPIO_PIN_17, GPIO_PIN_27,
        GPIO_PIN_28, GPIO_PIN_29, GPIO_PIN_30
    };
    uint8_t gpio_pin_list_out[] = {
        GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, 
        GPIO_PIN_16, GPIO_PIN_17, GPIO_PIN_27,
        GPIO_PIN_28, GPIO_PIN_29, GPIO_PIN_30
    };
    #endif

    if (argc < 1 || argc > 2) {
        return AT_CMD_RESPOND(AT_RESULT_CODE_ERROR);
    }

    // Parse the IO type
    AT_CMD_PARSE_UNQUOTED_STRING(0, io_type, sizeof(io_type));
    AT_CMD_DEBUG_INFO("GPIO IO type = %s\n", io_type);

    if (strcmp(io_type, "I") == 0) {
        gpio = qcc74x_device_get_by_name("gpio");

        // Set GPIO as input
        for (int i = 0; i < sizeof(gpio_pin_list_in); i++) {
            qcc74x_gpio_init(gpio, gpio_pin_list_in[i], GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);
        }

#if GPIO_LOOPBACK_TEST
        qcc74x_mtimer_delay_ms(1);
        // Read GPIO
        gpio_pin_read = 0;
        for (int i = 0; i < sizeof(gpio_pin_list_in); i++) {
            gpio_pin_read |= qcc74x_gpio_read(gpio, gpio_pin_list_in[i]) << gpio_pin_list_in[i];
        }
        return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_OK, gpio_pin_read);
        #else
        at_result = AT_RESULT_CODE_OK;
#endif
    } else if (strcmp(io_type, "O") == 0 && argc == 2) {
        AT_CMD_PARSE_NUMBER(1, &value);
        AT_CMD_DEBUG_INFO("GPIO output value = %d\n", value);
        gpio = qcc74x_device_get_by_name("gpio");

        // Set GPIO as output and write the value
        for (int i = 0; i < sizeof(gpio_pin_list_out); i++) {
            qcc74x_gpio_init(gpio, gpio_pin_list_out[i], GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);

            if (value) {
                qcc74x_gpio_set(gpio, gpio_pin_list_out[i]);
            } else {
                qcc74x_gpio_reset(gpio, gpio_pin_list_out[i]);
            }
        }

        at_result = AT_RESULT_CODE_OK;
    } else {
        at_result = AT_RESULT_CODE_ERROR;
    }

    return AT_CMD_RESPOND(at_result);
}

static int at_query_cmd_pn(int argc, const char **argv)
{
    int result;
    int is_null = 1;
    uint32_t field_value = 0;
    uint32_t pn_value[QC_PN_LEN/4];
    int i,j;
    char pn_char;

    result = otp_get_part_number(&pn_value);

    for (i = 0; i < (QC_PN_LEN / 4); i++){
        if (pn_value[i] != 0) {
            is_null = 0;
        }
    }

    if (is_null) {
        mfg_atcmd_print("Null PN\n");
        return QC_AT_CMD_RESPONSE(result, field_value);
    }

    for (i = 0; i < (QC_PN_LEN / 4); i++){
        for (j = 0; j < 4; j++){
            pn_char = (pn_value[i] >> (j * 8)) & 0xFF;
            if (pn_char == 0x3){ /*end of text*/
                mfg_atcmd_print("\n");
                return QC_AT_CMD_RESPONSE(result, field_value);
            }
            mfg_atcmd_print("%c", pn_char);
        }
    }
    mfg_atcmd_print("\n");
    return QC_AT_CMD_RESPONSE(result, field_value);
}

static int at_setup_cmd_pn(int argc, const char **argv)
{
    int result;
    int is_null = 1;

    uint32_t field_value = 0;
    char pn[QC_PN_LEN] = {0};
    uint8_t ascii[QC_PN_LEN] = {0};
    uint32_t pn_value[QC_PN_LEN/4] = {0};
    uint8_t i,j,len;
    if (argc != 1) {
        AT_CMD_DEBUG_INFO("AT+PN: wrong arg number\n");
        return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_ERROR, 0);
    }

    result = otp_get_part_number(&pn_value);
    for (i = 0; i < (QC_PN_LEN / 4); i++){
        if (pn_value[i] != 0) {
            is_null = 0;
        }
    }

    if (is_null) {
        AT_CMD_PARSE_UNQUOTED_STRING(0, pn, sizeof(pn));
        len = strlen(pn);
        mfg_atcmd_print("Save PN = ");

        for (i = 0; i < len; i++) {
            mfg_atcmd_print("%c", pn[i]);
            ascii[i] = (int)pn[i];
        }
        mfg_atcmd_print(", Len = %d\n", len);

        if (i < (QC_PN_LEN - 1)) {
            ascii[i] = 0x3; /*end of text*/
        }

        for (i = 0; i < (QC_PN_LEN / 4); i++) {
            pn_value[i] = (ascii[i * 4 + 3] << 24) |
                          (ascii[i * 4 + 2] << 16) |
                          (ascii[i * 4 + 1] << 8)  |
                          (ascii[i * 4]);
        }
        // mfg_atcmd_print("before otp_set_part_number \n");
        result = otp_set_part_number(&pn_value);

        return QC_AT_CMD_RESPONSE(result, field_value);
    } else {
        mfg_atcmd_print("PN not Null\n");
        return QC_AT_CMD_RESPONSE(result, field_value);
    }

}


static int at_query_cmd_mac(int argc, const char **argv)
{
    int result;
    uint32_t field_value = 0;
    uint8_t mac_0[6] = {0};
    uint8_t mac_1[6] = {0};
    uint8_t mac_2[6] = {0};
    int i;

    result = otp_get_mac_addr(mac_0,mac_1,mac_2);

    mfg_atcmd_print("MAC at Slot 0: \n");
    for (i = 0; i < 6; i++) {
        if (i != 5) {
            mfg_atcmd_print("%02x:", mac_0[i]);
        } else {
            mfg_atcmd_print("%02x\r\n", mac_0[i]);
        }
    }
    mfg_atcmd_print("MAC at Slot 1: \n");
    for (i = 0; i < 6; i++) {
        if (i != 5) {
            mfg_atcmd_print("%02x:", mac_1[i]);
        } else {
            mfg_atcmd_print("%02x\r\n", mac_1[i]);
        }
    }
    mfg_atcmd_print("MAC at Slot 2: \n");
    for (i = 0; i < 6; i++) {
        if (i != 5) {
            mfg_atcmd_print("%02x:", mac_2[i]);
        } else {
            mfg_atcmd_print("%02x\r\n", mac_2[i]);
        }
    }
    return QC_AT_CMD_RESPONSE(result, field_value);
}

const at_cmd_struct mfg_at_cmd[] = {
    {"+GET",        NULL, NULL, at_setup_cmd_get,         NULL, 1, 1},
    // {"+TSTWRITE",   NULL, NULL, at_setup_cmd_tstwrite,    NULL, 2, 2},
    // {"+TSTREAD",    NULL, NULL, at_setup_cmd_tstread,     NULL, 1, 1},
    {"+TSTSHTDWN",  NULL, NULL, at_setup_cmd_tstshtdwn,   NULL, 0, 0},
    {"+TSTHIB",     NULL, NULL, at_setup_cmd_tsthib,      NULL, 0, 2},
    {"+TSTSTDBY",   NULL, NULL, at_setup_cmd_tststdby,    NULL, 0, 2},
    {"+GPIO",       NULL, NULL, at_setup_cmd_gpio,        NULL, 1, 2},
    {"+PN",         NULL, at_query_cmd_pn, at_setup_cmd_pn, NULL, 1, 1},
    {"+MAC",        NULL, at_query_cmd_mac, NULL, NULL, 0, 0}
};

const int mfg_at_cmd_num = sizeof(mfg_at_cmd)/sizeof(mfg_at_cmd[0]);
