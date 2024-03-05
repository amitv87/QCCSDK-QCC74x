#include "touch.h"

int touch_init(touch_coord_t *max_value)
{
    return _TOUCH_FUNC_DEFINE(init, max_value);
}

int touch_read(uint8_t *point_num, touch_coord_t *touch_coord, uint8_t max_num)
{
    return _TOUCH_FUNC_DEFINE(read, point_num, touch_coord, max_num);
}
