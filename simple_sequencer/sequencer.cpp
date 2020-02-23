#include "scale.h"
#include "sequencer.h"

#define SEQ_MIN_US 6000000UL
#define SEQ_TICKS_PER_STEP 100UL

NTS1 nts1;

seq_state_t seq_state = {
  .last_tick_us = 0x0,
  .ticks = 0x0,
  .bank = 0x0,
  .step = 0x0,
  .flags = 0x0,
  .curr_note = 0x0,
  .next_bank = 0xFF,
  .active_transpose = 0,
  .is_playing = false
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
  if (seq_state.curr_note > 0x0) {
    nts1.noteOff(seq_state.curr_note);
    seq_state.curr_note = 0x0;
  }
  // Init seq_state.
  seq_state.last_tick_us = 0x0;
  seq_state.ticks = 0x0;
  seq_state.bank = 0x0;
  seq_state.step = 0x0;
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
  for (uint8_t i = 0; i < SEQ_NUM_BANKS; ++i) {
    for (uint8_t j = 0; j < SEQ_NUM_STEPS; ++j) {
      seq_config.notes[j][i] = 0x42; // Middle C
    }
  }
  seq_config.bank_active = 1U;
  seq_config.scale = 0;
  seq_config.base_transpose = 0;
}

uint8_t snap_to_scale(uint8_t note) {
  // Find a enabled pitch class on the current scale.
  // Looking at the upper notes.
  uint16_t scale_mask = 0x1 << (note % 12);
  while (!(seq_scales[seq_config.scale] & scale_mask)) {
    ++note;
    scale_mask <<= 1;
    if (scale_mask == 0x1000) {
      scale_mask = 0x1;
    }
  }
  return note;
}

void seq_timer_handler(HardwareTimer *timer) {
  const uint32_t now_us = micros();
  if (seq_state.flags & SEQ_FLAG_RESET) {
    seq_reset();
    seq_state.last_tick_us = now_us;
    seq_state.flags &= ~SEQ_FLAG_RESET;
  }

  if (!seq_state.is_playing) {
    return;
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
      // Note on.
      uint8_t note = seq_config.notes[seq_state.bank][seq_state.step];
      note += seq_state.active_transpose;
      note += seq_config.base_transpose;
      seq_state.curr_note = snap_to_scale(note);
      if (seq_state.curr_note > 0x0) {
        nts1.noteOn(seq_state.curr_note, 0x7F);
      }
    } else if (seq_state.ticks >= (SEQ_TICKS_PER_STEP >> 1)) {
      // Note off at 1/2 of the step.
      if (seq_state.curr_note > 0x0) {
        nts1.noteOff(seq_state.curr_note);
        seq_state.curr_note = 0x0;
      }
    }
  }
}
