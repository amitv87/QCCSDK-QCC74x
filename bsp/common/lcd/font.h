#ifndef __FONT_FONT_ASCII_16X8_H__
#define __FONT_FONT_ASCII_16X8_H__

#define FONT_ASCII_16X8  1
#define FONT_ASCII_32X16 0
#define FONT_ASCII_64X32 0

#if FONT_ASCII_16X8
extern const unsigned char font_ascii_16x8[];
#endif

#if FONT_ASCII_64X32
extern const unsigned char ascii_64x32[];
#endif

extern const unsigned char asc2_1206[95][12];
extern const unsigned char asc2_1608[95][16];
extern const unsigned char asc2_2412[95][36];
extern const unsigned char Hzk[][32];
extern const unsigned char c_chFont1608[95][16];
#endif
