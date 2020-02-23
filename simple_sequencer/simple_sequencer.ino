#include "simple_sequencer.h"
#include "ui.h"
#include "sequencer.h"

HardwareTimer *setup_timer(uint8_t pin, uint32_t overflw, void (*handler)(HardwareTimer *)) {
  // cf. https://github.com/stm32duino/STM32Examples/blob/master/examples/Peripherals/HardwareTimer/InputCapture/InputCapture.ino
  TIM_TypeDef *Instance = (TIM_TypeDef *)pinmap_peripheral(digitalPinToPinName(pin), PinMap_PWM);
  uint32_t channel = STM_PIN_CHANNEL(pinmap_function(digitalPinToPinName(pin), PinMap_PWM));
  
  HardwareTimer *timer = new HardwareTimer(Instance);
  // timer->setMode(channel, TIMER_INPUT_CAPTURE_RISING, pin);
  timer->setMode(channel, TIMER_OUTPUT_COMPARE);

  timer->setPrescaleFactor(PRESCALER_FACTOR);
  timer->setOverflow(overflw);
  timer->attachInterrupt(channel, handler);
  return timer;
}

void setup() {
  Serial.begin(115200);

  ui_init();
  seq_init();
  nts1.init();

  HardwareTimer *ui_timer = setup_timer(D2, 2000, ui_timer_handler);
  HardwareTimer *seq_timer = setup_timer(D4, 1000, seq_timer_handler);
  ui_timer->resume();
  seq_timer->resume();
}

void loop() {
  nts1.idle();
}
