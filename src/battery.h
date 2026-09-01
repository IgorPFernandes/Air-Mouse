#pragma once

#include <stdint.h>

namespace Battery {

void begin();

float volts();

// 0 a 100, por curva aproximada de celula LiPo sob carga leve.
uint8_t percent();

}  // namespace Battery
