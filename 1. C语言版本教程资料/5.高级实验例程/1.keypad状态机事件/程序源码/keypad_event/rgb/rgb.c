#include "gpiohs.h"
#include "fpioa.h"
#include "rgb.h"
#include "pin_config.h"

/* 初始化RGB灯 */
void rgb_init(rgb_color_t color)
{
    switch (color)
    {
    case EN_RGB_RED:
        fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
        gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
        break;
    case EN_RGB_GREEN:
        fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
        gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
        gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
        break;
    case EN_RGB_BLUE:
        fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);
        gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);
        gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
        break;
    case EN_RGB_ALL:
        fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
        gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
        fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
        gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
        fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);
        gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);

        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
        gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
        gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
        break;
    
    default:
        break;
    }
}

/* 设置RGB-红灯状态 */
void rgb_red_state(rgb_state_t state)
{
    gpiohs_set_pin(RGB_R_GPIONUM, state);
}

/* 设置RGB-绿灯状态 */
void rgb_green_state(rgb_state_t state)
{
    gpiohs_set_pin(RGB_G_GPIONUM, state);
}

/* 设置RGB-蓝灯状态 */
void rgb_blue_state(rgb_state_t state)
{
    gpiohs_set_pin(RGB_B_GPIONUM, state);
}

/* 设置RGB灯全点亮 */
void rgb_all_on(void)
{
    gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_LOW);
    gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_LOW);
    gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_LOW);
}

/* 设置RGB灯全熄灭 */
void rgb_all_off(void)
{
    gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
}
