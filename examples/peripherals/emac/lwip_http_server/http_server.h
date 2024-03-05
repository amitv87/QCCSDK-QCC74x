

#ifndef __HTTP_SERVER__H__
#define __HTTP_SERVER__H__

#define HTTP_SERVER_PORT 80

extern SemaphoreHandle_t console_lock;
extern sys_sem_t s_x_btn_semaphore;
extern char button_clicked_flag;

#define OS_MSG(fmt, ...)                                             \
    do {                                                             \
        if (xSemaphoreTake(console_lock, portMAX_DELAY) == pdTRUE) { \
            printf(fmt, ##__VA_ARGS__);                              \
            xSemaphoreGive(console_lock);                            \
        }                                                            \
    } while (0);

typedef enum {
    HTTP_POST,
    HTTP_GET,
    HTTP_UNKNOWN,
} HtmlRequestMethod_TypeDef;

typedef struct {
    const char *url;
    const char *path;
} HtmlServerRouter_TypeDef;

void http_server_init(void);

void http_server_netconn_init(void);

#endif