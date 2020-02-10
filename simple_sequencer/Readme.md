This is a 8-step-8-bank step sequncer with full sound edit functionality as same to the original NTS-1 panel.

# 3 modes
This sequencer has the following 3 modes:

- Play mode
- Sequencer edit mode
- Sound edit mode

## Mode changes

We can swich between the 3 modes with the following sw combinations.

- sw8 + sw9 + sw0 --> Play mode
- sw8 + sw9 + sw1 --> Sequencer edit mode
- sw8 + sw9 + sw2 --> Sound edit mode

# Controls

## Play mode

- VR : tempo
- sw8 + VR : transpose
- sw0 - sw7 : Next bank number. After the current bank, it will jump to the designated bank (bank0 - bank7).
- sw8 + sw0 - sw8 + sw7 : Bank on.
- sw8 + Long sw0 - sw8 + Long sw7 : Bank off.
- sw9 : Start playing.
- Long sw9 : Stop playing.

## Sequencer edit mode

- VR : note (1 - 127) (0 means no sound)
- sw0 - sw7 : Step to set the note.
- sw8 + sw0 - sw8 + sw7 : Bank to set the note.

## Sound edit mode

- VR : Set params
- sw8 + sw? : Submode
  - sw8 + sw0 : OSC
  - sw8 + sw1 : Edit parameters for user OSC
  - sw8 + sw2 : Filter
  - sw8 + sw3 : EG
  - sw8 + sw4 : Mod
  - sw8 + sw5 : Delay
  - sw8 + sw6 : Reverb
- sw? : Param types for the current submode. See below.
 
### OSC submode

- sw0 : Oscilator type
- sw1 : Shape
- sw2 : Shift shape
- sw4 : LFO rate
- sw5 : LFO depth

### Edit parameters for user OSC

- sw0 : USR0
- sw1 : USR1
- sw2 : USR2
- sw3 : USR3
- sw4 : USR4
- sw5 : USR5

### Filter

- sw0 : Filter type
- sw1 : Cutoff
- sw2 : Peak
- sw4 : LFO rate
- sw5 : LFO depth

### EG

- sw0 : EG type
- sw1 : Attach
- sw2 : Release
- sw4 : LFO rate
- sw5 : LFO depth

### Mod

- sw0 : Mod type
- sw1 : time
- sw2 : depth

### Delay and Reverb

- sw0 : type
- sw1 : time
- sw2 : depth
- sw3 : mix
