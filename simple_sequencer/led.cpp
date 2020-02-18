#include "led.h"
#include "ui.h"
#include "sequencer.h"

#define LED_BLINK_US 100000

void led_raw_onoff(uint8_t led_on) {
  for (uint8_t i = 0; i < led_count; ++i) {
    nts1_digital_write(led_pins[i], (led_on & (1U << i)) ? HIGH : LOW);
  }
}

void led_update_internal(const uint32_t now_us, uint8_t led_enabled, uint8_t led_blink) {
  // ex.
  // led_enabled : 0011 0011
  // led_blink   : 0101 0101
  // Pass through led_enabled if and only if led_blink is off.
  // [a]         : 0010 0010
  // If led_blink is on, switch on / off based on now_us.
  // [b]         : 0101 0101 or 0000 0000
  // Then, [a] | [b]
  //             : 0111 0111 or 0010 0010
  const uint8_t blink_toggle = ((now_us / LED_BLINK_US) % 2 == 0) ? 0U : ~0U;
  led_raw_onoff((led_enabled & ~led_blink) | (led_blink & blink_toggle));
}

void led_update(const uint32_t now_us) {
  switch(ui_state.mode) {
  case UI_MODE_PLAY:
    if (is_raw_pressed(sw8)) {
      led_update_internal(now_us, seq_config.bank_active, 0U);
    } else {
      led_update_internal(now_us, (1U << seq_state.bank), (1U << seq_state.step));
    }
    break;
  case UI_MODE_SEQ_EDIT:
    led_update_internal(now_us, (1U << ui_state.curr_bank), (1U << ui_state.curr_step));
    break;
  case UI_MODE_SOUND_EDIT:
    led_update_internal(now_us, (1U << ui_state.submode), (1U << ui_state.nts1_params_index));
    break;
  }
}
