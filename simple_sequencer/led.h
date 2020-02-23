#ifndef _LED_H
#define _LED_H

#include "Arduino.h"

typedef struct led_state {
  uint8_t global_duty_cycle;
  uint8_t dim_duty_cycle;
} led_state_t;

extern led_state_t led_state;

extern void led_update(uint32_t now_us);

#endif
