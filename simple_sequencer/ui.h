#ifndef _UI_H
#define _UI_H

#include <nts-1.h>
#include "Arduino.h"

#define is_pressed(i) (ui_state.sw_pressed & (1U << i))
#define is_long_pressed(i) (ui_state.sw_long_pressed & (1U << i))
#define is_raw_pressed(i) (ui_state.sw_raw_pressed & (1U << i))
#define sw_pressed(a, i) (a |= (1U << i))
#define sw_released(a, i) (a &= ~(1U << i))

#define UI_SCAN_INTERVAL_US 5000
#define UI_LONG_PRESS_THRESHOLD_US 300000
#define UI_PRESS_THRESHOLD_US 30000

#define NOTE_DURATION_US 500000 // 500 msec

enum {
  UI_MODE_PLAY = 0,
  UI_MODE_SEQ_EDIT,
  UI_MODE_SCALE_EDIT,
  UI_MODE_SOUND_EDIT,
  ui_mode_count
};

enum {
  UI_SUBMODE_OSC = 0,
  UI_SUBMODE_OSC_USER,
  UI_SUBMODE_FILTER,
  UI_SUBMODE_EG,
  UI_SUBMODE_MOD,
  UI_SUBMODE_DELAY,
  UI_SUBMODE_REVERB,
  UI_SUBMODE_ARPEGGIATOR,
  ui_submode_count
};

// Switch definition.
enum {
  sw0 = 0, sw1, sw2, sw3, sw4, sw5, sw6, sw7, sw8, sw9,
  sw_count
};

const uint8_t sw_pins[sw_count] = {
  D34, D35, D36, D37, D38, D40, D41, D42, D46, D47
};

// LED definition
enum {
  led0 = 0, led1, led2, led3, led4, led5, led6, led7,
  led_count
};

const uint8_t led_pins[led_count] = {
  D16, D17, D18, D19, D22, D23, D24, D25
};

// VR definition
enum {
  vr0,
  vr_count
};

const uint8_t vr_pins[vr_count] = {
  A10
};

const uint8_t nts1_params[ui_submode_count][8] = {{
    // For UI_SUBMODE_OSC
    NTS1::PARAM_ID_OSC_TYPE,
    NTS1::PARAM_ID_OSC_SHAPE,
    NTS1::PARAM_ID_OSC_SHIFT_SHAPE,
    0xFF,
    NTS1::PARAM_ID_OSC_LFO_RATE,
    NTS1::PARAM_ID_OSC_LFO_DEPTH,
    0xFF,
    0xFF,
  }, {
    // For UI_SUBMODE_OSC_USER
    NTS1::PARAM_SUBID_OSC_EDIT1,
    NTS1::PARAM_SUBID_OSC_EDIT2,
    NTS1::PARAM_SUBID_OSC_EDIT3,
    0xFF,
    NTS1::PARAM_SUBID_OSC_EDIT4,
    NTS1::PARAM_SUBID_OSC_EDIT5,
    NTS1::PARAM_SUBID_OSC_EDIT6,
    0xFF,
  }, {
    // For UI_SUBMODE_FILTER
    NTS1::PARAM_ID_FILT_TYPE,
    NTS1::PARAM_ID_FILT_CUTOFF,
    NTS1::PARAM_ID_FILT_PEAK,
    0xFF,
    NTS1::PARAM_ID_FILT_LFO_RATE,
    NTS1::PARAM_ID_FILT_LFO_DEPTH,
    0xFF,
    0xFF,
  }, {
    // For UI_SUBMODE_EG
    NTS1::PARAM_ID_AMPEG_TYPE,
    NTS1::PARAM_ID_AMPEG_ATTACK,
    NTS1::PARAM_ID_AMPEG_RELEASE,
    0xFF,
    NTS1::PARAM_ID_AMPEG_LFO_RATE,
    NTS1::PARAM_ID_AMPEG_LFO_DEPTH,
    0xFF,
    0xFF,
  }, {
    // For UI_SUBMODE_MOD
    NTS1::PARAM_ID_MOD_TYPE,
    NTS1::PARAM_ID_MOD_TIME,
    NTS1::PARAM_ID_MOD_DEPTH,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
  }, {
    // For UI_SUBMODE_DELAY
    NTS1::PARAM_ID_DEL_TYPE,
    NTS1::PARAM_ID_DEL_TIME,
    NTS1::PARAM_ID_DEL_DEPTH,
    NTS1::PARAM_ID_DEL_MIX,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
  }, {
    // For UI_SUBMODE_REVERB
    NTS1::PARAM_ID_REV_TYPE,
    NTS1::PARAM_ID_REV_TIME,
    NTS1::PARAM_ID_REV_DEPTH,
    NTS1::PARAM_ID_REV_MIX,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
  } , {
    // For UI_SUBMODE_ARPEGGIATOR
    NTS1::PARAM_ID_ARP_PATTERN,
    NTS1::PARAM_ID_ARP_INTERVALS,
    NTS1::PARAM_ID_ARP_LENGTH,
    NTS1::PARAM_ID_ARP_STATE,
    NTS1::PARAM_ID_ARP_TEMPO,
    0xFF,
    0xFF,
    0xFF,
  }
};

const uint8_t ui_submode_leds[ui_submode_count] = {
  0x37, 0x77, 0x37, 0x37, 0x07, 0x0f, 0x0f, 0x1f
};

typedef struct ui_state {
  uint32_t last_scan_us;

  uint16_t mode;
  uint16_t submode;
  uint8_t params_index;

  uint16_t sw_pressed;
  uint16_t sw_long_pressed;
  uint16_t sw_raw_pressed;
  uint16_t sw_ignore_next;
  uint32_t sw_last_event_us[sw_count];

  uint32_t vr_value;
  bool vr_updated;

  uint8_t curr_bank;
  uint8_t curr_step;

  uint8_t curr_monitor_note;
  uint8_t next_monitor_note;
  uint32_t note_us;
} ui_state_t;

extern ui_state_t ui_state;

extern void ui_init();

extern void ui_timer_handler(HardwareTimer *timer);

#endif
