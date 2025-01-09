#include "gpiohs.h"
#include "rgb.h"
#include "pin_config.h"



void rgb_init(rgb_color_t color)
{
    switch (color)
    {
    case EN_RGB_RED:
        gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
        break;
    case EN_RGB_GREEN:
        gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
        gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
        break;
    case EN_RGB_BLUE:
        gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);
        gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
        break;
    case EN_RGB_ALL:
        gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
        gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
        gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);

        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
        gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
        gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
        break;
    
    default:
        break;
    }
}

void app_rgb_red_state(rgb_state_t state)
{
    gpiohs_set_pin(RGB_R_GPIONUM, state);
}

void app_rgb_green_state(rgb_state_t state)
{
    gpiohs_set_pin(RGB_G_GPIONUM, state);
}

void app_rgb_blue_state(rgb_state_t state)
{
    gpiohs_set_pin(RGB_B_GPIONUM, state);
}

void rgb_all_on(void)
{
    gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_LOW);
    gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_LOW);
    gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_LOW);
}

void rgb_all_off(void)
{
    gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
}
