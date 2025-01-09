/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        麦克风录音，扬声器播放
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 麦克风录音，扬声器播放。
*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "i2s.h"
#include "sysctl.h"
#include "fpioa.h"
#include "uarths.h"
#include "gpiohs.h"
#include "pin_config.h"

/* 麦克风增益值，可以根据实际调大录音的音量 */
#define MIC_GAIN      1
#define FRAME_LEN     512
int16_t rx_buf[FRAME_LEN * 2 * 2];
uint32_t g_index = 0;
uint32_t g_tx_len = 0;

uint32_t g_rx_dma_buf[FRAME_LEN * 2 * 2];
uint8_t i2s_rec_flag = 0;

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
    /* mic */
    fpioa_set_function(PIN_MIC0_WS,   FUNC_MIC0_WS);
    fpioa_set_function(PIN_MIC0_DATA, FUNC_MIC0_DATA);
    fpioa_set_function(PIN_MIC0_SCK,  FUNC_MIC0_SCK);

    /* speak dac */
    fpioa_set_function(PIN_SPK_WS,   FUNC_SPK_WS);
    fpioa_set_function(PIN_SPK_DATA, FUNC_SPK_DATA);
    fpioa_set_function(PIN_SPK_BCK,  FUNC_SPK_BCK);
}

/**
* Function       i2s_receive_dma_cb
* @author        Gengyue
* @date          2020.05.27
* @brief         I2S0接收麦克风数据中断回调函数
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
int i2s_receive_dma_cb(void *ctx)
{
    uint32_t i;

    if(g_index)
    {
        /* 接收DMA数据 */
        i2s_receive_data_dma(I2S_DEVICE_0, &g_rx_dma_buf[g_index], FRAME_LEN * 2, DMAC_CHANNEL1);
        g_index = 0;
        for(i = 0; i < FRAME_LEN; i++)
        {
            /* 保存数据 */
            rx_buf[2 * i] = (int16_t)((g_rx_dma_buf[2 * i + 1] * MIC_GAIN) & 0xffff);
            rx_buf[2 * i + 1] = (int16_t)((g_rx_dma_buf[2 * i + 1] * MIC_GAIN) & 0xffff);
        }
        i2s_rec_flag = 1;
    }
    else
    {
        i2s_receive_data_dma(I2S_DEVICE_0, &g_rx_dma_buf[0], FRAME_LEN * 2, DMAC_CHANNEL1);
        g_index = FRAME_LEN * 2;
        for(i = FRAME_LEN; i < FRAME_LEN * 2; i++)
        {
            rx_buf[2 * i] = (int16_t)((g_rx_dma_buf[2 * i + 1] * MIC_GAIN) & 0xffff);
            rx_buf[2 * i + 1] = (int16_t)((g_rx_dma_buf[2 * i + 1] * MIC_GAIN) & 0xffff);
        }
        i2s_rec_flag = 2;
    }
    return 0;
}

/**
* Function       init_mic
* @author        Gengyue
* @date          2020.05.27
* @brief         初始化麦克风配置
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
void init_mic(void)
{
    /* I2S设备0初始化为接收模式 */
    i2s_init(I2S_DEVICE_0, I2S_RECEIVER, 0x0C);

    /* 通道参数设置 */
    i2s_rx_channel_config(
        I2S_DEVICE_0, /* I2S设备0 */
        I2S_CHANNEL_1, /* 通道1 */
        RESOLUTION_16_BIT, /* 接收数据16bit */
        SCLK_CYCLES_32, /* 单个数据时钟为32 */
        TRIGGER_LEVEL_4, /* FIFO深度为4 */
        STANDARD_MODE); /* 标准模式 */
    
    /* 设置采样率 */
    i2s_set_sample_rate(I2S_DEVICE_0, 16000);

    /* 设置DMA中断回调 */
    dmac_set_irq(DMAC_CHANNEL1, i2s_receive_dma_cb, NULL, 4);

    /* I2S通过DMA接收数据，保存到rx_buf中 */
    i2s_receive_data_dma(I2S_DEVICE_0, &rx_buf[g_index], FRAME_LEN * 2, DMAC_CHANNEL1);
}

/**
* Function       init_speaker
* @author        Gengyue
* @date          2020.05.27
* @brief         初始化DAC（扬声器）配置
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
void init_speaker(void)
{
    /* 初始化I2S，第三个参数为设置通道掩码，通道0:0x03,通道1：0x0C,通道2：0x30,通道3:0xC0 */
    i2s_init(I2S_DEVICE_2, I2S_TRANSMITTER, 0x03);
    
    /* 设置I2S发送数据的通道参数 */
    i2s_tx_channel_config(
        I2S_DEVICE_2, /* I2S设备号*/
        I2S_CHANNEL_0, /* I2S通道 */
        RESOLUTION_16_BIT, /* 接收数据位数 */
        SCLK_CYCLES_32, /* 单个数据时钟数 */
        TRIGGER_LEVEL_4, /* DMA触发时FIFO深度 */
        RIGHT_JUSTIFYING_MODE); /* 工作模式 */
    /* 设置采样率 */
    i2s_set_sample_rate(I2S_DEVICE_2, 16000);
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

    /* 设置系统时钟 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 320000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

    /* 初始化中断，使能全局中断，初始化dmac */
    plic_init();
    sysctl_enable_irq();
    dmac_init();

    /* 初始化扬声器和麦克风 */
    init_speaker();
    init_mic();

    while (1)
    {
        if(i2s_rec_flag == 1)
        {
            i2s_play(
                I2S_DEVICE_2,  /* I2S设备号 */
                DMAC_CHANNEL0, /* DMA通道号 */ 
                (uint8_t *)(&rx_buf[0]), /* 播放的PCM数据 */
                FRAME_LEN * 4, /* PCM数据的长度 */
                1024, /* 单次发送数量 */
                16,   /* 单次采样位宽 */
                2);   /* 声道数 */
            
            i2s_rec_flag = 0;
        }
        else if(i2s_rec_flag == 2)
        {
            i2s_play(
                I2S_DEVICE_2, /* I2S设备号 */
                DMAC_CHANNEL0, /* DMA通道号 */ 
                (uint8_t *)(&rx_buf[FRAME_LEN * 2]), /* 播放的PCM数据 */
                FRAME_LEN * 4, /* PCM数据的长度 */
                1024, /* 单次发送数量 */
                16, /* 单次采样位宽 */
                2); /* 声道数 */
            
            i2s_rec_flag = 0;
        }
    }

    return 0;
}
