#ifndef _RGB_H_
#define _RGB_H_

typedef enum _rgb_color_t
{
    EN_RGB_RED = 0,
    EN_RGB_GREEN,
    EN_RGB_BLUE,
    EN_RGB_ALL
} rgb_color_t;

typedef enum _rgb_state_t
{
    LIGHT_ON = 0,
    LIGHT_OFF = 1,
} rgb_state_t;

void rgb_init(rgb_color_t color);
void rgb_red_state(rgb_state_t state);
void rgb_green_state(rgb_state_t state);
void rgb_blue_state(rgb_state_t state);
void rgb_all_on(void);
void rgb_all_off(void);

#endif  /* _RGB_H_ */
