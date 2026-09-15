#include "power.h"

#include <Arduino.h>
#include <esp_pm.h>
#include <esp_sleep.h>

#include "config.h"
#include "mpu6050.h"
#include "status_led.h"

namespace {

bool g_lightSleep = false;

// O botao e o pino de interrupcao do sensor precisam estar na faixa que o
// ESP32-C3 consegue usar para acordar do sono profundo. Verificar em tempo de
// compilacao evita produzir um aparelho que simplesmente nunca acorda.
static_assert(PIN_BTN_LEFT <= 5, "PIN_BTN_LEFT precisa estar entre GPIO 0 e GPIO 5");
#if WAKE_ON_MOTION_ENABLED
static_assert(PIN_MPU_INT <= 5, "PIN_MPU_INT precisa estar entre GPIO 0 e GPIO 5");
#endif

}  // namespace

namespace Power {

void begin() {
  setCpuFrequencyMhz(CPU_FREQ_MHZ);

#if LIGHT_SLEEP_ENABLED
  // O light sleep automatico depende de o core ter sido compilado com power
  // management. Nas distribuicoes em que nao esta, esp_pm_configure devolve
  // ESP_ERR_NOT_SUPPORTED e o firmware segue sem ele, so que consumindo mais.
  esp_pm_config_esp32c3_t pm = {};
  pm.max_freq_mhz = CPU_FREQ_MHZ;
  pm.min_freq_mhz = 10;
  pm.light_sleep_enable = true;

  g_lightSleep = esp_pm_configure(&pm) == ESP_OK;
#endif
}

bool lightSleepActive() {
  return g_lightSleep;
}

bool wokeFromMotion() {
#if WAKE_ON_MOTION_ENABLED
  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_GPIO) return false;
  // A mascara indica quais pinos estavam ativos no momento do despertar.
  return (esp_sleep_get_gpio_wakeup_status() & (1ULL << PIN_MPU_INT)) != 0;
#else
  return false;
#endif
}

void deepSleep() {
  StatusLed::off();

  uint64_t wakeMask = 1ULL << PIN_BTN_LEFT;

#if WAKE_ON_MOTION_ENABLED
  // Com o sensor em deteccao de movimento, pegar o aparelho ja o acorda, sem
  // precisar apertar nada. O custo em repouso e de cerca de 20 uA.
  if (Mpu6050::enterMotionDetect(WAKE_ON_MOTION_THRESHOLD)) {
    pinMode(PIN_MPU_INT, INPUT_PULLUP);
    wakeMask |= 1ULL << PIN_MPU_INT;
  } else {
    Mpu6050::sleep();
  }
#else
  Mpu6050::sleep();
#endif

  esp_deep_sleep_enable_gpio_wakeup(wakeMask, ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_deep_sleep_start();
}

}  // namespace Power
