#ifndef __CODEC_DEBUG_H__
#define __CODEC_DEBUG_H__

#include <stdint.h>

/* enable debug */
#define ENABLE_CODEC_DEBUG_OUTPUT           (1)
#define ENABLE_CODEC_DEBUG_INPUT            (1)

/* type debug */
#define TYPE_CODEC_DEBUG_OUTPUT             (1)
#define TYPE_CODEC_DEBUG_INPUT              (2)

void codec_debug_input(uint32_t type, uint8_t *buf, uint32_t buf_len);

#endif /* __CODEC_DEBUG_H__ */
