#include <Arduino.h>
#include <Wire.h>

#include "battery.h"
#include "ble_mouse.h"
#include "button.h"
#include "config.h"
#include "mpu6050.h"
#include "pointer.h"
#include "power.h"
#include "status_led.h"

namespace {

Button g_left(PIN_BTN_LEFT);
Button g_right(PIN_BTN_RIGHT);
Button g_middle(PIN_BTN_MID);

bool     g_sensorReady = false;
uint32_t g_lastFrameUs = 0;
uint32_t g_lastActivityMs = 0;
uint32_t g_lastBatteryMs = 0;
uint8_t  g_lastButtons = 0;

#if DEBUG_SERIAL
#define LOG(...) Serial.printf(__VA_ARGS__)
#else
#define LOG(...) ((void)0)
#endif

void startSensor() {
  Wire.begin(PIN_SDA, PIN_SCL, 400000);

  if (!Mpu6050::begin()) {
    g_sensorReady = false;
    StatusLed::set(StatusLed::State::kSensorError);
    LOG("MPU6050 nao respondeu. Verifique SDA, SCL, 3V3 e GND.\n");
    return;
  }

  LOG("Calibrando o giroscopio, mantenha o aparelho parado...\n");

  bool calibrated = false;
  for (int attempt = 0; attempt < 3 && !calibrated; attempt++) {
    calibrated = Mpu6050::calibrate();
  }

#if DEBUG_SERIAL
  const GyroBias &bias = Mpu6050::bias();
  LOG(calibrated ? "Calibrado. Bias: %.2f / %.2f / %.2f deg/s\n"
                 : "Calibracao instavel. Bias: %.2f / %.2f / %.2f deg/s\n",
      bias.x, bias.y, bias.z);
#endif

  g_sensorReady = true;
}

uint8_t readButtons() {
  uint8_t buttons = 0;
  if (g_left.pressed()) buttons |= BleMouse::kButtonLeft;
  if (g_right.pressed()) buttons |= BleMouse::kButtonRight;
  return buttons;
}

bool isIdle() {
  return millis() - g_lastActivityMs > IDLE_SLEEP_MS;
}

void refreshBattery() {
  if (millis() - g_lastBatteryMs < BATTERY_UPDATE_MS) return;
  g_lastBatteryMs = millis();
  BleMouse::setBatteryLevel(Battery::percent());
}

}  // namespace

void setup() {
#if DEBUG_SERIAL
  Serial.begin(115200);
  delay(300);
#endif

  StatusLed::begin();
  StatusLed::set(StatusLed::State::kBooting);
  StatusLed::update();

  g_left.begin();
  g_right.begin();
  g_middle.begin();

  Battery::begin();
  Pointer::reset();
  startSensor();

  BleMouse::begin(DEVICE_NAME, DEVICE_MANUFACTURER);

  if (g_sensorReady) StatusLed::set(StatusLed::State::kAdvertising);

  g_lastFrameUs = micros();
  g_lastActivityMs = millis();
  g_lastBatteryMs = millis();

  LOG("Anunciando como \"%s\".\n", DEVICE_NAME);
}

void loop() {
  StatusLed::update();

  g_left.update();
  g_right.update();
  g_middle.update();

  if (g_sensorReady) {
    StatusLed::set(BleMouse::connected() ? StatusLed::State::kConnected
                                         : StatusLed::State::kAdvertising);
  }

  // Cadencia fixa: um dt constante mantem o filtro e a curva de aceleracao
  // previsiveis, independente do tempo gasto no radio.
  const uint32_t periodUs = 1000000UL / REPORT_HZ;
  const uint32_t nowUs = micros();
  if (nowUs - g_lastFrameUs < periodUs) return;

  const float dt = (nowUs - g_lastFrameUs) / 1000000.0f;
  g_lastFrameUs = nowUs;

  ImuSample sample;
  if (!g_sensorReady || !Mpu6050::read(sample)) return;

  const uint8_t buttons = readButtons();
  const PointerOutput motion = Pointer::update(sample, dt, g_middle.pressed());

  if (Pointer::speedDps() > IDLE_MOTION_DPS || buttons != 0 || g_middle.pressed()) {
    g_lastActivityMs = millis();
  }

  const bool hasMotion = motion.dx != 0 || motion.dy != 0 || motion.wheel != 0;
  if (hasMotion || buttons != g_lastButtons) {
    BleMouse::sendReport(buttons, motion.dx, motion.dy, motion.wheel, 0);
    g_lastButtons = buttons;
  }

#if DEBUG_PLOT_MS > 0
  static uint32_t lastPlotMs = 0;
  if (millis() - lastPlotMs > DEBUG_PLOT_MS) {
    lastPlotMs = millis();
    LOG("dx=%d dy=%d wheel=%d bateria=%.2fV (%u%%)\n", motion.dx, motion.dy, motion.wheel,
        Battery::volts(), Battery::percent());
  }
#endif

  refreshBattery();

  if (isIdle()) {
    LOG("Inatividade, entrando em sono profundo.\n");
#if DEBUG_SERIAL
    Serial.flush();
#endif
    Power::deepSleepUntilWakeButton();
  }
}
