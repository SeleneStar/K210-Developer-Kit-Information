/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        水平测试板
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 显示一个机器人图标，根据icm20607传感器计算开发板的角度，
*                 摆动开发板可以让机器人图标移动，平放在桌面时，机器人图标在中间。
*/
#include "sleep.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "icm20607.h"
#include "pin_config.h"
#include "angle.h"
#include "plic.h"
#include "lcd.h"
#include "lvgl_display.h"


/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         硬件初始化，绑定GPIO口
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void hardware_init(void)
{
    /* I2C ICM20607 */
    fpioa_set_function(PIN_ICM_SCL, FUNC_ICM_SCL);
    fpioa_set_function(PIN_ICM_SDA, FUNC_ICM_SDA);

    /* SPI lcd */
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);
    sysctl_set_spi0_dvp_data(1);
}

/**
* Function       io_set_power
* @author        Gengyue
* @date          2020.05.27
* @brief         设置LCD显示IO口电平电压1.8V
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
static void io_set_power(void)
{
    /* 设置显示器电压为1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
}


/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         主函数，程序的入口
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
int main(void)
{
    /* 硬件引脚初始化 */
    hardware_init();
    io_set_power();
    
    /* 系统中断使能 */
    plic_init();
    sysctl_enable_irq();

    /* LCD初始化，并显示一秒图片 */
    lcd_init();
    lcd_draw_picture_half(0, 0, 320, 240, gImage_logo);
    sleep(1);

    /* 初始化ICM20607 */
    icm20607_init();
    msleep(100);

    /* 初始化lvgl */
    lvgl_disp_init();

    /* 创建机器人图标 */
    lvgl_creat_image();

    int16_t lvgl_x, lvgl_y;
    while (1)
    {
        /* 读取角度 */
        get_icm_attitude();

        /* 转化数据 */
        lvgl_x = lvgl_get_x(g_attitude.roll);
        lvgl_y = lvgl_get_y(g_attitude.pitch);

        /* 修改机器人图标的位置 */
        lvgl_move_image(lvgl_x, lvgl_y);
        msleep(1);
    }
    
    return 0;
}
