#ifndef _SIMPLE_SEQUENCER_H
#define _SIMPLE_SEQUENCER_H

#define PRESCALER_FACTOR 1

#define _SERIAL_DEBUG

#define _ON_NTS1

// Set one of the followngs:
// 0: MIDI
// 1: NTS1 SPI
// 2: MIDI + NTS1 SPI
#define _SEQ_DEVICE 2

#if _SEQ_DEVICE == 0
  #define _USE_MIDI
  #undef  _USE_NTS1_SPI
#elif _SEQ_DEVICE == 1
  #undef  _USE_MIDI
  #define _USE_NTS1_SPI
#elif _SEQ_DEVICE == 2
  #define _USE_MIDI
  #define _USE_NTS1_SPI
#else
  #undef  _USE_MIDI
  #define _USE_NTS1_SPI
#endif

#endif
