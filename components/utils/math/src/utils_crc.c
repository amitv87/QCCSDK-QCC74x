#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <utils_crc.h>

extern const uint32_t crc32Tab[256];

void utils_crc32_stream_init(struct crc32_stream_ctx *ctx)
{
    ctx->crc = 0xffffffff;
}

void utils_crc32_stream_feed(struct crc32_stream_ctx *ctx, uint8_t data)
{
    ctx->crc = crc32Tab[(ctx->crc ^ data) & 0xFF] ^ (ctx->crc >> 8);
}

void utils_crc32_stream_feed_block(struct crc32_stream_ctx *ctx, const uint8_t *data, uint32_t len)
{
    while (len--)
        ctx->crc = crc32Tab[(ctx->crc ^ *data++) & 0xFF] ^ (ctx->crc >> 8);
}

uint32_t utils_crc32_stream_results(struct crc32_stream_ctx *ctx)
{
    return ctx->crc ^ 0xffffffff;
}

#define POLY 0x8408
uint16_t utils_crc16_ccitt(const void *dataIn, uint32_t len)
{
    const uint8_t *data_p = (const uint8_t *)dataIn;
    uint8_t i;
    uint8_t data;
    uint16_t crc;

    crc = 0xFFFF;

    if (len == 0)
        return (~crc);

    do {
        for (i = 0, data = (unsigned int)0xff & *data_p++;
            i < 8;
            i++, data >>= 1) {
        if ((crc & 0x0001) ^ (data & 0x0001))
            crc = (crc >> 1) ^ POLY;
        else
            crc >>= 1;
        }
    } while (--len);

    crc = ~crc;

    data = crc;

    return (crc);
}

const static uint8_t sht75_crc_table[] = {
0,   49,  98,  83,  196, 245, 166, 151, 185, 136, 219, 234, 125, 76,  31,  46,
67,  114, 33,  16,  135, 182, 229, 212, 250, 203, 152, 169, 62,  15,  92,  109,
134, 183, 228, 213, 66,  115, 32,  17,  63,  14,  93,  108, 251, 202, 153, 168,
197, 244, 167, 150, 1,   48,  99,  82,  124, 77,  30,  47,  184, 137, 218, 235,
61,  12,  95,  110, 249, 200, 155, 170, 132, 181, 230, 215, 64,  113, 34,  19,
126, 79,  28,  45,  186, 139, 216, 233, 199, 246, 165, 148, 3,   50,  97,  80,
187, 138, 217, 232, 127, 78,  29,  44,  2,   51,  96,  81,  198, 247, 164, 149,
248, 201, 154, 171, 60,  13,  94,  111, 65,  112, 35,  18,  133, 180, 231, 214,
122, 75,  24,  41,  190, 143, 220, 237, 195, 242, 161, 144, 7,   54,  101, 84,
57,  8,   91,  106, 253, 204, 159, 174, 128, 177, 226, 211, 68,  117, 38,  23,
252, 205, 158, 175, 56,  9,   90,  107, 69,  116, 39,  22,  129, 176, 227, 210,
191, 142, 221, 236, 123, 74,  25,  40,  6,   55,  100, 85,  194, 243, 160, 145,
71,  118, 37,  20,  131, 178, 225, 208, 254, 207, 156, 173, 58,  11,  88,  105,
4,   53,  102, 87,  192, 241, 162, 147, 189, 140, 223, 238, 121, 72,  27,  42,
193, 240, 163, 146, 5,   52,  103, 86,  120, 73,  26,  43,  188, 141, 222, 239,
130, 179, 224, 209, 70,  119, 36,  21,  59,  10,  89,  104, 255, 206, 157, 172
};

#define CRC_START_8 0x00

uint8_t utils_crc8( const unsigned char *input_str, size_t num_bytes ) {

    size_t a;
    uint8_t crc;
    const unsigned char *ptr;

    crc = CRC_START_8;
    ptr = input_str;

    if ( ptr != NULL ) for (a=0; a<num_bytes; a++) {
        crc = sht75_crc_table[(*ptr++) ^ crc];
    }

    return crc;
}
