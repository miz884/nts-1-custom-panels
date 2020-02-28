This is a 8-step-8-bank step sequncer with full sound edit functionality as same to the original NTS-1 panel.

# 4 modes
This sequencer has the following 4 modes:

- Play mode
- Sequencer edit mode
- Sequencer scale select mode
- Sound edit mode

## Mode changes

It can swich between the 4 modes with the following sw combinations.

- sw9 + sw10 + sw1 --> Play mode
- sw9 + sw10 + sw2 --> Sequencer edit mode
- sw9 + sw10 + sw3 --> Sequencer scale select mode
- sw9 + sw10 + sw4 --> Sound edit mode

# Save and Load configs

It stores the sequencer config on the flash memory which will be kept even after the reset.

- sw9 + sw10 + Long sw7 --> Load the Sequencer config
- sw9 + sw10 + Long sw8 --> Save the Sequencer config

The following configurations will be stored:

- Tempo.
- Notes on the all 64 steps (8 steps x 8 banks).
- Active banks.
- Scale.
- Base transpose.

# Controls

## Play mode

- VR : tempo
- sw9 + VR : active transpose (-64 - 63)
- sw1 - sw8 : Next bank number. After the current bank, it will jump to the designated bank (bank1 - bank8).
- sw9 + (sw1 - sw8) : Bank on.
- sw9 + (Long sw1 - Long sw8) : Bank off.
- sw10 : Start playing.
- Long sw10 : Stop playing.

## Sequencer edit mode

If the sequencer is not playing, the VR notes can be set to the designated steps.

- VR : off, note 0 - 127
- sw1 - sw8 : Step to set the note.
- sw9 + (sw1 - sw8) : Bank to set the note.
- sw10 : Next step.

If the sequencer is playing, the VR note works as an adhoc note. It will sound at the next sequencer step. But they are not stored.

- VR : off, note 0 - 127

## Sequencer scale select mode

- VR : base transpose (-11 - 11)
- sw1 - sw6 : Select scales.

### Scales

- sw1 : Chromatic scale
- sw2 : Major scale
- sw3 : Minor scale
- sw4 : Blue note scale
- sw5 : Gypsy scale
- sw6 : Ryukyu scale

## Sound edit mode

- VR : Set params
- sw9 + sw? : Submode
  - sw9 + sw1 : OSC
  - sw9 + sw2 : Edit parameters for user OSC
  - sw9 + sw3 : Filter
  - sw9 + sw4 : EG
  - sw9 + sw5 : Mod
  - sw9 + sw6 : Delay
  - sw9 + sw7 : Reverb
- sw? : Param types for the current submode. See below.
 
### OSC submode

- sw1 : Oscilator type
- sw2 : Shape
- sw3 : Shift shape
- sw5 : LFO rate
- sw6 : LFO depth

### Edit parameters for user OSC

- sw1 : USR0
- sw2 : USR1
- sw3 : USR2
- sw4 : USR3
- sw5 : USR4
- sw6 : USR5

### Filter

- sw1 : Filter type
- sw2 : Cutoff
- sw3 : Peak
- sw5 : LFO rate
- sw6 : LFO depth

### EG

- sw1 : EG type
- sw2 : Attach
- sw3 : Release
- sw5 : LFO rate
- sw6 : LFO depth

### Mod

- sw1 : Mod type
- sw2 : time
- sw3 : depth

### Delay and Reverb

- sw1 : type
- sw2 : time
- sw3 : depth
- sw4 : mix
