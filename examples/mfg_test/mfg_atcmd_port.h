#ifndef MFG_ATCMD_PORT_H
#define MFG_ATCMD_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *at_name;                        
    int (*at_test_cmd)(int argc, const char **argv);      
    int (*at_query_cmd)(int argc, const char **argv);
    int (*at_setup_cmd)(int argc, const char **argv);
    int (*at_exe_cmd)(int argc, const char **argv);
    int para_num_min;
    int para_num_max;
} at_cmd_struct;

typedef enum {
    AT_RESULT_CODE_OK           = 0x00, /*!< "OK" */
    AT_RESULT_CODE_ERROR        = 0x01, /*!< "ERROR" */
    AT_RESULT_CODE_FAIL         = 0x02, /*!< "ERROR" */
    AT_RESULT_CODE_SEND_OK      = 0x03, /*!< "SEND OK" */
    AT_RESULT_CODE_SEND_FAIL    = 0x04, /*!< "SEND FAIL" */
    AT_RESULT_CODE_IGNORE       = 0x05, /*!< response nothing, just change internal status */
    AT_RESULT_CODE_PROCESS_DONE = 0x06, /*!< response nothing, just change internal status */
    AT_RESULT_CODE_MAX
} at_result_code_string_index;

#define AT_CMD_HEAD "AT"

#define AT_CMD_MSG_WEL              "\r\nready\r\n"
#define AT_CMD_MSG_BUSY             "\r\nbusy p...\r\n"
#define AT_CMD_MSG_OK               "\r\nOK\r\n"
#define AT_CMD_MSG_ERROR            "\r\nERROR\r\n"
#define AT_CMD_MSG_FAIL             "\r\nERROR\r\n"
#define AT_CMD_MSG_SEND_OK          "\r\nSEND OK\r\n"
#define AT_CMD_MSG_SEND_FAIL        "\r\nSEND FAIL\r\n"
#define AT_CMD_MSG_SEND_CANCELLED   "\r\nSEND CANCELLED\r\n"
#define AT_CMD_MSG_WAIT_DATA        "\r\n>"
#define AT_CMD_MSG_QUIT_THROUGHPUT  "\r\n+QUITT\r\n"

/**
 * @brief module number,Now just AT module
 *
 */
typedef enum {
    AT_MODULE_NUM = 0x01   /*!< AT module */
} at_module;

/**
 * @brief subcategory number
 *
 */
typedef enum {
    AT_SUB_OK                       = 0x00,              /*!< OK */
    AT_SUB_COMMON_ERROR             = 0x01,              /*!< reserved */
    AT_SUB_NO_TERMINATOR            = 0x02,              /*!< terminator character not found ("\r\n" expected) */
    AT_SUB_NO_AT                    = 0x03,              /*!< Starting "AT" not found (or at, At or aT entered) */
    AT_SUB_PARA_LENGTH_MISMATCH     = 0x04,              /*!< parameter length mismatch */
    AT_SUB_PARA_TYPE_MISMATCH       = 0x05,              /*!< parameter type mismatch */
    AT_SUB_PARA_NUM_MISMATCH        = 0x06,              /*!< parameter number mismatch */
    AT_SUB_PARA_INVALID             = 0x07,              /*!< the parameter is invalid */
    AT_SUB_PARA_PARSE_FAIL          = 0x08,              /*!< parse parameter fail */
    AT_SUB_UNSUPPORT_CMD            = 0x09,              /*!< the command is not supported */
    AT_SUB_CMD_EXEC_FAIL            = 0x0A,              /*!< the command execution failed */
    AT_SUB_CMD_PROCESSING           = 0x0B,              /*!< processing of previous command is in progress */
    AT_SUB_CMD_OP_ERROR             = 0x0C,              /*!< the command operation type is error */
} at_error_code;

#define ESP_AT_ERROR_NO(subcategory,extension)  \
        ((AT_MODULE_NUM << 24) | ((subcategory) << 16) | (extension))

#define AT_CMD_ERROR_OK                           ESP_AT_ERROR_NO(AT_SUB_OK,0x00)                                       /*!< No Error */
#define AT_CMD_ERROR_NON_FINISH                   ESP_AT_ERROR_NO(AT_SUB_NO_TERMINATOR,0x00)                            /*!< terminator character not found ("\r\n" expected) */
#define AT_CMD_ERROR_NOT_FOUND_AT                 ESP_AT_ERROR_NO(AT_SUB_NO_AT,0x00)                                    /*!< Starting "AT" not found (or at, At or aT entered) */
#define AT_CMD_ERROR_PARA_LENGTH(which_para)      ESP_AT_ERROR_NO(AT_SUB_PARA_LENGTH_MISMATCH,which_para)               /*!< parameter length mismatch */
#define AT_CMD_ERROR_PARA_TYPE(which_para)        ESP_AT_ERROR_NO(AT_SUB_PARA_TYPE_MISMATCH,which_para)                 /*!< parameter type mismatch */
#define AT_CMD_ERROR_PARA_NUM(need,given)         ESP_AT_ERROR_NO(AT_SUB_PARA_NUM_MISMATCH,(((need) << 8) | (given)))   /*!< parameter number mismatch */
#define AT_CMD_ERROR_PARA_INVALID(which_para)     ESP_AT_ERROR_NO(AT_SUB_PARA_INVALID,which_para)                       /*!< the parameter is invalid */
#define AT_CMD_ERROR_PARA_PARSE_FAIL(which_para)  ESP_AT_ERROR_NO(AT_SUB_PARA_PARSE_FAIL,which_para)                    /*!< parse parameter fail */
#define AT_CMD_ERROR_CMD_UNSUPPORT                ESP_AT_ERROR_NO(AT_SUB_UNSUPPORT_CMD,0x00)                            /*!< the command is not supported */
#define AT_CMD_ERROR_CMD_EXEC_FAIL(result)        ESP_AT_ERROR_NO(AT_SUB_CMD_EXEC_FAIL,result)                          /*!< the command execution failed */
#define AT_CMD_ERROR_CMD_PROCESSING               ESP_AT_ERROR_NO(AT_SUB_CMD_PROCESSING,0x00)                           /*!< processing of previous command is in progress */
#define AT_CMD_ERROR_CMD_OP_ERROR                 ESP_AT_ERROR_NO(AT_SUB_CMD_OP_ERROR,0x00)                             /*!< the command operation type is error */

#define AT_CMD_RESPONSE(s) printf(s)

#define AT_CMD_PARSE_STRING(i, string, max) do { \
    if (!at_arg_get_string(argv[i], string, max)) { \
        at_cmd_set_error(AT_CMD_ERROR_PARA_PARSE_FAIL(i)); \
        return AT_RESULT_CODE_ERROR; \
    } \
} while(0);

#define AT_CMD_PARSE_UNQUOTED_STRING(i, string, max) do { \
    if (!at_arg_get_unquoted_string(argv[i], string, max)) { \
        at_cmd_set_error(AT_CMD_ERROR_PARA_PARSE_FAIL(i)); \
        return AT_RESULT_CODE_ERROR; \
    } \
} while(0);

#define AT_CMD_PARSE_NUMBER(i, num) do {\
    if (!at_arg_get_number(argv[i], num)) { \
        at_cmd_set_error(AT_CMD_ERROR_PARA_PARSE_FAIL(i)); \
        return AT_RESULT_CODE_ERROR; \
    } \
} while(0);

#define AT_CMD_PARSE_HEXSTR_NUMBER(i, num) do {\
    if (!at_arg_get_hexstr_number(argv[i], num)) { \
        at_cmd_set_error(AT_CMD_ERROR_PARA_PARSE_FAIL(i)); \
        return AT_RESULT_CODE_ERROR; \
    } \
} while(0);

#define AT_CMD_PARSE_OPT_STRING(i, string, max, valid) do { \
    if(argc > i && !at_arg_is_null(argv[i])) { \
        if (!at_arg_get_string(argv[i], string, max)) { \
            at_cmd_set_error(AT_CMD_ERROR_PARA_PARSE_FAIL(i)); \
            return AT_RESULT_CODE_ERROR; \
        } \
        valid = 1; \
    } \
} while(0);

#define AT_CMD_PARSE_OPT_NUMBER(i, num, valid) do {\
    if(argc > i && !at_arg_is_null(argv[i])) { \
        if (!at_arg_get_number(argv[i], num)) { \
            at_cmd_set_error(AT_CMD_ERROR_PARA_PARSE_FAIL(i)); \
            return AT_RESULT_CODE_ERROR; \
        } \
        valid = 1; \
    } \
} while(0);

#define AT_CMD_DEBUG_INFO(...)  //printf(__VA_ARGS__)
#define AT_CMD_ERROR_INFO(...)  printf(__VA_ARGS__)
#define AT_CMD_RESPOND(response) \
    _Generic((response), \
        int: ((response) == 0) ? (AT_CMD_RESPONSE("OK"), AT_RESULT_CODE_OK) : (AT_CMD_RESPONSE("ERROR"), AT_RESULT_CODE_ERROR), \
        const char*: (AT_CMD_RESPONSE(response), AT_RESULT_CODE_OK), \
        char*: (AT_CMD_RESPONSE(response), AT_RESULT_CODE_OK) \
    )
#define QC_AT_CMD_RESPONSE(result, value) \
    ((result) == 0 ? (printf("P:0x%08x\n", (value)), AT_RESULT_CODE_OK) : (printf("F:0x%08x\n", (value)), AT_RESULT_CODE_ERROR))

/**
 * @brief Process an input command string as an AT command.
 *
 * This function takes an input command string and attempts to recognize
 * and process it as an AT command. The function identifies the type of
 * the command (Test, Query, Set, or Execute) and invokes the appropriate
 * handler from the ATCMD module.
 *
 * @param[in] command The input command string to be processed.
 *
 * @return 1 if the command is recognized and processed by the ATCMD module,
 *         0 if the command is not recognized as an AT command.
 */
int at_command_input(const char *command);

#ifdef __cplusplus
}
#endif

#endif/* AT_CORE_H */
