#pragma once

#include <stdint.h>

// LED RGB de anodo comum. Acumula duas funcoes: o estado do aparelho antes de
// conectar e a confirmacao do nivel de velocidade depois.
//
// Conectado e em uso, o LED fica apagado. Um LED aceso consome mais que o
// aparelho inteiro em repouso, entao a cor aparece so por RGB_CONFIRM_MS
// quando ha o que confirmar.
namespace RgbLed {

enum class Color : uint8_t {
  kOff,
  kRed,
  kGreen,
  kYellow,
  kBlue,
};

enum class State {
  kBooting,
  kAdvertising,
  kConnected,
  kSensorError,
};

void begin();

void setState(State state);

// Acende a cor por RGB_CONFIRM_MS e depois apaga. Usado na troca de
// velocidade e ao conectar.
void flash(Color color);

// Chamar a cada iteracao do laco: piscadas e temporizacao sem bloquear.
void update();

void off();

}  // namespace RgbLed
