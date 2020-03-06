#ifndef _SCALE_H
#define _SCALE_H

#include "Arduino.h"

#define NUM_SCALES 6
#define AVAILABLE_SCALE_MASK 0x3F

const uint16_t seq_scales[] = {
  // 1 #1 2 #2 | 3 4 #4 5 | #5 6 #6 7 --> 0000 1111 1111 1111
  // 1 b2 2 b3 | 3 4 b5 5 | b6 6 b7 7 --> 0000 1111 1111 1111
  0x0FFF, // Chromatic scale: 1 #1 2 #2 3 4 #4 5 #5 6 #6 7 --> 0000 1111 1111 1111
  0x0AB5, // Major scale: 1 2 3 4 5 6 7 --> 0000 1010 1011 0101
  0x05AD, // Minor scale: 1 2 b3 4 5 b6 b7 --> 0000 0101 1010 1101
  0x0EFD, // Blue note scale: 1 2 b3 3 4 b5 5 6 b7 7 --> 0000 1110 1111 1101
  0x09B3, // Gypsy scale: 1 b2 3 4 5 b6 7 --> 0000 1001 1011 0011
  0x08B1, // Ryukyu scale: 1 3 4 5 7 --> 0000 1000 1011 0001
};

#endif
