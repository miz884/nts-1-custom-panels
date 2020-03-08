#ifndef _NTS1_WRAPPER_H
#define _NTS1_WRAPPER_H

#include "Arduino.h"

extern void nts1_wrapper_init();

extern void nts1_wrapper_loop();

extern void nts1_wrapper_all_note_off();

extern uint8_t nts1_wrapper_paramChange(uint8_t id, uint8_t subid, uint16_t value);

extern uint8_t nts1_wrapper_noteOn(uint8_t note, uint8_t velo);

extern uint8_t nts1_wrapper_noteOff(uint8_t note);

#endif
