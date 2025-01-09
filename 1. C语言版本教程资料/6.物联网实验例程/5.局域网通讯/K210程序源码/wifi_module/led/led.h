#ifndef _LED_H_
#define _LED_H_
#include "stdint.h"

#define LED_ON         0
#define LED_OFF        1
#define LED_0          0
#define LED_1          1
#define LED_ALL        2

void led_init(uint8_t num);
void led0_state(uint8_t state);
void led1_state(uint8_t state);


#endif  /* _LED_H_ */
