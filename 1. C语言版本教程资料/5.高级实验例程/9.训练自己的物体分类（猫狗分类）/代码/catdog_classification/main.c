/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       liusen
* @version      V1.0
* @date         2020.07.03
* @brief        猫狗分类
* @details      
* @par History  见如下说明
*                 
* version:	V1.0 
*/
#include <stdio.h>
#include <unistd.h>
#include "kpu.h"
#include <platform.h>
#include <printf.h>
#include <string.h>
#include <stdlib.h>
#include "bsp.h"
#include <sysctl.h>
#include "plic.h"
#include "utils.h"
#include <float.h>
#include "uarths.h"
#include "fpioa.h"
#include "lcd.h"
#include "dvp.h"
#include "ov2640.h"
#include "ov9655.h"
#include "gc2145.h"
#include "uarths.h"
#include "image_process.h"
#include "board_config.h"
#include "nt35310.h"
#include "gpiohs.h"
#include "gpio.h"
#include "spi.h"
#include "incbin.h"
#include "image.h"

#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#define CLASS10 1
#define min(a,b) (((a) < (b)) ? (a) : (b))

#define PROB_THRESH     (0.7f)

#define PLL0_OUTPUT_FREQ 1000000000UL
#define PLL1_OUTPUT_FREQ 400000000UL
#define PLL2_OUTPUT_FREQ 45158400UL
#define OVXXXX_ADDR    0x60 
#define OV9655_PID_1   0x9657   
#define OV9655_PID_2   0x9656  
#define OV2640_PID     0x2642

enum OV_type
{
    OV_error = 0,
    OV_9655 = 1,
    OV_2640 = 2,
};

volatile uint8_t ircut_value = 0x01;
volatile uint8_t r_ircut_value = 0x00;

volatile uint32_t g_ai_done_flag;
volatile uint8_t g_dvp_finish_flag;
static image_t kpu_image, display_image, crop_image;

extern const unsigned char gImage_image[] __attribute__((aligned(128)));



static uint16_t lcd_gram[320 * 240] __attribute__((aligned(32)));
kpu_model_context_t task1;

INCBIN(model, "catanddog.kmodel");

static void ai_done(void* userdata)
{
    g_ai_done_flag = 1;
    
    float *features;
    size_t count;
    kpu_get_output(&task1, 0, (uint8_t **)&features, &count);
    count /= sizeof(float);

    size_t i;
    for (i = 0; i < count; i++)
    {
        if (i % 64 == 0)
            printf("\n");
        printf("%f, ", features[i]);
    }

    printf("\n");
}

static int dvp_irq(void *ctx)
{
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    }
    else
    {
        dvp_start_convert();
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }
    return 0;
}

static void io_init(void)
{
    /* Init DVP IO map and function settings */
    fpioa_set_function(OV_RST_PIN, FUNC_CMOS_RST);
    fpioa_set_function(OV_PWDN_PIN, FUNC_CMOS_PWDN);
    fpioa_set_function(OV_XCLK_PIN, FUNC_CMOS_XCLK);
    fpioa_set_function(OV_VSYNC_PIN, FUNC_CMOS_VSYNC);
    fpioa_set_function(OV_HREF_PIN, FUNC_CMOS_HREF);
    fpioa_set_function(OV_PCLK_PIN, FUNC_CMOS_PCLK);
    fpioa_set_function(OV_SCCB_SCLK_PIN, FUNC_SCCB_SCLK);
    fpioa_set_function(OV_SCCB_SDA_PIN, FUNC_SCCB_SDA);

    /* Init SPI IO map and function settings */
    fpioa_set_function(LCD_DC_PIN, FUNC_LCD_DC);
    fpioa_set_function(LCD_CS_PIN, FUNC_LCD_CS);
    fpioa_set_function(LCD_RW_PIN, FUNC_LCD_RW);
    fpioa_set_function(LCD_RST_PIN, FUNC_LCD_RST);

    sysctl_set_spi0_dvp_data(1);

}

static void io_set_power(void)
{
    /* Set dvp and spi pin to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

void rgb888_to_lcd(uint8_t *src, uint16_t *dest, size_t width, size_t height)
{
    size_t i, chn_size = width * height;
    for (size_t i = 0; i < width * height; i++)
    {
        uint8_t r = src[i];
        uint8_t g = src[chn_size + i];
        uint8_t b = src[chn_size * 2 + i];

        uint16_t rgb = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
        size_t d_i = i % 2 ? (i - 1) : (i + 1);
        dest[d_i] = rgb;
    }
}

void lcd_ram_draw_rgb888(uint8_t *src, uint16_t *dest, size_t width, size_t height, size_t x_off, size_t y_off, size_t stride)
{
    size_t x, y, chn_size = width * height;
    for (size_t y = 0; y < min(height, 240); y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            size_t i = y * width + x;
            uint8_t r = src[i];
            uint8_t g = src[chn_size + i];
            uint8_t b = src[chn_size * 2 + i];

            uint16_t rgb = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
            i = (y + y_off) * stride + x + x_off;
            size_t d_i = i % 2 ? (i - 1) : (i + 1);
            dest[d_i] = rgb;
        }
    }
}

int argmax(float* src, size_t count)
{
    float max = FLT_MIN;
    size_t i, max_i = 0;
    for (i = 0; i < count; i++)
    {
        if (src[i] > max)
        {
            max = src[i];
            max_i = i;
        }
    }

    return max_i;
}

int OVxxxx_read_id(void) //只适用OV9655和2640
{
    uint16_t manuf_id = 0;
    uint16_t device_id = 0;
    uint16_t *manuf_id_P = &manuf_id;
    uint16_t *device_id_P = &device_id;

    *manuf_id_P = (dvp_sccb_receive_data(OVXXXX_ADDR, 0x1C) << 8) | dvp_sccb_receive_data(OVXXXX_ADDR, 0x1D); //读制造商标识符
    *device_id_P = (dvp_sccb_receive_data(OVXXXX_ADDR, 0x0A) << 8) | dvp_sccb_receive_data(OVXXXX_ADDR, 0x0B); //读PID和VER
    

    if(device_id==OV9655_PID_1 ||device_id==OV9655_PID_2)//OV9655
    {
        printf("This is the OV9655 camera\n");
        printf("manuf_id:0x%04x,device_id:0x%04x\n", manuf_id, device_id);
        return OV_9655;
    }
    else if(device_id==OV2640_PID)//OV2640
    {
        printf("This is the OV2640 camera\n");
        printf("manuf_id:0x%04x,device_id:0x%04x\n", manuf_id, device_id);
        return OV_2640;
    }
    else
    {
        printf("manuf_id:0x%04x,device_id:0x%04x\n", manuf_id, device_id);
        printf("Camera failure\n");
        return OV_error;
    }
    return 0;
}

int main()
{
    /* Set CPU and dvp clk */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
    plic_init();
    io_set_power();
    io_init();
    
    /* LCD init */
    printf("LCD init\n");
    lcd_init();
    lcd_set_direction(DIR_YX_LRUD);
    lcd_clear(BLACK);
    
    lcd_draw_picture_half(0, 0, 320, 240, logo);
    lcd_draw_string(70, 40, "Hello Yahboom!", RED);
    lcd_draw_string(70, 60, "cat/dog classification demo!", BLUE);
    sleep(1);

    /* DVP init */
    printf("DVP init\n");
    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    dvp_set_image_size(320, 240);

    int OV_type;
    OV_type=OVxxxx_read_id();
    /* 初始化摄像头 */
    if(OV_type == OV_9655)
    {
        ov9655_init();
    }   
    else if(OV_type == OV_2640) 
    {
        ov2640_init();
    }
     else //读取gc2145摄像头
    {
        uint16_t device_id;
        gc2145_read_id(&device_id);
        printf("device_id:0x%04x\n", device_id);
        if(device_id != GC2145_ID)
        {
            printf("Camera failure\n");
            return 0;//打不开摄像头，结束
        }
        printf("This is the GC2145 camera\n");
        gc2145_init();//初始化
    }
 

    kpu_image.pixel = 3;
    kpu_image.width = 320;
    kpu_image.height = 240;
    image_init(&kpu_image);
    
    display_image.pixel = 2; //2->3 LIUSEN
    display_image.width = 320;
    display_image.height = 240;
    image_init(&display_image);
    
    crop_image.pixel = 3;
    crop_image.width = 224;
    crop_image.height = 224;
    image_init(&crop_image);

    //存放AI图像的地址，供AI模块进行算法处理（红色、绿色、蓝色/分量地址）
    dvp_set_ai_addr((uint32_t)kpu_image.addr, (uint32_t)(kpu_image.addr + 320 * 240), (uint32_t)(kpu_image.addr + 320 * 240 * 2));
    //设置采集图像在内存中的存放地址，可以用来显示
    dvp_set_display_addr((uint32_t)display_image.addr);
    //图像开始采集中断| 图像结束采集中断
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    //禁用自动接收图像模式
    dvp_disable_auto();

    /* DVP interrupt config */
    printf("DVP interrupt config\n");


    plic_set_priority(IRQN_DVP_INTERRUPT, 1);               //设置中断优先级
    plic_irq_register(IRQN_DVP_INTERRUPT, dvp_irq, NULL);   //注册外部中断函数
    plic_irq_enable(IRQN_DVP_INTERRUPT);                    //使能外部中断
    /* init model */
    //加载 kmodel，需要与 nncase 配合使用
    if (kpu_load_kmodel(&task1, model_data) != 0)
    {
        printf("Cannot load kmodel.\n");
        exit(-1);
    }

    
    sysctl_enable_irq();
      
    /* system start */
    printf("System start\n");
    while (1)
    {
        g_dvp_finish_flag = 0;
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        while (g_dvp_finish_flag == 0)
            ;
            
        image_crop(&kpu_image, &crop_image, 48, 8);

        g_ai_done_flag = 0;

        if (kpu_run_kmodel(&task1, crop_image.addr, DMAC_CHANNEL5, ai_done, NULL) != 0)
        {
            printf("Cannot run kmodel.\n");
            exit(-1);
        }
		while (!g_ai_done_flag);

        float *features;
        size_t output_size;
        // 获取 KPU 最终处理的结果  KPU任务句柄  结果的索引值  结果  大小（字节）
        kpu_get_output(&task1, 0, &features, &output_size);
        size_t cls = argmax(features, 2);

        const char *text = NULL;
        //{'cat': 0, 'dog': 1},
        switch (cls)
        {
            case 0:
                text = "cat";
                break;
            case 1:
                text = "dog";
                break;
        }

        
        /* display pic*/
        if (features[cls] > PROB_THRESH)
			ram_draw_string(display_image.addr, 150, 20, text, RED);
		lcd_draw_picture(0, 0, 320, 240, (uint32_t *)display_image.addr);
    }
    
}