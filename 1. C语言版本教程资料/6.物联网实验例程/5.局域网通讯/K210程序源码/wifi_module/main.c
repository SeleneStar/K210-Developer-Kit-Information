/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        串口实验
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 开机串口发送数据，WiFi模块接收数据传给K210处理.
*                 协议内容:$led0_1#：LED0点亮，$led0_0#：LED0熄灭，
*                         $led1_1#：LED1点亮，$led1_0#：LED1熄灭。
*/
#include <string.h>
#include "pin_config.h"
#include "led.h"

#define MAX_DATA        10

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
    /* USB串口 */
    fpioa_set_function(PIN_UART_USB_RX, FUNC_UART_USB_RX);
    fpioa_set_function(PIN_UART_USB_TX, FUNC_UART_USB_TX);

    /* WIFI模块串口 */
    fpioa_set_function(PIN_UART_WIFI_RX, FUNC_UART_WIFI_RX);
    fpioa_set_function(PIN_UART_WIFI_TX, FUNC_UART_WIFI_TX);

    /* LED灯 */
    led_init(LED_ALL);
}

void parse_data(char *data)
{
    // uart_send_data_dma(UART_USB_NUM, DMAC_CHANNEL0, data, sizeof(data));
    /* 解析并对比发送过来的数据 */
    if (0 == memcmp(data, "led0_0", 6))
    {
        led0_state(LED_OFF);
    }
    else if (0 == memcmp(data, "led0_1", 6))
    {
        led0_state(LED_ON);
    }
    else if (0 == memcmp(data, "led1_0", 6))
    {
        led1_state(LED_OFF);
    }
    else if (0 == memcmp(data, "led1_1", 6))
    {
        led1_state(LED_ON);
    }
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
    hardware_init();
    // 初始化USB串口，设置波特率为115200
    uart_init(UART_USB_NUM);
    uart_configure(UART_USB_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* 初始化WiFi模块的串口 */
    uart_init(UART_WIFI_NUM);
    uart_configure(UART_WIFI_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* 开机发送hello yahboom! */
    char *hello = {"hello yahboom!\n"};
    uart_send_data(UART_USB_NUM, hello, strlen(hello));
    
    /* 接收和发送缓存的数据 */
    char recv = 0, send = 0;
    /* 接收WiFi模块数据标志 */
    int rec_flag = 0;
    /* 保存接收的数据 */
    char recv_data[MAX_DATA] = {0}; 
    /* recv_data下标 */
    uint16_t index = 0;

    while (1)
    {
        /* 接收WIFI模块的信息 */
        if(uart_receive_data(UART_WIFI_NUM, &recv, 1))
        {
            /* 发送接收到的数据到USB串口显示 */
            uart_send_data(UART_USB_NUM, &recv, 1);
            
            /* 判断是否符合数据要求 */
            switch(rec_flag)
            {
            case 0:
                /* 以‘$’符号为数据开始 */
                if(recv == '$')
                {
                    rec_flag = 1;
                    index = 0;
                    for (int i = 0; i < MAX_DATA; i++)
                        recv_data[i] = 0;
                }
                break;
            case 1:
                if (recv == '#')
                {
                    /* 以‘#’符号为数据结束 */
                    rec_flag = 0;
                    parse_data(recv_data);
                }
                else if (index >= MAX_DATA)
                {
                    /* 超过最大数据没有接收到结束符‘#’，则清除 */
                    rec_flag = 0;
                    index = 0;
                }
                else
                {
                    /* 保存数据到recv_data中 */
                    recv_data[index++] = recv;
                }
                break;
            default:
                break;
            }
        }

        /* 接收串口的信息，并发送给WiFi模块 */
        if(uart_receive_data(UART_USB_NUM, &send, 1))
        {
            uart_send_data(UART_WIFI_NUM, &send, 1);
        }
    }
    return 0;
}
