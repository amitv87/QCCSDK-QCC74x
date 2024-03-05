#ifndef _QCC74x_TIMESTAMP_H
#define _QCC74x_TIMESTAMP_H

#include <stdint.h>

typedef struct
{
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t mday;
    uint8_t mon;
    uint16_t year;
    uint8_t wday;
} qcc74x_timestamp_t;

extern void qcc74x_timestamp_utc2time(uint32_t utc, qcc74x_timestamp_t *time);

#endif
