/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        lvgl图形化操作实验
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 总共有三个操作界面，点击顶栏的字符切换，或者左右滑动切换。
*                 第一个界面“Write”：是显示字符和虚拟键盘，点击输入框的位置，
*                 会弹出一个虚拟键盘，可以在上面触摸输入对应的字符。
*                 第二个界面“List”：上下滑动可以切换显示对应的图标和字符，
*                 触摸一下，就会在第一个界面上输入列表上的字符。
*                 第三个界面“Chart”：显示一个频谱图像，底部是一个滑动杆，
*                 拖动滑动杆可以实时改变频谱上的条形图的高低。
*/
#include <stdint.h>
#include <unistd.h>
#include "sleep.h"
#include "sysctl.h"
#include "fpioa.h"
#include "pin_config.h"
#include "lcd.h"
#include "st7789.h"
#include "lvgl/lvgl.h"
#include "lv_examples/lv_apps/demo/demo.h"
#include "ft6236u.h"
#include "gui_lvgl.h"

/**
* Function       io_set_power
* @author        Gengyue
* @date          2020.05.27
* @brief         设置显示屏IO口电压1.8V
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
static void io_set_power(void)
{
    /* 设置显示器电压为1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
}

/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         硬件初始化
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
static void hardware_init(void)
{
    /* SPI lcd */
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);
    sysctl_set_spi0_dvp_data(1);

    /* I2C FT6236 */
    fpioa_set_function(PIN_FT_SCL, FUNC_FT_SCL);
    fpioa_set_function(PIN_FT_SDA, FUNC_FT_SDA);
    fpioa_set_function(PIN_FT_INT, FUNC_FT_INT);
    // fpioa_set_function(PIN_FT_RST, FUNC_FT_RST);
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
    printf("system start init\n");
    hardware_init();
    io_set_power();

    /* 初始化触摸屏并显示图片 */
    lcd_init();
    ft6236_init();
    lcd_draw_picture_half(0, 0, 320, 240, gImage_logo);
    sleep(1);

    /* lvgl初始化 */
    lvgl_disp_input_init();

    /* 运行例程 */
    demo_create();

    printf("system start ok\n");
    printf("Please touch the screen\n");
    while (1)
        ;
    return 0;
}
