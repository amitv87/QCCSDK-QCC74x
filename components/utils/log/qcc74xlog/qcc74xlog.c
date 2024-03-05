#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "qcc74xlog.h"

/** @addtogroup std_func
 * @{
 */

#ifndef qcc74xlogc_fopen
#define qcc74xlogc_fopen fopen
#endif

#ifndef qcc74xlogc_fclose
#define qcc74xlogc_fclose fclose
#endif

#ifndef qcc74xlogc_fwrite
#define qcc74xlogc_fwrite fwrite
#endif

#ifndef qcc74xlogc_ftell
#define qcc74xlogc_ftell ftell
#endif

#ifndef qcc74xlogc_fflush
#define qcc74xlogc_fflush fflush
#endif

#ifndef qcc74xlogc_memcpy
#define qcc74xlogc_memcpy memcpy
#endif

#ifndef qcc74xlogc_snprintf
#define qcc74xlogc_snprintf snprintf
#endif

#ifndef qcc74xlogc_vsnprintf
#define qcc74xlogc_vsnprintf vsnprintf
#endif

#ifndef qcc74xlogc_remove
#define qcc74xlogc_remove remove
#endif

#ifndef qcc74xlogc_rename
#define qcc74xlogc_rename rename
#endif

#ifndef qcc74xlogc_strcmp
#define qcc74xlogc_strcmp strcmp
#endif

/**
 * @}
 */

/** @addtogroup QCC74xLOG_LOCAL_MARCO
 * @{
 */

#ifdef CONFIG_QCC74xLOG_DEBUG
#define _QCC74xLOG_CHECK(_expr, _ret) \
    if (!(_expr))                 \
    return _ret
#else
#define _QCC74xLOG_CHECK(_expr, _ret) ((void)0)
#endif

#define _qcc74xlog_t(_ptr)            ((qcc74xlog_t *)(_ptr))

#define _msg_t(_ptr)              ((struct _qcc74xlog_msg *)(_ptr))
#define _tag_t(_ptr)              ((struct _qcc74xlog_tag *)(_ptr))

#define _direct_t(_ptr)           ((qcc74xlog_direct_t *)(_ptr))
#define _direct_buffer_t(_ptr)    ((qcc74xlog_direct_buffer_t *)(_ptr))
#define _direct_stream_t(_ptr)    ((qcc74xlog_direct_stream_t *)(_ptr))
#define _direct_file_t(_ptr)      ((qcc74xlog_direct_file_t *)(_ptr))
#define _direct_file_time_t(_ptr) ((qcc74xlog_direct_file_time_t *)(_ptr))
#define _direct_file_size_t(_ptr) ((qcc74xlog_direct_file_size_t *)(_ptr))

#define _layout_simple_t(_ptr)    ((qcc74xlog_layout_simple_t *)(_ptr))
#define _layout_format_t(_ptr)    ((qcc74xlog_layout_format_t *)(_ptr))
#define _layout_yaml_t(_ptr)      ((qcc74xlog_layout_yaml_t *)(_ptr))

/*!< default log record flag */
#ifndef QCC74xLOG_FLAG_DEFAULT
#define QCC74xLOG_FLAG_DEFAULT ( \
    QCC74xLOG_FLAG_LEVEL |       \
    QCC74xLOG_FLAG_TAG |         \
    QCC74xLOG_FLAG_FUNC |        \
    QCC74xLOG_FLAG_LINE |        \
    QCC74xLOG_FLAG_FILE |        \
    QCC74xLOG_FLAG_CLK |         \
    QCC74xLOG_FLAG_TIME |        \
    QCC74xLOG_FLAG_THREAD)
#endif

/*!< line buffer size (in stack) */
/*!< flush use 4xline buffer size in stack */
/*!< log   use 2xline buffer size in stack */
/*!< pay attention to prevent stack overflow */
#ifndef QCC74xLOG_LINE_BUFFER_SIZE
#define QCC74xLOG_LINE_BUFFER_SIZE 256
#endif

/*!< default log record level */
#ifndef QCC74xLOG_LEVEL_DEFAULT
#define QCC74xLOG_LEVEL_DEFAULT QCC74xLOG_LEVEL_INFO
#endif

/*!< default direct print level */
#ifndef QCC74xLOG_DIRECT_LEVEL_DEFAULT
#define QCC74xLOG_DIRECT_LEVEL_DEFAULT QCC74xLOG_LEVEL_INFO
#endif

/*!< file size rotate min size */
#ifndef QCC74xLOG_FILE_SIZE_MIN
#define QCC74xLOG_FILE_SIZE_MIN (128 * 1024)
#endif

/*!< file time rotate min interval */
#ifndef QCC74xLOG_FILE_INTERVAL_MIN
#define QCC74xLOG_FILE_INTERVAL_MIN (10 * 60)
#endif

/**
 * @}
 */
#if defined(CONFIG_LOG_NCOLOR) && CONFIG_LOG_NCOLOR
static char *qcc74xlog_color_strings[6] = {
    "",
    "",
    "",
    "",
    "",
    ""
};
#else
static char *qcc74xlog_color_strings[6] = {
    QCC74xLOG_COLOR_START QCC74xLOG_COLOR_RESET QCC74xLOG_COLOR_FATAL QCC74xLOG_COLOR_END,
    QCC74xLOG_COLOR_START QCC74xLOG_COLOR_RESET QCC74xLOG_COLOR_ERROR QCC74xLOG_COLOR_END,
    QCC74xLOG_COLOR_START QCC74xLOG_COLOR_RESET QCC74xLOG_COLOR_WARN QCC74xLOG_COLOR_END,
    QCC74xLOG_COLOR_START QCC74xLOG_COLOR_RESET QCC74xLOG_COLOR_INFO QCC74xLOG_COLOR_END,
    QCC74xLOG_COLOR_START QCC74xLOG_COLOR_RESET QCC74xLOG_COLOR_DEBUG QCC74xLOG_COLOR_END,
    QCC74xLOG_COLOR_START QCC74xLOG_COLOR_RESET QCC74xLOG_COLOR_TRACE QCC74xLOG_COLOR_END
};
#endif

static char *qcc74xlog_level_strings[6] = {
    QCC74xLOG_LEVEL_FATAL_STRING,
    QCC74xLOG_LEVEL_ERROR_STRING,
    QCC74xLOG_LEVEL_WARN_STRING,
    QCC74xLOG_LEVEL_INFO_STRING,
    QCC74xLOG_LEVEL_DEBUG_STRING,
    QCC74xLOG_LEVEL_TRACE_STRING
};

static uint32_t global_filter = 0xffffffff;

static char *qcc74xlog_dummy_string = "";

extern struct _qcc74xlog_tag __qcc74xlog_tags_start__;
extern struct _qcc74xlog_tag __qcc74xlog_tags_end__;

/**
 *   @brief         dummy function
 */
static int dummy(void)
{
    return 0;
}

/** @addtogroup QCC74xLOG_TIMESTAMP
 * @{
 */

#ifdef QCC74xLOG_TIMESTAMP_ENABLE

static const uint8_t month_day[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static const uint8_t leap_month_day[12] = {
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/**
 *   @brief         check leap year
 *   @param  year                   year
 *   @return uint8_t 1 leap year, 0 noleap year
 */
static uint8_t check_leap_year(uint16_t year)
{
    if (year % 4) {
        return 0;
    } else {
        if ((year % 100 == 0) && (year % 400 != 0)) {
            return 0;
        } else {
            return 1;
        }
    }
}

/**
 *   @brief         calculate week
 *   @param  time                   time
 *   @return uint8_t
 */
static void cal_weekday(qcc74xlog_tm_t *time)
{
    uint32_t y, m, d, w;

    y = time->year;
    m = time->mon;
    d = time->mday;

    if ((m == 1) || (m == 2)) {
        m += 12;
        y--;
    }

    w = (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400 + 1) % 7;

    time->wday = (uint8_t)w;
}

/**
 *   @brief         timestamp to time
 *   @param  timestamp              timestamp
 *   @param  time                   time
 */
void qcc74xlog_unix2time(uint32_t timestamp, qcc74xlog_tm_t *time)
{
#define QCC74xLOG_FOUR_YEAR_DAY      ((365 << 2) + 1)
#define QCC74xLOG_SEC_NUM_PER_DAY    (24 * 60 * 60)
#define QCC74xLOG_SEC_NUM_PER_HOUR   (60 * 60)
#define QCC74xLOG_SEC_NUM_PER_MINUTE (60)

#ifndef QCC74xLOG_TIMEZONE
#define QCC74xLOG_TIMEZONE 8
#endif

    uint32_t totle_day_num;
    uint32_t current_sec_num;

    uint16_t remain_day;

    uint16_t temp_year;

    const uint8_t *p = NULL;

    totle_day_num = timestamp / QCC74xLOG_SEC_NUM_PER_DAY;
    current_sec_num = timestamp % QCC74xLOG_SEC_NUM_PER_DAY;

    time->hour = current_sec_num / QCC74xLOG_SEC_NUM_PER_HOUR;
    time->min = (current_sec_num % QCC74xLOG_SEC_NUM_PER_HOUR) / QCC74xLOG_SEC_NUM_PER_MINUTE;
    time->sec = (current_sec_num % QCC74xLOG_SEC_NUM_PER_HOUR) % QCC74xLOG_SEC_NUM_PER_MINUTE;

    time->hour += QCC74xLOG_TIMEZONE;

    if (time->hour > 23) {
        time->hour -= 24;
        totle_day_num++;
    }

    time->year = 1970 + (totle_day_num / QCC74xLOG_FOUR_YEAR_DAY) * 4;
    remain_day = totle_day_num % QCC74xLOG_FOUR_YEAR_DAY;

    temp_year = check_leap_year(time->year) ? 366 : 365;
    while (remain_day >= temp_year) {
        time->year++;
        remain_day -= temp_year;
        temp_year = check_leap_year(time->year) ? 366 : 365;
    }

    p = check_leap_year(time->year) ? leap_month_day : month_day;
    remain_day++;
    time->mon = 0;
    while (remain_day > *(p + time->mon)) {
        remain_day -= *(p + time->mon);
        time->mon++;
    }

    time->mon++;
    time->mday = remain_day;

    time->wday = time->mday + 2 * time->mon + 3 * (time->mon + 1) / 5 +
                 time->year + time->year / 4 - time->year / 100 + time->year / 400;

    cal_weekday(time);
}

#endif
/**
 * @}
 */

/** @addtogroup QCC74xLOG_LINE_QUEUE
 * @{
 */

#define qcc74xlog_dlist_init(_plist) \
    (_plist)->next = (_plist);   \
    (_plist)->prev = (_plist)

#define qcc74xlog_dlist_insert_after(_plist, _pnode) \
    (_plist)->next->prev = (_pnode);             \
    (_pnode)->next = (_plist)->next;             \
    (_plist)->next = (_pnode);                   \
    (_pnode)->prev = (_plist)

#define qcc74xlog_dlist_insert_before(_plist, _pnode) \
    (_plist)->prev->next = (_pnode);              \
    (_pnode)->prev = (_plist)->prev;              \
    (_plist)->prev = (_pnode);                    \
    (_pnode)->next = (_plist)

#define qcc74xlog_dlist_remove(_pnode)         \
    (_pnode)->next->prev = (_pnode)->prev; \
    (_pnode)->prev->next = (_pnode)->next; \
    (_pnode)->next = (_pnode);             \
    (_pnode)->prev = (_pnode)

#define qcc74xlog_dlist_move_head(_plist, _pnode) \
    qcc74xlog_dlist_remove(_pnode);               \
    qcc74xlog_dlist_insert_after(_plist, _pnode)

#define qcc74xlog_dlist_move_tail(_plist, _pnode) \
    qcc74xlog_dlist_remove(_pnode);               \
    qcc74xlog_dlist_insert_before(_plist, _pnode)

#define qcc74xlog_dlist_is_empty(_plist) \
    ((void *)((_plist)->next) == (void *)(_plist))

#define QCC74xLOG_DLIST_CONTAINER_OF(_ptr, _type, _member) \
    ((_type *)((char *)(_ptr) - (unsigned int)(&((_type *)0)->_member)))

#define QCC74xLOG_DLIST_NEW(_list_name) \
    struct _qcc74xlog_list _list_name = { &(_list_name), &(_list_name) }

#define QCC74xLOG_DLIST_ENTRY(_pnode, _type, _member) \
    QCC74xLOG_DLIST_CONTAINER_OF(_pnode, _type, _member)

#define QCC74xLOG_DLIST_ENTRY_FIRST(_pnode, _type, _member) \
    QCC74xLOG_DLIST_CONTAINER_OF((_pnode)-next, _type, _member)

#define QCC74xLOG_DLIST_ENTRY_FIRST_ORNULL(_pnode, _type, _member) \
    (qcc74xlog_dlist_is_empty(_pnode) ? NULL : QCC74xLOG_DLIST_ENTRY_FIRST(_pnode, _type, _member))

#define QCC74xLOG_DLIST_FOREACH(_pos, _head)            for ((_pos) = (_head)->next; (_pos) != (_head); (_pos) = (_pos)->next)

#define QCC74xLOG_DLIST_FOREACH_NEXT                    QCC74xLOG_DLIST_FOREACH

#define QCC74xLOG_DLIST_FOREACH_PREV(_pos, _head)       for ((_pos) = (_head)->prev; (_pos) != (_head); (_pos) = (_pos)->prev)

#define QCC74xLOG_DLIST_FOREACH_S(_pos, _n, _head)      for ((_pos) = (_head)->next, (_n) = (_pos)->next; (_pos) != (_head); (_pos) = (_n), (_n) = (_pos)->next)

#define QCC74xLOG_DLIST_FOREACH_NEXT_S                  QCC74xLOG_DLIST_FOREACH_S

#define QCC74xLOG_DLIST_FOREACH_PREV_S(_pos, _n, _head) for ((_pos) = (_head)->prev, (_n) = (_pos)->prev; (_pos) != (_head); (_pos) = (_n), (_n) = (_pos)->prev)

/**
 * @}
 */

/** @addtogroup QCC74xLOG_LINE_QUEUE
 * @{
 */

/**
 *   @brief         free oldest message in queue
 *   @param  log                    recorder
 *   @param  free                   space to be released
 *   @return uint16_t               space actually released
 */
static void queue_free(qcc74xlog_t *log, uint16_t free)
{
    char *pool = (char *)(log->queue.pool);
    uint16_t rpos = log->queue.rpos;
    uint16_t space = 0;

    while (free > space) {
        /*!< oldest message size */
        uint16_t size = _msg_t(pool + rpos)->size;

        if (rpos + size >= log->queue.size) {
            rpos = rpos + size - log->queue.size;
        } else {
            rpos += size;
        }

        space += size;
    }

    log->queue.rpos = rpos;
    log->queue.free += space;
}

/**
 *   @brief         enqueue
 *   @param  log                    recorder
 *   @param  msg                    store message area
 */
static void queue_put(qcc74xlog_t *log, struct _qcc74xlog_msg *msg)
{
    char *pool = (char *)(log->queue.pool);
    uint16_t remain = log->queue.size - log->queue.wpos;

    /*!< max message size */
    if (msg->size > log->queue.size) {
        msg->size = log->queue.size;
    }

    /*!< free the oldest msg to store the latest msg */
    if (log->queue.free < msg->size) {
        queue_free(log, msg->size - log->queue.free);
    }

    /*!< ring loop */
    if (msg->size >= remain) {
        /*!< store to ring tail */
        qcc74xlogc_memcpy(pool + log->queue.wpos, msg, remain);

        log->queue.wpos = msg->size - remain;

        /*!< store to ring head */
        qcc74xlogc_memcpy(pool, (char *)msg + remain, log->queue.wpos);
    } else {
        qcc74xlogc_memcpy(pool + log->queue.wpos, msg, msg->size);

        log->queue.wpos += msg->size;
    }

    log->queue.free -= msg->size;
}

/**
 *   @brief         dequeue
 *   @param  log                    recorder
 *   @param  msg                    load message area
 */
static void queue_get(qcc74xlog_t *log, struct _qcc74xlog_msg *msg)
{
    char *pool = (char *)(log->queue.pool);
    uint16_t remain;
    uint16_t size;

    if (log->queue.free == log->queue.size) {
        msg->zero = 0xbd;
        return;
    }

    remain = log->queue.size - log->queue.rpos;
    size = _msg_t(pool + log->queue.rpos)->size;

    /*!< ring loop */
    if (size >= remain) {
        /*!< load from ring tail */
        qcc74xlogc_memcpy(msg, pool + log->queue.rpos, remain);

        log->queue.rpos = size - remain;

        /*!< load from ring head */
        qcc74xlogc_memcpy((char *)msg + remain, pool, log->queue.rpos);
    } else {
        qcc74xlogc_memcpy(msg, pool + log->queue.rpos, size);
        log->queue.rpos += size;
    }

    log->queue.free += size;

    ((char *)msg)[msg->size] = '\0';
}

/**
 * @}
 */

/** @addtogroup QCC74xLOG_FILTER
 * @{
 */

/**
 *   @brief         find the last bit set
 *   @param  word                   word
 *   @return int                    bit index
 */
static int qcc74xlog_fls(uint32_t word)
{
    int bit = 32;

    if (!word) {
        bit -= 1;
    }
    if (!(word & 0xffff0000)) {
        word <<= 16;
        bit -= 16;
    }
    if (!(word & 0xff000000)) {
        word <<= 8;
        bit -= 8;
    }
    if (!(word & 0xf0000000)) {
        word <<= 4;
        bit -= 4;
    }
    if (!(word & 0xc0000000)) {
        word <<= 2;
        bit -= 2;
    }
    if (!(word & 0x80000000)) {
        word <<= 1;
        bit -= 1;
    }

    return bit;
}

/**
 *   @brief         alloc a filter
 *   @return uint32_t  (0 on faild) filter bit
 */
static uint32_t qcc74xlog_filter_alloc(void)
{
    uint8_t shift = qcc74xlog_fls(global_filter) - 1;

    if (shift >= 0) {
        global_filter = global_filter & ~(0x1 << shift);
        return 0x1 << shift;
    } else {
        return 0;
    }
}

/**
 *   @brief         free a filter
 *   @param  filter                 filter bit
 */
static void qcc74xlog_filter_free(uint32_t filter)
{
    global_filter = global_filter | filter;
}

/**
 *   @brief         config filter
 *   @param  filter                 filter bit
 *   @param  tag_string             tag string pointer
 *   @return int
 */
static int qcc74xlog_filter_set(uint32_t filter, void *tag_string, uint8_t enable)
{
    struct _qcc74xlog_tag *ps = &__qcc74xlog_tags_start__;
    struct _qcc74xlog_tag *pe = &__qcc74xlog_tags_end__;

    while (ps < pe) {
        if ((tag_string == ps->tag) || (0 == qcc74xlogc_strcmp(tag_string, ps->tag))) {
            if (enable) {
                ps->en |= filter;
            } else {
                ps->en &= ~filter;
            }
            return 0;
        }
        ps += 1;
    }

    return -1;
}

/**
 * @}
 */

/** @addtogroup QCC74xLOG_BASE_API
 * @{
 */

int qcc74xlog_global_filter(void *tag_string, uint8_t enable)
{
    _QCC74xLOG_CHECK(tag_string != NULL, -1);

    return qcc74xlog_filter_set(0xffffffff, tag_string, enable);
}

/**
 *   @brief         create recorder, thread unsafe
 *   @param  log                    recorder
 *   @param  pool                   queue pool
 *   @param  size                   pool size
 *   @return int
 */
int qcc74xlog_create(qcc74xlog_t *log, void *pool, uint16_t size, uint8_t mode)
{
    _QCC74xLOG_CHECK(log != NULL, -1);
    _QCC74xLOG_CHECK(pool != NULL, -1);
    _QCC74xLOG_CHECK(size > 0, -1);

    qcc74xlog_dlist_init(&(log->direct));

    log->status = QCC74xLOG_STATUS_READY;
    log->flags = QCC74xLOG_FLAG_DEFAULT;
    log->level = QCC74xLOG_LEVEL_DEFAULT;
    log->mode = mode;

    log->queue.free = size;
    log->queue.wpos = 0;
    log->queue.rpos = 0;
    log->queue.size = size;
    log->queue.pool = pool;

    log->enter_critical = dummy;
    log->exit_critical = dummy;
    log->flush_notice = dummy;

    uint32_t filter = qcc74xlog_filter_alloc();
    if (filter > 0) {
        log->filter = filter;
    } else {
        return -1;
    }

    return 0;
}

/**
 *   @brief         delete recorder, thread unsafe
 *   @param  log                    recorder
 *   @return int
 */
int qcc74xlog_delete(qcc74xlog_t *log)
{
    _QCC74xLOG_CHECK(log, -1);

    struct _qcc74xlog_list *node;
    struct _qcc74xlog_list *next;
    void *direct;

    switch (log->status) {
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_SUSPEND:

            break;
        default:
            return 0;
    }

    if (log->enter_critical()) {
        return -1;
    }

    switch (log->status) {
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_SUSPEND:

            break;
        default:
            log->exit_critical();
            return -1;
    }

    QCC74xLOG_DLIST_FOREACH_NEXT_S(node, next, &(log->direct))
    {
        direct = QCC74xLOG_DLIST_ENTRY(node, qcc74xlog_direct_t, list);

        if (_direct_t(direct)->status == QCC74xLOG_DIRECT_STATUS_RUNNING) {
            _direct_t(direct)->status = QCC74xLOG_DIRECT_STATUS_READY;
        }

        qcc74xlog_dlist_remove(node);
    }

    log->status = QCC74xLOG_STATUS_ILLEGAL;

    qcc74xlog_filter_free(log->filter);

    log->exit_critical();

    return 0;
}

/**
 *   @brief         append directed output, thread safe
 *   @param  log                    recorder
 *   @param  direct                 directed output
 *   @return int
 */
int qcc74xlog_append(qcc74xlog_t *log, qcc74xlog_direct_t *direct)
{
    _QCC74xLOG_CHECK(log != NULL, -1);
    _QCC74xLOG_CHECK(direct != NULL, -1);

    switch (log->status) {
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_SUSPEND:
            break;
        default:
            return -1;
    }

    if (log->enter_critical()) {
        return -1;
    }

    switch (log->status) {
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_SUSPEND:
            break;
        default:
            log->exit_critical();
            return -1;
    }

    if (direct->status != QCC74xLOG_DIRECT_STATUS_READY) {
        log->exit_critical();
        return -1;
    }
    if (!qcc74xlog_dlist_is_empty(&(direct->list))) {
        log->exit_critical();
        return -1;
    }

    qcc74xlog_dlist_insert_before(&(log->direct), &(direct->list));
    direct->status = QCC74xLOG_DIRECT_STATUS_SUSPEND;

    log->exit_critical();

    return 0;
}

/**
 *   @brief         delete directed output, thread safe
 *   @param  log                    recorder
 *   @param  direct                 directed output
 *   @return int
 */
int qcc74xlog_remove(qcc74xlog_t *log, qcc74xlog_direct_t *direct)
{
    _QCC74xLOG_CHECK(log != NULL, -1);
    _QCC74xLOG_CHECK(direct != NULL, -1);

    struct _qcc74xlog_list *node;

    switch (log->status) {
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_SUSPEND:
            break;
        default:
            return -1;
    }

    if (log->enter_critical()) {
        return -1;
    }

    switch (log->status) {
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_SUSPEND:
            break;
        default:
            log->exit_critical();
            return -1;
    }

    if (direct->status != QCC74xLOG_DIRECT_STATUS_SUSPEND) {
        log->exit_critical();
        return -1;
    }

    if (qcc74xlog_dlist_is_empty(&(direct->list))) {
        log->exit_critical();
        return -1;
    }

    QCC74xLOG_DLIST_FOREACH_NEXT(node, &(log->direct))
    {
        if (QCC74xLOG_DLIST_ENTRY(node, qcc74xlog_direct_t, list) == direct) {
            qcc74xlog_dlist_remove(&(direct->list));
            direct->status = QCC74xLOG_DIRECT_STATUS_READY;

            log->exit_critical();
            return 0;
        }
    }

    log->exit_critical();
    return -1;
}

/**
 *   @brief         suspend recorder, thread safe
 *   @param  log                    recorder
 *   @return int
 */
int qcc74xlog_suspend(qcc74xlog_t *log)
{
    _QCC74xLOG_CHECK(log != NULL, -1);

    switch (log->status) {
        case QCC74xLOG_STATUS_SUSPEND:
            return 0;
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_RUNNING:
            break;
        default:
            return -1;
    }

    if (log->enter_critical()) {
        return -1;
    }

    switch (log->status) {
        case QCC74xLOG_STATUS_SUSPEND:
            log->exit_critical();
            return 0;
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_RUNNING:
            break;
        default:
            log->exit_critical();
            return -1;
    }

    log->status = QCC74xLOG_STATUS_SUSPEND;

    log->exit_critical();

    return 0;
}

/**
 *   @brief         resume recorder, thread safe
 *   @param  log                   recorder
 *   @return int
 */
int qcc74xlog_resume(qcc74xlog_t *log)
{
    _QCC74xLOG_CHECK(log != NULL, -1);

    switch (log->status) {
        case QCC74xLOG_STATUS_RUNNING:
            return 0;
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_SUSPEND:
            break;
        default:
            return -1;
    }

    if (log->enter_critical()) {
        return -1;
    }

    switch (log->status) {
        case QCC74xLOG_STATUS_RUNNING:
            log->exit_critical();
            return 0;
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_SUSPEND:
            break;
        default:
            log->exit_critical();
            return -1;
    }

    log->status = QCC74xLOG_STATUS_RUNNING;

    log->exit_critical();

    return 0;
}

/**
 *   @brief         config recorder, thread safe
 *   @param  log                    recorder
 *   @param  command                config command
 *   @param  param                  config param
 *   @return int
 */
int qcc74xlog_control(qcc74xlog_t *log, uint32_t command, uint32_t param)
{
    _QCC74xLOG_CHECK(log != NULL, -1);

    switch (log->status) {
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_SUSPEND:
            break;
        case QCC74xLOG_STATUS_RUNNING:
            if (command != QCC74xLOG_CMD_LEVEL) {
                return -1;
            }
            break;
        default:
            return -1;
    }

    switch (command) {
        case QCC74xLOG_CMD_ENTER_CRITICAL:
            if ((void *)param == NULL) {
                log->enter_critical = dummy;
            } else {
                log->enter_critical = (void *)param;
            }
            return 0;
        case QCC74xLOG_CMD_EXIT_CRITICAL: {
            if ((void *)param == NULL) {
                log->exit_critical = dummy;
            } else {
                log->exit_critical = (void *)param;
            }
        }
            return 0;
        default:
            break;
    }

    if (log->enter_critical()) {
        return -1;
    }

    switch (log->status) {
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_SUSPEND:
            break;
        case QCC74xLOG_STATUS_RUNNING:
            if (command != QCC74xLOG_CMD_LEVEL) {
                return -1;
            }
            break;
        default:
            log->exit_critical();
            return -1;
    }

    switch (command) {
        case QCC74xLOG_CMD_ILLEGAL:
            return -1;
        case QCC74xLOG_CMD_FLAG:
            log->flags = param & 0xff;
            break;
        case QCC74xLOG_CMD_LEVEL:
            log->level = param & QCC74xLOG_LEVEL_MASK;
            break;
        case QCC74xLOG_CMD_QUEUE_POOL:
            log->queue.pool = (void *)param;
            break;
        case QCC74xLOG_CMD_QUEUE_SIZE:
            log->queue.size = param & 0xffff;
            break;
        case QCC74xLOG_CMD_QUEUE_RST:
            log->queue.wpos = 0;
            log->queue.rpos = 0;
            log->queue.free = log->queue.size;
            break;
        case QCC74xLOG_CMD_FLUSH_NOTICE:
            if ((void *)param == NULL) {
                log->flush_notice = dummy;
            } else {
                log->flush_notice = (void *)param;
            }
            break;
        case QCC74xLOG_CMD_MODE:
            log->mode = param & 0xff;
            break;
        default:
            log->exit_critical();
            return -1;
    }

    log->exit_critical();

    return 0;
}

/**
 *   @brief         config recorder filter, thread safe
 *   @param  log                    recorder
 *   @param  tag_string             tag string
 *   @param  enable                 tag enable
 *   @return int
 */
int qcc74xlog_filter(qcc74xlog_t *log, void *tag_string, uint8_t enable)
{
    _QCC74xLOG_CHECK(log != NULL, -1);
    _QCC74xLOG_CHECK(tag_string != NULL, -1);

    int ret;

    switch (log->status) {
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_RUNNING:
        case QCC74xLOG_STATUS_SUSPEND:
            break;
        default:
            return -1;
    }

    if (log->enter_critical()) {
        return -1;
    }

    ret = qcc74xlog_filter_set(log->filter, tag_string, enable);

    if (log->exit_critical()) {
        return -1;
    }

    return ret;
}

/**
 *   @brief         record log msg, thread safe
 *                  tag, file, func only recorded pointer
 *   @param  log                    recorder
 *   @param  level                  level threshold
 *   @param  tag                    tag
 *   @param  file                   const file name
 *   @param  func                   const func name
 *   @param  line                   file line
 *   @param  format                 format string
 *   @param  ...                    format params
 *   @return int
 */
int qcc74xlog(void *log, uint8_t level, void *tag, const char *const file, const char *const func, const long line, const char *format, ...)
{
    _QCC74xLOG_CHECK(log != NULL, -1);
    _QCC74xLOG_CHECK(format != NULL, -1);

    int ret;
    va_list args;
    uint8_t msg[QCC74xLOG_LINE_BUFFER_SIZE + sizeof(struct _qcc74xlog_msg)];
    uint16_t size = sizeof(struct _qcc74xlog_msg);

    /*!< working only during running and suspend */
    switch (_qcc74xlog_t(log)->status) {
        case QCC74xLOG_STATUS_RUNNING:
            break;
        default:
            return -1;
    }

    /*!< level filter */
    if ((level & QCC74xLOG_LEVEL_MASK) > _qcc74xlog_t(log)->level) {
        return 0;
    }

    /*!< if advanced tag, use tag filter */
    if (((uintptr_t)(&__qcc74xlog_tags_start__) <= (uintptr_t)(tag)) &&
        ((uintptr_t)(tag) < (uintptr_t)(&__qcc74xlog_tags_end__))) {
        /*!< tag filter */
        if ((tag != NULL) &&
            ((_tag_t(tag)->en & _qcc74xlog_t(log)->filter) != _qcc74xlog_t(log)->filter)) {
            return 0;
        }
    }

    /*!< record clock tick */
    if (_qcc74xlog_t(log)->flags & QCC74xLOG_FLAG_CLK) {
        _msg_t(msg)->clk = qcc74xlog_clock();
    } else {
        _msg_t(msg)->clk = 0;
    }

    /*!< record timestamp */
    if (_qcc74xlog_t(log)->flags & QCC74xLOG_FLAG_TIME) {
        _msg_t(msg)->time = qcc74xlog_time();
    } else {
        _msg_t(msg)->time = 0;
    }

    /*!< record level */
    _msg_t(msg)->level = level;

    /*!< record tag, only record pointer */
    if (_qcc74xlog_t(log)->flags & QCC74xLOG_FLAG_TAG) {
        _msg_t(msg)->tag = tag;
    } else {
        _msg_t(msg)->tag = NULL;
    }

    /*!< record func, only record pointer */
    if (_qcc74xlog_t(log)->flags & QCC74xLOG_FLAG_FUNC) {
        _msg_t(msg)->func = func;
    } else {
        _msg_t(msg)->func = qcc74xlog_dummy_string;
    }

    /*!< record line */
    if (_qcc74xlog_t(log)->flags & QCC74xLOG_FLAG_LINE) {
        _msg_t(msg)->line = line;
    } else {
        _msg_t(msg)->line = 0;
    }

    /*!< record file, only record pointer */
    if (_qcc74xlog_t(log)->flags & QCC74xLOG_FLAG_FILE) {
        _msg_t(msg)->file = file;
    } else {
        _msg_t(msg)->file = qcc74xlog_dummy_string;
    }

    /*!< record thread, only record pointer */
    if (_qcc74xlog_t(log)->flags & QCC74xLOG_FLAG_THREAD) {
        _msg_t(msg)->thread = qcc74xlog_thread();
    } else {
        _msg_t(msg)->thread = qcc74xlog_dummy_string;
    }

    /*!< set zero */
    _msg_t(msg)->zero = 0;

    /*!< print string to msg->string */
    va_start(args, format);
    ret = qcc74xlogc_vsnprintf(_msg_t(msg)->string, QCC74xLOG_LINE_BUFFER_SIZE, format, args);
    va_end(args);

    /*!< check true size */
    if ((ret >= 0) && (ret <= QCC74xLOG_LINE_BUFFER_SIZE)) {
        size += ret;
    } else {
        size += QCC74xLOG_LINE_BUFFER_SIZE;
    }

    /*!< align 4 byte */
    if (size & 0x3) {
        _msg_t(msg)->size = (size & ~0x3) + 0x4;
    } else {
        _msg_t(msg)->size = size;
    }

    if (_qcc74xlog_t(log)->enter_critical()) {
        return -1;
    }
    /*!< working only during running and suspend */
    switch (_qcc74xlog_t(log)->status) {
        case QCC74xLOG_STATUS_RUNNING:
            break;
        default:
            _qcc74xlog_t(log)->exit_critical();
            return -1;
    }

    queue_put(_qcc74xlog_t(log), (void *)(&msg));

    if (_qcc74xlog_t(log)->mode & QCC74xLOG_MODE_ASYNC) {
        _qcc74xlog_t(log)->flush_notice();
        _qcc74xlog_t(log)->exit_critical();
        return 0;
    }

    _qcc74xlog_t(log)->exit_critical();

    return qcc74xlog_flush(_qcc74xlog_t(log));
}

/**
 *   @brief         flush all msg in queue, thread safe
 *   @param  log                    recorder
 *   @return int
 */
int qcc74xlog_flush(void *log)
{
    _QCC74xLOG_CHECK(log != NULL, -1);

    uint8_t msg[QCC74xLOG_LINE_BUFFER_SIZE + sizeof(struct _qcc74xlog_msg)];
    char buf[QCC74xLOG_LINE_BUFFER_SIZE * 2];
    struct _qcc74xlog_list *node;
    void *direct;
    uint32_t filter = _qcc74xlog_t(log)->filter;

    /*!< working only during running */
    switch (_qcc74xlog_t(log)->status) {
        case QCC74xLOG_STATUS_SUSPEND:
            return -1;
        case QCC74xLOG_STATUS_RUNNING:
            break;
        default:
            return -1;
    }

    do {
        /*!< reset msg string */
        _msg_t(msg)->string[0] = '\0';

        if (_qcc74xlog_t(log)->enter_critical()) {
            return -1;
        }

        /*!< working only during running */
        switch (_qcc74xlog_t(log)->status) {
            case QCC74xLOG_STATUS_SUSPEND:
                _qcc74xlog_t(log)->exit_critical();
                return -1;
            case QCC74xLOG_STATUS_RUNNING:
                break;
            default:
                _qcc74xlog_t(log)->exit_critical();
                return -1;
        }

        /*!< get msg(on stack) from queue(on ram) */
        queue_get(_qcc74xlog_t(log), (void *)(&msg));

        _qcc74xlog_t(log)->exit_critical();

        if (_msg_t(msg)->zero != 0) {
            if (_msg_t(msg)->zero == 0xbd) {
                /*!< no msg */
                return 0;
            } else {
                /*!< error */
                return -1;
            }
        }

        /*!< color */
        char *color;
        color = qcc74xlog_color_strings[_msg_t(msg)->level & QCC74xLOG_LEVEL_MASK];

        /*!< level */
        char *level;
        if (_qcc74xlog_t(log)->flags & QCC74xLOG_FLAG_LEVEL) {
            level = qcc74xlog_level_strings[_msg_t(msg)->level & QCC74xLOG_LEVEL_MASK];
        } else {
            level = qcc74xlog_dummy_string;
        }

#ifdef QCC74xLOG_TIMESTAMP_ENABLE
        /*!< time */
        qcc74xlog_tm_t tm;

        qcc74xlog_unix2time(_msg_t(msg)->time, &tm);
#endif

        /*!< check if advanced tag */
        uint8_t advanced_tag;
        if (((uintptr_t)(&__qcc74xlog_tags_start__) <= (uintptr_t)(_msg_t(msg)->tag)) &&
            ((uintptr_t)(_msg_t(msg)->tag) < (uintptr_t)(&__qcc74xlog_tags_end__))) {
            advanced_tag = 1;
        } else {
            advanced_tag = 0;
        }

        _qcc74xlog_t(log)->enter_critical();
        /*!< foreach direct, execute layout to buf(on stack), then output buf(on stack) */
        QCC74xLOG_DLIST_FOREACH_NEXT(node, &(_qcc74xlog_t(log)->direct))
        {
            _qcc74xlog_t(log)->exit_critical();

            int size;
            direct = QCC74xLOG_DLIST_ENTRY(node, qcc74xlog_direct_t, list);

            /*!< check direct status */
            if (_direct_t(direct)->status != QCC74xLOG_DIRECT_STATUS_RUNNING) {
                continue;
            }

            /*!< level filter */
            if ((_msg_t(msg)->level & QCC74xLOG_LEVEL_MASK) > _direct_t(direct)->level) {
                continue;
            }

            if (advanced_tag) {
                /*!< tag filter */
                if ((_msg_t(msg)->tag != NULL) &&
                    ((_tag_t(_msg_t(msg)->tag)->en & filter) != filter)) {
                    continue;
                }
            }

            char *tag;
            if ((_msg_t(msg)->tag == NULL)) {
                tag = qcc74xlog_dummy_string;
            } else {
                tag = advanced_tag ? _tag_t(_msg_t(msg)->tag)->tag : _msg_t(msg)->tag;
            }

            /*!< raw output */
            if (_msg_t(msg)->level & QCC74xLOG_LEVEL_RAW) {
                size = qcc74xlogc_snprintf(
                    buf,
                    QCC74xLOG_LINE_BUFFER_SIZE * 2,
                    "%s", _msg_t(msg)->string);
                goto output;
            }

            /*!< nolayout */
            if (_direct_t(direct)->layout == NULL) {
                goto simple_layout;
            }

            /*!< layout */
            switch (_direct_t(direct)->layout->type) {
                case QCC74xLOG_LAYOUT_TYPE_FORMAT:
                    if (_direct_t(direct)->color) {
#ifdef QCC74xLOG_TIMESTAMP_ENABLE
                        size = _layout_format_t(_direct_t(direct)->layout)
                                   ->snprintf(
                                       buf,
                                       QCC74xLOG_LINE_BUFFER_SIZE * 2,
                                       color,
                                       level,
                                       tag,
                                       &tm,
                                       _msg_t(msg));
#else
                        size = _layout_format_t(_direct_t(direct)->layout)
                                   ->snprintf(
                                       buf,
                                       QCC74xLOG_LINE_BUFFER_SIZE * 2,
                                       color,
                                       level,
                                       tag,
                                       NULL,
                                       _msg_t(msg));
#endif
                    } else {
#ifdef QCC74xLOG_TIMESTAMP_ENABLE
                        size = _layout_format_t(_direct_t(direct)->layout)
                                   ->snprintf(
                                       buf,
                                       QCC74xLOG_LINE_BUFFER_SIZE * 2,
                                       qcc74xlog_dummy_string,
                                       level,
                                       tag,
                                       &tm,
                                       _msg_t(msg));
#else
                        size = _layout_format_t(_direct_t(direct)->layout)
                                   ->snprintf(
                                       buf,
                                       QCC74xLOG_LINE_BUFFER_SIZE * 2,
                                       qcc74xlog_dummy_string,
                                       level,
                                       tag,
                                       NULL,
                                       _msg_t(msg));
#endif
                    }
                    goto output;

                    /*!< TODO Layout yaml format */
                case QCC74xLOG_LAYOUT_TYPE_YAML:
                case QCC74xLOG_LAYOUT_TYPE_SIMPLE:
                default:
                    goto simple_layout;
            }

        simple_layout:

            if (_direct_t(direct)->color) {
                /*!< default and simple color format */
#ifdef QCC74xLOG_TIMESTAMP_ENABLE
                size = qcc74xlogc_snprintf(
                    buf,
                    QCC74xLOG_LINE_BUFFER_SIZE * 2,
                    QCC74xLOG_SIMPLE_LAYOUT_STRING(
                        color,
                        level,
                        tag,
                        &tm,
                        _msg_t(msg)));
#else
                size = qcc74xlogc_snprintf(
                    buf,
                    QCC74xLOG_LINE_BUFFER_SIZE * 2,
                    QCC74xLOG_SIMPLE_LAYOUT_STRING(
                        color,
                        level,
                        tag,
                        NULL,
                        _msg_t(msg)));
#endif

            } else {
                /*!< default and simple no color format */
#ifdef QCC74xLOG_TIMESTAMP_ENABLE
                size = qcc74xlogc_snprintf(
                    buf,
                    QCC74xLOG_LINE_BUFFER_SIZE * 2,
                    QCC74xLOG_SIMPLE_LAYOUT_STRING(
                        qcc74xlog_dummy_string,
                        level,
                        tag,
                        &tm,
                        _msg_t(msg)));
#else
                size = qcc74xlogc_snprintf(
                    buf,
                    QCC74xLOG_LINE_BUFFER_SIZE * 2,
                    QCC74xLOG_SIMPLE_LAYOUT_STRING(
                        qcc74xlog_dummy_string,
                        level,
                        tag,
                        NULL,
                        _msg_t(msg)));
#endif
            }

        output:
            /*!< check true size */
            if ((size < 0) || (size > QCC74xLOG_LINE_BUFFER_SIZE * 2)) {
                size = QCC74xLOG_LINE_BUFFER_SIZE * 2;
            }

            if (_direct_t(direct)->lock()) {
                /*!< drop log message */
            } else {
                /*!< call write */
                _direct_t(direct)->write(direct, buf, size);

                _direct_t(direct)->unlock();
            }

            _qcc74xlog_t(log)->enter_critical();
        }

        _qcc74xlog_t(log)->exit_critical();
    } while (1);

    return -1;
}

/**
 * @}
 */

/** @addtogroup QCC74xLOG_DIRECT_API
 * @{
 */

/**
 *   @brief         create directed output, thread unsafe
 *   @param  direct                 directed output
 *   @param  type                   directed output type
 *   @return int
 */
int qcc74xlog_direct_create(qcc74xlog_direct_t *direct, uint8_t type, uint8_t color, void(*lock), void(*unlock))
{
    _QCC74xLOG_CHECK(direct != NULL, -1);
    _QCC74xLOG_CHECK(
        type != QCC74xLOG_DIRECT_TYPE_BUFFER ||
            type != QCC74xLOG_DIRECT_TYPE_STREAM ||
            type != QCC74xLOG_DIRECT_TYPE_FILE ||
            type != QCC74xLOG_DIRECT_TYPE_FILE_TIME ||
            type != QCC74xLOG_DIRECT_TYPE_FILE_SIZE,
        -1);

    qcc74xlog_dlist_init(&(direct->list));

    direct->write = NULL;
    direct->layout = NULL;
    direct->status = QCC74xLOG_DIRECT_STATUS_INIT;
    direct->level = QCC74xLOG_DIRECT_LEVEL_DEFAULT;
    direct->type = type;
    direct->color = color;

    if ((lock == NULL) || (unlock == NULL)) {
        direct->lock = dummy;
        direct->unlock = dummy;
    } else {
        direct->lock = lock;
        direct->unlock = unlock;
    }

    return 0;
}

/**
 *   @brief         delete directed output, thread unsafe
 *   @param  direct                 directed output
 *   @return int
 */
int qcc74xlog_direct_delete(qcc74xlog_direct_t *direct)
{
    _QCC74xLOG_CHECK(direct != NULL, -1);

    switch (direct->status) {
        case QCC74xLOG_DIRECT_STATUS_INIT:
            break;
        default:
            return -1;
    }

    if (direct->lock()) {
        return -1;
    }

    switch (direct->status) {
        case QCC74xLOG_DIRECT_STATUS_INIT:
            break;
        default:
            direct->unlock();
            return -1;
    }

    qcc74xlog_dlist_remove(&(direct->list));

    direct->write = NULL;
    direct->status = QCC74xLOG_DIRECT_STATUS_ILLEGAL;
    direct->type = QCC74xLOG_DIRECT_TYPE_ILLEGAL;

    direct->lock = dummy;

    int (*unlock)(void) = direct->unlock;
    direct->unlock = dummy;

    qcc74xlog_filter_free(direct->filter);

    unlock();

    return 0;
}

/**
 *   @brief         suspend directed output, thread safe
 *   @param  direct                 directed output
 *   @return int
 */
int qcc74xlog_direct_suspend(qcc74xlog_direct_t *direct)
{
    _QCC74xLOG_CHECK(direct != NULL, -1);

    switch (direct->status) {
        case QCC74xLOG_DIRECT_STATUS_RUNNING:
        case QCC74xLOG_DIRECT_STATUS_SUSPEND:
            break;
        default:
            return -1;
    }

    direct->status = QCC74xLOG_STATUS_SUSPEND;

    return 0;
}

/**
 *   @brief         resume directed output, thread safe
 *   @param  direct                 directed output
 *   @return int
 */
int qcc74xlog_direct_resume(qcc74xlog_direct_t *direct)
{
    _QCC74xLOG_CHECK(direct != NULL, -1);

    switch (direct->status) {
        case QCC74xLOG_DIRECT_STATUS_RUNNING:
        case QCC74xLOG_DIRECT_STATUS_SUSPEND:
            break;
        default:
            return -1;
    }

    direct->status = QCC74xLOG_STATUS_RUNNING;

    return 0;
}

/**
 *   @brief         link to a layout, thread unsafe
 *   @param  direct                 directed output
 *   @param  layout                 layout
 *   @return int
 */
int qcc74xlog_direct_link(qcc74xlog_direct_t *direct, qcc74xlog_layout_t *layout)
{
    _QCC74xLOG_CHECK(direct != NULL, -1);
    _QCC74xLOG_CHECK(layout != NULL, -1);

    if ((direct->status != QCC74xLOG_DIRECT_STATUS_READY) &&
        (direct->status != QCC74xLOG_DIRECT_STATUS_SUSPEND)) {
        return -1;
    }

    if (layout->status != QCC74xLOG_LAYOUT_STATUS_READY) {
        return -1;
    }

    direct->layout = layout;

    return 0;
}

/**
 *   @brief         config direct
 *   @param  direct
 *   @param  command
 *   @param  param
 *   @return int
 */
int qcc74xlog_direct_control(qcc74xlog_direct_t *direct, uint32_t command, uint32_t param)
{
    _QCC74xLOG_CHECK(direct != NULL, -1);

    switch (direct->status) {
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_SUSPEND:
            break;
        case QCC74xLOG_STATUS_RUNNING:
            if (command != QCC74xLOG_DIRECT_CMD_LEVEL) {
                return -1;
            }
            break;
        default:
            return -1;
    }

    switch (command) {
        case QCC74xLOG_DIRECT_CMD_LEVEL:
            direct->level = param & QCC74xLOG_LEVEL_MASK;
            break;

        case QCC74xLOG_DIRECT_CMD_COLOR:
            direct->color = param != 0;
            break;

        case QCC74xLOG_DIRECT_CMD_LOCK:
            if ((void *)param == NULL) {
                direct->lock = dummy;
            } else {
                direct->lock = (void *)param;
            }
            break;

        case QCC74xLOG_DIRECT_CMD_UNLOCK:
            if ((void *)param == NULL) {
                direct->unlock = dummy;
            } else {
                direct->unlock = (void *)param;
            }
            break;

        default:
            return -1;
    }

    return 0;
}

/**
 *   @brief         config direct filter, thread safe
 *   @param  direct                 direct
 *   @param  tag_string             tag string
 *   @param  enable                 tag enable
 *   @return int
 */
int qcc74xlog_direct_filter(qcc74xlog_direct_t *direct, void *tag_string, uint8_t enable)
{
    _QCC74xLOG_CHECK(direct != NULL, -1);
    _QCC74xLOG_CHECK(tag_string != NULL, -1);

    int ret;

    switch (direct->status) {
        case QCC74xLOG_STATUS_READY:
        case QCC74xLOG_STATUS_RUNNING:
        case QCC74xLOG_STATUS_SUSPEND:
            break;
        default:
            return -1;
    }

    ret = qcc74xlog_filter_set(direct->filter, tag_string, enable);

    return ret;
}

/**
 * @}
 */

/** @addtogroup QCC74xLOG_DIRECT_TYPE_API
 * @{
 */

/** @addtogroup QCC74xLOG_DIRECT_BUFFER_API
 * @{
 */
#ifdef QCC74xLOG_DIRECT_BUFFER_ENABLE
#if 0
static void qcc74xlog_driect_write_buffer(qcc74xlog_direct_t *direct, void *ptr, uint16_t size)
{
    /*!< TODO */
    return;
}

int qcc74xlog_direct_init_buffer_s(qcc74xlog_direct_t *direct, void *buffer, void *size)
{
    /*!< TODO */
    (void)qcc74xlog_driect_write_buffer;
    return 0;
}

int qcc74xlog_direct_deinit_buffer_s(qcc74xlog_direct_t *direct)
{
    /*!< TODO */
    return 0;
}
#endif
#endif
/**
 * @}
 */

/** @addtogroup QCC74xLOG_DIRECT_STREAM_API
 * @{
 */
#ifdef QCC74xLOG_DIRECT_STREAM_ENABLE
/**
 *   @brief         write data to stream
 *   @param  log                    recorder
 *   @param  ptr                    data pointer
 *   @param  size                   data size
 *   @return int
 */
static void qcc74xlog_direct_write_stream(qcc74xlog_direct_t *direct, void *ptr, uint16_t size)
{
    _direct_stream_t(direct)->stream_output(ptr, size);
}

/**
 *   @brief         init stream type direct, thread safe
 *   @param  direct                 directed output
 *   @param  stream_output          stream output function pointer
 *   @return int
 */
int qcc74xlog_direct_init_stream(qcc74xlog_direct_t *direct, uint16_t (*stream_output)(void *, uint16_t))
{
    _QCC74xLOG_CHECK(direct != NULL, -1);
    _QCC74xLOG_CHECK(stream_output != NULL, -1);

    if (direct->lock()) {
        return -1;
    }

    if (direct->type != QCC74xLOG_DIRECT_TYPE_STREAM) {
        direct->unlock();
        return -1;
    }

    if (direct->status != QCC74xLOG_DIRECT_STATUS_INIT) {
        direct->unlock();
        return -1;
    }

    _direct_stream_t(direct)->stream_output = stream_output;

    direct->status = QCC74xLOG_DIRECT_STATUS_READY;
    direct->write = qcc74xlog_direct_write_stream;

    direct->unlock();

    return 0;
}

/**
 *   @brief         deinit stream type direct
 *   @param  direct                 directed output
 *   @return int 
 */
int qcc74xlog_direct_deinit_stream(qcc74xlog_direct_t *direct)
{
    _QCC74xLOG_CHECK(direct != NULL, -1);

    if (direct->type != QCC74xLOG_DIRECT_TYPE_STREAM) {
        return -1;
    }

    if (direct->status != QCC74xLOG_DIRECT_STATUS_READY) {
        return -1;
    }

    if (direct->lock()) {
        return -1;
    }

    direct->status = QCC74xLOG_DIRECT_STATUS_INIT;
    direct->write = NULL;

    _direct_stream_t(direct)->stream_output = NULL;

    direct->unlock();

    return 0;
}

#endif

/**
 * @}
 */

/** @addtogroup QCC74xLOG_DIRECT_FILE_API
 * @{
 */
#ifdef QCC74xLOG_DIRECT_FILE_ENABLE
/**
 *   @brief         write data to file
 *   @param  direct                 directed output
 *   @param  ptr                    data ptr
 *   @param  size                   data size
 */
static void qcc74xlog_direct_write_file(qcc74xlog_direct_t *direct, void *ptr, uint16_t size)
{
    _QCC74xLOG_CHECK(direct != NULL, );
    _QCC74xLOG_CHECK(ptr != NULL, );

    if (_direct_file_t(direct)->fp == NULL) {
        return;
    }

    qcc74xlogc_fwrite(ptr, 1, size, _direct_file_t(direct)->fp);
    qcc74xlogc_fflush(_direct_file_t(direct)->fp);
}

/**
 *   @brief         
 *   @param  direct                 
 *   @param  path                                 
 *   @return int 
 */
int qcc74xlog_direct_init_file(qcc74xlog_direct_t *direct, const char *path)
{
    _QCC74xLOG_CHECK(direct != NULL, -1);
    _QCC74xLOG_CHECK(path != NULL, -1);

    char fullpath[256];

    if (direct->lock()) {
        return -1;
    }

    if (direct->type != QCC74xLOG_DIRECT_TYPE_FILE) {
        direct->unlock();
        return -1;
    }

    if (direct->status != QCC74xLOG_DIRECT_STATUS_INIT) {
        direct->unlock();
        return -1;
    }

    size_t pathsize = strlen(path);
    qcc74xlogc_memcpy(fullpath, path, pathsize);
    qcc74xlogc_snprintf(fullpath + pathsize, 16, ".log");

    _direct_file_t(direct)->fp = qcc74xlogc_fopen(fullpath, "a+");
    if (_direct_file_t(direct)->fp == NULL) {
        direct->unlock();
        return -1;
    }

    _direct_file_t(direct)->path = path;

    direct->status = QCC74xLOG_DIRECT_STATUS_READY;
    direct->write = qcc74xlog_direct_write_file;

    direct->unlock();

    return 0;
}

/**
 *   @brief
 *   @param  direct
 *   @return int
 */
int qcc74xlog_direct_deinit_file(qcc74xlog_direct_t *direct)
{
    _QCC74xLOG_CHECK(direct != NULL, -1);

    if (direct->type != QCC74xLOG_DIRECT_TYPE_FILE) {
        return -1;
    }

    if (direct->status != QCC74xLOG_DIRECT_STATUS_READY) {
        return -1;
    }

    if (direct->lock()) {
        return -1;
    }

    if (qcc74xlogc_fclose(_direct_file_t(direct)->fp)) {
        direct->unlock();
        return -1;
    }

    direct->status = QCC74xLOG_DIRECT_STATUS_INIT;
    direct->write = NULL;

    _direct_file_t(direct)->fp = NULL;
    _direct_file_t(direct)->path = NULL;

    direct->unlock();

    return 0;
}

#endif
/**
 * @}
 */

/** @addtogroup QCC74xLOG_DIRECT_FILE_TIME_API
 * @{
 */

#if (defined(QCC74xLOG_DIRECT_FILE_SIZE_ENABLE) || defined(QCC74xLOG_DIRECT_FILE_TIME_ENABLE))

/**
 *   @brief         file rotate
 *   @param  direct                 file_size/time directed output
 */
static void file_rotate(qcc74xlog_direct_file_size_t *direct)
{
    char oldpath[256];
    char newpath[256];

    void *fp;

    size_t pathsize = strlen(direct->path);
    qcc74xlogc_memcpy(oldpath, direct->path, pathsize);
    qcc74xlogc_memcpy(newpath, direct->path, pathsize);

    if (direct->fp) {
        qcc74xlogc_fclose(direct->fp);
    }

    for (int i = direct->keep - 1; i >= 0; i--) {
        qcc74xlogc_snprintf(oldpath + pathsize, 16, i ? "_%d.log" : ".log", i - 1);
        qcc74xlogc_snprintf(newpath + pathsize, 16, "_%d.log", i);

        fp = qcc74xlogc_fopen(newpath, "r");
        if (fp != NULL) {
            qcc74xlogc_fclose(fp);
            qcc74xlogc_remove(newpath);
        }

        fp = qcc74xlogc_fopen(oldpath, "r");
        if (fp != NULL) {
            qcc74xlogc_fclose(fp);
            qcc74xlogc_rename(oldpath, newpath);
        }
    }

    direct->fp = qcc74xlogc_fopen(oldpath, "a+");
}

#endif

#ifdef QCC74xLOG_DIRECT_FILE_TIME_ENABLE

/**
 *   @brief         write data to file
 *   @param  direct
 *   @param  ptr
 *   @param  size
 */
static void qcc74xlog_direct_write_file_time(qcc74xlog_direct_t *direct, void *ptr, uint16_t size)
{
    _QCC74xLOG_CHECK(direct != NULL, );
    _QCC74xLOG_CHECK(ptr != NULL, );

    uint32_t timestamp;

    if (_direct_file_time_t(direct)->fp == NULL) {
        return;
    }

    timestamp = qcc74xlog_time();

    if (timestamp > _direct_file_time_t(direct)->timestamp + _direct_file_time_t(direct)->interval) {
        file_rotate((void *)direct);
        _direct_file_time_t(direct)->timestamp = timestamp;
    }

    qcc74xlogc_fwrite(ptr, 1, size, _direct_file_size_t(direct)->fp);
    qcc74xlogc_fflush(_direct_file_size_t(direct)->fp);
}

/**
 *   @brief         
 *   @param  direct                 directed output
 *   @param  path                   file path
 *   @param  interval               file rotate interval
 *   @param  keep                   max keep file count
 *   @return int 
 */
int qcc74xlog_direct_init_file_time(qcc74xlog_direct_t *direct, const char *path, uint32_t interval, uint32_t keep)
{
    _QCC74xLOG_CHECK(direct != NULL, -1);
    _QCC74xLOG_CHECK(path != NULL, -1);
    _QCC74xLOG_CHECK(interval >= QCC74xLOG_FILE_INTERVAL_MIN, -1);
    _QCC74xLOG_CHECK(keep > 0, -1);

    if (interval < QCC74xLOG_FILE_INTERVAL_MIN) {
        interval = QCC74xLOG_FILE_INTERVAL_MIN;
    }

    char fullpath[256];

    if (direct->lock()) {
        return -1;
    }

    if (direct->type != QCC74xLOG_DIRECT_TYPE_FILE_TIME) {
        direct->unlock();
        return -1;
    }

    if (direct->status != QCC74xLOG_DIRECT_STATUS_INIT) {
        direct->unlock();
        return -1;
    }

    size_t pathsize = strlen(path);
    qcc74xlogc_memcpy(fullpath, path, pathsize);
    qcc74xlogc_snprintf(fullpath + pathsize, 16, ".log");

    _direct_file_time_t(direct)->fp = qcc74xlogc_fopen(fullpath, "a+");
    if (_direct_file_time_t(direct)->fp == NULL) {
        direct->unlock();
        return -1;
    }

    _direct_file_time_t(direct)->timestamp = qcc74xlog_time();
    _direct_file_time_t(direct)->keep = keep;
    _direct_file_time_t(direct)->interval = interval;
    _direct_file_time_t(direct)->path = path;

    direct->status = QCC74xLOG_DIRECT_STATUS_READY;
    direct->write = qcc74xlog_direct_write_file_time;

    direct->unlock();

    return 0;
}

/**
 *   @brief
 *   @param  direct
 *   @return int
 */
int qcc74xlog_direct_deinit_file_time(qcc74xlog_direct_t *direct)
{
    _QCC74xLOG_CHECK(direct != NULL, -1);

    if (direct->type != QCC74xLOG_DIRECT_TYPE_FILE_TIME) {
        return -1;
    }

    if (direct->status != QCC74xLOG_DIRECT_STATUS_READY) {
        return -1;
    }

    if (direct->lock()) {
        return -1;
    }

    if (qcc74xlogc_fclose(_direct_file_time_t(direct)->fp)) {
        direct->unlock();
        return -1;
    }

    direct->status = QCC74xLOG_DIRECT_STATUS_INIT;
    direct->write = NULL;
    _direct_file_time_t(direct)->fp = NULL;
    _direct_file_time_t(direct)->path = NULL;
    _direct_file_time_t(direct)->keep = 0;

    direct->unlock();

    return 0;
}

#endif

/**
 * @}
 */

/** @addtogroup QCC74xLOG_DIRECT_FILE_SIZE_API
 * @{
 */

#ifdef QCC74xLOG_DIRECT_FILE_SIZE_ENABLE

/**
 *   @brief         write data to file
 *   @param  direct
 *   @param  ptr
 *   @param  size
 */
static void qcc74xlog_direct_write_file_size(qcc74xlog_direct_t *direct, void *ptr, uint16_t size)
{
    _QCC74xLOG_CHECK(direct != NULL, );
    _QCC74xLOG_CHECK(ptr != NULL, );

    if (_direct_file_size_t(direct)->fp == NULL) {
        return;
    }

    /*!< fseek(_direct_file_size_t(direct)->fp, 0L, SEEK_END); */
    size_t fsize = qcc74xlogc_ftell(_direct_file_size_t(direct)->fp);

    if (fsize > _direct_file_size_t(direct)->size) {
        file_rotate((void *)direct);
    }

    qcc74xlogc_fwrite(ptr, 1, size, _direct_file_size_t(direct)->fp);
    qcc74xlogc_fflush(_direct_file_size_t(direct)->fp);
}

/**
 *   @brief         
 *   @param  direct                 
 *   @param  path                   
 *   @param  size                   
 *   @param  keep                                
 *   @return int 
 */
int qcc74xlog_direct_init_file_size(qcc74xlog_direct_t *direct, const char *path, uint32_t size, uint32_t keep)
{
    _QCC74xLOG_CHECK(direct != NULL, -1);
    _QCC74xLOG_CHECK(path != NULL, -1);
    _QCC74xLOG_CHECK(keep > 0, -1);

    if (size < QCC74xLOG_FILE_SIZE_MIN) {
        size = QCC74xLOG_FILE_SIZE_MIN;
    }

    char fullpath[256];

    if (direct->lock()) {
        return -1;
    }

    if (direct->type != QCC74xLOG_DIRECT_TYPE_FILE_SIZE) {
        direct->unlock();
        return -1;
    }

    if (direct->status != QCC74xLOG_DIRECT_STATUS_INIT) {
        direct->unlock();
        return -1;
    }

    size_t pathsize = strlen(path);
    qcc74xlogc_memcpy(fullpath, path, pathsize);
    qcc74xlogc_snprintf(fullpath + pathsize, 16, ".log");

    _direct_file_size_t(direct)->fp = qcc74xlogc_fopen(fullpath, "a+");
    if (_direct_file_size_t(direct)->fp == NULL) {
        direct->unlock();
        return -1;
    }

    _direct_file_size_t(direct)->keep = keep;
    _direct_file_size_t(direct)->size = size;
    _direct_file_size_t(direct)->path = path;

    direct->status = QCC74xLOG_DIRECT_STATUS_READY;
    direct->write = qcc74xlog_direct_write_file_size;

    direct->unlock();

    return 0;
}

/**
 *   @brief
 *   @param  direct
 *   @return int
 */
int qcc74xlog_direct_deinit_file_size(qcc74xlog_direct_t *direct)
{
    _QCC74xLOG_CHECK(direct != NULL, -1);

    if (direct->type != QCC74xLOG_DIRECT_TYPE_FILE_SIZE) {
        return -1;
    }

    if (direct->status != QCC74xLOG_DIRECT_STATUS_READY) {
        return -1;
    }

    if (direct->lock()) {
        return -1;
    }

    if (qcc74xlogc_fclose(_direct_file_size_t(direct)->fp)) {
        direct->unlock();
        return -1;
    }

    direct->status = QCC74xLOG_DIRECT_STATUS_INIT;
    direct->write = NULL;
    _direct_file_size_t(direct)->fp = NULL;
    _direct_file_size_t(direct)->path = NULL;
    _direct_file_size_t(direct)->keep = 0;

    direct->unlock();

    return 0;
}

#endif

/**
 * @}
 */

/**
 * @}
 */

/** @addtogroup QCC74xLOG_LAYOUT_API
 * @{
 */

/**
 *   @brief         create layout, thread unsafe
 *   @param  layout                 layout
 *   @param  type                   layout type
 *   @param  color                  color enable
 *   @return int
 */
int qcc74xlog_layout_create(qcc74xlog_layout_t *layout, uint8_t type)
{
    _QCC74xLOG_CHECK(layout != NULL, -1);
    _QCC74xLOG_CHECK(
        type != QCC74xLOG_LAYOUT_TYPE_SIMPLE ||
            type != QCC74xLOG_LAYOUT_TYPE_FORMAT ||
            type != QCC74xLOG_LAYOUT_TYPE_YAML,
        -1);

    switch (type) {
        case QCC74xLOG_LAYOUT_TYPE_SIMPLE:
            layout->status = QCC74xLOG_LAYOUT_STATUS_READY;
            break;
        case QCC74xLOG_LAYOUT_TYPE_FORMAT:
            layout->status = QCC74xLOG_LAYOUT_STATUS_INIT;
            break;
        case QCC74xLOG_LAYOUT_TYPE_YAML:

            break;
    }

    layout->type = type;

    return 0;
}

/**
 *   @brief         delete layout, thread unsafe
 *   @param  layout                 layout
 *   @return int
 */
int qcc74xlog_layout_delete(qcc74xlog_layout_t *layout)
{
    _QCC74xLOG_CHECK(layout != NULL, -1);

    layout->status = QCC74xLOG_LAYOUT_STATUS_ILLEGAL;

    return 0;
}

/**
 *   @brief         set formatting of layout format, thread unsafe
 *   @param  layout                 layout
 *   @param  snprintf               format
 *   @return int
 */
int qcc74xlog_layout_format(qcc74xlog_layout_t *layout, int (*u_snprintf)(void *ptr, uint16_t size, char *color, char *level, char *tag, qcc74xlog_tm_t *tm, struct _qcc74xlog_msg *msg))
{
    _QCC74xLOG_CHECK(layout != NULL, -1);
    _QCC74xLOG_CHECK(u_snprintf != NULL, -1);

    if (layout->type != QCC74xLOG_LAYOUT_TYPE_FORMAT) {
        return -1;
    }

    if (layout->status != QCC74xLOG_LAYOUT_STATUS_INIT) {
        return -1;
    }

    _layout_format_t(layout)->snprintf = u_snprintf;
    layout->status = QCC74xLOG_LAYOUT_STATUS_READY;

    return 0;
}

/**
 * @}
 */
