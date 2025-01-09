/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        看门狗实验
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 默认WDT_TIMEOUT_REBOOT为1，看门狗如果没有在设置的时间内feed，则超时后重启系统。
*                 如果修改WDT_TIMEOUT_REBOOT为0，则看门狗超时后只会串口发信息提示超时，不会重启。
*/
#include <stdio.h>
#include <unistd.h>
#include "wdt.h"
#include "sysctl.h"

#define WDT_TIMEOUT_REBOOT    1

/**
* Function       wdt0_irq_cb
* @author        Gengyue
* @date          2020.05.27
* @brief         看门狗中断回调
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
int wdt0_irq_cb(void *ctx)
{
    #if WDT_TIMEOUT_REBOOT
    printf("%s:The system will reboot soon!\n", __func__);
    while(1);
    #else
    printf("%s:The system is busy but not reboot!\n", __func__);
    wdt_clear_interrupt(WDT_DEVICE_0);
    #endif
    return 0;
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
    /* 打印系统启动信息 */
    printf("system start!\n");
    /* 记录feed的次数 */
    int times = 0;

    /* 系统中断初始化 */
    plic_init();
    sysctl_enable_irq();

    /* 启动看门狗，设置超时时间为2秒后调用中断函数wdt0_irq_cb */
    int timeout = wdt_init(WDT_DEVICE_0, 2000, wdt0_irq_cb, NULL);

    /* 打印看门狗实际超时的时间 */
    printf("wdt timeout is %d ms!\n", timeout);
    
    while(1)
    {
        sleep(1);
        if(times++ < 5)
        {
            /* 打印feed的次数 */
            printf("wdt_feed %d times!\n", times);

            /* 重置看门狗的计时器，重新开始计时 */
            wdt_feed(WDT_DEVICE_0);
        }
    }
}

