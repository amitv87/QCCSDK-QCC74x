#define DBG_TAG "MAIN"

#include "qcc74x_mtimer.h"
#include "qcc74x_uart.h"
#include "qcc74x_rtc.h"
#include "board.h"
#include "qcc74xlog.h"

/*!< 添加QCC74xLOG 标签可以使用标签过滤功能 */
/*!< 不添加则标签仍然能显示, 但标签过滤无效 */

/*!< 定义一个 QCC74xLOG 标签 */
QCC74xLOG_DEFINE_TAG(MAIN, DBG_TAG, true);

/*!< 取消之前的QCC74xLOG 标签 */
#undef QCC74xLOG_TAG

/*!< 将QCC74xLOG 标签定义为新的标签 */
#define QCC74xLOG_TAG QCC74xLOG_GET_TAG(MAIN)

/*!< qcc74xlog 裸机同步例程 */
/*!< qcc74xlog barebone sync example */

struct qcc74x_device_s *uart0 = NULL;
static struct qcc74x_device_s *rtc = NULL;

/*!< 当前的UTC时间戳 2022-12-16 17:52 */
uint32_t timestamp_base = 1671184300;

/** @defgroup   example_qcc74xlog_port port
-----------------------------------------------------------------------------
* @{
----------------------------------------------------------------------------*/
uint64_t qcc74xlog_clock(void)
{
    return qcc74x_mtimer_get_time_us();
}

uint32_t qcc74xlog_time(void)
{
    return QCC74x_RTC_TIME2SEC(qcc74x_rtc_get_time(rtc)) + timestamp_base;
}

char *qcc74xlog_thread(void)
{
    return "";
}
/*---------------------------------------------------------------------------
* @}            example_qcc74xlog_port port
----------------------------------------------------------------------------*/

#define EXAMPLE_LOG_POOL_SIZE 4096

qcc74xlog_t example_recorder;
static uint32_t example_pool[EXAMPLE_LOG_POOL_SIZE / 4];
qcc74xlog_direct_stream_t example_uart_stream;

uint16_t example_uart_stream_output(void *ptr, uint16_t size)
{
    for (size_t i = 0; i < size; i++) {
        qcc74x_uart_putchar(uart0, ((char *)ptr)[i]);
    }
    return size;
}

void example_log_init(void)
{
    void *record = (void *)&example_recorder;
    void *direct = (void *)&example_uart_stream;

    /*!< 创建一个记录器, 配置内存池, 内存池大小, 模式为同步 */
    /*!< create recorder */
    if (0 != qcc74xlog_create(record, example_pool, EXAMPLE_LOG_POOL_SIZE, QCC74xLOG_MODE_SYNC)) {
        printf("qcc74xlog_create faild\r\n");
    }

    /*!< 创建输出器, 类型为流输出器, 使能颜色输出, 互斥锁设为NULL */
    /*!< create stream direct */
    qcc74xlog_direct_create(direct, QCC74xLOG_DIRECT_TYPE_STREAM, QCC74xLOG_DIRECT_COLOR_ENABLE, NULL, NULL);
    /*!< 配置流输出的输出函数 */
    qcc74xlog_direct_init_stream((void *)direct, example_uart_stream_output);

    /*!< 添加输出器到记录器 */
    /*!< connect direct and recorder */
    qcc74xlog_append(record, direct);

    /*!< 恢复输出器到工作模式 */
    /*!< resume direct */
    qcc74xlog_direct_resume(direct);

    /*!< 恢复记录器到工作模式 */
    /*!< resume record */
    qcc74xlog_resume(record);
}

extern void test_log(void);

int main(void)
{
    board_init();

    /*!< uart0 already initialized in bsp/board */
    uart0 = qcc74x_device_get_by_name("uart0");
    rtc = qcc74x_device_get_by_name("rtc");
    qcc74x_rtc_set_time(rtc, 0);

    example_log_init();

    uint8_t test = 0;

    while (1) {
        if (test == 0) {
            qcc74xlog_global_filter("MAIN", true);
            qcc74xlog_global_filter("TEST", true);
            printf("\r\n============================== enable all output\r\n\r\n");
        } else if (test == 1) {
            qcc74xlog_global_filter("MAIN", false);
            qcc74xlog_global_filter("TEST", true);
            printf("\r\n============================== enable only MAIN output\r\n\r\n");
        } else if (test == 2) {
            qcc74xlog_global_filter("MAIN", false);
            qcc74xlog_global_filter("TEST", false);
            printf("\r\n============================== disable all output\r\n\r\n");
        }

        if (++test >= 3) {
            test = 0;
        }

        QCC74xLOG_F(&example_recorder, "hello world this is fatal error\r\n");
        QCC74xLOG_E(&example_recorder, "hello world this is error\r\n");
        QCC74xLOG_W(&example_recorder, "hello world this is warning\r\n");
        QCC74xLOG_I(&example_recorder, "hello world this is information\r\n");
        QCC74xLOG_D(&example_recorder, "hello world this is degug information\r\n");
        QCC74xLOG_T(&example_recorder, "hello world this is trace information\r\n");
        test_log();
        qcc74x_mtimer_delay_ms(1000);
    }
}
