#include "scale.h"
#include "sequencer.h"

#define SEQ_MIN_US 60000000UL
#define SEQ_TICKS_PER_STEP 100UL

NTS1 nts1;

seq_state_t seq_state = {
  .last_tick_us = 0x0,
  .ticks = 0x0,
  .bank = 0x0,
  .step = 0x0,
  .flags = 0x0,
  .curr_note = NO_NOTE,
  .adhoc_next_note = NO_NOTE,
  .next_bank = 0xFF,
  .active_transpose = 0,
  .is_playing = false,
};

seq_config_t seq_config = {
  .tempo = 1200,
  .notes = { 0x42 }, // Middle C
  .bank_active = 1U,
  .scale = 0,
  .base_transpose = 0,
};

void seq_reset() {
  // Note off if any pending notes.
  if (is_valid_note(seq_state.curr_note)) {
    nts1.noteOff(seq_state.curr_note);
    seq_state.curr_note = NO_NOTE;
  }
  // Init seq_state.
  seq_state.last_tick_us = 0x0;
  seq_state.ticks = 0x0;
  seq_state.bank = 0x0;
  seq_state.step = 0x0;
  seq_state.adhoc_next_note = NO_NOTE;
  seq_state.next_bank = 0xFF;
  seq_state.active_transpose = 0;
}

void seq_init() {
  seq_reset();
  // Init seq_state.
  seq_state.flags = 0x0;
  seq_state.is_playing = false;
  // Init seq_config.
  seq_config.tempo = 1200;
  for (uint8_t i = 0; i < SEQ_NUM_STEPS * SEQ_NUM_BANKS; ++i) {
    seq_config.notes[i] = 0x42; // Middle C
  }
  seq_config.bank_active = 1U;
  seq_config.scale = 0;
  seq_config.base_transpose = 0;
}

int8_t seq_snap_to_scale(int8_t note) {
  // Find a enabled pitch class on the current scale.
  // Looking at the upper notes.
  uint16_t scale_mask = 0x1 << (note % 12);
  while (!(seq_scales[seq_config.scale] & scale_mask)) {
    ++note;
    scale_mask <<= 1;
    if (scale_mask == (0x1 << 12)) {
      scale_mask = 0x1;
    }
  }
  return is_valid_note(note) ? note : NO_NOTE;
}

uint8_t seq_next_note_index() {
  // Next step.
  ++seq_state.step;
  if (seq_state.step >= SEQ_NUM_STEPS) {
    // Next bank
    seq_state.step = 0;
    if (seq_state.next_bank != 0xFF) {
      // If the next bank is specified explicitly, jump to it.
      seq_state.bank = seq_state.next_bank;
      seq_state.next_bank = 0xFF;
    } else {
      // Otherwise, jump to next active bank.
      const uint8_t prev_bank = seq_state.bank;
      for (uint8_t i = 0; i < SEQ_NUM_BANKS; ++i) {
        if (seq_config.bank_active & (1U << i)) {
          if (i < prev_bank && i < seq_state.bank) {
            // To keep the 1st H bit for the case if the prev is the last H.
            seq_state.bank = i;
          }
          if (i > prev_bank) {
            seq_state.bank = i;
            break;
          }
        }
      }
    }
  }
  return  seq_state.bank * SEQ_NUM_STEPS + seq_state.step;
}

int8_t seq_apply_note_modifiers(int8_t note) {
  if (!is_valid_note(note)) {
    return NO_NOTE;
  }
  // Apply transposes.
  note += seq_state.active_transpose;
  note += seq_config.base_transpose;
  if (!is_valid_note(note)) {
    return NO_NOTE;
  }
  // Snap to scale.
  return seq_snap_to_scale(note);
}

int8_t seq_get_next_note() {
  if (!seq_state.is_playing) {
    return NO_NOTE;
  }
  int8_t note = seq_config.notes[seq_next_note_index()];
  if (is_valid_note(seq_state.adhoc_next_note)) {
    note = seq_state.adhoc_next_note;
    seq_state.adhoc_next_note = NO_NOTE;
  }
  return seq_apply_note_modifiers(note);
}

void seq_timer_handler(HardwareTimer *timer) {
  const uint32_t now_us = micros();
  if (seq_state.flags & SEQ_FLAG_RESET) {
    seq_reset();
    seq_state.last_tick_us = now_us;
    seq_state.flags &= ~SEQ_FLAG_RESET;
  }

  // 60 (sec/min) / (tempo / 10) (step/min) / SEQ_TICKS_PER_STEP (ticks/step)
  // --> 10 * 60 / tempo / SEQ_TICKS_PER_STEP (sec/ticks)
  const uint32_t us_per_tick = 10 * SEQ_MIN_US / (seq_config.tempo * SEQ_TICKS_PER_STEP);
  const uint32_t us_since_last_tick = now_us - seq_state.last_tick_us;
  if (us_since_last_tick >= us_per_tick) {
    // Next tick.
    ++seq_state.ticks;
    seq_state.last_tick_us += us_per_tick;
    if (seq_state.ticks >= SEQ_TICKS_PER_STEP) {
      // Next step
      seq_state.ticks = 0;
      int8_t note = seq_get_next_note();
      // Note on.
      if (is_valid_note(note)) {
        seq_state.curr_note = note;
        nts1.noteOn((uint8_t) note, 0x7F);
      } else {
        seq_state.curr_note = NO_NOTE;
      }
    } else if (seq_state.ticks >= (SEQ_TICKS_PER_STEP >> 1)) {
      // Note off at 1/2 of the step.
      if (is_valid_note(seq_state.curr_note)) {
        nts1.noteOff(seq_state.curr_note);
        seq_state.curr_note = NO_NOTE;
      }
    }
  }
}
