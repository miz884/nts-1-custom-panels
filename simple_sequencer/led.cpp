#include "led.h"
#include "sequencer.h"
#include "ui.h"

#define LED_BLINK_US 100000
#define LED_DIM_US 10000

void led_update_raw(uint8_t led_mask) {
  for (uint8_t i = 0; i < led_count; ++i) {
    nts1_digital_write(led_pins[i], (led_mask & (1U << i)) ? HIGH : LOW);
  }
}

void led_update_internal(const uint32_t now_us, uint8_t led_on, 
                         uint8_t led_dim, uint8_t led_blink) {
  // ex.
  // led_on    : 0000 1111
  // led_dim   : 0011 0011
  // led_blink : 0101 0101
  // Pass through led_on if and only if led_blink is off.
  // [a]         : 0000 1010
  // If led_blink is on, switch on / off based on now_us.
  // [b]         : 0000 0000 or 0101 0101
  // Then, [a] | [b]
  //             : 0000 1010 or 0101 1111
  // For diabled bit, if it is dimmed, blink it quickly.
  //             : 0000 1010 or 0101 1111
  //             : 0011 1011 or 0111 1111
  const uint8_t blink_toggle = ((now_us / LED_BLINK_US) % 2 == 0) ? 0U : ~0U;
  uint8_t led_mask = (led_on & ~led_blink) | (led_blink & blink_toggle);
  const uint8_t dim_toggle =  ((now_us / LED_DIM_US) % 2 == 0) ? 0U : ~0U;
  led_mask = led_mask | (~led_mask & dim_toggle);
  led_update_raw(led_mask);
}

void led_update(const uint32_t now_us) {
  switch(ui_state.mode) {
  case UI_MODE_PLAY:
    if (is_raw_pressed(sw8)) {
      // Shift (sw8) + sw? --> bank on / off
      // Active banks --> ON
      // Available banks (all) --> Dim
      // No blinks
      led_update_internal(now_us, seq_config.bank_active, 0xFF, 0U);
    } else {
      // Current bank --> ON
      // Active banks --> Dim
      // Current step --> Blink
      led_update_internal(now_us, (1U << seq_state.bank),
        seq_config.bank_active, (1U << seq_state.step));
    }
    break;
  case UI_MODE_SEQ_EDIT:
    // Current bank --> ON
    // Current step --> Blink
    // Dim otherwise
    led_update_internal(now_us, (1U << ui_state.curr_bank),
      0xFF, (1U << ui_state.curr_step));
    break;
  case UI_MODE_SOUND_EDIT:
    if (is_raw_pressed(sw8)) {
      // Shift (sw8) + sw? --> submode change
      // Submode --> ON
      // Params --> Blink
      // Dim for available submodes.
      led_update_internal(now_us, (1U << ui_state.submode),
        0x7F, (1U << ui_state.params_index));
    } else {
      // Submode --> ON
      // Params --> Blink
      // Dim for available params.
      led_update_internal(now_us, (1U << ui_state.submode),
        ui_submode_leds[ui_state.params_index], (1U << ui_state.params_index));
    }
    break;
  }
}
