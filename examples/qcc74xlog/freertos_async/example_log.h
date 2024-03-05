#ifndef _EXAMPLE_LOG_H
#define _EXAMPLE_LOG_H

#include <stdint.h>
#include <string.h>
#include "qcc74xlog.h"

extern qcc74xlog_t example_recorder;

#define LOG_F(...)  QCC74xLOG_F((void *)&example_recorder, __VA_ARGS__)
#define LOG_E(...)  QCC74xLOG_E((void *)&example_recorder, __VA_ARGS__)
#define LOG_W(...)  QCC74xLOG_W((void *)&example_recorder, __VA_ARGS__)
#define LOG_I(...)  QCC74xLOG_I((void *)&example_recorder, __VA_ARGS__)
#define LOG_D(...)  QCC74xLOG_D((void *)&example_recorder, __VA_ARGS__)
#define LOG_T(...)  QCC74xLOG_T((void *)&example_recorder, __VA_ARGS__)
#define LOG_FLUSH() qcc74xlog_flush(&example_recorder)

extern void error_handler(void);

#ifdef CONFIG_ASSERT_DISABLE

#define _ASSERT_PARAM(x) ((void)(0))
#define _ASSERT_FUNC(x)  ((void)(x))
#define _CALL_ERROR()    error_handler()

#else

#define _CALL_ERROR()                       \
    do {                                    \
        printf("(Call Error Handler)\r\n"); \
        LOG_F("Call Error Handler\r\n");    \
        LOG_FLUSH();                        \
        error_handler();                    \
    } while (0)

#define _ASSERT_PARAM(x)                        \
    if ((uint32_t)(x) == 0) {                   \
        printf("(Assertion Faild)\r\n");        \
        printf("(%s)\r\n", (const char *)(#x)); \
        LOG_F("Assertion Faild\r\n");           \
        LOG_F("%s\r\n", (const char *)(#x));    \
        LOG_FLUSH();                            \
        error_handler();                        \
    }

#define _ASSERT_FUNC(x)                             \
    do {                                            \
        if ((uint32_t)(x) == 0) {                   \
            printf("(Assertion Faild)\r\n");        \
            printf("(%s)\r\n", (const char *)(#x)); \
            LOG_F("Assertion Faild\r\n");           \
            LOG_F("%s\r\n", (const char *)(#x));    \
            LOG_FLUSH();                            \
            error_handler();                        \
        }                                           \
    } while (0)
#endif

#endif
