#ifndef _GC9503V_DPI_H_
#define _GC9503V_DPI_H_

#include "../lcd_conf.h"

/* Do not modify the following information */

#if defined LCD_DPI_GC9503V

#if (GC9503V_DPI_INIT_INTERFACE == 1)

/* Software spi 9-bit mode PIN */
#define GC9503V_SPI_CS_HIGH qcc74x_gpio_set(gc9503v_gpio, GC9503V_DPI_SPI_CS_PIN)
#define GC9503V_SPI_CS_LOW  qcc74x_gpio_reset(gc9503v_gpio, GC9503V_DPI_SPI_CS_PIN)

#define GC9503V_SPI_CLK_HIGH qcc74x_gpio_set(gc9503v_gpio, GC9503V_DPI_SPI_CLK_PIN)
#define GC9503V_SPI_CLK_LOW  qcc74x_gpio_reset(gc9503v_gpio, GC9503V_DPI_SPI_CLK_PIN)

#define GC9503V_SPI_MOSI_HIGH qcc74x_gpio_set(gc9503v_gpio, GC9503V_DPI_SPI_DAT_PIN)
#define GC9503V_SPI_MOSI_LOW  qcc74x_gpio_reset(gc9503v_gpio, GC9503V_DPI_SPI_DAT_PIN)

#define DUMMY_DELAY      \
    asm volatile("nop"); \
    asm volatile("nop"); \
    asm volatile("nop"); \
    asm volatile("nop"); \
    asm volatile("nop"); \
    asm volatile("nop"); \
    asm volatile("nop"); \
    asm volatile("nop"); \
    asm volatile("nop"); \
    asm volatile("nop"); \
    asm volatile("nop"); \
    asm volatile("nop"); \
    asm volatile("nop"); \
    asm volatile("nop");

#endif

#if (GC9503V_DPI_PIXEL_FORMAT == 1)
#define GC9503V_DPI_COLOR_DEPTH 16
typedef uint16_t gc9503v_dpi_color_t;
#elif (GC9503V_DPI_PIXEL_FORMAT == 2)
#define GC9503V_DPI_COLOR_DEPTH 32
typedef uint32_t gc9503v_dpi_color_t;
#else
#error "GC9503V pixel format select error"
#endif

/* Gc9503v needs to be initialized using the DBI(typeC) or SPI interface */
typedef struct {
    uint8_t cmd; /* 0xFF : delay(databytes)ms */
    const char *data;
    uint8_t databytes; /* Num of data in data; or delay time */
} gc9503v_dpi_init_cmd_t;

int gc9503v_dpi_init(gc9503v_dpi_color_t *screen_buffer);
int gc9503v_dpi_screen_switch(gc9503v_dpi_color_t *screen_buffer);
gc9503v_dpi_color_t *gc9503v_dpi_get_screen_using(void);
int gc9503v_dpi_frame_callback_register(uint32_t callback_type, void (*callback)(void));

#endif
#endif
