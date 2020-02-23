#include "led.h"
#include "scale.h"
#include "sequencer.h"
#include "ui.h"

ui_state_t ui_state = {
  .last_scan_us = 0x0,

  .mode = UI_MODE_PLAY,
  .submode = UI_SUBMODE_OSC,
  .params_index = 0x0,

  .sw_pressed = 0x0,
  .sw_long_pressed = 0x0,
  .sw_raw_pressed = 0x0,
  .sw_ignore_next = 0x0,
  .sw_last_event_us = { 0U },

  .vr_value = 0x0,
  .vr_updated = false,

  .curr_bank = 0x0,
  .curr_step = 0x0,
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
  // Init ui_state
  ui_state.last_scan_us = 0x0;

  ui_state.mode = UI_MODE_PLAY;
  ui_state.submode = UI_SUBMODE_OSC;
  ui_state.params_index = 0x0;

  ui_state.sw_pressed = 0x0;
  ui_state.sw_long_pressed = 0x0;
  ui_state.sw_raw_pressed = 0x0;
  ui_state.sw_ignore_next = 0x0;
  for (uint8_t i = 0; i < sw_count; ++i) {
    ui_state.sw_last_event_us[i] = 0U;
  }

  ui_state.vr_value = 0x0;
  ui_state.vr_updated = false;

  ui_state.curr_bank = 0x0;
  ui_state.curr_step = 0x0;
}

void ui_scan(const uint32_t now_us) {
  // Scan switches.
  ui_state.sw_pressed = 0x0;
  ui_state.sw_long_pressed = 0x0;
  const uint16_t prev_sw_raw_pressed = ui_state.sw_raw_pressed;
  ui_state.sw_raw_pressed = 0x0;
  for (uint8_t i = 0; i < sw_count; ++i) {
    const uint32_t val = nts1_digital_read(sw_pins[i]);
    if (val == 0) {
      sw_pressed(ui_state.sw_raw_pressed, i);
    }
    // Check H -> L (push)
    if (!(prev_sw_raw_pressed & (1U << i)) && val == 0) {
      ui_state.sw_last_event_us[i] = now_us;
    }
    // Check L -> H (release)
    if ((prev_sw_raw_pressed & (1U << i)) && val > 0) {
      if (ui_state.sw_ignore_next & (1U << i)) {
        ui_state.sw_ignore_next &= ~(1U << i);
        continue;
      }
      const uint32_t duration = now_us - ui_state.sw_last_event_us[i];
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
      ui_state.vr_updated = true;
    }
  }
}

void ui_handle_mode_change() {
  if (is_pressed(sw0)) {
    ui_state.mode = UI_MODE_PLAY;
  } else if (is_pressed(sw1)) {
    ui_state.mode = UI_MODE_SEQ_EDIT;
  } else if (is_pressed(sw2)) {
    ui_state.mode = UI_MODE_SCALE_EDIT;
  } else if (is_pressed(sw3)) {
    ui_state.mode = UI_MODE_SOUND_EDIT;
  }
}

void ui_handle_play_sw() {
  if (is_long_pressed(sw8) && seq_state.active_transpose != 0) {
    // Reset active transpose when sw8 is released.
    seq_state.active_transpose = 0;
  }
  if (!(ui_state.sw_pressed & 0x3FF)) return;
  // long Play (sw9) --> stop seq.
  if (is_long_pressed(sw9)) {
    seq_state.is_playing = false;
  } else if (is_pressed(sw9)) {
    // Play (sw9) --> start seq.
    if (!seq_state.is_playing) {
      seq_state.flags |= SEQ_FLAG_RESET;
    }
    seq_state.is_playing = true;
  }
  // Shift (sw8) + sw? --> bank on / off
  if (is_raw_pressed(sw8)) {
    ui_state.sw_ignore_next |= (1U << sw8);
    // Shift (sw8) + sw? --> bank on
    seq_config.bank_active |= ui_state.sw_pressed & 0xFF;
    // Shift (sw8) + long sw? --> bank off
    seq_config.bank_active &= ~(ui_state.sw_long_pressed & 0xFF);
  } else {
    // sw? --> next bank
    for (int8_t i = 0; i < sw_count; ++i) {
      if (is_pressed(i)) {
        seq_state.next_bank = i;
        break;
      }
    }
  }
}

void ui_handle_seq_edit_sw () {
  if (!(ui_state.sw_pressed & 0xFF)) return;
  for (int8_t i = 0; i < sw_count; ++i) {
    if (is_pressed(i)) {
      if (is_raw_pressed(sw8)) {
        ui_state.sw_ignore_next |= (1U << sw8);
        // Shift (sw8) + sw? --> bank change
        ui_state.curr_bank = i;
      } else {
        // sw? --> step change
        ui_state.curr_step = i;
      }
      break;
    }
  }
}

void ui_handle_scale_edit_sw () {
  if (!(ui_state.sw_pressed & 0xFF)) return;
  // Select the smallest in pushed switches.
  uint8_t sw = 0x0;
  for (int8_t i = 0; i < sw_count; ++i) {
    if (is_pressed(i)) {
      sw = i;
      break;
    }
  }
  if (sw < NUM_SCALES) {
    seq_config.scale = sw;
  }
}

void ui_handle_sound_edit_sw() {
  if (!(ui_state.sw_pressed & 0xFF)) return;
  // Select the smallest in pushed switches.
  uint8_t sw = 0x0;
  for (int8_t i = 0; i < sw_count; ++i) {
    if (is_pressed(i)) {
      sw = i;
      break;
    }
  }
  // Shift (sw8) + sw? --> submode change
  if (is_raw_pressed(sw8)) {
    ui_state.sw_ignore_next |= (1U << sw8);
    if (sw < ui_submode_count) {
      ui_state.submode = sw;
    }
  } else {
    // sw? --> param change
    if (nts1_params[ui_state.submode][sw] != 0xFF) {
      ui_state.params_index = sw;
    }
  }
}

void ui_handle_sw() {
  // mode change (sw8 + sw9 + sw?)
  if (is_raw_pressed(sw8) && is_raw_pressed(sw9)) {
    ui_state.sw_ignore_next |= (1U << sw8) | (1U << sw9);
    ui_handle_mode_change();
    return;
  }
  // sw for each mode
  switch(ui_state.mode) {
    case UI_MODE_PLAY:
      ui_handle_play_sw();
      return;
    case UI_MODE_SEQ_EDIT:
      ui_handle_seq_edit_sw();
      return;
    case UI_MODE_SCALE_EDIT:
      ui_handle_scale_edit_sw();
      return;
    case UI_MODE_SOUND_EDIT:
      ui_handle_sound_edit_sw();
      return;
  }
}

void ui_handle_vr() {
  const uint32_t val = ui_state.vr_value;
  switch(ui_state.mode) {
    case UI_MODE_PLAY:
      if (is_raw_pressed(sw8)) {
        // -64 - 63
        seq_state.active_transpose = 127 * val / 1023 - 64;
      } else {
        // 20.0 - 320.0
        seq_config.tempo = 200 + 3000 * val / 1023;
      }
      break;
    case UI_MODE_SEQ_EDIT:
      seq_config.notes[ui_state.curr_bank][ui_state.curr_step] = (val >> 3);
      break;
    case UI_MODE_SCALE_EDIT:
      // -11 - 11
      seq_config.base_transpose = 22 * val / 1023 - 11;
      break;
    case UI_MODE_SOUND_EDIT:
      uint16_t max_val = 1023U;
      switch (nts1_params[ui_state.submode][ui_state.params_index]) {
        case 0xFF:
          return;
        case NTS1::PARAM_ID_AMPEG_TYPE:
          max_val = 4;
          break;
        case NTS1::PARAM_ID_OSC_TYPE:
        case NTS1::PARAM_ID_MOD_TYPE:
          // Default 4 + User 16 = 20. 0 = Off
          max_val = 20;
          break;
        case NTS1::PARAM_ID_DEL_TYPE:
        case NTS1::PARAM_ID_REV_TYPE:
          // Default 5 + User 8 = 13. 0 = Off
          max_val = 13;
          break;
      }
      if (ui_state.submode == UI_SUBMODE_OSC_USER) {
        nts1.paramChange(NTS1::PARAM_ID_OSC_EDIT,
          nts1_params[ui_state.submode][ui_state.params_index], val);
      } else {
        nts1.paramChange(nts1_params[ui_state.submode][ui_state.params_index],
          NTS1::INVALID_PARAM_SUB_ID, (uint16_t)(val / max_val));
      }
      break;
  }
  ui_state.vr_updated = false;
}

void ui_timer_handler(HardwareTimer *timer) {
  const uint32_t now_us = micros();
  if (now_us - ui_state.last_scan_us > UI_SCAN_INTERVAL_US) {
    ui_state.last_scan_us = now_us;
    ui_scan(now_us);
    if (ui_state.sw_pressed > 0 || ui_state.sw_long_pressed > 0) {
      ui_handle_sw();
    }
    if (ui_state.vr_updated) {
      ui_handle_vr();
    }
    led_update(now_us);
  }
}
