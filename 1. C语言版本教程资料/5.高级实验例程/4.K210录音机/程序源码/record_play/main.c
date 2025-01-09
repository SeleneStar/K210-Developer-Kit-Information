#include <stdio.h>
#include "bsp.h"
#include "dmac.h"
#include "ff.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "i2s.h"
#include "keypad.h"
#include "malloc.h"
#include "pin_config.h"
#include "plic.h"
#include "rgb.h"
#include "bsp_sdcard.h"
#include "sysctl.h"
#include "timer.h"
#include "uarths.h"
#include "wav_decode.h"
#include "bsp_recorder.h"
#include "bsp_speaker.h"



static void hardware_init(void);
static int sdcard_test(void);
static int fs_test(void);

keypad_status_t key_value = EN_KEY_NONE;
size_t oldlen, newlen;
int g_record_state = 0;

static void hardware_init(void)
{
    /* TF卡 */
    //io26--miso--d1
    //io27--clk---sclk
    //io28--mosi--d0
    //io29--cs----cs
    fpioa_set_function(PIN_TF_MISO, FUNC_TF_SPI_MISO);
    fpioa_set_function(PIN_TF_CLK,  FUNC_TF_SPI_CLK);
    fpioa_set_function(PIN_TF_MOSI, FUNC_TF_SPI_MOSI);
    fpioa_set_function(PIN_TF_CS,   FUNC_TF_SPI_CS);

    /* 扬声器 */
    fpioa_set_function(PIN_I2S_WS,  FUNC_I2S2_WS);
    fpioa_set_function(PIN_I2S_DA,  FUNC_I2S2_OUT_D1);
    fpioa_set_function(PIN_I2S_BCK, FUNC_I2S2_SCLK);

    /* 麦克风 */
    fpioa_set_function(PIN_MIC0_WS,   FUNC_I2S0_WS);
    fpioa_set_function(PIN_MIC0_DATA, FUNC_I2S0_IN_D0);
    fpioa_set_function(PIN_MIC0_SCK,  FUNC_I2S0_SCLK);

    /* keypad */
    fpioa_set_function(PIN_KEYPAD_LEFT,   FUNC_KEYPAD_LEFT);
    fpioa_set_function(PIN_KEYPAD_MIDDLE, FUNC_KEYPAD_MIDDLE);
    fpioa_set_function(PIN_KEYPAD_RIGHT,  FUNC_KEYPAD_RIGHT);

    /* RGB灯 */
    fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
    fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
    fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);
}

int main(void)
{
    /* 系统时钟设置 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 320000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    //uarths_init();

    hardware_init();      // 硬件引脚初始化
    dmac_init();          // dmac初始化
    plic_init();          // 中断初始化
    sysctl_enable_irq();  // 使能全局中断
    keypad_init();        // keypad初始化
    rgb_init(EN_RGB_ALL); // LED灯初始化

    printf("****************system is start!*****************\n");

    if(sdcard_test())
    {
        printf("SD card err\r\n");
        return -1;
    }
    if(fs_test())
    {
        printf("FAT32 err\r\n");
        return -1;
    }
    
    printf("wave:%ld\r\n", sizeof(wav_file_t));
    printf("wave:%ld\r\n", sizeof(wave_header_t));
    while(1)
    {
        //获取按键值    
        key_value = key_out_fifo();
        switch(key_value)
        {
            case EN_KEY_LEFT_DOWN: //停止和保存录音
                printf("EN_KEY_LEFT_DOWN\r\n");
                if(g_record_state == 1)
                {
                    bsp_recorder_stopsave("0:REC.wav");
                    g_record_state = 2;
                }
                app_rgb_red_state(LIGHT_OFF); //关闭红灯
                break;

            case EN_KEY_MIDDLE_DOWN: //录音
                printf("EN_KEY_MIDDLE_DOWN\r\n");
                if(g_record_state != 1)
                {
                    bsp_recorder_start(_T("0:REC.wav"));
                    g_record_state = 1;
                }
                
                break;

            case EN_KEY_RIGHT_DOWN: //播放录音（仅在非录音状态下有效）
                printf("EN_KEY_RIGHT_DOWN\r\n");
                app_rgb_blue_state(LIGHT_ON);
                if(g_record_state == 2)
                {
                    bsp_play_wav(_T("0:REC.wav"));
                    //g_record_state = 2;
                }
                break;

            default:
                break;
        }
        newlen = get_free_heap_size();
        if(oldlen != newlen)
        {
            oldlen = newlen;
            printk("get_free_heap_size=%ld\n", oldlen);
        }
        //rgb_green_state(LIGHT_ON); 
        msleep(10);
        
    }
       
    return 0;
}

static int sdcard_test(void)
{
    uint8_t status;

    printf("/******************sdcard test*****************/\r\n");
    status = bsp_sd_init();
    printf("sd init %d\r\n", status);
    if(status != 0)
    {
        return status;
    }

    printf("card info status %d\r\n", status);
    printf("CardCapacity:%ld\r\n", cardinfo.CardCapacity);
    printf("CardBlockSize:%d\r\n", cardinfo.CardBlockSize);
    return 0;
}

static int fs_test(void)
{
    static FATFS sdcard_fs;
    FRESULT status;
    DIR dj;
    FILINFO fno;

    printf("/********************fs test*******************/\r\n");
    status = f_mount(&sdcard_fs, _T("0:"), 1);
    printf("mount sdcard:%d\r\n", status);
    if(status != FR_OK)
        return status;

    printf("printf filename\r\n");
    status = f_findfirst(&dj, &fno, _T("0:"), _T("*"));
    while(status == FR_OK && fno.fname[0])
    {
        if(fno.fattrib & AM_DIR)
            printf("dir:%s\r\n", fno.fname);
        else
            printf("file:%s\r\n", fno.fname);
        status = f_findnext(&dj, &fno);
    }
    f_closedir(&dj);
    return 0;
}

