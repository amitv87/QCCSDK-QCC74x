#ifndef __QCCTL_LOG_H
#define __QCCTL_LOG_H

#include <stdio.h>
#include <stdarg.h>

enum {
    QCC74x_LOG_LEVEL_FATAL = 0,
    QCC74x_LOG_LEVEL_ERR,
    QCC74x_LOG_LEVEL_WARN,
    QCC74x_LOG_LEVEL_INFO,
    QCC74x_LOG_LEVEL_DEBUG,
    QCC74x_LOG_LEVEL_VERBOSE,
};

/* this can be overridden by other source files */
#define QCC74x_TAG  " qcctl "

#define qcc74x_logf(fmt, ...) qcc74x_log_print(QCC74x_LOG_LEVEL_FATAL,   " F " QCC74x_TAG fmt, ##__VA_ARGS__)
#define qcc74x_loge(fmt, ...) qcc74x_log_print(QCC74x_LOG_LEVEL_ERR,     " E " QCC74x_TAG fmt, ##__VA_ARGS__)
#define qcc74x_logw(fmt, ...) qcc74x_log_print(QCC74x_LOG_LEVEL_WARN,    " W " QCC74x_TAG fmt, ##__VA_ARGS__)
#define qcc74x_logi(fmt, ...) qcc74x_log_print(QCC74x_LOG_LEVEL_INFO,    " I " QCC74x_TAG fmt, ##__VA_ARGS__)
#define qcc74x_logd(fmt, ...) qcc74x_log_print(QCC74x_LOG_LEVEL_DEBUG,   " D " QCC74x_TAG fmt, ##__VA_ARGS__)
#define qcc74x_logv(fmt, ...) qcc74x_log_print(QCC74x_LOG_LEVEL_VERBOSE, " V " QCC74x_TAG fmt, ##__VA_ARGS__)


int qcc74x_log_level_get(void);

int qcc74x_log_level_set(int level);

void qcc74x_log_print(int level, const char *fmt, ...);

/* __QCCTL_LOG_H */
#endif
