#ifndef _QCC74xLOG_CONF_H
#define _QCC74xLOG_CONF_H

#include "qcc74xlog.h"

/*!< 启用LOG, 禁用将使LOG内容不参与编译 */
/*!< log enable */
#define QCC74xLOG_ENABLE

/*!< 全局启用的LOG等级, 小于此等级的LOG内容不参与编译 */
/*!< log enable level */
#define QCC74xLOG_LEVEL_ENABLE         QCC74xLOG_LEVEL_TRACE

/*!< 默认LOG记录器配置的LOG等级, 小于此等级的LOG将不被记录 */
/*!< 可以动态调节LOG记录器等级来调整记录内容 */
/*!< default log record level */
#define QCC74xLOG_LEVEL_DEFAULT        QCC74xLOG_LEVEL_TRACE

/*!< 默认LOG输出器配置的LOG等级, 小于此等级的LOG将不被输出 */
/*!< 可以动态调节LOG输出器等级来调整某个输出器输出的内容 */
/*!< default direct print level */
#define QCC74xLOG_DIRECT_LEVEL_DEFAULT QCC74xLOG_LEVEL_TRACE

/*!< 使能时间戳 */
/*!< enable timestamp to time */
#define QCC74xLOG_TIMESTAMP_ENABLE

/*!< 使能BUFFER输出器, 未完成配置无效果 */
/*!< enable buffer directed output */
#define QCC74xLOG_DIRECT_BUFFER_ENABLE

/*!< 使能流输出器 */
/*!< enable stream directed output */
#define QCC74xLOG_DIRECT_STREAM_ENABLE

/*!< 使能文件输出器 */
/*!< enable file directed output */
#define QCC74xLOG_DIRECT_FILE_ENABLE

/*!< 使能按时间分割的文件输出器 */
/*!< enable file time directed output */
#define QCC74xLOG_DIRECT_FILE_TIME_ENABLE

/*!< 使能按文件大小分割的文件输出器 */
/*!< enable file size directed output */
#define QCC74xLOG_DIRECT_FILE_SIZE_ENABLE

/*!< 使能短文件名 */
/*!< enable short file name */
#define QCC74xLOG_SHORT_FILENAME

/*!< 使能文件名记录, 占用flash高 */
/*!< enable file name record, flash use high */
#define QCC74xLOG_FILENAME_ENABLE

/*!< 使能函数名记录, 占用flash中等 */
/*!< enable function name record, flash use medium */
#define QCC74xLOG_FUNCTION_ENABLE

/*!< 使能文件行数记录, 占用flash低 */
/*!< enable line record, flash use low */
#define QCC74xLOG_LINE_ENABLE

/*!< 行缓冲大小, 使用的是栈上空间, 请确保栈空间足够 */
/*!< 行缓冲设置大小不足时, 一次长LOG输出可能不完整 */
/*!< line buffer size (in stack) */
/*!< flush use 3xline buffer size in stack */
/*!< log   use 1xline buffer size in stack */
/*!< pay attention to prevent stack overflow */
#define QCC74xLOG_LINE_BUFFER_SIZE  256

/*!< 最小文件尺寸分割大小 */
/*!< file size rotate min size */
#define QCC74xLOG_FILE_SIZE_MIN     (1024)

/*!< 最小时间分割大小 */
/*!< file time rotate min interval */
#define QCC74xLOG_FILE_INTERVAL_MIN (60)

/*!< 默认记录器配置的记录功能 */
/*!< 可以动态修改记录器配置调节记录功能, 提高速度 */
/*!< default record flag config */
/*!< |  item | time occupancy | */
/*!< | level |            low | */
/*!< |   tag |            low | */
/*!< |  func |         medium | */
/*!< |  line |            low | */
/*!< |  file |      very high | */
/*!< | clock |         medium | */
/*!< |  time |           high | */
/*!< |thread |         medium | */
#define QCC74xLOG_FLAG_DEFAULT      ( \
    (0xff & QCC74xLOG_FLAG_LEVEL) |   \
    (0xff & QCC74xLOG_FLAG_TAG) |     \
    (0xff & QCC74xLOG_FLAG_FUNC) |    \
    (0xff & QCC74xLOG_FLAG_LINE) |    \
    (0xff & QCC74xLOG_FLAG_FILE) |    \
    (0xff & QCC74xLOG_FLAG_CLK) |     \
    (0xff & QCC74xLOG_FLAG_TIME) |    \
    (0x00 & QCC74xLOG_FLAG_THREAD))

/*!< 不同日志等级颜色配置 */
/*!< color config */
#define QCC74xLOG_COLOR_FATAL QCC74xLOG_COLOR_FG_MAGENTA QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_NORMAL
#define QCC74xLOG_COLOR_ERROR QCC74xLOG_COLOR_FG_RED QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_NORMAL
#define QCC74xLOG_COLOR_WARN  QCC74xLOG_COLOR_FG_YELLOW QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_NORMAL
#define QCC74xLOG_COLOR_INFO  QCC74xLOG_COLOR_FG_NONE QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_RESET
#define QCC74xLOG_COLOR_DEBUG QCC74xLOG_COLOR_FG_WHITE QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_NORMAL
#define QCC74xLOG_COLOR_TRACE QCC74xLOG_COLOR_FG_WHITE QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_FAINT

/*!< 简易排版的格式 */
/*!< simple layout */
#if 0
#define QCC74xLOG_SIMPLE_LAYOUT_STRING(_color, _level, _tag, _tm, _msg) \
    "%s"                                                            \
    "[%s][%10lu][%d-%02d-%02d %02d:%02d:%02d]"                      \
    "[%s:%s:%d]"                                                    \
    "<%s> %s",                                                      \
        (_color),                                                   \
        (_level),                                                   \
        ((_msg)->clkl),                                             \
        (_tm)->year, (_tm)->mon, (_tm)->mday,                       \
        (_tm)->hour, (_tm)->min, (_tm)->sec,                        \
        ((_msg)->file), ((_msg)->func), ((_msg)->line),             \
        (_tag),                                                     \
        ((_msg)->string)
#else
#define QCC74xLOG_SIMPLE_LAYOUT_STRING(_color, _level, _tag, _tm, _msg) \
    "%s"                                                            \
    "[%c:%10lu]"                                                    \
    "[%s:%d]"                                                       \
    "%10s> %s",                                                     \
        (_color),                                                   \
        (_level[0]),                                                \
        ((_msg)->clkl),                                             \
        ((_msg)->file), ((_msg)->line),                             \
        (_tag),                                                     \
        ((_msg)->string)
#endif

/*!< 不同日志等级提示信息配置 */
/*!< level string config */
#define QCC74xLOG_LEVEL_FATAL_STRING "FATL"
#define QCC74xLOG_LEVEL_ERROR_STRING "ERRO"
#define QCC74xLOG_LEVEL_WARN_STRING  "WARN"
#define QCC74xLOG_LEVEL_INFO_STRING  "INFO"
#define QCC74xLOG_LEVEL_DEBUG_STRING "DBUG"
#define QCC74xLOG_LEVEL_TRACE_STRING "TRAC"

#endif
