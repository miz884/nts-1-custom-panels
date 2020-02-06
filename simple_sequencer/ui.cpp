#include "ui.h"
#include "sequencer.h"

#define is_pressed(i) (ui_state.sw_pressed & (1U << i))
#define is_long_pressed(i) (ui_state.sw_long_pressed & (1U << i))
#define sw_pressed(a, i) (a |= (1U << i))
#define sw_released(a, i) (a &= ~(1U << i)) 

#define UI_SCAN_INTERVAL_US 1000
#define UI_LONG_PRESS_THRESHOLD_US 10000
#define UI_PRESS_THRESHOLD_US 500000

ui_state_t ui_state = {
  .mode = UI_MODE_PLAY,
  .submode = UI_SUBMODE_OSC,
  .sw_pressed = 0x0,
  .sw_long_pressed = 0x0,
  .vr_value = 0x0,
};

typedef struct ui_internal_state {
  uint32_t last_scan_us;
  uint16_t sw_raw_pressed;
  uint32_t sw_last_event_us[sw_count];
  bool vr_updated;
  uint8_t nts1_param_id;
  uint8_t nts1_param_subid;
  uint8_t curr_step;
  uint8_t curr_bank;
} ui_internal_state_t;

ui_internal_state_t ui_internal_state = {
  .last_scan_us = 0x0,
  .sw_raw_pressed = 0x0,
  .sw_last_event_us = { 0U },
  .vr_updated = false,
  .nts1_param_id = 0x0,
  .nts1_param_subid = 0x0,
  .curr_step = 0x0,
  .curr_bank = 0x0,
};

void ui_init() {
  // Setup pins.
  for (uint8_t i = 0; i < sw_count; ++i) {
    pinMode(sw_pins[i], INPUT_PULLUP);
  }
  for (uint8_t i = 0; i < led_count; ++i) {
    pinMode(led_pins[i], OUTPUT);
  }
  for (uint8_t i = 0; i < vr_count; ++i) {
    pinMode(vr_pins[i], INPUT);
  }
  // Init ui_internal_state
  ui_internal_state.last_scan_us = 0x0;
  ui_internal_state.sw_raw_pressed = 0x0;
  for (uint8_t i = 0; i < sw_count; ++i) {
    ui_internal_state.sw_last_event_us[i] = 0U;
  }
  ui_internal_state.vr_updated = false;
  ui_internal_state.nts1_param_id = 0x0,
  ui_internal_state.nts1_param_subid = 0x0,
  ui_internal_state.curr_step = 0x0,
  ui_internal_state.curr_bank = 0x0,
  // Init ui_state
  ui_state.mode = UI_MODE_PLAY;
  ui_state.submode = UI_SUBMODE_OSC;
  ui_state.sw_pressed = 0x0;
  ui_state.sw_long_pressed = 0x0;
  ui_state.vr_value = 0x0;
}

void ui_scan(uint32_t now_us) {
  // Scan switches.
  const uint16_t prev_sw_raw_pressed = ui_internal_state.sw_raw_pressed;
  ui_internal_state.sw_raw_pressed = 0x0;
  for (uint8_t i = 0; i < sw_count; ++i) {
    const uint32_t val = nts1_digital_read(sw_pins[i]);
    if (val > 0) {
      sw_pressed(ui_internal_state.sw_raw_pressed, i);
    }
    // Check L -> H
    if (!(prev_sw_raw_pressed & (1U << i)) && val > 0) {
      ui_internal_state.sw_last_event_us[i] = now_us;
    }
    // Check H -> L
    if ((prev_sw_raw_pressed & (1U << i)) && val == 0) {
      const uint32_t duration = now_us - ui_internal_state.sw_last_event_us[i];
      if (duration > UI_LONG_PRESS_THRESHOLD_US) {
        sw_pressed(ui_state.sw_long_pressed, i);
      }
      if (duration > UI_PRESS_THRESHOLD_US) {
        sw_pressed(ui_state.sw_pressed, i);
      }
    }
  }
  // Scan VRs.
  for (uint8_t i = 0; i < vr_count; ++i) {
    const uint32_t val = analogRead(vr_pins[i]);
    if (val != ui_state.vr_value) {
      ui_state.vr_value = val;
      ui_internal_state.vr_updated = true;
    }
  }
}

void ui_update_leds(uint8_t mask) {
  for (uint8_t i = 0; i < led_count; ++i) {
    nts1_digital_write(led_pins[i], (mask & (1U << i)) ? HIGH : LOW);
  }
}

void ui_handle_mode_change() {
  bool mode_changed = true;
  if (is_pressed(sw0)) {
    ui_state.mode = UI_MODE_PLAY;
  } else if (is_pressed(sw1)) {
    ui_state.mode = UI_MODE_SEQ_EDIT;
  } else if (is_pressed(sw1)) {
    ui_state.mode = UI_MODE_SOUND_EDIT;
  } else {
    mode_changed = false;
  }
  if (mode_changed) {
    ui_state.sw_pressed = 0x0;
    ui_state.sw_long_pressed = 0x0;
  }
}

void ui_handle_play_sw() {
  bool changed = false;
  // long Play (sw9) --> stop seq.
  if (is_long_pressed(sw9)) {
    seq_state.is_playing = false;
    changed = true;
  } else if (is_pressed(sw9)) {
    // Play (sw9) --> start seq.
    seq_state.is_playing = true;    
    seq_state.flags |= SEQ_FLAG_RESET;
    changed = true;
  }
  // Shift (sw8) + sw? --> bank on / off
  if (ui_internal_state.sw_raw_pressed & (1U << sw8)) {
    // Shift (sw8) + pressed --> bank on
    if (ui_state.sw_pressed & 0xFF) {
      seq_config.bank_active |= ui_state.sw_pressed & 0xFF;
      changed = true;
    }
    // Shift (sw8) + long sw? --> bank off
    if (ui_state.sw_long_pressed & 0xFF) {
      seq_config.bank_active &= ~(ui_state.sw_long_pressed & 0xFF);
      changed = true;
    }
  } else if (ui_state.sw_pressed & 0xFF) {
    // sw? --> next bank
    for (int8_t i = 0; i < sw_count; ++i) {
      if (is_pressed(i)) {
        seq_state.next_bank = i;
        seq_config.bank_active |= (1U << i);
        changed = true;
        break;
      }
    }
  }
  if (changed) {
    ui_state.sw_pressed = 0x0;
    ui_state.sw_long_pressed = 0x0;
  }
}

void ui_handle_seq_edit_sw () {
  if (!(ui_state.sw_pressed & 0xFF)) return;
  bool changed = false;
  for (int8_t i = 0; i < sw_count; ++i) {
    if (is_pressed(i)) {
      // Shift (sw8) + sw? --> bank change
      if (ui_internal_state.sw_raw_pressed & (1U << sw8)) {
        ui_internal_state.curr_bank = i;
      } else {
        // sw? --> step change
        ui_internal_state.curr_step = i;
      }
      changed = true;
      break;
    }
  }
  if (changed) {
    ui_state.sw_pressed = 0x0;
    ui_state.sw_long_pressed = 0x0;
  }
}

void ui_handle_sound_edit_sw() {
  if (!(ui_state.sw_pressed & 0xFF)) return;
  uint8_t sw = 0x0;
  for (int8_t i = 0; i < sw_count; ++i) {
    if (is_pressed(i)) {
      sw = i;
      break;
    }
  }
  // Shift (sw8) + sw? --> submode change
  if (ui_internal_state.sw_raw_pressed & (1U << sw8)) {
    ui_state.submode = sw;
  } else {
    // sw? --> param change
    switch(ui_state.submode) {
      case UI_SUBMODE_OSC:
        switch(sw) {
          case sw0:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_OSC_TYPE;
            break;
          case sw1:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_OSC_SHAPE;
            break;
          case sw2:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_OSC_SHIFT_SHAPE;
            break;
          case sw4:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_OSC_LFO_RATE;
            break;
          case sw5:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_OSC_LFO_DEPTH;
            break;
        }
        break;
      case UI_SUBMODE_OSC_USER:
        switch(sw) {
          case sw0:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_OSC_EDIT;
            ui_internal_state.nts1_param_subid = NTS1::PARAM_SUBID_OSC_EDIT1;
            break;
          case sw1:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_OSC_EDIT;
            ui_internal_state.nts1_param_subid = NTS1::PARAM_SUBID_OSC_EDIT2;
            break;
          case sw2:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_OSC_EDIT;
            ui_internal_state.nts1_param_subid = NTS1::PARAM_SUBID_OSC_EDIT3;
            break;
          case sw3:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_OSC_EDIT;
            ui_internal_state.nts1_param_subid = NTS1::PARAM_SUBID_OSC_EDIT4;
            break;
          case sw4:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_OSC_EDIT;
            ui_internal_state.nts1_param_subid = NTS1::PARAM_SUBID_OSC_EDIT5;
            break;
          case sw5:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_OSC_EDIT;
            ui_internal_state.nts1_param_subid = NTS1::PARAM_SUBID_OSC_EDIT6;
            break;
        }
        break;
      case UI_SUBMODE_FILTER:
        switch(sw) {
          case sw0:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_FILT_TYPE;
            break;
          case sw1:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_FILT_CUTOFF;
            break;
          case sw2:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_FILT_PEAK;
            break;
          case sw4:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_FILT_LFO_RATE;
            break;
          case sw5:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_FILT_LFO_DEPTH;
            break;
        }
        break;
      case UI_SUBMODE_EG:
        switch(sw) {
          case sw0:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_AMPEG_TYPE;
            break;
          case sw1:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_AMPEG_ATTACK;
            break;
          case sw2:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_AMPEG_RELEASE;
            break;
          case sw4:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_AMPEG_LFO_RATE;
            break;
          case sw5:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_AMPEG_LFO_DEPTH;
            break;
        }
        break;
      case UI_SUBMODE_MOD:
        switch(sw) {
          case sw0:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_MOD_TYPE;
            break;
          case sw1:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_MOD_TIME;
            break;
          case sw2:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_MOD_DEPTH;
            break;
        }
        break;
      case UI_SUBMODE_DELAY:
        switch(sw) {
          case sw0:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_DEL_TYPE;
            break;
          case sw1:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_DEL_TIME;
            break;
          case sw2:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_DEL_DEPTH;
            break;
          case sw3:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_DEL_MIX;
            break;
        }
        break;
      case UI_SUBMODE_REVERB:
        switch(sw) {
          case sw0:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_REV_TYPE;
            break;
          case sw1:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_REV_TIME;
            break;
          case sw2:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_REV_DEPTH;
            break;
          case sw3:
            ui_internal_state.nts1_param_id = NTS1::PARAM_ID_REV_MIX;
            break;
        }
        break;
    }
  }
  ui_state.sw_pressed = 0x0;
  ui_state.sw_long_pressed = 0x0;
}

void ui_handle_sw() {
  // mode change (sw8 + sw9 + sw?)
  if (is_pressed(sw8) && is_pressed(sw9)) {
    ui_handle_mode_change();
    return;
  }
  // sw for each mode
  switch(ui_state.mode) {
    case UI_MODE_PLAY:
      ui_handle_play_sw();
      break;
    case UI_MODE_SEQ_EDIT:
      ui_handle_seq_edit_sw();
      break;
    case UI_MODE_SOUND_EDIT:
      ui_handle_sound_edit_sw();
      break;
    default:
      break;
  }
}

void ui_handle_vr() {
  const uint32_t val = ui_state.vr_value;
  switch(ui_state.mode) {
    case UI_MODE_PLAY:
      seq_config.tempo = 200 + 3000 * val / 1023;
      break;
    case UI_MODE_SEQ_EDIT:
      seq_config.notes[ui_internal_state.curr_bank][ui_internal_state.curr_step] = (val >> 3);
      break;
    case UI_MODE_SOUND_EDIT:
      uint16_t max_val = 1023U;
      // TODO Adjust the max values for each TYPE.
      switch (ui_internal_state.nts1_param_id) {
        case NTS1::PARAM_ID_OSC_TYPE:
          max_val = 16;
          break;
        case NTS1::PARAM_ID_AMPEG_TYPE:
          max_val = 4;
          break;
        case NTS1::PARAM_ID_MOD_TYPE:
          max_val = 16;
          break;
        case NTS1::PARAM_ID_DEL_TYPE:
          max_val = 16;
          break;
        case NTS1::PARAM_ID_REV_TYPE:
          max_val = 16;
          break;
      }
      if (ui_internal_state.nts1_param_id == NTS1::PARAM_ID_OSC_EDIT) {
        nts1.paramChange(ui_internal_state.nts1_param_id, ui_internal_state.nts1_param_subid, (uint16_t)(val / max_val));
      } else {
        nts1.paramChange(ui_internal_state.nts1_param_id, NTS1::INVALID_PARAM_SUB_ID, val);
      }
      break;
  }
  ui_internal_state.vr_updated = false;
}

void ui_timer_handler(HardwareTimer *timer) {
  uint32_t now_us = micros();
  if (now_us - ui_internal_state.last_scan_us > UI_SCAN_INTERVAL_US) {
    ui_internal_state.last_scan_us = now_us;
    ui_scan(now_us);
    if (ui_state.sw_pressed > 0 || ui_state.sw_long_pressed > 0) {
      ui_handle_sw();
    }
    if (ui_internal_state.vr_updated) {
      ui_handle_vr();
    }
  }
}
