#include "status_led.h"

#include <Arduino.h>

#include "config.h"

namespace {

StatusLed::State g_state = StatusLed::State::kBooting;

void write(bool on) {
#if LED_ACTIVE_LOW
  digitalWrite(PIN_LED, on ? LOW : HIGH);
#else
  digitalWrite(PIN_LED, on ? HIGH : LOW);
#endif
}

}  // namespace

namespace StatusLed {

void begin() {
  pinMode(PIN_LED, OUTPUT);
  write(false);
}

void set(State state) {
  g_state = state;
}

void update() {
  const uint32_t t = millis();

  switch (g_state) {
    case State::kBooting:
      write(true);
      break;
    case State::kSensorError:
      write((t % 200) < 100);
      break;
    case State::kAdvertising:
      write((t % 1600) < 80);
      break;
    case State::kConnected:
      write(false);
      break;
  }
}

void off() {
  write(false);
}

}  // namespace StatusLed
