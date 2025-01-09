/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        陀螺仪读取数据
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 通过串口，把陀螺仪的XYZ轴打印出来。
*/
#include "sleep.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "i2c_ctl.h"
#include "icm20607.h"
#include "pin_config.h"

#define GYRO_DATA     1
#define ACC_DATA      0

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
    // 硬件引脚初始化
    hardware_init();
    /* 扫描并打印当前I2C总线的设备地址 */
    printf("I2C SCAN START! \n");
    i2c_read_addr();
    printf("I2C SCAN END! \n");
    msleep(500);

    /* 初始化ICM20607 */
    icm20607_init();
    printf("icm20607 init ok!\n");

    #if ACC_DATA
    int16_t val_ax = 0;
    int16_t val_ay = 0;
    int16_t val_az = 0;
    #else
    int16_t val_gx = 0;
    int16_t val_gy = 0;
    int16_t val_gz = 0;
    #endif

    while (1)
    {
        #if GYRO_DATA
        val_gx = getRawGyroscopeX();
        val_gy = getRawGyroscopeY();
        val_gz = getRawGyroscopeZ();
        printf("gx=%d, gy=%d, gz=%d\n", val_gx, val_gy, val_gz);
        val_gx = val_gy = val_gz = 0;
        #elif ACC_DATA
        val_ax = getRawAccelerationX();
        val_ay = getRawAccelerationY();
        val_az = getRawAccelerationZ();
        printf("ax=%d, ay=%d, az=%d\n", val_ax, val_ay, val_az);
        val_ax = val_ay = val_az = 0;
        #else
        printf("Please set the GYRO_DATA or ACC_DATA to 1\n");
        #endif
        msleep(5);
    }

    return 0;
}
