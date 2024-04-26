#ifndef _QCC74xLOG_H
#define _QCC74xLOG_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#if __has_include("qcc74xlog_conf_user.h")
#include "qcc74xlog_conf_user.h"
#else
#include "qcc74xlog_conf.h"
#endif

/** @addtogroup QCC74xLOG_CSI Control Sequence Introducer
 * @{
 */
#define QCC74xLOG_CSI_START      "\033["
#define QCC74xLOG_CSI_CUU        "A"
#define QCC74xLOG_CSI_CUD        "B"
#define QCC74xLOG_CSI_CUF        "C"
#define QCC74xLOG_CSI_CUB        "D"
#define QCC74xLOG_CSI_CNL        "E"
#define QCC74xLOG_CSI_CPL        "F"
#define QCC74xLOG_CSI_CHA        "G"
#define QCC74xLOG_CSI_CUP        "H"
#define QCC74xLOG_CSI_ED         "J"
#define QCC74xLOG_CSI_EL         "K"
#define QCC74xLOG_CSI_SU         "S"
#define QCC74xLOG_CSI_SD         "T"
#define QCC74xLOG_CSI_SGR        "m"
/**
 * @}
 */

/** @addtogroup QCC74xLOG_SGR Select Graphic Rendition
 * @{
 */
#define QCC74xLOG_SGR_RESET      "0"
#define QCC74xLOG_SGR_BOLD       "1"
#define QCC74xLOG_SGR_FAINT      "2"
#define QCC74xLOG_SGR_ITALICS    "3"
#define QCC74xLOG_SGR_UNDERLINE  "4"
#define QCC74xLOG_SGR_BLINKS     "5"
#define QCC74xLOG_SGR_BLINKR     "6"
#define QCC74xLOG_SGR_REVERSE    "7"
#define QCC74xLOG_SGR_HIDE       "8"
#define QCC74xLOG_SGR_STRIKE     "9"
#define QCC74xLOG_SGR_NORMAL     "22"
#define QCC74xLOG_SGR_FG_BLACK   "30"
#define QCC74xLOG_SGR_FG_RED     "31"
#define QCC74xLOG_SGR_FG_GREEN   "32"
#define QCC74xLOG_SGR_FG_YELLOW  "33"
#define QCC74xLOG_SGR_FG_BLUE    "34"
#define QCC74xLOG_SGR_FG_MAGENTA "35"
#define QCC74xLOG_SGR_FG_CYAN    "36"
#define QCC74xLOG_SGR_FG_WHITE   "37"
#define QCC74xLOG_SGR_BG_BLACK   "40"
#define QCC74xLOG_SGR_BG_RED     "41"
#define QCC74xLOG_SGR_BG_GREEN   "42"
#define QCC74xLOG_SGR_BG_YELLOW  "43"
#define QCC74xLOG_SGR_BG_BLUE    "44"
#define QCC74xLOG_SGR_BG_MAGENTA "45"
#define QCC74xLOG_SGR_BG_CYAN    "46"
#define QCC74xLOG_SGR_BG_WHITE   "47"
/**
 * @}
 */

/** @addtogroup QCC74xLOG_COLOR
 * @{
 */
#define QCC74xLOG_COLOR_START    QCC74xLOG_CSI_START
#define QCC74xLOG_COLOR_END      QCC74xLOG_CSI_SGR
#define QCC74xLOG_CLOLR_SEP      ";"
#define QCC74xLOG_COLOR_DEFAULT
#define QCC74xLOG_COLOR_RESET QCC74xLOG_SGR_RESET QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_FG_NONE
#define QCC74xLOG_COLOR_FG_BLACK   QCC74xLOG_SGR_FG_BLACK QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_FG_RED     QCC74xLOG_SGR_FG_RED QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_FG_GREEN   QCC74xLOG_SGR_FG_GREEN QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_FG_YELLOW  QCC74xLOG_SGR_FG_YELLOW QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_FG_BLUE    QCC74xLOG_SGR_FG_BLUE QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_FG_MAGENTA QCC74xLOG_SGR_FG_MAGENTA QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_FG_CYAN    QCC74xLOG_SGR_FG_CYAN QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_FG_WHITE   QCC74xLOG_SGR_FG_WHITE QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_BG_NONE
#define QCC74xLOG_COLOR_BG_BLACK   QCC74xLOG_SGR_BG_BLACK QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_BG_RED     QCC74xLOG_SGR_BG_RED QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_BG_GREEN   QCC74xLOG_SGR_BG_GREEN QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_BG_YELLOW  QCC74xLOG_SGR_BG_YELLOW QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_BG_BLUE    QCC74xLOG_SGR_BG_BLUE QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_BG_MAGENTA QCC74xLOG_SGR_BG_MAGENTA QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_BG_CYAN    QCC74xLOG_SGR_BG_CYAN QCC74xLOG_CLOLR_SEP
#define QCC74xLOG_COLOR_BG_WHITE   QCC74xLOG_SGR_BG_WHITE QCC74xLOG_CLOLR_SEP
/**
 * @}
 */

/** @addtogroup QCC74xLOG_COLOR_CONTROL
 * @{
 */
#ifndef QCC74xLOG_COLOR_FATAL
#define QCC74xLOG_COLOR_FATAL QCC74xLOG_COLOR_FG_MAGENTA QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_BLINKS
#endif

#ifndef QCC74xLOG_COLOR_ERROR
#define QCC74xLOG_COLOR_ERROR QCC74xLOG_COLOR_FG_RED QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_NORMAL
#endif

#ifndef QCC74xLOG_COLOR_WARN
#define QCC74xLOG_COLOR_WARN QCC74xLOG_COLOR_FG_YELLOW QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_NORMAL
#endif

#ifndef QCC74xLOG_COLOR_INFO
#define QCC74xLOG_COLOR_INFO QCC74xLOG_COLOR_FG_NONE QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_RESET
#endif

#ifndef QCC74xLOG_COLOR_DEBUG
#define QCC74xLOG_COLOR_DEBUG QCC74xLOG_COLOR_FG_WHITE QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_NORMAL
#endif

#ifndef QCC74xLOG_COLOR_TRACE
#define QCC74xLOG_COLOR_TRACE QCC74xLOG_COLOR_FG_WHITE QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_FAINT
#endif

/**
 * @}
 */

/** @addtogroup QCC74xLOG_LEVEL_STRING
 * @{
 */
#ifndef QCC74xLOG_LEVEL_FATAL_STRING
#define QCC74xLOG_LEVEL_FATAL_STRING "FATL"
#endif

#ifndef QCC74xLOG_LEVEL_ERROR_STRING
#define QCC74xLOG_LEVEL_ERROR_STRING "ERRO"
#endif

#ifndef QCC74xLOG_LEVEL_WARN_STRING
#define QCC74xLOG_LEVEL_WARN_STRING "WARN"
#endif

#ifndef QCC74xLOG_LEVEL_INFO_STRING
#define QCC74xLOG_LEVEL_INFO_STRING "INFO"
#endif

#ifndef QCC74xLOG_LEVEL_DEBUG_STRING
#define QCC74xLOG_LEVEL_DEBUG_STRING "DBUG"
#endif

#ifndef QCC74xLOG_LEVEL_TRACE_STRING
#define QCC74xLOG_LEVEL_TRACE_STRING "TRAC"
#endif
/**
 * @}
 */

/** @addtogroup QCC74xLOG_LEVEL
 * @{
 */
#define QCC74xLOG_LEVEL_FATAL           0x00 /*!< level fatal, create a panic */
#define QCC74xLOG_LEVEL_ERROR           0x01 /*!< level error                 */
#define QCC74xLOG_LEVEL_WARN            0x02 /*!< level warning               */
#define QCC74xLOG_LEVEL_INFO            0x03 /*!< level information           */
#define QCC74xLOG_LEVEL_DEBUG           0x04 /*!< level debug                 */
#define QCC74xLOG_LEVEL_TRACE           0x05 /*!< level trace information     */
#define QCC74xLOG_LEVEL_MASK            0x7F /*!< level mask */
#define QCC74xLOG_LEVEL_RAW             0x80 /*!< level raw bit */
/**
 * @}
 */

/** @addtogroup QCC74xLOG_FLAG
 * @{
 */
#define QCC74xLOG_FLAG_LEVEL            ((uint8_t)0x01) /*!< supported print level     */
#define QCC74xLOG_FLAG_TAG              ((uint8_t)0x02) /*!< supported record tag      */
#define QCC74xLOG_FLAG_FUNC             ((uint8_t)0x04) /*!< supported record function */
#define QCC74xLOG_FLAG_LINE             ((uint8_t)0x08) /*!< supported record line     */
#define QCC74xLOG_FLAG_FILE             ((uint8_t)0x10) /*!< supported record file     */
#define QCC74xLOG_FLAG_CLK              ((uint8_t)0x20) /*!< supported record clock    */
#define QCC74xLOG_FLAG_TIME             ((uint8_t)0x40) /*!< supported record time     */
#define QCC74xLOG_FLAG_THREAD           ((uint8_t)0x80) /*!< supported record thread   */
/**
 * @}
 */

/** @addtogroup QCC74xLOG_BASE_MARCO
 * @{
 */

/** @addtogroup QCC74xLOG_STATUS
 * @{
 */
#define QCC74xLOG_STATUS_ILLEGAL        ((uint8_t)0)
#define QCC74xLOG_STATUS_READY          ((uint8_t)1)
#define QCC74xLOG_STATUS_RUNNING        ((uint8_t)2)
#define QCC74xLOG_STATUS_SUSPEND        ((uint8_t)3)
/**
 * @}
 */

/** @addtogroup QCC74xLOG_MODE
 * @{
 */
#define QCC74xLOG_MODE_SYNC             ((uint8_t)0x00)
#define QCC74xLOG_MODE_ASYNC            ((uint8_t)0x01)
/**
 * @}
 */

/** @addtogroup QCC74xLOG_COMMAND
 * @{
 */
#define QCC74xLOG_CMD_ILLEGAL           ((uint32_t)0x00)
#define QCC74xLOG_CMD_FLAG              ((uint32_t)0x01)
#define QCC74xLOG_CMD_LEVEL             ((uint32_t)0x02)
#define QCC74xLOG_CMD_QUEUE_POOL        ((uint32_t)0x03)
#define QCC74xLOG_CMD_QUEUE_SIZE        ((uint32_t)0x04)
#define QCC74xLOG_CMD_QUEUE_RST         ((uint32_t)0x05)
#define QCC74xLOG_CMD_ENTER_CRITICAL    ((uint32_t)0x06)
#define QCC74xLOG_CMD_EXIT_CRITICAL     ((uint32_t)0x07)
#define QCC74xLOG_CMD_FLUSH_NOTICE      ((uint32_t)0x08)
#define QCC74xLOG_CMD_MODE              ((uint32_t)0x09)
/**
 * @}
 */

/**
 * @}
 */

/** @addtogroup QCC74xLOG_DIRECT_MARCO
 * @{
 */

/** @addtogroup QCC74xLOG_DIRECT_STATUS
 * @{
 */
#define QCC74xLOG_DIRECT_STATUS_ILLEGAL ((uint8_t)0x00)
#define QCC74xLOG_DIRECT_STATUS_READY   ((uint8_t)0x01)
#define QCC74xLOG_DIRECT_STATUS_RUNNING ((uint8_t)0x02)
#define QCC74xLOG_DIRECT_STATUS_SUSPEND ((uint8_t)0x03)
#define QCC74xLOG_DIRECT_STATUS_INIT    ((uint8_t)0x04)
#define QCC74xLOG_DIRECT_STATUS_IOERR   ((uint8_t)0x05)
/**
 * @}
 */

/** @addtogroup QCC74xLOG_DIRECT_COMMAND
 * @{
 */
#define QCC74xLOG_DIRECT_CMD_ILLEGAL    ((uint32_t)0x00)
#define QCC74xLOG_DIRECT_CMD_LEVEL      ((uint32_t)0x02)
#define QCC74xLOG_DIRECT_CMD_LOCK       ((uint32_t)0x06)
#define QCC74xLOG_DIRECT_CMD_UNLOCK     ((uint32_t)0x07)
#define QCC74xLOG_DIRECT_CMD_COLOR      ((uint32_t)0x0A)
/**
 * @}
 */

/** @addtogroup QCC74xLOG_DIRECT_TYPE
 * @{
 */
#define QCC74xLOG_DIRECT_TYPE_ILLEGAL   ((uint8_t)0x00)
#define QCC74xLOG_DIRECT_TYPE_BUFFER    ((uint8_t)0x01)
#define QCC74xLOG_DIRECT_TYPE_STREAM    ((uint8_t)0x02)
#define QCC74xLOG_DIRECT_TYPE_FILE      ((uint8_t)0x03)
#define QCC74xLOG_DIRECT_TYPE_FILE_TIME ((uint8_t)0x04)
#define QCC74xLOG_DIRECT_TYPE_FILE_SIZE ((uint8_t)0x05)
/**
 * @}
 */

/** @addtogroup QCC74xLOG_DIRECT_COLOR
 * @{
 */
#define QCC74xLOG_DIRECT_COLOR_DISABLE  ((uint8_t)0)
#define QCC74xLOG_DIRECT_COLOR_ENABLE   ((uint8_t)1)
/**
 * @}
 */

/**
 * @}
 */

/** @addtogroup QCC74xLOG_LAYOUT_MARCO
 * @{
 */

/** @addtogroup QCC74xLOG_LAYOUT_STATUS
 * @{
 */
#define QCC74xLOG_LAYOUT_STATUS_ILLEGAL ((uint8_t)0x00)
#define QCC74xLOG_LAYOUT_STATUS_READY   ((uint8_t)0x01)
#define QCC74xLOG_LAYOUT_STATUS_INIT    ((uint8_t)0x04)
/**
 * @}
 */

/** @addtogroup QCC74xLOG_LAYOUT_TYPE
 * @{
 */
#define QCC74xLOG_LAYOUT_TYPE_SIMPLE    ((uint8_t)0)
#define QCC74xLOG_LAYOUT_TYPE_FORMAT    ((uint8_t)1)
#define QCC74xLOG_LAYOUT_TYPE_YAML      ((uint8_t)2)
/**
 * @}
 */

/**
 * @}
 */

/** @addtogroup QCC74xLOG_STRUCT_TIMESTAMP
 * @{
 */

typedef struct
{
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t mday;
    uint8_t mon;
    uint16_t year;
    uint8_t wday;
} qcc74xlog_tm_t;

/**
 * @}
 */

/** @addtogroup QCC74xLOG_STRUCT
 * @{
 */

/**
 *   @brief         dual list
 */
struct _qcc74xlog_list {
    struct _qcc74xlog_list *next;
    struct _qcc74xlog_list *prev;
};

/**
 *   @brief         tag
 */
struct _qcc74xlog_tag {
    char *tag;
    /*!< max 32 filter */
    uint32_t en;
};

/**
 *   @brief         message
 */
struct _qcc74xlog_msg {
    union {
        uint32_t head; /*!< first 4byte */
        struct
        {
            uint16_t size; /*!< msg total size */
            uint8_t level; /*!< msg level */
            uint8_t zero;  /*!< always zero */
        };
    };

    uint32_t time;    /*!< rtc timestamp */
    union {
        uint64_t clk; /*!< cpu clock tick */
        struct
        {
            uint32_t clkl; /*!< low 4byte */
            uint32_t clkh; /*!< high 4byte */
        };
    };

    uint32_t line;      /*!< msg line */
    const char *func;   /*!< msg function, must be static string, only record pointer */
    const char *file;   /*!< msg file,     must be static string, only record pointer */
    void *tag;          /*!< msg tag */
    const char *thread; /*!< msg thread,   must be static string, only record pointer */
    char string[0];     /*!< msg string */
};

/**
 *   @brief         recorder base type
 */
typedef struct
{
    struct _qcc74xlog_list direct; /*!< dlist */
    int (*enter_critical)(void);
    int (*exit_critical)(void);
    int (*flush_notice)(void);

    uint8_t status;
    uint8_t flags;
    uint8_t level;
    uint8_t mode;

    uint32_t filter;

    struct
    {
        uint16_t free;
        uint16_t size;
        uint16_t wpos;
        uint16_t rpos;
        void *pool;
    } queue;
} qcc74xlog_t;

#define _QCC74xLOG_STRUCT_LAYOUT_EXTENDS \
    uint8_t status;                  \
    uint8_t _rsvd0;                  \
    uint8_t _rsvd1;                  \
    uint8_t type;

/**
 *   @brief         layout base type
 */
typedef struct
{
    _QCC74xLOG_STRUCT_LAYOUT_EXTENDS
} qcc74xlog_layout_t;

/**
 *   @brief         layout simple type
 */
typedef struct
{
    _QCC74xLOG_STRUCT_LAYOUT_EXTENDS
} qcc74xlog_layout_simple_t;

/**
 *   @brief         layout format type
 */
typedef struct
{
    _QCC74xLOG_STRUCT_LAYOUT_EXTENDS
    int (*snprintf)(void *ptr, uint16_t size, char *color, char *level, char *tag, qcc74xlog_tm_t *tm, struct _qcc74xlog_msg *msg);
} qcc74xlog_layout_format_t;

/**
 *   @brief         layout yaml type
 */
typedef struct
{
    _QCC74xLOG_STRUCT_LAYOUT_EXTENDS
} qcc74xlog_layout_yaml_t;

#define _QCC74xLOG_STRUCT_DIRECT_EXTENDS                                       \
    struct _qcc74xlog_list list;                                               \
    void (*write)(struct qcc74xlog_direct * direct, void *ptr, uint16_t size); \
    qcc74xlog_layout_t *layout;                                                \
    uint8_t status;                                                        \
    uint8_t color;                                                         \
    uint8_t level;                                                         \
    uint8_t type;                                                          \
    uint32_t filter;                                                       \
    int (*lock)(void);                                                     \
    int (*unlock)(void)

/**
 *   @brief         direct base type
 */
typedef struct qcc74xlog_direct {
    _QCC74xLOG_STRUCT_DIRECT_EXTENDS;
} qcc74xlog_direct_t;

/**
 *   @brief         direct buffer type
 */
typedef struct
{
    _QCC74xLOG_STRUCT_DIRECT_EXTENDS;
} qcc74xlog_direct_buffer_t;

/**
 *   @brief         direct stream type
 */
typedef struct
{
    _QCC74xLOG_STRUCT_DIRECT_EXTENDS;
    uint16_t (*stream_output)(void *, uint16_t);
} qcc74xlog_direct_stream_t;

/**
 *   @brief         direct file type
 */
typedef struct
{
    _QCC74xLOG_STRUCT_DIRECT_EXTENDS;
    void *fp;
    const char *path;
} qcc74xlog_direct_file_t;

/**
 *   @brief         direct file time division type
 */
typedef struct
{
    _QCC74xLOG_STRUCT_DIRECT_EXTENDS;
    void *fp;         /*!< file pointer now */
    const char *path; /*!< file path */
    uint32_t interval;
    uint32_t keep;
    uint32_t timestamp;
} qcc74xlog_direct_file_time_t;

/**
 *   @brief         direct file size division type
 */
typedef struct
{
    _QCC74xLOG_STRUCT_DIRECT_EXTENDS;
    void *fp;
    const char *path;
    uint32_t size;
    uint32_t keep;
} qcc74xlog_direct_file_size_t;

/**
 * @}
 */

extern uint64_t qcc74xlog_clock(void);
extern uint32_t qcc74xlog_time(void);
extern char *qcc74xlog_thread(void);

extern int qcc74xlog_global_filter(void *tag_string, uint8_t enable);

extern int qcc74xlog_create(qcc74xlog_t *log, void *pool, uint16_t size, uint8_t mode);
extern int qcc74xlog_delete(qcc74xlog_t *log);
extern int qcc74xlog_append(qcc74xlog_t *log, qcc74xlog_direct_t *direct);
extern int qcc74xlog_remove(qcc74xlog_t *log, qcc74xlog_direct_t *direct);
extern int qcc74xlog_suspend(qcc74xlog_t *log);
extern int qcc74xlog_resume(qcc74xlog_t *log);
extern int qcc74xlog_control(qcc74xlog_t *log, uint32_t command, uint32_t param);
extern int qcc74xlog_filter(qcc74xlog_t *log, void *tag_string, uint8_t enable);
extern int qcc74xlog(void *log, uint8_t level, void *tag, const char *const file, const char *const func, const long line, const char *format, ...);
extern int qcc74xlog_flush(void *log);

extern int qcc74xlog_direct_create(qcc74xlog_direct_t *direct, uint8_t type, uint8_t color, void(*lock), void(*unlock));
extern int qcc74xlog_direct_delete(qcc74xlog_direct_t *direct);
extern int qcc74xlog_direct_suspend(qcc74xlog_direct_t *direct);
extern int qcc74xlog_direct_resume(qcc74xlog_direct_t *direct);
extern int qcc74xlog_direct_link(qcc74xlog_direct_t *direct, qcc74xlog_layout_t *layout);
extern int qcc74xlog_direct_control(qcc74xlog_direct_t *direct, uint32_t command, uint32_t param);
extern int qcc74xlog_direct_filter(qcc74xlog_direct_t *direct, void *tag_string, uint8_t enable);

extern int qcc74xlog_direct_init_buffer(qcc74xlog_direct_t *direct, void *buffer, void *size);
extern int qcc74xlog_direct_deinit_buffer(qcc74xlog_direct_t *direct);

extern int qcc74xlog_direct_init_stream(qcc74xlog_direct_t *direct, uint16_t (*stream_output)(void *, uint16_t));
extern int qcc74xlog_direct_deinit_stream(qcc74xlog_direct_t *direct);

extern int qcc74xlog_direct_init_file(qcc74xlog_direct_t *direct, const char *path);
extern int qcc74xlog_direct_deinit_file(qcc74xlog_direct_t *direct);

extern int qcc74xlog_direct_init_file_size(qcc74xlog_direct_t *direct, const char *path, uint32_t size, uint32_t keep);
extern int qcc74xlog_direct_deinit_file_size(qcc74xlog_direct_t *direct);

extern int qcc74xlog_direct_init_file_time(qcc74xlog_direct_t *direct, const char *path, uint32_t interval, uint32_t keep);
extern int qcc74xlog_direct_deinit_file_time(qcc74xlog_direct_t *direct);

extern int qcc74xlog_layout_create(qcc74xlog_layout_t *layout, uint8_t type);
extern int qcc74xlog_layout_delete(qcc74xlog_layout_t *layout);
extern int qcc74xlog_layout_format(qcc74xlog_layout_t *layout, int (*u_snprintf)(void *ptr, uint16_t size, char *color, char *level, char *tag, qcc74xlog_tm_t *tm, struct _qcc74xlog_msg *msg));

#ifdef QCC74xLOG_TIMESTAMP_ENABLE
extern void qcc74xlog_unix2time(uint32_t timestamp, qcc74xlog_tm_t *time);
#endif

#ifndef QCC74xLOG_LEVEL_ENABLE
#define QCC74xLOG_LEVEL_ENABLE QCC74xLOG_LEVEL_INFO
#endif

#ifdef _WIN32_DELIMITER
#define __PATH_DELIMITER '\\'
#else
#define __PATH_DELIMITER '/'
#endif

#ifndef __QCC74xLOG_FILENAME__
#ifndef QCC74xLOG_FILENAME_ENABLE
#define __QCC74xLOG_FILENAME__ ""
#else
#ifndef QCC74xLOG_SHORT_FILENAME
#define __QCC74xLOG_FILENAME__ __FILE__
#else
#define __QCC74xLOG_FILENAME__ (strrchr(__FILE__, __PATH_DELIMITER) + 1)
#endif
#endif
#endif

#ifndef QCC74xLOG_FUNCTION_ENABLE
#define __QCC74xLOG_FUNCTION__ ""
#else
#define __QCC74xLOG_FUNCTION__ __FUNCTION__
#endif

#ifndef QCC74xLOG_LINE_ENABLE
#define __QCC74xLOG_LINE__ 0
#else
#define __QCC74xLOG_LINE__ __LINE__
#endif

#define __QCC74xLOG_WRAP(x) #x

/*!< define a tag */
#define QCC74xLOG_DEFINE_TAG(name, _string, enable)                                                                        \
    __attribute__((unused)) struct _qcc74xlog_tag __qcc74xlog_tag_##name##__ __attribute__((section(".qcc74xlog_tags_array"))) = { \
        .tag = _string,                                                                                                \
        .en = enable ? 0xffffffff : 0x00000000                                                                         \
    }

/*!< extern a tag */
#define QCC74xLOG_EXTERN_TAG(name) extern struct _qcc74xlog_tag __qcc74xlog_tag_##name##__ __attribute__((section(".qcc74xlog_tags_array")))

/*!< get tag name */
#define QCC74xLOG_GET_TAG(name)    &__qcc74xlog_tag_##name##__

/*!< set tag */
#define QCC74xLOG_TAG              NULL

#ifdef QCC74xLOG_ENABLE

#define QCC74xLOG_X(_log, _level, _tag, ...) qcc74xlog((void *)(_log), (_level), (_tag), __QCC74xLOG_FILENAME__, __QCC74xLOG_FUNCTION__, __QCC74xLOG_LINE__, ##__VA_ARGS__)

#if (QCC74xLOG_LEVEL_ENABLE >= QCC74xLOG_LEVEL_FATAL)
#define QCC74xLOG_F(_log, ...)  QCC74xLOG_X(_log, QCC74xLOG_LEVEL_FATAL, (QCC74xLOG_TAG), ##__VA_ARGS__)
#define QCC74xLOG_RF(_log, ...) QCC74xLOG_X(_log, QCC74xLOG_LEVEL_RAW | QCC74xLOG_LEVEL_FATAL, (QCC74xLOG_TAG), ##__VA_ARGS__)
#else
#define QCC74xLOG_F(_log, ...)  ((void)(_log))
#define QCC74xLOG_RF(_log, ...) ((void)(_log))
#endif

#if (QCC74xLOG_LEVEL_ENABLE >= QCC74xLOG_LEVEL_ERROR)
#define QCC74xLOG_E(_log, ...)  QCC74xLOG_X(_log, QCC74xLOG_LEVEL_ERROR, (QCC74xLOG_TAG), ##__VA_ARGS__)
#define QCC74xLOG_RE(_log, ...) QCC74xLOG_X(_log, QCC74xLOG_LEVEL_RAW | QCC74xLOG_LEVEL_ERROR, (QCC74xLOG_TAG), ##__VA_ARGS__)
#else
#define QCC74xLOG_E(_log, ...)  ((void)(_log))
#define QCC74xLOG_RE(_log, ...) ((void)(_log))
#endif

#if (QCC74xLOG_LEVEL_ENABLE >= QCC74xLOG_LEVEL_WARN)
#define QCC74xLOG_W(_log, ...)  QCC74xLOG_X(_log, QCC74xLOG_LEVEL_WARN, (QCC74xLOG_TAG), ##__VA_ARGS__)
#define QCC74xLOG_RW(_log, ...) QCC74xLOG_X(_log, QCC74xLOG_LEVEL_RAW | QCC74xLOG_LEVEL_WARN, (QCC74xLOG_TAG), ##__VA_ARGS__)
#else
#define QCC74xLOG_W(_log, ...)  ((void)(_log))
#define QCC74xLOG_RW(_log, ...) ((void)(_log))
#endif

#if (QCC74xLOG_LEVEL_ENABLE >= QCC74xLOG_LEVEL_INFO)
#define QCC74xLOG_I(_log, ...)  QCC74xLOG_X(_log, QCC74xLOG_LEVEL_INFO, (QCC74xLOG_TAG), ##__VA_ARGS__)
#define QCC74xLOG_RI(_log, ...) QCC74xLOG_X(_log, QCC74xLOG_LEVEL_RAW | QCC74xLOG_LEVEL_INFO, (QCC74xLOG_TAG), ##__VA_ARGS__)
#else
#define QCC74xLOG_I(_log, ...)  ((void)(_log))
#define QCC74xLOG_RI(_log, ...) ((void)(_log))
#endif

#if (QCC74xLOG_LEVEL_ENABLE >= QCC74xLOG_LEVEL_DEBUG)
#define QCC74xLOG_D(_log, ...)  QCC74xLOG_X(_log, QCC74xLOG_LEVEL_DEBUG, (QCC74xLOG_TAG), ##__VA_ARGS__)
#define QCC74xLOG_RD(_log, ...) QCC74xLOG_X(_log, QCC74xLOG_LEVEL_RAW | QCC74xLOG_LEVEL_DEBUG, (QCC74xLOG_TAG), ##__VA_ARGS__)
#else
#define QCC74xLOG_D(_log, ...)  ((void)(_log))
#define QCC74xLOG_RD(_log, ...) ((void)(_log))
#endif

#if (QCC74xLOG_LEVEL_ENABLE >= QCC74xLOG_LEVEL_TRACE)
#define QCC74xLOG_T(_log, ...)  QCC74xLOG_X(_log, QCC74xLOG_LEVEL_TRACE, (QCC74xLOG_TAG), ##__VA_ARGS__)
#define QCC74xLOG_RT(_log, ...) QCC74xLOG_X(_log, QCC74xLOG_LEVEL_RAW | QCC74xLOG_LEVEL_TRACE, (QCC74xLOG_TAG), ##__VA_ARGS__)
#else
#define QCC74xLOG_T(_log, ...)  ((void)(_log))
#define QCC74xLOG_RT(_log, ...) ((void)(_log))
#endif

#else

#define QCC74xLOG_X(_log, _level, _tag, ...) (void)(_log), (void)(_level), (void)(_tag),

#define QCC74xLOG_F(_log, ...)               ((void)(_log))
#define QCC74xLOG_RF(_log, ...)              ((void)(_log))
#define QCC74xLOG_E(_log, ...)               ((void)(_log))
#define QCC74xLOG_RE(_log, ...)              ((void)(_log))
#define QCC74xLOG_W(_log, ...)               ((void)(_log))
#define QCC74xLOG_RW(_log, ...)              ((void)(_log))
#define QCC74xLOG_I(_log, ...)               ((void)(_log))
#define QCC74xLOG_RI(_log, ...)              ((void)(_log))
#define QCC74xLOG_D(_log, ...)               ((void)(_log))
#define QCC74xLOG_RD(_log, ...)              ((void)(_log))
#define QCC74xLOG_T(_log, ...)               ((void)(_log))
#define QCC74xLOG_RT(_log, ...)              ((void)(_log))

#endif

#endif
