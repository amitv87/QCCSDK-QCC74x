#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"
#include "example_log.h"

QCC74xLOG_DEFINE_TAG(LOG, "log", true);

#undef QCC74xLOG_TAG
#define QCC74xLOG_TAG QCC74xLOG_GET_TAG(LOG)

/* flush notice ------------------------------------------------------------------*/

static EventGroupHandle_t event_group_server_log_flush_notice;
static int
log_flush_notice(void)
{
    xEventGroupSetBits(event_group_server_log_flush_notice, 0x00000001);
    return 0;
}

/* Task Function ------------------------------------------------------------------*/

TaskHandle_t task_server_log;
void task_function_server_log(void *param)
{
    LOG_I("start server log task\r\n");

    while (1) {
        xEventGroupWaitBits(event_group_server_log_flush_notice, 0x00000001, pdTRUE, pdFALSE, portMAX_DELAY);
        LOG_FLUSH();
    }

    LOG_E("delete server_log task\r\n");
    vTaskDelete(NULL);
}

/* QCC74xLOG Varibale  ------------------------------------------------------------------*/

extern qcc74xlog_t example_recorder;
extern qcc74xlog_direct_stream_t example_uart_stream;

#ifdef EXAMPLE_ENABLE_FILESYSTEM
static qcc74xlog_direct_file_time_t qcc74xlog_direct_filetime;
static qcc74xlog_direct_file_size_t qcc74xlog_direct_filesize;
#endif

/* QCC74xLOG Mutex  ------------------------------------------------------------------*/
static SemaphoreHandle_t sem_server_log;

static int
qcc74xlog_enter_critical(void)
{
    if (pdTRUE == xSemaphoreTake(sem_server_log, (TickType_t)200)) {
        return 0;
    } else {
        return -1;
    }
}

static int
qcc74xlog_exit_critical(void)
{
    xSemaphoreGive(sem_server_log);
    return 0;
}

/* QCC74xLOG Stream Mutex  ------------------------------------------------------------------*/
SemaphoreHandle_t sem_server_uart0;

static int
qcc74xlog_direct_stream_lock(void)
{
    if (pdTRUE == xSemaphoreTake(sem_server_uart0, (TickType_t)200)) {
        return 0;
    } else {
        return -1;
    }
}

static int
qcc74xlog_direct_stream_unlock(void)
{
    xSemaphoreGive(sem_server_uart0);
    return 0;
}

/* APP init ------------------------------------------------------------------*/

void app_server_log_init(void)
{
    LOG_I("app server log start init\r\n");

    LOG_FLUSH();

    sem_server_uart0 = xSemaphoreCreateMutex();
    _ASSERT_PARAM(NULL != sem_server_uart0);

    /*!< create event flag */
    event_group_server_log_flush_notice = xEventGroupCreate();
    _ASSERT_PARAM(NULL != event_group_server_log_flush_notice);

    /*!< create mutex */
    sem_server_log = xSemaphoreCreateMutex();
    _ASSERT_PARAM(NULL != sem_server_log);

    /*!< suspen recorder */
    _ASSERT_FUNC(0 == qcc74xlog_suspend(&example_recorder));

    /*!< reconfig async mode */
    _ASSERT_FUNC(0 == qcc74xlog_control(&example_recorder, QCC74xLOG_CMD_FLUSH_NOTICE, (uint32_t)log_flush_notice));
    _ASSERT_FUNC(0 == qcc74xlog_control(&example_recorder, QCC74xLOG_CMD_MODE, QCC74xLOG_MODE_ASYNC));

    /*!< recofig uart0 direct stream, set lock unlock function */
    _ASSERT_FUNC(0 == qcc74xlog_direct_suspend((void *)&example_uart_stream));
    _ASSERT_FUNC(0 == qcc74xlog_direct_control((void *)&example_uart_stream, QCC74xLOG_DIRECT_CMD_LOCK, (uint32_t)qcc74xlog_direct_stream_lock));
    _ASSERT_FUNC(0 == qcc74xlog_direct_control((void *)&example_uart_stream, QCC74xLOG_DIRECT_CMD_UNLOCK, (uint32_t)qcc74xlog_direct_stream_unlock));
    _ASSERT_FUNC(0 == qcc74xlog_direct_resume((void *)&example_uart_stream));

#ifdef EXAMPLE_ENABLE_FILESYSTEM
    /*!< create and config file time direct, keep common log info */
    /*!< lock unlcok function not need, fatfs support re-entrancy mode */
    _ASSERT_FUNC(0 == qcc74xlog_direct_create((void *)&qcc74xlog_direct_filetime, QCC74xLOG_DIRECT_TYPE_FILE_TIME, QCC74xLOG_DIRECT_COLOR_DISABLE, NULL, NULL));
    /*!< 6 * 60 * 60 second create a file */
    /*!< keep max 120 files */
    _ASSERT_FUNC(0 == qcc74xlog_direct_init_file_time((void *)&qcc74xlog_direct_filetime, "sd:/log/common", 6 * 60 * 60, 120));
    _ASSERT_FUNC(0 == qcc74xlog_append(&example_recorder, (void *)&qcc74xlog_direct_filetime));
    _ASSERT_FUNC(0 == qcc74xlog_direct_resume((void *)&qcc74xlog_direct_filetime));

    /*!< create and config file time direct, only keep error and warning log info */
    /*!< lock unlcok function not need, fatfs support re-entrancy mode */
    _ASSERT_FUNC(0 == qcc74xlog_direct_create((void *)&qcc74xlog_direct_filesize, QCC74xLOG_DIRECT_TYPE_FILE_SIZE, QCC74xLOG_DIRECT_COLOR_DISABLE, NULL, NULL));
    /*!< 128 * 1024 bytes create a file */
    /*!< keep max 16 files */
    _ASSERT_FUNC(0 == qcc74xlog_direct_init_file_size((void *)&qcc74xlog_direct_filesize, "sd:/log/error", 128 * 1024, 16));
    _ASSERT_FUNC(0 == qcc74xlog_direct_control((void *)&qcc74xlog_direct_filesize, QCC74xLOG_DIRECT_CMD_LEVEL, QCC74xLOG_LEVEL_WARN));
    _ASSERT_FUNC(0 == qcc74xlog_append(&example_recorder, (void *)&qcc74xlog_direct_filesize));
    _ASSERT_FUNC(0 == qcc74xlog_direct_resume((void *)&qcc74xlog_direct_filesize));
#endif

    /*!< config mutex */
    _ASSERT_FUNC(0 == qcc74xlog_control(&example_recorder, QCC74xLOG_CMD_ENTER_CRITICAL, (uint32_t)qcc74xlog_enter_critical));
    _ASSERT_FUNC(0 == qcc74xlog_control(&example_recorder, QCC74xLOG_CMD_EXIT_CRITICAL, (uint32_t)qcc74xlog_exit_critical));

    /*!< resume recorder */
    _ASSERT_FUNC(0 == qcc74xlog_resume(&example_recorder));

    /*!< create flush task */
    _ASSERT_FUNC(pdPASS == xTaskCreate(
                               /*!< task function */ task_function_server_log,
                               /*!< task name     */ "LOG",
                               /*!< stack size    */ 1024,
                               /*!< task param    */ NULL,
                               /*!< task priority */ 3,
                               /*!< task handle   */ &task_server_log));

    LOG_I("app server log init success\r\n");
}
