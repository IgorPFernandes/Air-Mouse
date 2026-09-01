#pragma once

#include <stdint.h>

#include "mpu6050.h"

struct PointerOutput {
  int16_t dx;
  int16_t dy;
  int8_t  wheel;
};

// Converte velocidade angular em deslocamento de cursor. Concentra todo o
// tratamento de sinal do projeto; ver docs/ARQUITETURA.md para o porque de
// cada etapa e a ordem entre elas.
namespace Pointer {

void reset();

// scrollMode desvia o movimento vertical para a roda em vez do cursor.
PointerOutput update(const ImuSample &sample, float dt, bool scrollMode);

// Velocidade angular do ultimo update, antes da zona morta. Usada para
// decidir se o aparelho esta em uso.
float speedDps();

}  // namespace Pointer
