#ifndef _SEQUENCER_H
#define _SEQUENCER_H

#include <nts-1.h>
#include "Arduino.h"

#define SEQ_NUM_BANKS 8
#define SEQ_NUM_STEPS 8

extern NTS1 nts1;

enum {
  SEQ_FLAG_RESET = 1U << 0,
};

typedef struct seq_state {
  uint32_t last_tick_us;
  uint8_t ticks;
  uint8_t bank;
  uint8_t step;
  uint8_t flags;
  uint8_t curr_note;
  uint8_t next_bank;
  int8_t transpose;
  bool is_playing;
} seq_state_t;

extern seq_state_t seq_state;

typedef struct seq_config {
  uint32_t tempo;
  uint8_t notes[SEQ_NUM_BANKS][SEQ_NUM_STEPS];
  uint8_t bank_active;
  uint16_t scale;
} seq_config_t;

extern seq_config_t seq_config;

extern void seq_init();

extern void seq_timer_handler(HardwareTimer *timer);

#endif
