#include "mfg_atcmd_port.h"
#include "mfg_atcmd.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_CMD_NAME_LENGTH 32
#define MAX_FIELD_NAME_LENGTH 32
#define MAX_ARGC 4
#define MAX_ARG_LENGTH 64

int at_cmd_set_error(uint32_t error)
{
    // AT_CMD_ERROR_INFO("at_cmd_set_error\n");
    return QC_AT_CMD_RESPONSE(AT_RESULT_CODE_ERROR, 0);
}

static int at_cmd_is_valid(char *cmd)
{
    int i, headLen;
    char head[16];

    snprintf((char *)head, sizeof(head), AT_CMD_HEAD);
    headLen = strlen(AT_CMD_HEAD);

    if (strlen(cmd) < headLen)
        return 0;

    for (i=0; i<headLen; i++) {
        if (toupper(head[i]) != toupper(cmd[i]))
            return 0;
    }

    return 1;
}

static int at_arg_is_string(const char *arg)
{
    int len = strlen(arg);

    if (arg[0] != '\"' || arg[len - 1] != '\"') {
        return 0;
    }

    return 1;
}

static int at_arg_is_null(const char *arg)
{
    if (strlen(arg) <= 0)
        return 1;
    else
        return 0;
}

int at_arg_get_number(const char *arg, int *value)
{
    int i;

    for (i=0; i<strlen(arg); i++) {
        if (!((arg[i] >= '0' && arg[i] <= '9') || (i == 0 && arg[i] == '-')))
            return 0;
    }

    *value = atoi(arg);
    return 1;
}

int at_arg_get_hexstr_number(const char *arg, uint32_t *value)
{
    char *endptr;

    if (arg == NULL || value == NULL) {
        return 0;
    }

    *value = strtoul(arg, &endptr, 16);

    // Invalid characters after the number
    if (endptr == arg || *endptr != '\0') {
        return 0;
    }

    return 1;
}

int at_arg_get_string(const char *arg, char *string, int max)
{
    int len;

    if (!at_arg_is_string(arg))
        return 0;

    len = strlen(arg)-2;
    if (len >= max)
        return 0;

    strlcpy(string, arg+1, max);
    string[len] = '\0';
    return 1;
}

int at_arg_get_unquoted_string(const char *arg, char *string, int max) 
{
    int len;

    if (arg[0] == '\"' || arg[strlen(arg) - 1] == '\"') {
        return 0;
    } 
    
    len = strlen(arg); 
    if (len >= max)
        return 0;
    
    strlcpy(string, arg, max);
    string[len] = '\0';
    return 1;
}

static void trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) {
        return;
    }

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    // Write new null terminator character
    end[1] = '\0';
}

int parse_at_arguments(const char *command, int *argc, char *argv[], int argc_min, int argc_max)
{
    const char *cmd_ptr = strchr(command, '=');

    AT_CMD_DEBUG_INFO("command=%s, argc_min=%d,argc_max=%d\n",command, argc_min,argc_max);

    // Initialize argc
    *argc = 0;

    // Command has no arguments
    if (argc_max == 0) {
        return 0;
    }

    // Handle = not found cases
    if (!cmd_ptr) {
        // if (argc_min > 0) {
        //     AT_CMD_DEBUG_INFO("Error: Missing '=' separator.\n");
        //     return -1;
        // } else {
        //     return 0;
        // }
        return 0;
    }

    // Move past the '=' character
    cmd_ptr++;

    // Parse each argument separated by commas
    while (*cmd_ptr != '\0' && *argc < argc_max) {
        if (isspace((unsigned char)*cmd_ptr)) {
            cmd_ptr++;
            continue;
        }

        const char *arg_start = cmd_ptr;
        while (*cmd_ptr != '\0' && *cmd_ptr != ',') {
            cmd_ptr++;
        }

        int arg_length = cmd_ptr - arg_start;
        if (arg_length >= MAX_ARG_LENGTH) {
            printf("Error: Argument too long.\n");
            return -1;
        }

        strncpy(argv[*argc], arg_start, arg_length);
        argv[*argc][arg_length] = '\0';
        trim_whitespace(argv[*argc]);
        (*argc)++;

        if (*cmd_ptr == ',') {
            cmd_ptr++;
        }
    }

    // if (*argc < argc_min || *argc > argc_max) {
    //     printf("Error: Wrong no arguments.\n");
    //     return -1;
    // }

    return 0;
}

int at_command_input(const char *command)
{
    char field_name[MAX_ARG_LENGTH];
    char field_value[MAX_ARG_LENGTH];
    char *argv[MAX_ARGC] = { field_name, field_value };
    int argc;
    char cmd_name[MAX_CMD_NAME_LENGTH];
    const char *cmd_end;

    AT_CMD_DEBUG_INFO("[at_command_input] Check Command: %s\n",command);
    if (!at_cmd_is_valid(command)) {
        return 0;
    }
    
    // Skip AT
    command = command + sizeof(AT_CMD_HEAD) - 1;
    AT_CMD_DEBUG_INFO("[at_command_input] Command: %s\n",command);

    // Figure out command name
    // AT+<CommandName>=<…>
    // AT+<CommandName>=?
    // AT+<CommandName>?
    // AT+<CommandName>
    cmd_end = strchr(command, '=');
    if (cmd_end == NULL) {
        cmd_end = strchr(command, '?');
        if (cmd_end == NULL) {
            strcpy(cmd_name, command);
        } else if (cmd_end == command + strlen(command) - 1) {
            strncpy(cmd_name, command, cmd_end - command);
            cmd_name[cmd_end - command] = '\0';
        } else if (*(cmd_end + 1) == '?') {
            strncpy(cmd_name, command, cmd_end - command);
            cmd_name[cmd_end - command] = '\0';
        } else {
            return 0;
        }
    } else {
        strncpy(cmd_name, command, cmd_end - command);
        cmd_name[cmd_end - command] = '\0';
    }

    AT_CMD_DEBUG_INFO("[at_command_input] Command Name: %s\n",cmd_name);

    // search cmd table
    for (int i = 0; i < mfg_at_cmd_num; i++) {
        const at_cmd_struct *cmd = &mfg_at_cmd[i];
        if (strcmp(cmd_name, cmd->at_name) == 0) {
            AT_CMD_DEBUG_INFO("[at_command_input] Command Struct found at index %d\n", i);

            AT_CMD_DEBUG_INFO("[at_command_input] Split into arguments\n", i);
            if (parse_at_arguments(command, &argc, argv, cmd->para_num_min, cmd->para_num_max)) {
                AT_CMD_ERROR_INFO("Error: argument parse error.\n");
                return 0;
            }
            AT_CMD_DEBUG_INFO("[at_command_input]  * argc = %d\n", argc);
            if (cmd->at_setup_cmd && argc) {
                for (int j = 0; j < argc; j++) {
                    AT_CMD_DEBUG_INFO("[at_command_input]  * argv[%d] = %s, len = %d\n", j, argv[j], strlen(argv[j]));
                }

                // if (argc >= cmd->para_num_min && argc <= cmd->para_num_max) {
                //     cmd->at_setup_cmd(argc, argv);
                //     return 1;
                // } else {
                //     AT_CMD_ERROR_INFO("Error: %s requires between %d and %d arguments.\n", cmd->at_name, cmd->para_num_min, cmd->para_num_max);
                //     return 0;
                // }

                // argument check left to command handler 
                cmd->at_setup_cmd(argc, argv);
                return 1;
            } else if (cmd->at_query_cmd && argc == 0) {
                cmd->at_query_cmd(argc, argv);
                return 1;
            }
        }
    }

    printf("Unsupported AT command: %s\n", command);
    return 0;
}
