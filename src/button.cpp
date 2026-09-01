#include "button.h"

#include <Arduino.h>

#include "config.h"

Button::Button(uint8_t pin)
    : pin_(pin), state_(false), lastReading_(false), lastChangeMs_(0) {}

void Button::begin() {
  pinMode(pin_, INPUT_PULLUP);
  state_ = false;
  lastReading_ = false;
  lastChangeMs_ = millis();
}

void Button::update() {
  const bool reading = digitalRead(pin_) == LOW;
  const uint32_t now = millis();

  if (reading != lastReading_) {
    lastReading_ = reading;
    lastChangeMs_ = now;
    return;
  }

  if (reading != state_ && (now - lastChangeMs_) >= DEBOUNCE_MS) {
    state_ = reading;
  }
}
