#include "battery.h"

#include <Arduino.h>

#include "config.h"

namespace {

constexpr uint8_t kSamples = 8;

// Pontos da curva de descarga de uma celula LiPo: tensao -> percentual.
// A regiao de 3,70 V a 3,85 V concentra a maior parte da capacidade util, por
// isso os degraus sao mais finos ali.
struct CurvePoint {
  float   volts;
  uint8_t percent;
};

constexpr CurvePoint kCurve[] = {
    {4.15f, 100}, {3.85f, 80}, {3.70f, 50}, {3.55f, 20}, {3.30f, 0},
};

}  // namespace

namespace Battery {

void begin() {
#if BATTERY_SENSE_ENABLED
  analogSetPinAttenuation(PIN_BAT_ADC, ADC_11db);
#if BATTERY_GATED
  // Alta impedancia em repouso: sem corrente pelo divisor fora da medicao.
  pinMode(PIN_BAT_GND, INPUT);
#endif
#endif
}

float volts() {
#if !BATTERY_SENSE_ENABLED
  return 4.2f;
#else
#if BATTERY_GATED
  pinMode(PIN_BAT_GND, OUTPUT);
  digitalWrite(PIN_BAT_GND, LOW);
  // O capacitor do divisor precisa carregar antes da primeira amostra valida.
  delay(2);
#endif

  // O ADC do ESP32-C3 e ruidoso o bastante para a leitura unica oscilar
  // varios pontos percentuais entre chamadas.
  uint32_t millivolts = 0;
  for (uint8_t i = 0; i < kSamples; i++) {
    millivolts += analogReadMilliVolts(PIN_BAT_ADC);
  }

#if BATTERY_GATED
  pinMode(PIN_BAT_GND, INPUT);
#endif

  return (millivolts / static_cast<float>(kSamples)) * BAT_DIVIDER_RATIO / 1000.0f;
#endif
}

uint8_t percent() {
#if !BATTERY_SENSE_ENABLED
  return 100;
#else
  const float v = volts();

  if (v >= kCurve[0].volts) return 100;

  for (size_t i = 1; i < sizeof(kCurve) / sizeof(kCurve[0]); i++) {
    const CurvePoint &high = kCurve[i - 1];
    const CurvePoint &low = kCurve[i];
    if (v >= low.volts) {
      const float span = high.volts - low.volts;
      const float ratio = (v - low.volts) / span;
      return static_cast<uint8_t>(low.percent + ratio * (high.percent - low.percent));
    }
  }

  return 0;
#endif
}

}  // namespace Battery
