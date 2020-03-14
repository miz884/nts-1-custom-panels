#include "flash.h"
#include "led.h"
#include "nts1_wrapper.h"
#include "scale.h"
#include "sequencer.h"
#include "simple_sequencer.h"
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

  .curr_monitor_note = NO_NOTE,
  .next_monitor_note = NO_NOTE,
  .note_us = 0,
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

  ui_state.curr_monitor_note = NO_NOTE;
  ui_state.next_monitor_note = NO_NOTE;
  ui_state.note_us = 0;
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
    const int32_t new_val = 0.95 * ui_state.vr_value + 0.05 * val;
    if (new_val != ui_state.vr_value) {
      ui_state.vr_value = new_val;
      ui_state.vr_updated = true;
#ifdef _SERIAL_DEBUG
      Serial.print("vr_updated: ");
      Serial.print(ui_state.vr_value);
      Serial.println("");
#endif
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
  } else if (is_long_pressed(sw6)) {
    load_config();
  } else if (is_long_pressed(sw7)) {
    save_config();
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
    seq_stop();
  } else if (is_pressed(sw9)) {
    // Play (sw9) --> start seq.
    seq_start();
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
    for (int8_t i = 0; i < SEQ_NUM_BANKS; ++i) {
      if (is_pressed(i)) {
        seq_state.next_bank = i;
        break;
      }
    }
  }
}

void ui_handle_seq_edit_sw () {
  if (is_pressed(sw9)) {
    ++ui_state.curr_step;
    if (ui_state.curr_step == SEQ_NUM_STEPS) {
      ui_state.curr_step = 0;
      ++ui_state.curr_bank;
      if (ui_state.curr_bank == SEQ_NUM_BANKS) {
        ui_state.curr_bank = 0;
      }
    }
  }
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
        seq_config.tempo = 20 + 500 * val / 1023;
      }
      break;
    case UI_MODE_SEQ_EDIT:
      ui_state.next_monitor_note = 128 * val / 1023 - 1;
      if (seq_state.is_playing) {
        seq_state.adhoc_next_note = ui_state.next_monitor_note;
      } else {
        seq_config.notes[ui_state.curr_bank * SEQ_NUM_STEPS + ui_state.curr_step] = ui_state.next_monitor_note;
      }
      break;
    case UI_MODE_SCALE_EDIT:
      // -64 - 63
      seq_config.base_transpose = 127 * val / 1023 - 64;
      break;
    case UI_MODE_SOUND_EDIT:
#ifdef _USE_MIDI
      const uint16_t max_val = 128U;
#else
      const uint16_t max_val = 1024U;
#endif
      if (nts1_params[ui_state.submode][ui_state.params_index] == 0xFF) {
        return;
      }
      // As for NTS1::*_TYPE parameters, we don't need to adjust their range.
      // Their value range will be 0-127 regardless of the number of types.
      // It divides the range based on the number of types internally.
      // e.g. If there are 4 types, 0-31 is assigned to 0, 32-63 to 1,
      // ..., and 96-127 is assigned to 3.
      if (ui_state.submode == UI_SUBMODE_OSC_USER) {
        nts1_wrapper_paramChange(NTS1::PARAM_ID_OSC_EDIT,
          nts1_params[ui_state.submode][ui_state.params_index], val);
      } else {
        nts1_wrapper_paramChange(nts1_params[ui_state.submode][ui_state.params_index],
          NTS1::INVALID_PARAM_SUBID, (uint16_t)(max_val * val / 1023U));
      }
      break;
  }
  ui_state.vr_updated = false;
}

void ui_monitor_note(uint32_t now_us) {
  // Note off on previous note if the previous note was valid,
  // AND there will be one of the followings:
  // - Timeout (NOTE_DURATION_US)
  // - Or the new note is specified.
  if (is_valid_note(ui_state.curr_monitor_note) &&
        ((now_us - ui_state.note_us) > NOTE_DURATION_US || is_valid_note(ui_state.next_monitor_note))) {
    if (!seq_state.is_playing) {
      nts1_wrapper_noteOff(ui_state.curr_monitor_note);
    }
    ui_state.curr_monitor_note = NO_NOTE;
    ui_state.note_us = 0;
  }
  if (is_valid_note(ui_state.next_monitor_note)) {
    if (!seq_state.is_playing) {
      ui_state.curr_monitor_note = ui_state.next_monitor_note;
      ui_state.note_us = now_us;
      nts1_wrapper_noteOn(ui_state.curr_monitor_note, 0x7F);
    }
    ui_state.next_monitor_note = NO_NOTE;
  }
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
    ui_monitor_note(now_us);
  }
  led_update(now_us);
}
