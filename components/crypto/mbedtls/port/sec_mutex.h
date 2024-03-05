#ifndef _SEC_MUTEX_H
#define _SEC_MUTEX_H

#include "qcc74x_core.h"

void qcc74x_sec_mutex_init(void);
int qcc74x_sec_aes_mutex_take(void);
int qcc74x_sec_aes_mutex_give(void);
int qcc74x_sec_sha_mutex_take(void);
int qcc74x_sec_sha_mutex_give(void);
int qcc74x_sec_pka_mutex_take(void);
int qcc74x_sec_pka_mutex_give(void);

#endif