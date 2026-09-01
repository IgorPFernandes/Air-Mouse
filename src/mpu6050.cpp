#include "mpu6050.h"

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

namespace {

constexpr uint8_t kAddr = 0x68;

constexpr uint8_t kRegSmplrtDiv   = 0x19;
constexpr uint8_t kRegConfig      = 0x1A;
constexpr uint8_t kRegGyroConfig  = 0x1B;
constexpr uint8_t kRegAccelConfig = 0x1C;
constexpr uint8_t kRegAccelXoutH  = 0x3B;
constexpr uint8_t kRegPwrMgmt1    = 0x6B;
constexpr uint8_t kRegWhoAmI      = 0x75;

// Fundo de escala: giroscopio +-500 deg/s, acelerometro +-2 g.
constexpr float kGyroLsbPerDps = 65.5f;
constexpr float kAccelLsbPerG  = 16384.0f;

GyroBias g_bias{0.0f, 0.0f, 0.0f};

bool writeReg(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(kAddr);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readRegs(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(kAddr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(static_cast<int>(kAddr), static_cast<int>(len)) != len) return false;
  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

bool readRaw(ImuSample &out) {
  uint8_t b[14];
  if (!readRegs(kRegAccelXoutH, b, sizeof(b))) return false;

  const int16_t ax = static_cast<int16_t>((b[0] << 8) | b[1]);
  const int16_t ay = static_cast<int16_t>((b[2] << 8) | b[3]);
  const int16_t az = static_cast<int16_t>((b[4] << 8) | b[5]);
  // b[6..7] = temperatura, nao usada.
  const int16_t gx = static_cast<int16_t>((b[8] << 8) | b[9]);
  const int16_t gy = static_cast<int16_t>((b[10] << 8) | b[11]);
  const int16_t gz = static_cast<int16_t>((b[12] << 8) | b[13]);

  out.ax = ax / kAccelLsbPerG;
  out.ay = ay / kAccelLsbPerG;
  out.az = az / kAccelLsbPerG;
  out.gx = gx / kGyroLsbPerDps;
  out.gy = gy / kGyroLsbPerDps;
  out.gz = gz / kGyroLsbPerDps;
  return true;
}

}  // namespace

namespace Mpu6050 {

bool begin() {
  uint8_t who = 0;
  if (!readRegs(kRegWhoAmI, &who, 1)) return false;
  if (who != 0x68 && who != 0x69 && who != 0x70) return false;

  if (!writeReg(kRegPwrMgmt1, 0x80)) return false;   // reset
  delay(100);
  if (!writeReg(kRegPwrMgmt1, 0x01)) return false;   // acorda, clock = PLL do eixo X
  delay(20);

  writeReg(kRegConfig, 0x03);        // DLPF ~44 Hz
  writeReg(kRegSmplrtDiv, 0x04);     // 1 kHz / (4 + 1) = 200 Hz
  writeReg(kRegGyroConfig, 0x08);    // +-500 deg/s
  writeReg(kRegAccelConfig, 0x00);   // +-2 g
  delay(20);
  return true;
}

bool calibrate(uint16_t samples) {
  double sx = 0, sy = 0, sz = 0;
  float minMag = INFINITY;
  float maxMag = -INFINITY;

  for (uint16_t i = 0; i < samples; i++) {
    ImuSample s;
    if (!readRaw(s)) return false;

    sx += s.gx;
    sy += s.gy;
    sz += s.gz;

    const float mag = fabsf(s.gx) + fabsf(s.gy) + fabsf(s.gz);
    minMag = fminf(minMag, mag);
    maxMag = fmaxf(maxMag, mag);
    delay(2);
  }

  // Variacao grande demais significa que a placa estava sendo movida: a media
  // nao representa o desvio de zero e usa-la faria o cursor derivar sozinho.
  if (maxMag - minMag > 25.0f) return false;

  g_bias.x = static_cast<float>(sx / samples);
  g_bias.y = static_cast<float>(sy / samples);
  g_bias.z = static_cast<float>(sz / samples);
  return true;
}

bool read(ImuSample &out) {
  if (!readRaw(out)) return false;
  out.gx -= g_bias.x;
  out.gy -= g_bias.y;
  out.gz -= g_bias.z;
  return true;
}

const GyroBias &bias() {
  return g_bias;
}

void sleep() {
  writeReg(kRegPwrMgmt1, 0x40);
}

}  // namespace Mpu6050
