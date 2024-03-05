#ifndef _ST7789V_SPI_H_
#define _ST7789V_SPI_H_

#include "../lcd_conf.h"

#if defined LCD_SPI_ST7789V

/* Do not modify the following */

#define ST7789V_SPI_COLOR_DEPTH 16

typedef struct {
    uint8_t cmd; /* 0xFF : delay(databytes)ms */
    const char *data;
    uint8_t databytes; /* Num of data in data; or delay time */
} st7789v_spi_init_cmd_t;

typedef uint16_t st7789v_spi_color_t;

int st7789v_spi_init();
void st7789v_spi_async_callback_enable(bool enable);
void st7789v_spi_async_callback_register(void (*callback)(void));
int st7789v_spi_set_dir(uint8_t dir, uint8_t mir_flag);
void st7789v_spi_set_draw_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void st7789v_spi_draw_point(uint16_t x, uint16_t y, st7789v_spi_color_t color);
void st7789v_spi_draw_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, st7789v_spi_color_t color);
void st7789v_spi_draw_picture_nonblocking(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, st7789v_spi_color_t *picture);
void st7789v_spi_draw_picture_blocking(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, st7789v_spi_color_t *picture);
int st7789v_spi_draw_is_busy(void);

#endif
#endif