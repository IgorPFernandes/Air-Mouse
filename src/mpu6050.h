#pragma once

#include <stdint.h>

// Leituras ja convertidas para unidades fisicas e com o bias removido.
struct ImuSample {
  float gx, gy, gz;   // velocidade angular, deg/s
  float ax, ay, az;   // aceleracao, g
};

struct GyroBias {
  float x, y, z;      // deg/s
};

namespace Mpu6050 {

bool begin();

// Mede o desvio de zero do giroscopio. Retorna false se a placa se moveu
// durante a medicao - nesse caso o valor obtido nao presta e vale repetir.
bool calibrate(uint16_t samples = 600);

bool read(ImuSample &out);

const GyroBias &bias();

void sleep();

}  // namespace Mpu6050
