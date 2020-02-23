#include "led.h"
#include "sequencer.h"
#include "scale.h"
#include "ui.h"

#define LED_DUTY_INV 10
#define LED_DIM_DUTY_INV 5
#define LED_BLINK_US 100000

led_state_t led_state = {
  .global_duty_cycle = 0,
  .dim_duty_cycle = 0,
};

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
  if (led_state.global_duty_cycle > 0) {
    led_update_raw(0U);
  } else {
    const uint8_t blink_toggle = ((now_us / LED_BLINK_US) % 2 == 0) ? 0U : ~0U;
    uint8_t led_mask = (led_on & ~led_blink) | (led_blink & blink_toggle);
    const uint8_t dim_toggle = (led_state.dim_duty_cycle > 0) ? 0U : ~0U;
    led_mask = led_mask | (~led_mask & led_dim & dim_toggle);
    led_update_raw(led_mask);
  }
}

void led_update(const uint32_t now_us) {
  ++led_state.global_duty_cycle;
  led_state.global_duty_cycle %= LED_DUTY_INV;
  ++led_state.dim_duty_cycle;
  led_state.dim_duty_cycle %= (LED_DUTY_INV * LED_DIM_DUTY_INV);
  switch(ui_state.mode) {
  case UI_MODE_PLAY:
    if (is_raw_pressed(sw8)) {
      // Shift (sw8) + sw? --> bank on / off
      // Active banks --> ON
      // No dim
      // No blinks
      led_update_internal(now_us, seq_config.bank_active, 0x0, 0x0);
    } else {
      // Current bank --> Blink
      // Active banks --> Dim
      // Current step --> ON
      led_update_internal(now_us, (1U << seq_state.step),
        seq_config.bank_active, (1U << seq_state.bank));
    }
    break;
  case UI_MODE_SEQ_EDIT:
    // Current bank --> ON
    // Current step --> Blink
    // No dim.
    led_update_internal(now_us, (1U << ui_state.curr_bank),
      0x0, (1U << ui_state.curr_step));
    break;
  case UI_MODE_SCALE_EDIT:
    // Current scale --> ON
    // Dim for available scales.
    led_update_internal(now_us, (1U << seq_config.scale),
      AVAILABLE_SCALE_MASK, 0x0);
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
        ui_submode_leds[ui_state.submode], (1U << ui_state.params_index));
    }
    break;
  }
}
