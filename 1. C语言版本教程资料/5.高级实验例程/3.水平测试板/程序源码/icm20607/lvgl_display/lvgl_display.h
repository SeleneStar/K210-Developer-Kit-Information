
#ifndef _LVGL_DISPLAY_H_
#define _LVGL_DISPLAY_H_

#include "stdint.h"

#define IMG_ROBOT_WIDTH       93
#define IMG_ROBOT_HIGH        47

void lvgl_disp_init(void);
void lvgl_creat_image(void);
void lvgl_move_image(int16_t x, int16_t y);
int16_t lvgl_get_x(float a);
int16_t lvgl_get_y(float a);
int16_t convert_value(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max);

#endif /* _LVGL_DISPLAY_H_ */
