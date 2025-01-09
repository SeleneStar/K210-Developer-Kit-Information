#include "keypad.h"
#include "gpiohs.h"
#include "timer.h"
#include "pin_config.h"

keypad_fifo_t keypad_fifo;         // 按键缓冲FIFO
keypad_t keypad[EN_KEY_ID_MAX];

// 读取按键对应引脚高低电平的值，按下返回低，松开返回高
static gpio_pin_value_t  get_key_value(key_id_t key_id)
{
    gpio_pin_value_t val = GPIO_PV_HIGH;
    switch (key_id)
    {
    case EN_KEY_ID_LEFT:
        val = gpiohs_get_pin(KEYPAD_LEFT_GPIONUM);
        break;
    case EN_KEY_ID_RIGHT:
        val = gpiohs_get_pin(KEYPAD_RIGHT_GPIONUM);
        break;
    case EN_KEY_ID_MIDDLE:
        val = gpiohs_get_pin(KEYPAD_MIDDLE_GPIONUM);
        break;
    default:
        break;
    }
    return val;
}

static uint8_t get_keys_state_hw(key_id_t key_id)
{
    gpio_pin_value_t key_value = get_key_value(key_id);
    
    if (key_value == GPIO_PV_LOW)
    {
        return PRESS;                // 按键被按下
    }
    else
    {
        return RELEASE;              // 按键松开
    }
}


// 将实际某个按键的状态存入FIFO中
static void key_in_fifo(keypad_status_t keypad_status)
{
    keypad_fifo.fifo_buffer[keypad_fifo.write] = keypad_status;
    if (++keypad_fifo.write >= KEY_FIFO_SIZE)
    {
        keypad_fifo.write = 0;
    }
}

/* 从FIFO读取一个按键事件 */
keypad_status_t key_out_fifo(void)
{
    keypad_status_t key_event;
    if (keypad_fifo.read == keypad_fifo.write)
    {
        return EN_KEY_NONE;
    }
    else
    {
        key_event = keypad_fifo.fifo_buffer[keypad_fifo.read];
        if (++keypad_fifo.read >= KEY_FIFO_SIZE)
        {
            keypad_fifo.read = 0;
        }
        return key_event;
    }
}

/* 状态机的方式检测按键,分析按键事件 */
static void detect_key_state(key_id_t key_id)
{
    keypad_t *p_key;
    p_key = &keypad[key_id];               // 指针指向按键事件结构体
    uint8_t current_key_state;             // 当前按键状态
    current_key_state = p_key->get_key_status(key_id);
    switch (p_key->key_state)
    {
    case EN_KEY_NULL:
        // 如果按键被按下
        if (current_key_state == PRESS)
        {
            p_key->key_state = EN_KEY_DOWN;
        }
        break;
    
    case EN_KEY_DOWN:
        // 如果状态还在保持
        if (current_key_state == p_key->prev_key_state)
        {
            p_key->key_state = EN_KEY_DOWN_RECHECK;
            if(p_key->report_flag & KEY_REPORT_DOWN)   //如果定义了按键按下上报功能
            {
                //存入按键按下事件
                key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 1));
            }
            if (p_key->short_key_down)	 //如果注册了回调函数 则执行
            {
                p_key->short_key_down(p_key->skd_arg);
            }
        }
        else
        {
            p_key->key_state = EN_KEY_NULL;
        }
        break;
    
    // 长按、连发和按键松开判断
    case EN_KEY_DOWN_RECHECK:
        //按键还在保持按下状态
        if(current_key_state == p_key->prev_key_state)
        {
            if(p_key->long_time > 0)
            {
                if (p_key->long_count < p_key->long_time)
                {
                    if ((p_key->long_count += KEY_TICKS) >= p_key->long_time)
                    {
                        if(p_key->report_flag & KEY_REPORT_LONG)
                        {
                            // 存入长按事件
                            key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 3));
                        }
                        if(p_key->long_key_down)   // 回调
                        {
                            p_key->long_key_down(p_key->lkd_arg);
                        }
                    }
                }
                else  // 连发
                {
                    if(p_key->repeat_speed > 0)
                    {
                        if ((p_key->repeat_count  += KEY_TICKS) >= p_key->repeat_speed)
                        {
                            p_key->repeat_count = 0;
                            //如果定义的连发上报
                            if(p_key->report_flag & KEY_REPORT_REPEAT)
                            {
                                key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 4));
                            }
                            if(p_key->repeat_key_down)
                            {
                                p_key->repeat_key_down(p_key->rkd_arg);
                            }
                        }
                    }
                }    
            }
        }
        else  // 按键松开
        {
            p_key->key_state = EN_KEY_UP;
        }
        break;
    
    case EN_KEY_UP:
        if (current_key_state == p_key->prev_key_state)
        {
            p_key->key_state = EN_KEY_UP_RECHECK;
            p_key->long_count = 0;  //长按计数清零
            p_key->repeat_count = 0;  //重复发送计数清零
            if(p_key->report_flag & KEY_REPORT_UP)
            {
                // 按键松开
                key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 2));
            }
            if (p_key->short_key_up)
            {
                p_key->short_key_up(p_key->sku_arg);
            }
        }
        else
        {
            p_key->key_state = EN_KEY_DOWN_RECHECK;
        }
        break;

    case EN_KEY_UP_RECHECK:
        if (current_key_state == p_key->prev_key_state)
        {
            p_key->key_state = EN_KEY_NULL;
        }
        else 
        {
            p_key->key_state = EN_KEY_UP;
        }
        break;
    default:
        break;
    }
    p_key->prev_key_state = current_key_state;
}

/* 检测按键，分析按键的对应事件 */
static void detect_key(key_id_t key_id)
{
    keypad_t *p_key;
    p_key = &keypad[key_id];               // 指针指向按键事件结构体
    if (p_key->get_key_status(key_id) == PRESS)  // 如果按键被按下
    {
        if (p_key->count < KEY_FILTER_TIME)
        {
            p_key->count = KEY_FILTER_TIME;
        }
        else if (p_key->count < 2 * KEY_FILTER_TIME)
        {
            p_key->count += KEY_TICKS;  // 滤波、消抖
        }
        else
        {
            if (p_key->state == RELEASE)
            {
                p_key->state = PRESS;
                if (p_key->report_flag & KEY_REPORT_DOWN)
                {
                    // 发送按钮按下的消息，存入按键按下事件
                    key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 1));
                }
                // 回调函数
                if (p_key->short_key_down)
                {
                    p_key->short_key_down(p_key->skd_arg);
                }
            }
            // 长按检测
            if (p_key->long_time > 0)
            {
                if (p_key->long_count < p_key->long_time)
                {
                    // 发送长按事件
                    if ((p_key->long_count += KEY_TICKS) >= p_key->long_time)
                    {
                        if (p_key->report_flag & KEY_REPORT_LONG)
                        {
                            // 长按的键值放入FIFO
                            key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 3));
                        }
                        // 回调函数
                        if (p_key->long_key_down)
                        {
                            p_key->long_key_down(p_key->lkd_arg);
                        }
                    }
                }
                else
                {
                    //如果定义了连发事件
                    if (p_key->repeat_speed > 0)
                    {
                        if ((p_key->repeat_count  += KEY_TICKS) >= p_key->repeat_speed)
                        {
                            p_key->repeat_count = 0;
                            //如果定义的连发上报
                            if(p_key->report_flag & KEY_REPORT_REPEAT)  
                            {
                                /*长按按键后，每隔repeat_speed发送1个按键 */
                                key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 4));
                            }
                            if(p_key->repeat_key_down)
                            {
                                //执行连发回调函数
                                p_key->repeat_key_down(p_key->rkd_arg);
                            }
                        }
                    }
                }
            }
        }
    }
    else    // 按键松开
    {
        if (p_key->count > KEY_FILTER_TIME)
        {
            p_key->count = KEY_FILTER_TIME;
        }
        else if (p_key->count !=0)
        {
            // 按键松开滤波
            p_key->count -= KEY_TICKS;
        }
        else
        {
            // 滤波结束
            if (p_key->state == PRESS)
            {
                p_key->state = RELEASE;

                if (p_key->report_flag & KEY_REPORT_UP)
                {
                    // 存入按键松开信息
                    key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 2));
                }
                if (p_key->short_key_up)
                {
                    p_key->short_key_up(p_key->sku_arg);
                }
            }
        }
        p_key->long_count = 0;   // 长按计数清零
        p_key->repeat_count = 0;    // 重复发送计数清零
    }
}

/* 定时器回调函数，功能是扫描keypad */
static int timer_irq_cb(void * ctx)
{
    scan_keypad();
}

/* 初始化并启动定时器0的通道0，每毫秒中断一次 */
static void mTimer_init(void)
{
    timer_init(TIMER_DEVICE_0);
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1e6);
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, timer_irq_cb, NULL);

    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
}

/* 扫描keypad */
void scan_keypad()
{
    for (uint8_t i = 0; i < EN_KEY_ID_MAX; i++)
    {
        #if USE_STATE_MACHINE
            detect_key_state((key_id_t)i);
        #else
            detect_key((key_id_t)i);
        #endif
    }
}

/* 获取keypad的状态，如果没有事件，默认为0 */
keypad_status_t get_keypad_state(void)
{
    return key_out_fifo();
}

/* 初始化keypad */
void keypad_init(void)
{
    /* 设置keypad三个通道为上拉输入 */
    gpiohs_set_drive_mode(KEYPAD_LEFT_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_MIDDLE_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_RIGHT_GPIONUM, GPIO_DM_INPUT_PULL_UP);

    
    /* FIFO读写指针清零 */
    keypad_fifo.read = 0;
    keypad_fifo.write = 0;
    /* keypad变量初始化 */
    for (int i = 0; i < EN_KEY_ID_MAX; i++)
    {
        keypad[i].long_time = KEY_LONG_TIME;			/* 长按时间 0 表示不检测长按键事件 */
        keypad[i].count = KEY_FILTER_TIME ;		        /* 计数器设置为滤波时间 */
        keypad[i].state = RELEASE;						/* 按键缺省状态，0为未按下 */
        keypad[i].repeat_speed = KEY_REPEAT_TIME;		/* 按键连发的速度，0表示不支持连发 */
        keypad[i].repeat_count = 0;						/* 连发计数器 */

        keypad[i].short_key_down = NULL;                /* 按键按下回调函数*/
        keypad[i].skd_arg = NULL;                       /* 按键按下回调函数参数*/
        keypad[i].short_key_up = NULL;                  /* 按键抬起回调函数*/
        keypad[i].sku_arg = NULL;                       /* 按键抬起回调函数参数*/
        keypad[i].long_key_down = NULL;                 /* 按键长按回调函数*/
        keypad[i].lkd_arg = NULL;                       /* 按键长按回调函数参数*/
		keypad[i].repeat_key_down = NULL;
		keypad[i].rkd_arg = NULL;
        keypad[i].get_key_status = get_keys_state_hw;
        /* 允许上报的按键事件 */
        keypad[i].report_flag = KEY_REPORT_DOWN | KEY_REPORT_UP | KEY_REPORT_LONG | KEY_REPORT_REPEAT ;
    }

    mTimer_init();
}
