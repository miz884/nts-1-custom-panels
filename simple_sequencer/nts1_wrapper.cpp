#include "nts1_wrapper.h"
#include "simple_sequencer.h"

#ifdef _USE_MIDI
HardwareSerial midi(USART1);
#else
#include <nts-1.h>
NTS1 nts1;
#endif

#ifdef _USE_MIDI
void nts1_wrapper_all_note_off() {
  for (uint8_t note = 0; note < 128; ++note) {
    midi.write(0x80);
    midi.write(note);
    midi.write(0x00);
  }
}
#endif

void nts1_wrapper_init() {
#ifdef _USE_MIDI
  midi.begin(31250);
  // Note off on all pitch class.
  nts1_wrapper_all_note_off();
#else
#endif
}

void nts1_wrapper_loop() {
#ifdef _USE_MIDI
#else
  nts1.idle();
#endif
}

uint8_t nts1_wrapper_paramChange(uint8_t id, uint8_t subid, uint16_t value) {
#ifdef _USE_MIDI
#else
  return nts1.paramChange(id, subid, value);
#endif
}

uint8_t nts1_wrapper_noteOn(uint8_t note, uint8_t velo) {
#ifdef _SERIAL_DEBUG
  Serial.print("noteOn: ");
  Serial.println(note);
#endif
#ifdef _USE_MIDI
  midi.write(0x90);
  midi.write(note);
  midi.write(velo);
  return 0;
#else
  return nts1.noteOn(note, velo);
#endif
}

uint8_t nts1_wrapper_noteOff(uint8_t note) {
#ifdef _USE_MIDI
  midi.write(0x80);
  midi.write(note);
  midi.write(127);
  return 0;
#else
  return nts1.noteOff(note);
#endif
}
