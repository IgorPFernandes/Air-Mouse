#include "rgb_led.h"

#include <Arduino.h>

#include "config.h"

namespace {

RgbLed::State g_state = RgbLed::State::kBooting;
RgbLed::Color g_flashColor = RgbLed::Color::kOff;
uint32_t      g_flashUntil = 0;

void writePin(uint8_t pin, bool on) {
#if RGB_COMMON_ANODE
  digitalWrite(pin, on ? LOW : HIGH);
#else
  digitalWrite(pin, on ? HIGH : LOW);
#endif
}

void show(RgbLed::Color color) {
  bool r = false, g = false, b = false;

  switch (color) {
    case RgbLed::Color::kRed:    r = true; break;
    case RgbLed::Color::kGreen:  g = true; break;
    case RgbLed::Color::kYellow: r = true; g = true; break;
    case RgbLed::Color::kBlue:   b = true; break;
    case RgbLed::Color::kOff:    break;
  }

  writePin(PIN_RGB_R, r);
  writePin(PIN_RGB_G, g);
  writePin(PIN_RGB_B, b);
}

}  // namespace

namespace RgbLed {

void begin() {
  pinMode(PIN_RGB_R, OUTPUT);
  pinMode(PIN_RGB_G, OUTPUT);
  pinMode(PIN_RGB_B, OUTPUT);
  show(Color::kOff);
}

void setState(State state) {
  g_state = state;
}

void flash(Color color) {
  g_flashColor = color;
  g_flashUntil = millis() + RGB_CONFIRM_MS;
}

void update() {
  const uint32_t now = millis();

  // A confirmacao de velocidade tem prioridade: e resposta direta a um clique
  // e precisa aparecer mesmo com a conexao caindo no mesmo instante.
  if (g_flashUntil != 0) {
    if (static_cast<int32_t>(now - g_flashUntil) < 0) {
      show(g_flashColor);
      return;
    }
    g_flashUntil = 0;
  }

  switch (g_state) {
    case State::kBooting:
      show(Color::kBlue);
      break;
    case State::kSensorError:
      show((now % 200) < 100 ? Color::kRed : Color::kOff);
      break;
    case State::kAdvertising:
      show((now % 1600) < 80 ? Color::kBlue : Color::kOff);
      break;
    case State::kConnected:
      show(Color::kOff);
      break;
  }
}

void off() {
  g_flashUntil = 0;
  show(Color::kOff);
}

}  // namespace RgbLed
