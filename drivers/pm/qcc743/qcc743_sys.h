#ifndef __QCC743_SYS_H__
#define __QCC743_SYS_H__

#include <stdint.h>


typedef enum {
    QCC74x_RST_POWER_OFF = 0,
    QCC74x_RST_HARDWARE_WATCHDOG,
    QCC74x_RST_FATAL_EXCEPTION,
    QCC74x_RST_SOFTWARE_WATCHDOG,
    QCC74x_RST_SOFTWARE,
    QCC74x_RST_HBN,
    QCC74x_RST_BOD,
} QCC74x_RST_REASON_E;


QCC74x_RST_REASON_E qcc74x_sys_rstinfo_get(void);
int qcc74x_sys_rstinfo_set(QCC74x_RST_REASON_E val);
int qcc74x_sys_rstinfo_getsting(char *info);
char * qcc74x_sys_rstinfo_getstring(void);
void qcc74x_sys_rstinfo_init(void);
int qcc74x_sys_reset_por(void);
void qcc74x_sys_reset_system(void);
int qcc74x_sys_isxipaddr(uint32_t addr);
int qcc74x_sys_init(void);
#endif
