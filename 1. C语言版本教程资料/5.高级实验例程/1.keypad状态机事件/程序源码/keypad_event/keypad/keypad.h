#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#include "stdio.h"

#define KEY_FIFO_SIZE	    50             // 按键FIFO缓冲大小
#define RELEASE             0              // 按键松开
#define PRESS               1              // 按键按下
#define KEY_STATUS          4              // 按键事件，松开、短按、长按、重复

/* 修改按键触发时间 */
#define KEY_TICKS           1              // 按键扫描周期（ms）,scan_keypad()函数在哪个固定扫描周期中该值就等于多少 原则上应该是10的公约数中的值 因为按键消抖是10ms
#define KEY_FILTER_TIME     10             // 按键消抖时间
#define KEY_LONG_TIME       1000           // 长按触发时间(ms)
#define KEY_REPEAT_TIME     200            // 连发间隔(ms),连发个数(/s)=1000/KEY_REPEAT_TIME

#define USE_STATE_MACHINE   1              // 状态机检测方式

/* 上报事件标志 */
#define KEY_REPORT_DOWN     (1<<0)         // 上报按键按下事件
#define KEY_REPORT_UP       (1<<1)         // 上报按键抬起事件
#define KEY_REPORT_LONG     (1<<2)         // 上报长按事件
#define KEY_REPORT_REPEAT   (1<<3)         // 上报连发事件

/* 定义按键事件*/
typedef enum _keypad_status_t
{
    EN_KEY_NONE = 0,   //表示无按键事件
    
    // 向左按键事件
    EN_KEY_LEFT_DOWN,
    EN_KEY_LEFT_UP,
    EN_KEY_LEFT_LONG,
    EN_KEY_LEFT_REPEAT,

    // 向右按键事件
    EN_KEY_RIGHT_DOWN,
    EN_KEY_RIGHT_UP,
    EN_KEY_RIGHT_LONG,
    EN_KEY_RIGHT_REPEAT,

    // 中间按键事件
    EN_KEY_MIDDLE_DOWN,
    EN_KEY_MIDDLE_UP,
    EN_KEY_MIDDLE_LONG,
    EN_KEY_MIDDLE_REPEAT,

}keypad_status_t;

/* 按键状态机 */
typedef enum _key_state_t
{
    EN_KEY_NULL = 0,    // 表示无按键按下
    EN_KEY_DOWN,
    EN_KEY_DOWN_RECHECK,
    EN_KEY_UP,
    EN_KEY_UP_RECHECK,
    EN_LONG,
    EN_REPEAT
}key_state_t;

/* 按键ID */
typedef enum _key_id_t
{
    EN_KEY_ID_LEFT = 0,
    EN_KEY_ID_RIGHT ,
    EN_KEY_ID_MIDDLE ,
    EN_KEY_ID_MAX 
}key_id_t;

// 按键结构体
typedef struct _keypad_t
{
    /* 函数指针*/
    uint8_t (*get_key_status)(key_id_t key_id); //按键按下的判断函数,1表示按下
    void (*short_key_down)(void * skd_arg);     //按键短按下回调函数
    void * skd_arg;                             //按键短按下回调函数传入的参数
    void(*short_key_up)(void * sku_arg);        //按键短按抬起回调函数
    void * sku_arg;                             //按键短按下回调函数传入的参数
    void (*long_key_down)(void *lkd_arg);       //长按事件回调函数
    void *lkd_arg;                              //长按事件回调函数参数
    void (*repeat_key_down)(void *rkd_arg);     //连发事件回调
    void *rkd_arg;                              //连发事件回调函数参数
    
    uint8_t  count;			    /* 滤波器计数器 */
    uint16_t long_count;		/* 长按计数器 */
    uint16_t long_time;		    /* 按键按下持续时间, 0表示不检测长按 */
    uint8_t  state;			    /* 按键当前状态（按下还是弹起） */
    uint8_t  repeat_speed;	    /* 连续按键周期 */
    uint8_t  repeat_count;	    /* 连续按键计数器 */

    uint8_t report_flag;        /* 上报事件标志*/
    
    key_state_t key_state ;      /* 按键状态机*/
    uint8_t prev_key_state;      /* 上一次按键的状态 */
}keypad_t;
extern keypad_t keypad[EN_KEY_ID_MAX];

/* FIFO结构体 */
typedef struct _keypad_fifo_t
{
    keypad_status_t fifo_buffer[KEY_FIFO_SIZE];    /* 键值缓冲区 */
    uint8_t read;					               /* 缓冲区读指针1 */
    uint8_t write;
} keypad_fifo_t;


void keypad_init(void);
void scan_keypad(void);
keypad_status_t get_keypad_state(void);

#endif  /* _KEYPAD_H_ */
