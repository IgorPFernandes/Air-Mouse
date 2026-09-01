#pragma once

#include <stdint.h>

// Botao ligado entre o GPIO e o terra, usando o pull-up interno.
class Button {
 public:
  explicit Button(uint8_t pin);

  void begin();
  void update();

  bool pressed() const { return state_; }

 private:
  uint8_t  pin_;
  bool     state_;
  bool     lastReading_;
  uint32_t lastChangeMs_;
};
