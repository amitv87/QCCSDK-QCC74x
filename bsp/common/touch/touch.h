#ifndef _TOUCH_H_
#define _TOUCH_H_

#include "qcc74x_core.h"
#include "touch_conf.h"

typedef struct
{
    uint16_t coord_x;
    uint16_t coord_y;
} touch_coord_t;

#if defined TOUCH_SPI_XPT2046

#include "xpt2046_spi.h"
#define TOUCH_INTERFACE_TYPE           TOUCH_INTERFACE_SPI
#define TOUCH_MAX_POINT                XPT2046_MAX_POINT
#define _TOUCH_FUNC_DEFINE(_func, ...) xpt2046_spi_##_func(__VA_ARGS__)

#elif defined TOUCH_I2C_FT6X36

#include "ft6x36_i2c.h"
#define TOUCH_INTERFACE_TYPE           TOUCH_INTERFACE_I2C
#define TOUCH_MAX_POINT                FT6X36_I2C_MAX_POINT
#define _TOUCH_FUNC_DEFINE(_func, ...) ft6x36_i2c_##_func(__VA_ARGS__)

#elif defined TOUCH_I2C_GT911

#include "gt911_i2c.h"
#define TOUCH_INTERFACE_TYPE           TOUCH_INTERFACE_I2C
#define TOUCH_MAX_POINT                GT911_I2C_MAX_POINT
#define _TOUCH_FUNC_DEFINE(_func, ...) gt911_i2c_##_func(__VA_ARGS__)

#elif
#error "Please select a touch type"
#endif

int touch_init(touch_coord_t *max_value);
int touch_read(uint8_t *point_num, touch_coord_t *touch_coord, uint8_t max_num);

#endif