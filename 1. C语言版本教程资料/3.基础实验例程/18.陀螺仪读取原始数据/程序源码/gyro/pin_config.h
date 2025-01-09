/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         pin_config.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        硬件引脚与软件GPIO的宏定义
* @details      
* @par History  见如下说明
*                 
* version:	由于K210使用fpioa现场可编程IO阵列，允许用户将255个内部功能映射到芯片外围的48个自由IO上
*           所以把硬件IO和软件GPIO功能抽出来单独设置，这样更容易理解。
*/
#ifndef _PIN_CONFIG_H_
#define _PIN_CONFIG_H_
/*****************************HEAR-FILE************************************/
#include "fpioa.h"

/*****************************HARDWARE-PIN*********************************/
// 硬件IO口，与原理图对应

#define PIN_ICM_SCL             (9)
#define PIN_ICM_SDA             (10)
#define PIN_ICM_INT             (11)

/*****************************SOICMWARE-GPIO********************************/
// 软件GPIO口，与程序对应
#define ICM_INT_GPIONUM         (2)

/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口
#define FUNC_ICM_INT             (FUNC_GPIOHS0 + ICM_INT_GPIONUM)
#define FUNC_ICM_SCL             (FUNC_I2C0_SCLK)
#define FUNC_ICM_SDA             (FUNC_I2C0_SDA)

#endif /* _PIN_CONFIG_H_ */
