#ifndef _SEQUENCER_H
#define _SEQUENCER_H

#include "Arduino.h"
#include "simple_sequencer.h"

#define SEQ_NUM_BANKS 8
#define SEQ_NUM_STEPS 8

#define NO_NOTE -1

#define is_valid_note(n) (0 <= n && n < 128)

enum {
  SEQ_FLAG_RESET = 1U << 0,
};

typedef struct seq_state {
  uint32_t last_tick_us;
  uint8_t ticks;
  uint8_t bank;
  uint8_t step;
  uint8_t flags;
  int8_t curr_note;
  int8_t adhoc_next_note;
  uint8_t next_bank;
  int8_t active_transpose;
  bool is_playing;
} seq_state_t;

extern seq_state_t seq_state;

typedef struct seq_config {
  uint32_t tempo;
  int8_t notes[SEQ_NUM_BANKS * SEQ_NUM_STEPS];
  uint8_t bank_active;
  uint8_t scale;
  int8_t base_transpose;
} seq_config_t;

extern seq_config_t seq_config;

extern void seq_init();

extern void seq_start();

extern void seq_stop();

extern void seq_timer_handler(HardwareTimer *timer);

#endif
