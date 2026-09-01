#include "power.h"

#include <Arduino.h>
#include <esp_sleep.h>

#include "config.h"
#include "mpu6050.h"
#include "status_led.h"

namespace Power {

void deepSleepUntilWakeButton() {
  StatusLed::off();
  Mpu6050::sleep();

  // No ESP32-C3 apenas os GPIO 0 a 5 podem acordar o chip do sono profundo,
  // e por isso o botao de despertar precisa estar nessa faixa.
  static_assert(PIN_BTN_LEFT <= 5, "O pino de despertar precisa estar entre GPIO 0 e GPIO 5");

  esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_BTN_LEFT, ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_deep_sleep_start();
}

}  // namespace Power
