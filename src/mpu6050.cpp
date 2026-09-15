#include "mpu6050.h"

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

namespace {

constexpr uint8_t kAddr = 0x68;

constexpr uint8_t kRegSmplrtDiv     = 0x19;
constexpr uint8_t kRegConfig        = 0x1A;
constexpr uint8_t kRegGyroConfig    = 0x1B;
constexpr uint8_t kRegAccelConfig   = 0x1C;
constexpr uint8_t kRegMotThr        = 0x1F;
constexpr uint8_t kRegMotDur        = 0x20;
constexpr uint8_t kRegIntPinCfg     = 0x37;
constexpr uint8_t kRegIntEnable     = 0x38;
constexpr uint8_t kRegAccelXoutH    = 0x3B;
constexpr uint8_t kRegMotDetectCtrl = 0x69;
constexpr uint8_t kRegPwrMgmt1      = 0x6B;
constexpr uint8_t kRegPwrMgmt2      = 0x6C;
constexpr uint8_t kRegWhoAmI        = 0x75;

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

bool configureFullPower() {
  if (!writeReg(kRegPwrMgmt1, 0x01)) return false;   // acorda, clock = PLL do eixo X
  delay(20);

  writeReg(kRegPwrMgmt2, 0x00);      // todos os eixos ativos
  writeReg(kRegConfig, 0x03);        // DLPF ~44 Hz
  writeReg(kRegSmplrtDiv, 0x04);     // 1 kHz / (4 + 1) = 200 Hz
  writeReg(kRegGyroConfig, 0x08);    // +-500 deg/s
  writeReg(kRegAccelConfig, 0x00);   // +-2 g, sem filtro passa-alta
  delay(20);
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

  return configureFullPower();
}

bool enterMotionDetect(uint8_t threshold) {
  // Sequencia de wake-on-motion do MPU6050. A deteccao usa o filtro passa-alta
  // do acelerometro, por isso o DHPF precisa sair de zero antes de armar a
  // interrupcao.
  if (!writeReg(kRegPwrMgmt1, 0x00)) return false;   // acorda, clock interno
  writeReg(kRegPwrMgmt2, 0x00);
  writeReg(kRegAccelConfig, 0x01);                   // +-2 g, DHPF 5 Hz

  writeReg(kRegMotThr, threshold);
  writeReg(kRegMotDur, 0x01);                        // 1 ms acima do limiar
  writeReg(kRegMotDetectCtrl, 0x15);                 // atraso de ligacao do acelerometro

  // INT ativo em nivel baixo e travado ate a leitura, para que o nivel se
  // mantenha enquanto o ESP32 acorda do sono profundo.
  writeReg(kRegIntPinCfg, 0xB0);
  writeReg(kRegIntEnable, 0x40);                     // apenas MOT_EN

  writeReg(kRegPwrMgmt2, 0x47);                      // ciclo a 5 Hz, giroscopio em espera
  return writeReg(kRegPwrMgmt1, 0x28);               // CYCLE = 1, sensor de temperatura desligado
}

bool exitMotionDetect() {
  writeReg(kRegIntEnable, 0x00);
  if (!configureFullPower()) return false;

  // O giroscopio leva cerca de 35 ms para estabilizar apos sair da espera;
  // ler antes disso devolve lixo que entraria no filtro.
  delay(50);

  ImuSample discard;
  for (uint8_t i = 0; i < 10; i++) {
    readRaw(discard);
    delay(2);
  }
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
