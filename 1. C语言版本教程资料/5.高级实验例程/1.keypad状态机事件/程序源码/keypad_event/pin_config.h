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
#define PIN_KEYPAD_LEFT       (1)
#define PIN_KEYPAD_MIDDLE     (2)
#define PIN_KEYPAD_RIGHT      (3)

#define PIN_RGB_R             (6)
#define PIN_RGB_G             (7)
#define PIN_RGB_B             (8)

/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应
#define KEYPAD_LEFT_GPIONUM    (1)
#define KEYPAD_MIDDLE_GPIONUM  (2)
#define KEYPAD_RIGHT_GPIONUM   (3)

#define RGB_R_GPIONUM          (4)
#define RGB_G_GPIONUM          (5)
#define RGB_B_GPIONUM          (6)

/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口
#define FUNC_KEYPAD_LEFT       (FUNC_GPIOHS0 + KEYPAD_LEFT_GPIONUM)
#define FUNC_KEYPAD_MIDDLE     (FUNC_GPIOHS0 + KEYPAD_MIDDLE_GPIONUM)
#define FUNC_KEYPAD_RIGHT      (FUNC_GPIOHS0 + KEYPAD_RIGHT_GPIONUM)

#define FUNC_RGB_R             (FUNC_GPIOHS0 + RGB_R_GPIONUM)
#define FUNC_RGB_G             (FUNC_GPIOHS0 + RGB_G_GPIONUM)
#define FUNC_RGB_B             (FUNC_GPIOHS0 + RGB_B_GPIONUM)

#endif /* _PIN_CONFIG_H_ */
