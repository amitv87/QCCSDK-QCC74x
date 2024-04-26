#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

#include <spisync_port.h>

#if TPDBG_ENABLE
#define ONE_LINE_MAX_NUM (32)

static char s_debug_buf[ONE_LINE_MAX_NUM*3 + 1];
static SemaphoreHandle_t s_debug_mutex = NULL;

static int buf_out(const char *str, const void *inbuf, int len)
{
    char *buf = (char *)inbuf;
    char *pbuffer = NULL;

    pbuffer = (char *)s_debug_buf;
    int m = 0, n = 0;
    int j = 0, k = 0, tmp = 0;

    tmp = (sizeof(s_debug_buf)) / 3; // Maximum array length

    if ((ONE_LINE_MAX_NUM > tmp) || (len < 1)) {
        return -1;
    }

    if (NULL == s_debug_mutex) {
        s_debug_mutex = xSemaphoreCreateMutex();
    }

    m = len / ONE_LINE_MAX_NUM;
    n = len % ONE_LINE_MAX_NUM;

    if (n > 0) {
        m++;
    }

    if (n > 0) {
        // Non-multiple
        for (k = 0; k < m; k++) {
            if ((k + 1) == m) {
                // Last frame data
                tmp = 0;
                for (j = 0; j < n; j++) {
                    tmp += snprintf(pbuffer + tmp, sizeof(s_debug_buf) - tmp, "%02x ", (uint8_t)buf[k * ONE_LINE_MAX_NUM + j]);
                }
                printf("%s %.*s\r\n", str, tmp, pbuffer);
            } else {
                tmp = 0;
                for (j = 0; j < ONE_LINE_MAX_NUM; j++) {
                    tmp += snprintf(pbuffer + tmp, sizeof(s_debug_buf) - tmp, "%02x ", (uint8_t)buf[k * ONE_LINE_MAX_NUM + j]);
                }
                printf("%s %.*s\r\n", str, tmp, pbuffer);
            }
        }
    } else {
        // Multiple
        for (k = 0; k < m; k++) {
            tmp = 0;
            for (j = 0; j < ONE_LINE_MAX_NUM; j++) {
                tmp += snprintf(pbuffer + tmp, sizeof(s_debug_buf) - tmp, "%02x ", (uint8_t)buf[k * ONE_LINE_MAX_NUM + j]);
            }
            printf("%s %.*s\r\n", str, tmp, pbuffer);
        }
    }

    if (s_debug_mutex) {
        xSemaphoreGive(s_debug_mutex);
    }

    return 0;
}

void spisync_buf(const char *str, void *buf, uint32_t len)
{
    if ((NULL == buf) || (len == 0)) {
        printf("arg error buf = %p, len = %ld\r\n", buf, len);
        return;
    }

    buf_out(str, buf, len);
}
#endif

void *malloc_aligned_with_padding(int size, int align_bytes)
{
    void *base_ptr = NULL;
    void *mem_ptr = NULL;
    // Allocate extra align_bytes for end alignment
    base_ptr = pvPortMalloc(size + 2 * align_bytes);
    if (base_ptr != NULL) {
        mem_ptr = (void *)(((uintptr_t)base_ptr + align_bytes - 1) & ~(align_bytes - 1));
        // If the base address is already aligned, move mem_ptr to the next alignment point
        if (mem_ptr == base_ptr) {
            mem_ptr = (void *)((uintptr_t)mem_ptr + align_bytes);
        }
        *((int *)mem_ptr - 1) = (int)((uintptr_t)mem_ptr - (uintptr_t)base_ptr);
    }
    return mem_ptr;
}

void free_aligned_with_padding(void *ptr)
{
    void *base_addr = NULL;
    base_addr = (void *)((uintptr_t)ptr - *((int *)ptr - 1));
    vPortFree(base_addr);
}

void *malloc_aligned_with_padding_nocache(int size, int align_bytes)
{
    return ((uint32_t)(malloc_aligned_with_padding(size, align_bytes))) & 0xBFFFFFFF;
}

void free_aligned_with_padding_nocache(void *ptr)
{
    free_aligned_with_padding(((uint32_t)(ptr)) | 0x60000000);
}

