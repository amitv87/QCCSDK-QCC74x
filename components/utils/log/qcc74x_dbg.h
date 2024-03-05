/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Erik Moqvist
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * This file is part of the dbg-macro project.
 */

#ifndef _DBG_H
#define _DBG_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <log.h>

#if (CONFIG_LOG_LEVEL >= 3)
#ifdef CONFIG_QCC74xLOG
#define __QCC74x_DBG_INFO_OUTPUT(tag, file, func, line, ...) \
    qcc74xlog((void *)&__qcc74xlog_recorder, QCC74xLOG_LEVEL_INFO, (tag), (file), (func), (line), ##__VA_ARGS__)
#define __QCC74x_DBG_RAWINFO_OUTPUT(tag, file, func, line, ...) \
    qcc74xlog((void *)&__qcc74xlog_recorder, QCC74xLOG_LEVEL_RAW | QCC74xLOG_LEVEL_INFO, (tag), (file), (func), (line), ##__VA_ARGS__)
#else
#define __QCC74x_DBG_INFO_OUTPUT(tag, file, func, line, ...)    LOG_I(__VA_ARGS__)
#define __QCC74x_DBG_RAWINFO_OUTPUT(tag, file, func, line, ...) LOG_RI(__VA_ARGS__)
#endif
#else
#define __QCC74x_DBG_INFO_OUTPUT(tag, file, func, line, ...)    ((void)0)
#define __QCC74x_DBG_RAWINFO_OUTPUT(tag, file, func, line, ...) ((void)0)
#endif

#define DBG_LINE_BUFFER_SIZE 256

#if (CONFIG_LOG_LEVEL >= 3)
/**
 * For primitive data types (int, float, etc.), strings and
 * pointers. Returns the result of given expression.
 */
#define DBG_VALUE(expr)                  \
    _Generic((expr),                     \
             char                        \
             : __qcc74x_dbg_char,          \
               signed char               \
             : __qcc74x_dbg_schar,         \
               unsigned char             \
             : __qcc74x_dbg_uchar,         \
               short                     \
             : __qcc74x_dbg_short,         \
               unsigned short            \
             : __qcc74x_dbg_ushort,        \
               int                       \
             : __qcc74x_dbg_int,           \
               unsigned int              \
             : __qcc74x_dbg_uint,          \
               long                      \
             : __qcc74x_dbg_long,          \
               unsigned long             \
             : __qcc74x_dbg_ulong,         \
               long long                 \
             : __qcc74x_dbg_llong,         \
               unsigned long long        \
             : __qcc74x_dbg_ullong,        \
               float                     \
             : __qcc74x_dbg_float,         \
               double                    \
             : __qcc74x_dbg_double,        \
               bool                      \
             : __qcc74x_dbg_bool,          \
               char *                    \
             : __qcc74x_dbg_char_p,        \
               const char *              \
             : __qcc74x_dbg_const_char_p,  \
               signed char *             \
             : __qcc74x_dbg_schar_p,       \
               const signed char *       \
             : __qcc74x_dbg_const_schar_p, \
               unsigned char *           \
             : __qcc74x_dbg_uchar_p,       \
               const unsigned char *     \
             : __qcc74x_dbg_const_uchar_p, \
               default                   \
             : __qcc74x_dbg_pointer)(QCC74xLOG_TAG, __FILE__, __FUNCTION__, __LINE__, #expr, expr)

/**
 * For array of primitive data types. Returns the result of given
 * expression.
 */
#define DBG_ARRAY(expr, length)           \
    _Generic((expr),                      \
             short *                      \
             : __qcc74x_dbg_short_p,        \
               const short *              \
             : __qcc74x_dbg_const_short_p,  \
               unsigned short *           \
             : __qcc74x_dbg_ushort_p,       \
               const unsigned short *     \
             : __qcc74x_dbg_const_ushort_p, \
               int *                      \
             : __qcc74x_dbg_int_p,          \
               const int *                \
             : __qcc74x_dbg_const_int_p,    \
               unsigned int *             \
             : __qcc74x_dbg_uint_p,         \
               const unsigned int *       \
             : __qcc74x_dbg_const_uint_p,   \
               long *                     \
             : __qcc74x_dbg_long_p,         \
               const long *               \
             : __qcc74x_dbg_const_long_p,   \
               unsigned long *            \
             : __qcc74x_dbg_ulong_p,        \
               const unsigned long *      \
             : __qcc74x_dbg_const_ulong_p,  \
               long long *                \
             : __qcc74x_dbg_llong_p,        \
               const long long *          \
             : __qcc74x_dbg_const_llong_p,  \
               unsigned long long *       \
             : __qcc74x_dbg_ullong_p,       \
               const unsigned long long * \
             : __qcc74x_dbg_const_ullong_p, \
               float *                    \
             : __qcc74x_dbg_float_p,        \
               const float *              \
             : __qcc74x_dbg_const_float_p,  \
               double *                   \
             : __qcc74x_dbg_double_p,       \
               const double *             \
             : __qcc74x_dbg_const_double_p, \
               bool *                     \
             : __qcc74x_dbg_bool_p,         \
               const bool *               \
             : __qcc74x_dbg_const_bool_p)(QCC74xLOG_TAG, __FILE__, __FUNCTION__, __LINE__, #expr, expr, length)

/**
 * To force boolean true/false output. Returns the result of given
 * expression.
 */
#define DBG_BOOL(expr) \
    __qcc74x_dbg_bool(QCC74xLOG_TAG, __FILE__, __FUNCTION__, __LINE__, #expr, expr)

/**
 * Hexdump of given size at given address. Returns the result of given
 * expression.
 */
#define DBG_HEXDUMP(expr, size)                   \
    _Generic((expr),                              \
             char *                               \
             : __qcc74x_dbg_hexdump_char_p,         \
               const char *                       \
             : __qcc74x_dbg_hexdump_const_char_p,   \
               signed char *                      \
             : __qcc74x_dbg_hexdump_schar_p,        \
               const signed char *                \
             : __qcc74x_dbg_hexdump_const_schar_p,  \
               unsigned char *                    \
             : __qcc74x_dbg_hexdump_uchar_p,        \
               const unsigned char *              \
             : __qcc74x_dbg_hexdump_const_uchar_p,  \
               short *                            \
             : __qcc74x_dbg_hexdump_short_p,        \
               const short *                      \
             : __qcc74x_dbg_hexdump_const_short_p,  \
               unsigned short *                   \
             : __qcc74x_dbg_hexdump_ushort_p,       \
               const unsigned short *             \
             : __qcc74x_dbg_hexdump_const_ushort_p, \
               int *                              \
             : __qcc74x_dbg_hexdump_int_p,          \
               const int *                        \
             : __qcc74x_dbg_hexdump_const_int_p,    \
               unsigned int *                     \
             : __qcc74x_dbg_hexdump_uint_p,         \
               const unsigned int *               \
             : __qcc74x_dbg_hexdump_const_uint_p,   \
               long *                             \
             : __qcc74x_dbg_hexdump_long_p,         \
               const long *                       \
             : __qcc74x_dbg_hexdump_const_long_p,   \
               unsigned long *                    \
             : __qcc74x_dbg_hexdump_ulong_p,        \
               const unsigned long *              \
             : __qcc74x_dbg_hexdump_const_ulong_p,  \
               long long *                        \
             : __qcc74x_dbg_hexdump_llong_p,        \
               const long long *                  \
             : __qcc74x_dbg_hexdump_const_llong_p,  \
               unsigned long long *               \
             : __qcc74x_dbg_hexdump_ullong_p,       \
               const unsigned long long *         \
             : __qcc74x_dbg_hexdump_const_ullong_p, \
               float *                            \
             : __qcc74x_dbg_hexdump_float_p,        \
               const float *                      \
             : __qcc74x_dbg_hexdump_const_float_p,  \
               double *                           \
             : __qcc74x_dbg_hexdump_double_p,       \
               const double *                     \
             : __qcc74x_dbg_hexdump_const_double_p, \
               default                            \
             : __qcc74x_dbg_hexdump_const_p)(QCC74xLOG_TAG, __FILE__, __FUNCTION__, __LINE__, #expr, expr, size)

#else
#define DBG_VALUE(expr) ((void)(expr))
#define DBG_ARRAY(expr, length) \
    ((void)(expr));             \
    ((void)(length))
#define DBG_BOOL(expr) ((void)(expr))
#define DBG_HEXDUMP(expr, size) \
    ((void)(expr));             \
    ((void)(size))
#endif

/* Colorful output selection. */
#if defined(CONFIG_LOG_NCOLOR) && CONFIG_LOG_NCOLOR
#define __QCC74x_DBG_FORMAT(format)                "%s = " format "\r\n"
#define __QCC74x_DBG_FORMAT_HEX(format, hexformat) "%s = " format " (0x" hexformat ")\r\n"
#define __QCC74x_DBG_FORMAT_ARRAY_BEG              "%s = ["
#define __QCC74x_DBG_FORMAT_ARRAY_END              "] (length: %u)\r\n"
#define __QCC74x_DBG_FORMAT_HEXDUMP_BEG            "%s (size: %u):\r\n"
#else
#define __QCC74x_DBG_EXPR                          "\033[0;34;1m"
#define __QCC74x_DBG_VALUE                         "\033[0;1m"
#define __QCC74x_DBG_RESET                         "\033[0m"
#define __QCC74x_DBG_FORMAT(format)                __QCC74x_DBG_EXPR "%s" __QCC74x_DBG_VALUE " = " format "\r\n" __QCC74x_DBG_RESET
#define __QCC74x_DBG_FORMAT_HEX(format, hexformat) __QCC74x_DBG_EXPR "%s" __QCC74x_DBG_VALUE " = " format " (0x" hexformat ")\r\n" __QCC74x_DBG_RESET
#define __QCC74x_DBG_FORMAT_ARRAY_BEG              __QCC74x_DBG_EXPR "%s" __QCC74x_DBG_VALUE " = ["
#define __QCC74x_DBG_FORMAT_ARRAY_END              "] (length: %u)\r\n" __QCC74x_DBG_RESET
#define __QCC74x_DBG_FORMAT_HEXDUMP_BEG            __QCC74x_DBG_EXPR "%s " __QCC74x_DBG_RESET "(size: %u):\r\n" __QCC74x_DBG_VALUE
#define __QCC74x_DBG_FORMAT_HEXDUMP_END            __QCC74x_DBG_RESET
#endif

#define __QCC74x_DBG_FUNC_CONST_P(name, type, format)                                                  \
    static inline const type *__qcc74x_dbg_const_##name##_p(                                           \
        void *tag_p,                                                                                 \
        const char *const file_p,                                                                    \
        const char *const func_p,                                                                    \
        const long line,                                                                             \
        const char *expr_p,                                                                          \
        const type *value_p,                                                                         \
        int length)                                                                                  \
    {                                                                                                \
        uint32_t idx = 0;                                                                            \
        char buff[DBG_LINE_BUFFER_SIZE];                                                             \
        __QCC74x_DBG_INFO_OUTPUT(tag_p, file_p, func_p, line, __QCC74x_DBG_FORMAT_ARRAY_BEG, expr_p);    \
        for (uint32_t i = 0; i < length; i++) {                                                      \
            int ret = snprintf(buff + idx, DBG_LINE_BUFFER_SIZE - idx, format ",", value_p[i]);      \
            if ((ret + idx) > (DBG_LINE_BUFFER_SIZE - 1)) {                                          \
                buff[idx] = '\0';                                                                    \
                __QCC74x_DBG_RAWINFO_OUTPUT(tag_p, file_p, func_p, line, "%s", buff);                  \
                i--;                                                                                 \
                idx = 0;                                                                             \
            } else {                                                                                 \
                idx += ret;                                                                          \
            }                                                                                        \
        }                                                                                            \
        __QCC74x_DBG_RAWINFO_OUTPUT(tag_p, file_p, func_p, line, "%s", buff);                          \
        __QCC74x_DBG_RAWINFO_OUTPUT(tag_p, file_p, func_p, line, __QCC74x_DBG_FORMAT_ARRAY_END, length); \
        return (value_p);                                                                            \
    }

#define __QCC74x_DBG_FUNC_P(name, type, format)  \
    static inline type *__qcc74x_dbg_##name##_p( \
        void *tag_p,                           \
        const char *const file_p,              \
        const char *const func_p,              \
        const long line,                       \
        const char *expr_p,                    \
        type *value_p,                         \
        int length)                            \
    {                                          \
        __qcc74x_dbg_const_##name##_p(           \
            tag_p,                             \
            file_p,                            \
            func_p,                            \
            line,                              \
            expr_p,                            \
            value_p,                           \
            length);                           \
        return (value_p);                      \
    }

#define __QCC74x_DBG_FUNC_HEXDUMP_CONST_P(name, type)                \
    static inline const type *__qcc74x_dbg_hexdump_const_##name##_p( \
        void *tag_p,                                               \
        const char *const file_p,                                  \
        const char *const func_p,                                  \
        const long line,                                           \
        const char *expr_p,                                        \
        const type *value_p,                                       \
        size_t size)                                               \
    {                                                              \
        __qcc74x_dbg_hexdump(                                        \
            tag_p,                                                 \
            file_p,                                                \
            func_p,                                                \
            line,                                                  \
            expr_p,                                                \
            (const void *)value_p,                                 \
            size);                                                 \
        return (value_p);                                          \
    }

#define __QCC74x_DBG_FUNC_HEXDUMP_P(name, type)          \
    static inline type *__qcc74x_dbg_hexdump_##name##_p( \
        void *tag_p,                                   \
        const char *const file_p,                      \
        const char *const func_p,                      \
        const long line,                               \
        const char *expr_p,                            \
        type *value_p,                                 \
        size_t size)                                   \
    {                                                  \
        __qcc74x_dbg_hexdump_const_##name##_p(           \
            tag_p,                                     \
            file_p,                                    \
            func_p,                                    \
            line,                                      \
            expr_p,                                    \
            value_p,                                   \
            size);                                     \
        return (value_p);                              \
    }

#define __QCC74x_DBG_FUNC_DEFINE(name, type, format) \
    static inline type __qcc74x_dbg_##name(          \
        void *tag_p,                               \
        const char *const file_p,                  \
        const char *const func_p,                  \
        const long line,                           \
        const char *expr_p,                        \
        type value)                                \
    {                                              \
        __QCC74x_DBG_INFO_OUTPUT(                    \
            tag_p, file_p, func_p, line,           \
            __QCC74x_DBG_FORMAT(format),             \
            expr_p, value);                        \
        return (value);                            \
    }                                              \
    __QCC74x_DBG_FUNC_CONST_P(name, type, format)    \
    __QCC74x_DBG_FUNC_P(name, type, format)          \
    __QCC74x_DBG_FUNC_HEXDUMP_CONST_P(name, type)    \
    __QCC74x_DBG_FUNC_HEXDUMP_P(name, type)

#define __QCC74x_DBG_FUNC_HEX_DEFINE(name, type, format, hexformat) \
    static inline type __qcc74x_dbg_##name(                         \
        void *tag_p,                                              \
        const char *const file_p,                                 \
        const char *const func_p,                                 \
        const long line,                                          \
        const char *expr_p,                                       \
        type value)                                               \
    {                                                             \
        __QCC74x_DBG_INFO_OUTPUT(                                   \
            tag_p, file_p, func_p, line,                          \
            __QCC74x_DBG_FORMAT_HEX(format, hexformat),             \
            expr_p, value, value);                                \
        return (value);                                           \
    }                                                             \
    __QCC74x_DBG_FUNC_CONST_P(name, type, format)                   \
    __QCC74x_DBG_FUNC_P(name, type, format)                         \
    __QCC74x_DBG_FUNC_HEXDUMP_CONST_P(name, type)                   \
    __QCC74x_DBG_FUNC_HEXDUMP_P(name, type)

#define __QCC74x_DBG_FUNC_CHAR_CONST_P(name, type)           \
    static inline const type *__qcc74x_dbg_const_##name##_p( \
        void *tag_p,                                       \
        const char *const file_p,                          \
        const char *const func_p,                          \
        const long line,                                   \
        const char *expr_p,                                \
        const type *value_p)                               \
    {                                                      \
        __QCC74x_DBG_INFO_OUTPUT(                            \
            tag_p, file_p, func_p, line,                   \
            __QCC74x_DBG_FORMAT("\"%s\""),                   \
            expr_p, value_p);                              \
        return (value_p);                                  \
    }

#define __QCC74x_DBG_FUNC_CHAR_P(name, type)     \
    static inline type *__qcc74x_dbg_##name##_p( \
        void *tag_p,                           \
        const char *const file_p,              \
        const char *const func_p,              \
        const long line,                       \
        const char *expr_p,                    \
        type *value_p)                         \
    {                                          \
        __qcc74x_dbg_const_##name##_p(           \
            tag_p,                             \
            file_p,                            \
            func_p,                            \
            line,                              \
            expr_p,                            \
            value_p);                          \
        return (value_p);                      \
    }

#define __QCC74x_DBG_FUNC_CHAR_DEFINE(name, type, format) \
    static inline type __qcc74x_dbg_##name(               \
        void *tag_p,                                    \
        const char *const file_p,                       \
        const char *const func_p,                       \
        const long line,                                \
        const char *expr_p,                             \
        type value)                                     \
    {                                                   \
        __QCC74x_DBG_INFO_OUTPUT(                         \
            tag_p, file_p, func_p, line,                \
            __QCC74x_DBG_FORMAT(format),                  \
            expr_p, value);                             \
        return (value);                                 \
    }                                                   \
    __QCC74x_DBG_FUNC_CHAR_CONST_P(name, type)            \
    __QCC74x_DBG_FUNC_CHAR_P(name, type)                  \
    __QCC74x_DBG_FUNC_HEXDUMP_CONST_P(name, type)         \
    __QCC74x_DBG_FUNC_HEXDUMP_P(name, type)

static inline const char *__qcc74x_dbg_format_bool(bool value)
{
    return (value ? "true" : "false");
}

static inline bool __qcc74x_dbg_bool(
    void *tag_p,
    const char *const file_p,
    const char *const func_p,
    const long line,
    const char *expr_p,
    bool value)
{
    __QCC74x_DBG_INFO_OUTPUT(
        tag_p, file_p, func_p, line,
        __QCC74x_DBG_FORMAT("%s"),
        expr_p, __qcc74x_dbg_format_bool(value));
    return (value);
}

static inline const bool *__qcc74x_dbg_const_bool_p(
    void *tag_p,
    const char *const file_p,
    const char *const func_p,
    const long line,
    const char *expr_p,
    const bool *value_p,
    int length)
{
    uint32_t idx = 0;
    char buff[DBG_LINE_BUFFER_SIZE];
    __QCC74x_DBG_INFO_OUTPUT(tag_p, file_p, func_p, line, __QCC74x_DBG_FORMAT_ARRAY_BEG, expr_p);

    for (uint32_t i = 0; i < length; i++) {
        int ret = snprintf(buff + idx, DBG_LINE_BUFFER_SIZE - idx, "%s,", __qcc74x_dbg_format_bool(value_p[i]));
        if ((ret + idx) > (DBG_LINE_BUFFER_SIZE - 1)) {
            buff[idx] = '\0';
            __QCC74x_DBG_RAWINFO_OUTPUT(tag_p, file_p, func_p, line, "%s", buff);
            i--;
            idx = 0;
        } else {
            idx += ret;
        }
    }
    __QCC74x_DBG_RAWINFO_OUTPUT(tag_p, file_p, func_p, line, "%s", buff);
    __QCC74x_DBG_RAWINFO_OUTPUT(tag_p, file_p, func_p, line, __QCC74x_DBG_FORMAT_ARRAY_END, length);
    return (value_p);
}

static inline bool *__qcc74x_dbg_bool_p(
    void *tag_p,
    const char *const file_p,
    const char *const func_p,
    const long line,
    const char *expr_p,
    bool *value_p,
    int length)
{
    (void)__qcc74x_dbg_const_bool_p(tag_p, file_p, func_p, line, expr_p, value_p, length);

    return (value_p);
}

static inline void *__qcc74x_dbg_pointer(
    void *tag_p,
    const char *const file_p,
    const char *const func_p,
    const long line,
    const char *expr_p,
    void *value_p)
{
    __QCC74x_DBG_INFO_OUTPUT(tag_p, file_p, func_p, line, __QCC74x_DBG_FORMAT("%08x"), expr_p, value_p);
    return (value_p);
}

static inline void __qcc74x_dbg_print_ascii(
    const uint8_t *buf_p,
    uint8_t size,
    char *buff,
    uint32_t idx)
{
    if (size < 8) {
        idx += snprintf(buff + idx, 84 - idx, " ");
    }

    for (uint8_t i = 0; i < 16 - size; i++) {
        idx += snprintf(buff + idx, 84 - idx, "   ");
    }

    idx += snprintf(buff + idx, 84 - idx, "'");

    for (uint8_t i = 0; i < size; i++) {
        idx += snprintf(buff + idx, 84 - idx, "%c", isprint((int)buf_p[i]) ? buf_p[i] : '.');
    }

    idx += snprintf(buff + idx, 84 - idx, "'\r\n");
}

static inline void __qcc74x_dbg_hexdump(
    void *tag_p,
    const char *const file_p,
    const char *const func_p,
    const long line,
    const char *expr_p,
    const void *buf_p,
    size_t size)
{
    int pos;
    const uint8_t *u8_buf_p;
    u8_buf_p = (const uint8_t *)buf_p;
    pos = 0;

    uint32_t idx = 0;
    char buff[84];

    __QCC74x_DBG_INFO_OUTPUT(tag_p, file_p, func_p, line, __QCC74x_DBG_FORMAT_HEXDUMP_BEG, expr_p, (int)size);

    while (size > 0) {
        if ((pos % 16) == 0) {
            idx = 0;
            idx += snprintf(buff + idx, 84 - idx, "    %08x: ", pos);
        }

        idx += snprintf(buff + idx, 84 - idx, "%02x ", u8_buf_p[pos] & 0xff);

        if ((pos % 16) == 7) {
            idx += snprintf(buff + idx, 84 - idx, " ");
        }

        if ((pos % 16) == 15) {
            __qcc74x_dbg_print_ascii(&u8_buf_p[pos - 15], 16, buff, idx);
            __QCC74x_DBG_RAWINFO_OUTPUT(tag_p, file_p, func_p, line, "%s", buff);
        }

        pos++;
        size--;
    }

#ifdef __QCC74x_DBG_FORMAT_HEXDUMP_END
    if ((pos % 16) != 0) {
        __qcc74x_dbg_print_ascii(&u8_buf_p[pos - (pos % 16)], pos % 16, buff, idx);
        __QCC74x_DBG_RAWINFO_OUTPUT(tag_p, file_p, func_p, line, "%s" __QCC74x_DBG_FORMAT_HEXDUMP_END, buff);
    } else {
        __QCC74x_DBG_RAWINFO_OUTPUT(tag_p, file_p, func_p, line, __QCC74x_DBG_FORMAT_HEXDUMP_END);
    }
#else
    if ((pos % 16) != 0) {
        __qcc74x_dbg_print_ascii(&u8_buf_p[pos - (pos % 16)], pos % 16, buff, idx);
        __QCC74x_DBG_RAWINFO_OUTPUT(tag_p, file_p, func_p, line, "%s", buff);
    }
#endif
}

static inline void *__qcc74x_dbg_hexdump_p(
    void *tag_p,
    const char *const file_p,
    const char *const func_p,
    const long line,
    const char *expr_p,
    void *buf_p,
    size_t size)
{
    __qcc74x_dbg_hexdump(tag_p, file_p, func_p, line, expr_p, buf_p, size);
    return (buf_p);
}

static inline const void *__qcc74x_dbg_hexdump_const_p(
    void *tag_p,
    const char *const file_p,
    const char *const func_p,
    const long line,
    const char *expr_p,
    const void *buf_p,
    size_t size)
{
    __qcc74x_dbg_hexdump(tag_p, file_p, func_p, line, expr_p, buf_p, size);
    return (buf_p);
}

__QCC74x_DBG_FUNC_CHAR_DEFINE(char, char, "%hhi")
__QCC74x_DBG_FUNC_CHAR_DEFINE(schar, signed char, "%hhi")
__QCC74x_DBG_FUNC_CHAR_DEFINE(uchar, unsigned char, "%hhu")
__QCC74x_DBG_FUNC_HEX_DEFINE(short, short, "%hi", "%hx")
__QCC74x_DBG_FUNC_HEX_DEFINE(ushort, unsigned short, "%hu", "%hx")
__QCC74x_DBG_FUNC_HEX_DEFINE(int, int, "%d", "%x")
__QCC74x_DBG_FUNC_HEX_DEFINE(uint, unsigned int, "%u", "%x")
__QCC74x_DBG_FUNC_HEX_DEFINE(long, long, "%ld", "%lx")
__QCC74x_DBG_FUNC_HEX_DEFINE(ulong, unsigned long, "%lu", "%lx")
__QCC74x_DBG_FUNC_HEX_DEFINE(llong, long long, "%lld", "%llx")
__QCC74x_DBG_FUNC_HEX_DEFINE(ullong, unsigned long long, "%llu", "%llx")
__QCC74x_DBG_FUNC_DEFINE(float, float, "%f")
__QCC74x_DBG_FUNC_DEFINE(double, double, "%lf")

#endif