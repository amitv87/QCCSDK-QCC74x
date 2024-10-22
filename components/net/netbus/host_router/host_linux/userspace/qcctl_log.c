#include "qcctl_log.h"

static int qcc74x_log_level = QCC74x_LOG_LEVEL_INFO;

int qcc74x_log_level_get(void)
{
    return qcc74x_log_level;
}

int qcc74x_log_level_set(int level)
{
    qcc74x_log_level = level;
    return 0;
}

void qcc74x_log_print(int level, const char *fmt, ...)
{
    va_list va;

    if (qcc74x_log_level_get() < level)
        return;

    va_start(va, fmt);
    vfprintf(stdout, fmt, va);
    va_end(va);
}
