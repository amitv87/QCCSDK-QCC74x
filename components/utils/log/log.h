#ifndef _LOG_H
#define _LOG_H

#include <stdint.h>
#include <string.h>

#ifndef DBG_TAG
#define DBG_TAG ""
#endif

#ifdef CONFIG_QCC74xLOG
#include "qcc74xlog.h"
extern qcc74xlog_t __qcc74xlog_recorder;
extern void *__qcc74xlog_recorder_pointer;

#undef QCC74xLOG_TAG
#define QCC74xLOG_TAG DBG_TAG
#else
#define QCC74xLOG_DEFINE_TAG(name, _string, enable)
#define QCC74xLOG_EXTERN_TAG(name)
#define QCC74xLOG_GET_TAG(name) NULL
#define QCC74xLOG_TAG           NULL
#endif

#ifdef CONFIG_LOG_DISABLE

#define LOG_F(...)  ((void)0)
#define LOG_RF(...) ((void)0)
#define LOG_E(...)  ((void)0)
#define LOG_RE(...) ((void)0)
#define LOG_W(...)  ((void)0)
#define LOG_RW(...) ((void)0)
#define LOG_I(...)  ((void)0)
#define LOG_RI(...) ((void)0)
#define LOG_D(...)  ((void)0)
#define LOG_RD(...) ((void)0)
#define LOG_T(...)  ((void)0)
#define LOG_RT(...) ((void)0)
#define LOG_FLUSH() ((void)0)

#else

#ifdef CONFIG_QCC74xLOG

#if (CONFIG_LOG_LEVEL >= 0)
#define LOG_F(...)  QCC74xLOG_F((void *)&__qcc74xlog_recorder, __VA_ARGS__)
#define LOG_RF(...) QCC74xLOG_RF((void *)&__qcc74xlog_recorder, __VA_ARGS__)
#else
#define LOG_F(...)  ((void)0)
#define LOG_RF(...) ((void)0)
#endif

#if (CONFIG_LOG_LEVEL >= 1)
#define LOG_E(...)  QCC74xLOG_E((void *)&__qcc74xlog_recorder, __VA_ARGS__)
#define LOG_RE(...) QCC74xLOG_RE((void *)&__qcc74xlog_recorder, __VA_ARGS__)
#else
#define LOG_E(...)  ((void)0)
#define LOG_RE(...) ((void)0)
#endif

#if (CONFIG_LOG_LEVEL >= 2)
#define LOG_W(...)  QCC74xLOG_W((void *)&__qcc74xlog_recorder, __VA_ARGS__)
#define LOG_RW(...) QCC74xLOG_RW((void *)&__qcc74xlog_recorder, __VA_ARGS__)
#else
#define LOG_W(...)  ((void)0)
#define LOG_RW(...) ((void)0)
#endif

#if (CONFIG_LOG_LEVEL >= 3)
#define LOG_I(...)  QCC74xLOG_I((void *)&__qcc74xlog_recorder, __VA_ARGS__)
#define LOG_RI(...) QCC74xLOG_RI((void *)&__qcc74xlog_recorder, __VA_ARGS__)
#else
#define LOG_I(...)  ((void)0)
#define LOG_RI(...) ((void)0)
#endif

#if (CONFIG_LOG_LEVEL >= 4)
#define LOG_D(...)  QCC74xLOG_D((void *)&__qcc74xlog_recorder, __VA_ARGS__)
#define LOG_RD(...) QCC74xLOG_RD((void *)&__qcc74xlog_recorder, __VA_ARGS__)
#else
#define LOG_D(...)  ((void)0)
#define LOG_RD(...) ((void)0)
#endif

#if (CONFIG_LOG_LEVEL >= 5)
#define LOG_T(...)  QCC74xLOG_T((void *)&__qcc74xlog_recorder, __VA_ARGS__)
#define LOG_RT(...) QCC74xLOG_RT((void *)&__qcc74xlog_recorder, __VA_ARGS__)
#else
#define LOG_T(...)  ((void)0)
#define LOG_RT(...) ((void)0)
#endif

#define LOG_FLUSH() qcc74xlog_flush(&__qcc74xlog_recorder)

#else

#if (CONFIG_LOG_LEVEL >= 0)
#if defined(CONFIG_LOG_NCOLOR) && CONFIG_LOG_NCOLOR
#define LOG_F(...) printf("[F][" DBG_TAG "] " __VA_ARGS__)
#else
#define LOG_F(...) printf("\033[0;35m[F][" DBG_TAG "] " __VA_ARGS__)
#endif
#define LOG_RF(...) printf(__VA_ARGS__)
#else
#define LOG_F(...)  ((void)0)
#define LOG_RF(...) ((void)0)
#endif

#if (CONFIG_LOG_LEVEL >= 1)
#if defined(CONFIG_LOG_NCOLOR) && CONFIG_LOG_NCOLOR
#define LOG_E(...) printf("[E][" DBG_TAG "] " __VA_ARGS__)
#else
#define LOG_E(...) printf("\033[0;31m[E][" DBG_TAG "] " __VA_ARGS__)
#endif
#define LOG_RE(...) printf(__VA_ARGS__)
#else
#define LOG_E(...)  ((void)0)
#define LOG_RE(...) ((void)0)
#endif

#if (CONFIG_LOG_LEVEL >= 2)
#if defined(CONFIG_LOG_NCOLOR) && CONFIG_LOG_NCOLOR
#define LOG_W(...) printf("[W][" DBG_TAG "] " __VA_ARGS__)
#else
#define LOG_W(...) printf("\033[0;33m[W][" DBG_TAG "] " __VA_ARGS__)
#endif
#define LOG_RW(...) printf(__VA_ARGS__)
#else
#define LOG_W(...)  ((void)0)
#define LOG_RW(...) ((void)0)
#endif

#if (CONFIG_LOG_LEVEL >= 3)
#if defined(CONFIG_LOG_NCOLOR) && CONFIG_LOG_NCOLOR
#define LOG_I(...) printf("[I][" DBG_TAG "] " __VA_ARGS__)
#else
#define LOG_I(...) printf("\033[0m[I][" DBG_TAG "] " __VA_ARGS__)
#endif
#define LOG_RI(...) printf(__VA_ARGS__)
#else
#define LOG_I(...)  ((void)0)
#define LOG_RI(...) ((void)0)
#endif

#if (CONFIG_LOG_LEVEL >= 4)
#if defined(CONFIG_LOG_NCOLOR) && CONFIG_LOG_NCOLOR
#define LOG_D(...) printf("[D][" DBG_TAG "] " __VA_ARGS__)
#else
#define LOG_D(...) printf("\033[0m[D][" DBG_TAG "] " __VA_ARGS__)
#endif
#define LOG_RD(...) printf(__VA_ARGS__)
#else
#define LOG_D(...)  ((void)0)
#define LOG_RD(...) ((void)0)
#endif

#if (CONFIG_LOG_LEVEL >= 5)
#if defined(CONFIG_LOG_NCOLOR) && CONFIG_LOG_NCOLOR
#define LOG_T(...) printf("[T][" DBG_TAG "] " __VA_ARGS__)
#else
#define LOG_T(...) printf("\033[0m[T][" DBG_TAG "] " __VA_ARGS__)
#endif
#define LOG_RT(...) printf(__VA_ARGS__)
#else
#define LOG_T(...)  ((void)0)
#define LOG_RT(...) ((void)0)
#endif

#define LOG_FLUSH() ((void)0)

#endif

#endif

extern void error_handler(void);

#if defined(CONFIG_LOG_NCOLOR) && CONFIG_LOG_NCOLOR
#define ASSERT_EXPR
#define ASSERT_VALUE
#define ASSERT_RESET
#else
#define ASSERT_EXPR  "\033[0;34;1m"
#define ASSERT_VALUE "\033[0;1m"
#define ASSERT_RESET "\033[0m"
#endif

#ifdef CONFIG_ASSERT_DISABLE

#define _CALL_ERROR()                            error_handler()
#define _ASSERT_PARAM_MESSAGE(x, ...)            ((void)(0))
#define _ASSERT_FUNC_MESSAGE(x, ...)             ((void)(x))
#define _ASSERT_ZERO_PARAM_MESSAGE(x, ...)       ((void)(0))
#define _ASSERT_ZERO_FUNC_MESSAGE(x, ...)        ((void)(x))
#define _ASSERT_EQUAL_PARAM_MESSAGE(val, x, ...) ((void)(0))
#define _ASSERT_EQUAL_FUNC_MESSAGE(val, x, ...) \
    ((void)(val));                              \
    ((void)(x))

#else

#define _CALL_ERROR()                       \
    do {                                    \
        printf("(Call Error Handler)\r\n"); \
        error_handler();                    \
    } while (0)

#define _ASSERT_PARAM_MESSAGE(x, ...)                                               \
    do {                                                                            \
        if ((uint32_t)(x) == 0) {                                                   \
            printf(__VA_ARGS__);                                                    \
            printf(" at %s:%d\r\n", __FILE__, __LINE__);                            \
            printf(" " ASSERT_EXPR #x ASSERT_VALUE " = false" ASSERT_RESET "\r\n"); \
            error_handler();                                                        \
        }                                                                           \
    } while (0)

#define _ASSERT_FUNC_MESSAGE(x, ...)                                                \
    do {                                                                            \
        if ((uint32_t)(x) == 0) {                                                   \
            printf(__VA_ARGS__);                                                    \
            printf(" at %s:%d\r\n", __FILE__, __LINE__);                            \
            printf(" " ASSERT_EXPR #x ASSERT_VALUE " = false" ASSERT_RESET "\r\n"); \
            error_handler();                                                        \
        }                                                                           \
    } while (0)

#define _ASSERT_ZERO_PARAM_MESSAGE(x, ...)                                           \
    do {                                                                             \
        int _x = (int)(x);                                                           \
        if (!_x == 0) {                                                              \
            printf(__VA_ARGS__);                                                     \
            printf(" at %s:%d\r\n", __FILE__, __LINE__);                             \
            printf(" " ASSERT_EXPR #x ASSERT_VALUE " = %d" ASSERT_RESET "\r\n", _x); \
            error_handler();                                                         \
        }                                                                            \
    } while (0)

#define _ASSERT_ZERO_FUNC_MESSAGE(x, ...)                                            \
    do {                                                                             \
        int _x = (int)(x);                                                           \
        if (!_x == 0) {                                                              \
            printf(__VA_ARGS__);                                                     \
            printf(" at %s:%d\r\n", __FILE__, __LINE__);                             \
            printf(" " ASSERT_EXPR #x ASSERT_VALUE " = %d" ASSERT_RESET "\r\n", _x); \
            error_handler();                                                         \
        }                                                                            \
    } while (0)

#define _ASSERT_EQUAL_PARAM_MESSAGE(val, x, ...)                                                                                  \
    do {                                                                                                                          \
        int _val = (int)(val);                                                                                                    \
        int _x = (int)(x);                                                                                                        \
        if (_val != _x) {                                                                                                         \
            printf(__VA_ARGS__);                                                                                                  \
            printf(" at %s:%d\r\n", __FILE__, __LINE__);                                                                          \
            printf(" " ASSERT_EXPR #val " == " #x ASSERT_VALUE " but " #val " = %d , " #x " = %d" ASSERT_RESET "\r\n", _val, _x); \
            error_handler();                                                                                                      \
        }                                                                                                                         \
    } while (0)

#define _ASSERT_EQUAL_FUNC_MESSAGE(val, x, ...)                                                                                   \
    do {                                                                                                                          \
        int _val = (int)(val);                                                                                                    \
        int _x = (int)(x);                                                                                                        \
        if (_val != _x) {                                                                                                         \
            printf(__VA_ARGS__);                                                                                                  \
            printf(" at %s:%d\r\n", __FILE__, __LINE__);                                                                          \
            printf(" " ASSERT_EXPR #val " == " #x ASSERT_VALUE " but " #val " = %d , " #x " = %d" ASSERT_RESET "\r\n", _val, _x); \
            error_handler();                                                                                                      \
        }                                                                                                                         \
    } while (0)

#endif

#define _ASSERT_TRUE_PARAM_MESSAGE  _ASSERT_PARAM_MESSAGE
#define _ASSERT_TRUE_FUNC_MESSAGE   _ASSERT_FUNC_MESSAGE
#define _ASSERT_FALSE_PARAM_MESSAGE _ASSERT_ZERO_PARAM_MESSAGE
#define _ASSERT_FALSE_FUNC_MESSAGE  _ASSERT_ZERO_FUNC_MESSAGE

#define _ASSERT_PARAM(x)            _ASSERT_PARAM_MESSAGE(x, "Assertion Failed")
#define _ASSERT_FUNC(x)             _ASSERT_FUNC_MESSAGE(x, "Assertion Failed")
#define _ASSERT_TRUE_PARAM(x)       _ASSERT_TRUE_PARAM_MESSAGE(x, "Assertion TRUE Failed")
#define _ASSERT_TRUE_FUNC(x)        _ASSERT_TRUE_FUNC_MESSAGE(x, "Assertion TRUE Failed")
#define _ASSERT_FALSE_PARAM(x)      _ASSERT_FALSE_PARAM_MESSAGE(x, "Assertion FALSE Failed")
#define _ASSERT_FALSE_FUNC(x)       _ASSERT_FALSE_FUNC_MESSAGE(x, "Assertion FALSE Failed")
#define _ASSERT_ZERO_PARAM(x)       _ASSERT_ZERO_PARAM_MESSAGE(x, "Assertion ZERO Failed")
#define _ASSERT_ZERO_FUNC(x)        _ASSERT_ZERO_FUNC_MESSAGE(x, "Assertion ZERO Failed")
#define _ASSERT_EQUAL_PARAM(val, x) _ASSERT_EQUAL_PARAM_MESSAGE(val, x, "Assertion EQUAL Failed")
#define _ASSERT_EQUAL_FUNC(val, x)  _ASSERT_EQUAL_FUNC_MESSAGE(val, x, "Assertion EQUAL Failed")

#define _STATIC_ASSERT              _Static_assert

extern void log_restart(void);
extern void log_start(void);

#include "qcc74x_dbg.h"

#endif
