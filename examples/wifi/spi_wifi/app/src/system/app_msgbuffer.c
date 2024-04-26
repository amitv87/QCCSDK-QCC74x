
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#include "shell.h"

#define DBG_TAG "MAIN"
#include "log.h"

#define MAX_BUFFER_SIZE 1024

// 定义全局变量，用于存储消息缓冲区的句柄
typedef struct _msgbuffer_desc {
    MessageBufferHandle_t msgbuffer;
    uint8_t cache_buffer[MAX_BUFFER_SIZE];
} msgbuffer_desc_t;

static msgbuffer_desc_t *s_tcmb = NULL;

// 初始化消息缓冲区
void msgbuffer_init()
{
    if (NULL == s_tcmb) {
        s_tcmb = pvPortMalloc(sizeof(msgbuffer_desc_t));
    }
    // 创建消息缓冲区，指定缓冲区大小
    s_tcmb->msgbuffer = xMessageBufferCreate(MAX_BUFFER_SIZE);
    if (s_tcmb->msgbuffer == NULL) {
        printf("Failed to create message buffer\n");
    } else {
        printf("Message buffer initialized\n");
    }
}

// 写入数据到消息缓冲区
void msgbuffer_write(const char *data, size_t data_len)
{
    size_t available_space;
    size_t current_len;
    if (NULL == s_tcmb) {
        printf("write error.\r\n");
        return;
    }

    printf("write data_len:%ld\r\n", data_len);

    available_space = xMessageBufferSpaceAvailable(s_tcmb->msgbuffer);
    printf("msgbuffer used:%d, free:%d(real can write = free - 4)\r\n",
            MAX_BUFFER_SIZE - available_space, available_space);

    // 检查消息缓冲区是否已初始化
    if (s_tcmb->msgbuffer != NULL) {
        // 写入数据到消息缓冲区
        current_len = xMessageBufferSend(s_tcmb->msgbuffer,
                                        data,
                                        data_len,
                                        0);//portMAX_DELAY
        if (current_len == data_len) {
            printf("Data written to message buffer: %s\n", data);
        } else {
            printf("Failed to write data to msgbuffer, current_len:%d, data_len:%d\r\n",
                    current_len, data_len);
        }
    } else {
        printf("Message buffer not initialized\n");
    }
 
    available_space = xMessageBufferSpaceAvailable(s_tcmb->msgbuffer);
    printf("msgbuffer used:%d, free:%d(real can write = free - 4)\r\n",
            MAX_BUFFER_SIZE - available_space, available_space);
}

// 从消息缓冲区读取数据
void msgbuffer_read()
{
    uint8_t *buffer;
    size_t available_space;

    if (NULL == s_tcmb) {
        printf("write error.\r\n");
        return;
    }

    buffer = s_tcmb->cache_buffer;

    // 检查消息缓冲区是否已初始化
    if (s_tcmb->msgbuffer != NULL) {
        // 获取消息缓冲区中可用空间大小
        available_space = xMessageBufferSpaceAvailable(s_tcmb->msgbuffer);
        printf("msgbuffer used:%d, free:%d(real can write = free - 4)\r\n",
                MAX_BUFFER_SIZE - available_space, available_space);

        // 创建一个缓冲区用于存储读取到的数据
        // 从消息缓冲区读取数据
        size_t bytes_read = xMessageBufferReceive(s_tcmb->msgbuffer,
                                    buffer,
                                    available_space,
                                    100);//portMAX_DELAY
        if (bytes_read > 0) {
            // 将读取到的数据打印出来
            buffer[bytes_read] = '\0'; // 添加字符串结束符
            printf("Data read from message buffer len:%d, buffer: %s\n",
                    bytes_read, buffer);
        } else {
            printf("Failed to read data from message buffer\n");
        }

        available_space = xMessageBufferSpaceAvailable(s_tcmb->msgbuffer);
        printf("msgbuffer used:%d, free:%d(real can write = free - 4)\r\n",
                MAX_BUFFER_SIZE - available_space, available_space);
    } else {
        printf("Message buffer not initialized\n");
    }
}

// 清理消息缓冲区
void msgbuffer_cleanup()
{
    if (NULL == s_tcmb) {
        return;
    }
    vMessageBufferDelete(s_tcmb->msgbuffer);
    vPortFree(s_tcmb);
    s_tcmb = NULL;
    printf("Message buffer cleaned up\n");
}

// 命令行函数，用于解析命令并调用相应的功能函数
void cmd_msgbuffer(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: msgbuffer [init | write <data> | read | cleanup]\n");
        return;
    }
    if (strcmp(argv[1], "init") == 0) {
        msgbuffer_init();
    } else if (strcmp(argv[1], "write") == 0) {
        if (argc < 3) {
            printf("Usage: msgbuffer write <data>\n");
            return;
        }
        msgbuffer_write(argv[2], strlen(argv[2]));
    } else if (strcmp(argv[1], "read") == 0) {
        msgbuffer_read();
    } else if (strcmp(argv[1], "cleanup") == 0) {
        msgbuffer_cleanup();
    } else {
        printf("Unknown command: %s\n", argv[1]);
    }
}
SHELL_CMD_EXPORT_ALIAS(cmd_msgbuffer, msgbuffer, msgbuffer test.);

void app_msgbuffer_init(void)
{
}
