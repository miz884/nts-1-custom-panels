#ifndef _UI_H
#define _UI_H

#include "Arduino.h"

typedef struct ui_state {
  uint16_t mode;
  uint16_t submode;
  uint16_t sw_pressed;
  uint16_t sw_long_pressed;
  uint32_t vr_value;
} ui_state_t;

extern ui_state_t ui_state;

enum {
  UI_MODE_PLAY = 0,
  UI_MODE_SEQ_EDIT,
  UI_MODE_SOUND_EDIT,
};

enum {
  UI_SUBMODE_OSC = 0,
  UI_SUBMODE_OSC_USER,
  UI_SUBMODE_FILTER,
  UI_SUBMODE_EG,
  UI_SUBMODE_MOD,
  UI_SUBMODE_DELAY,
  UI_SUBMODE_REVERB,
};

// Switch definition.
enum {
  sw0 = 0, 
  sw1, 
  sw2,
  sw3,
  sw4,
  sw5,
  sw6,
  sw7,
  sw8,
  sw9,
  sw_count
};

const uint8_t sw_pins[sw_count] = {
  D34,
  D35,
  D36,
  D37,
  D38,
  D40,
  D41,
  D42,
  D46,
  D47
};

// LED definition
enum {
  led0 = 0, 
  led1, 
  led2,
  led3,
  led4,
  led5,
  led6,
  led7,
  led_count
};

const uint8_t led_pins[led_count] = {
  D16,
  D17,
  D18,
  D19,
  D22,
  D23,
  D24,
  D25
};

// VR definition
enum {
  vr0,
  vr_count
};

const uint8_t vr_pins[vr_count] = {
  A10
};

extern void ui_init();

extern void ui_timer_handler(HardwareTimer *timer);

#endif
