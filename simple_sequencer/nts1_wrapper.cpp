#include "nts1_wrapper.h"
#include "simple_sequencer.h"

#ifdef _USE_MIDI
HardwareSerial midi(USART1);
#else
#endif

#include <nts-1.h>
NTS1 nts1;

void nts1_wrapper_all_note_off() {
  for (uint8_t note = 0; note < 128; ++note) {
    nts1_wrapper_noteOff(note);
  }
}

void nts1_wrapper_init() {
#ifdef _ON_NTS1
  nts1.init();
#endif
#ifdef _USE_MIDI
  midi.begin(31250);
#endif
  // Note off on all pitch class.
  nts1_wrapper_all_note_off();
}

void nts1_wrapper_loop() {
#ifdef _ON_NTS1
  nts1.idle();
#endif
#ifdef _USE_MIDI
#endif
}

uint8_t nts1_wrapper_paramChange(uint8_t id, uint8_t subid, uint16_t value) {
#ifdef _USE_MIDI
  uint8_t msg = 0xff;
  switch (id) {
    case NTS1::PARAM_ID_OSC_TYPE:
      msg = 53;
      break;
    case NTS1::PARAM_ID_OSC_SHAPE:
      msg = 54;
      break;
    case NTS1::PARAM_ID_OSC_SHIFT_SHAPE:
      msg = 55;
      break;
    case NTS1::PARAM_ID_OSC_LFO_RATE:
      msg = 24;
      break;
    case NTS1::PARAM_ID_OSC_LFO_DEPTH:
      msg = 26;
      break;
    // For UI_SUBMODE_FILTER
    case NTS1::PARAM_ID_FILT_TYPE:
      msg = 42;
      break;
    case NTS1::PARAM_ID_FILT_CUTOFF:
      msg = 43;
      break;
    case NTS1::PARAM_ID_FILT_PEAK:
      msg = 44;
      break;
    case NTS1::PARAM_ID_FILT_LFO_DEPTH:
      msg = 45;
      break;
    case NTS1::PARAM_ID_FILT_LFO_RATE:
      msg = 46;
      break;
    // For UI_SUBMODE_EG
    case NTS1::PARAM_ID_AMPEG_TYPE:
      msg = 14;
      break;
    case NTS1::PARAM_ID_AMPEG_ATTACK:
      msg = 16;
      break;
    case NTS1::PARAM_ID_AMPEG_RELEASE:
      msg = 19;
      break;
    case NTS1::PARAM_ID_AMPEG_LFO_RATE:
      msg = 20;
      break;
    case NTS1::PARAM_ID_AMPEG_LFO_DEPTH:
      msg = 21;
      break;
    // For UI_SUBMODE_MOD
    case NTS1::PARAM_ID_MOD_TYPE:
      msg = 88;
      break;
    case NTS1::PARAM_ID_MOD_TIME:
      msg = 28;
      break;
    case NTS1::PARAM_ID_MOD_DEPTH:
      msg = 29;
      break;
    // For UI_SUBMODE_DELAY
    case NTS1::PARAM_ID_DEL_TYPE:
      msg = 89;
      break;
    case NTS1::PARAM_ID_DEL_TIME:
      msg = 30;
      break;
    case NTS1::PARAM_ID_DEL_DEPTH:
      msg = 31;
      break;
    case NTS1::PARAM_ID_DEL_MIX:
      msg = 33;
      break;
    // For UI_SUBMODE_REVERB
    case NTS1::PARAM_ID_REV_TYPE:
      msg = 90;
      break;
    case NTS1::PARAM_ID_REV_TIME:
      msg = 34;
      break;
    case NTS1::PARAM_ID_REV_DEPTH:
      msg = 35;
      break;
    case NTS1::PARAM_ID_REV_MIX:
      msg = 36;
      break;
    // Arpeggiator
    case NTS1::PARAM_ID_ARP_PATTERN:
      msg = 117;
      break;
    case NTS1::PARAM_ID_ARP_INTERVALS:
      msg = 118;
      break;
    case NTS1::PARAM_ID_ARP_LENGTH:
      msg = 119;
      break;
    case NTS1::PARAM_ID_ARP_STATE:
    case NTS1::PARAM_ID_ARP_TEMPO:
      // Not supported in MIDI mode.
      return 1;
  }
#ifdef _SERIAL_DEBUG
  Serial.print("param change: ");
  Serial.print(msg);
  Serial.print(" ");
  Serial.println(value);
#endif
  midi.write(0xB0);
  midi.write(msg);
  midi.write(value);
  return 0;
#else
#ifdef _ON_NTS1
  return nts1.paramChange(id, subid, value);
#endif
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
#ifdef _ON_NTS1
  return nts1.noteOn(note, velo);
#endif
#endif
}

uint8_t nts1_wrapper_noteOff(uint8_t note) {
#ifdef _USE_MIDI
  midi.write(0x80);
  midi.write(note);
  midi.write(127);
  return 0;
#else
#ifdef _ON_NTS1
  return nts1.noteOff(note);
#endif
#endif
}
