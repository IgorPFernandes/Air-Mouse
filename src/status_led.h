#pragma once

namespace StatusLed {

enum class State {
  kBooting,
  kAdvertising,
  kConnected,
  kSensorError,
};

void begin();

void set(State state);

// Chamar a cada iteracao do loop: a piscada e gerada sem bloquear.
void update();

void off();

}  // namespace StatusLed
