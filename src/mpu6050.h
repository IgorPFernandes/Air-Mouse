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

// Modo de baixo consumo: giroscopio em espera, acelerometro amostrando a 5 Hz
// e o pino INT indo a nivel baixo quando a aceleracao passa do limiar
// (unidades de 32 mg). Cerca de 20 uA contra os 3,9 mA do modo pleno.
bool enterMotionDetect(uint8_t threshold);

// Retorna ao modo pleno. O giroscopio leva cerca de 35 ms para estabilizar,
// intervalo ja aguardado aqui; o bias medido na calibracao e preservado.
bool exitMotionDetect();

void sleep();

}  // namespace Mpu6050
