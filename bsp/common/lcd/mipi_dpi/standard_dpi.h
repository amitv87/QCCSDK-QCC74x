#ifndef _STANDARD_DBI_H_
#define _STANDARD_DBI_H_

#include "../lcd_conf.h"

#if defined LCD_DPI_STANDARD

/* Do not modify the following */

#if (STANDARD_DPI_PIXEL_FORMAT == 1)
#define STANDARD_DPI_COLOR_DEPTH 16
typedef uint16_t standard_dpi_color_t;
#elif (STANDARD_DPI_PIXEL_FORMAT == 2)
#define STANDARD_DPI_COLOR_DEPTH 32
typedef uint32_t standard_dpi_color_t;
#endif

int standard_dpi_init(standard_dpi_color_t *screen_buffer);
int standard_dpi_screen_switch(standard_dpi_color_t *screen_buffer);
standard_dpi_color_t *standard_dpi_get_screen_using(void);
int standard_dpi_frame_callback_register(uint32_t callback_type, void (*callback)(void));

#endif
#endif
