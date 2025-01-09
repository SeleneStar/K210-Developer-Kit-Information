/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        keypad状态机事件
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 拨轮开关keypad通过状态机的方式检测按键的事件，然后根据事件，
*                 控制RGB灯，短按是亮红灯，长按红灯灭，蓝灯闪烁，松开蓝灯熄灭。
*/
#include <stdio.h>
#include "fpioa.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "keypad.h"
#include "rgb.h"
#include "sleep.h"
#include "pin_config.h"


/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         硬件初始化
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void hardware_init(void)
{
    fpioa_set_function(PIN_KEYPAD_LEFT,   FUNC_KEYPAD_LEFT);
    fpioa_set_function(PIN_KEYPAD_MIDDLE, FUNC_KEYPAD_MIDDLE);
    fpioa_set_function(PIN_KEYPAD_RIGHT,  FUNC_KEYPAD_RIGHT);
}

/**
* Function       key_press
* @author        Gengyue
* @date          2020.05.27
* @brief         按键按下事件回调
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void key_press(void * arg)
{
    rgb_red_state(LIGHT_ON);
}

/**
* Function       key_release
* @author        Gengyue
* @date          2020.05.27
* @brief         按键释放事件回调
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void key_release(void * arg)
{
    rgb_blue_state(LIGHT_OFF);
}

/**
* Function       key_long_press
* @author        Gengyue
* @date          2020.05.27
* @brief         长按事件回调
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void key_long_press(void * arg)
{
    rgb_red_state(LIGHT_OFF);
}

/**
* Function       key_repeat
* @author        Gengyue
* @date          2020.05.27
* @brief         重复触发事件回调
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void key_repeat(void * arg)
{
    static int state = 1;
    rgb_blue_state(state = !state);
}

/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         主函数，程序的入口
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
int main(void)
{
    /* 硬件初始化 */
    hardware_init();

    /* 初始化系统中断并使能全局中断 */
    plic_init();
    sysctl_enable_irq();

    /* 初始化RGB灯 */
    rgb_init(EN_RGB_ALL);

    /* 初始化keypad */
    keypad_init();
    
    /* 设置keypad回调 */
    keypad[EN_KEY_ID_LEFT].short_key_down = key_press;
    keypad[EN_KEY_ID_LEFT].short_key_up = key_release;
    keypad[EN_KEY_ID_LEFT].long_key_down = key_long_press;
    keypad[EN_KEY_ID_LEFT].repeat_key_down = key_repeat;

    keypad[EN_KEY_ID_MIDDLE].short_key_down = key_press;
    keypad[EN_KEY_ID_MIDDLE].short_key_up = key_release;
    keypad[EN_KEY_ID_MIDDLE].long_key_down = key_long_press;
    keypad[EN_KEY_ID_MIDDLE].repeat_key_down = key_repeat;

    keypad[EN_KEY_ID_RIGHT].short_key_down = key_press;
    keypad[EN_KEY_ID_RIGHT].short_key_up = key_release;
    keypad[EN_KEY_ID_RIGHT].long_key_down = key_long_press;
    keypad[EN_KEY_ID_RIGHT].repeat_key_down = key_repeat;

    /* keypad状态值 */
    keypad_status_t key_value = EN_KEY_NONE;
    printf("Please control keypad to get status!\n");

    while (1)
    {
        /* 读取keypad的状态值，如果没有事件，默认为0 */
        key_value = get_keypad_state();
        if (key_value != 0)
        {
            switch (key_value)
            {
            case EN_KEY_LEFT_DOWN:
                printf("KEY_LEFT_DOWN:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_LEFT_UP:
                printf("KEY_LEFT_UP:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_LEFT_LONG:
                printf("KEY_LEFT_LONG:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_LEFT_REPEAT:
                printf("KEY_LEFT_REPEAT:%d \n", (uint16_t)key_value);
                break;
            
            case EN_KEY_MIDDLE_DOWN:
                printf("KEY_MIDDLE_DOWN:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_MIDDLE_UP:
                printf("KEY_MIDDLE_UP:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_MIDDLE_LONG:
                printf("KEY_MIDDLE_LONG:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_MIDDLE_REPEAT:
                printf("KEY_MIDDLE_REPEAT:%d \n", (uint16_t)key_value);
                break;
            
            case EN_KEY_RIGHT_DOWN:
                printf("KEY_RIGHT_DOWN:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_RIGHT_UP:
                printf("KEY_RIGHT_UP:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_RIGHT_LONG:
                printf("KEY_RIGHT_LONG:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_RIGHT_REPEAT:
                printf("KEY_RIGHT_REPEAT:%d \n", (uint16_t)key_value);
                break;
            
            default:
                break;
            } 
        }
        msleep(1);
    }
}
