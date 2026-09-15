#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include "battery.h"
#include "ble_mouse.h"
#include "button.h"
#include "config.h"
#include "mpu6050.h"
#include "pointer.h"
#include "power.h"
#include "recenter.h"
#include "rgb_led.h"

namespace {

// ATIVO: giroscopio a taxa cheia, cadencia de REPORT_HZ_ACTIVE e slave latency
// zero. OCIOSO: giroscopio em espera, sensor so no acelerometro a 5 Hz, slave
// latency alta e conexao mantida; sair custa ~50 ms. O sono profundo e tratado
// por Power::deepSleep().
enum class Mode { kActive, kIdle };

Button g_left(PIN_BTN_LEFT);
Button g_right(PIN_BTN_RIGHT);
Button g_middle(PIN_BTN_MID);
Button g_center(PIN_BTN_CENTER);
Button g_speed(PIN_BTN_SPEED);

// Sobrevive ao sono profundo, que apaga a RAM comum mas preserva a do RTC.
// Assim o nivel de velocidade escolhido continua valendo ao acordar.
RTC_DATA_ATTR uint8_t g_savedSpeed = SPEED_DEFAULT;

Mode     g_mode = Mode::kActive;
bool     g_sensorReady = false;
bool     g_advSlowed = false;
bool     g_wasConnected = false;
bool     g_centerWasPressed = false;
bool     g_speedWasPressed = false;
uint32_t g_lastFrameUs = 0;
uint32_t g_lastActivityMs = 0;
uint32_t g_lastBatteryMs = 0;
uint8_t  g_lastButtons = 0;

#if DEBUG_SERIAL
#define LOG(...) Serial.printf(__VA_ARGS__)
#else
#define LOG(...) ((void)0)
#endif

RgbLed::Color speedColor(Pointer::Speed speed) {
  switch (speed) {
    case Pointer::Speed::kFast: return RgbLed::Color::kGreen;
    case Pointer::Speed::kSlow: return RgbLed::Color::kRed;
    default:                    return RgbLed::Color::kYellow;
  }
}

const char *speedName(Pointer::Speed speed) {
  switch (speed) {
    case Pointer::Speed::kFast: return "rapido (verde)";
    case Pointer::Speed::kSlow: return "lento (vermelho)";
    default:                    return "medio (amarelo)";
  }
}

bool anyButtonPressed() {
  return g_left.pressed() || g_right.pressed() || g_middle.pressed() ||
         g_center.pressed() || g_speed.pressed();
}

void startSensor() {
  Wire.begin(PIN_SDA, PIN_SCL, 400000);

  if (!Mpu6050::begin()) {
    g_sensorReady = false;
    RgbLed::setState(RgbLed::State::kSensorError);
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

void enterIdle() {
  if (g_mode == Mode::kIdle) return;
  g_mode = Mode::kIdle;

  BleMouse::setLowLatency(false);
  Pointer::reset();

#if WAKE_ON_MOTION_ENABLED
  // O giroscopio sozinho custa 3,9 mA, mais que o ESP32 ocioso. Em repouso ele
  // sai de cena e o acelerometro assume a deteccao de movimento.
  if (g_sensorReady) Mpu6050::enterMotionDetect(WAKE_ON_MOTION_THRESHOLD);
#endif

  LOG("Ocioso.\n");
}

void enterActive() {
  if (g_mode == Mode::kActive) return;
  g_mode = Mode::kActive;

#if WAKE_ON_MOTION_ENABLED
  if (g_sensorReady) Mpu6050::exitMotionDetect();
#endif

  Pointer::reset();
  BleMouse::setLowLatency(true);
  g_lastFrameUs = micros();

  LOG("Ativo.\n");
}

// Em repouso o giroscopio esta desligado, entao quem sinaliza movimento e o
// pino INT do sensor.
bool motionDetected() {
#if WAKE_ON_MOTION_ENABLED
  return digitalRead(PIN_MPU_INT) == LOW;
#else
  ImuSample sample;
  if (!Mpu6050::read(sample)) return false;
  return fabsf(sample.gy) > IDLE_MOTION_DPS || fabsf(sample.gz) > IDLE_MOTION_DPS;
#endif
}

// Borda de subida: age no instante do aperto, nao enquanto segurado.
bool justPressed(const Button &button, bool &previous) {
  const bool now = button.pressed();
  const bool edge = now && !previous;
  previous = now;
  return edge;
}

void cycleSpeed() {
  const Pointer::Speed next = Pointer::nextSpeed();
  Pointer::setSpeed(next);
  g_savedSpeed = static_cast<uint8_t>(next);
  RgbLed::flash(speedColor(next));
  LOG("Velocidade: %s\n", speedName(next));
}

void refreshBattery() {
  if (millis() - g_lastBatteryMs < BATTERY_UPDATE_MS) return;
  g_lastBatteryMs = millis();
  BleMouse::setBatteryLevel(Battery::percent());
}

// Cede a CPU ate o proximo quadro. Diferente de um laco de espera, isto deixa
// o FreeRTOS entrar em light sleep no intervalo.
void waitForNextFrame(uint32_t periodUs) {
  const uint32_t elapsedUs = micros() - g_lastFrameUs;
  if (elapsedUs >= periodUs) return;

  const uint32_t remainingMs = (periodUs - elapsedUs) / 1000;
  if (remainingMs > 0) delay(remainingMs);
}

}  // namespace

void setup() {
#if DEBUG_SERIAL
  Serial.begin(115200);
  delay(300);
#endif

  Power::begin();

  RgbLed::begin();
  RgbLed::setState(RgbLed::State::kBooting);
  RgbLed::update();

  g_left.begin();
  g_right.begin();
  g_middle.begin();
  g_center.begin();
  g_speed.begin();

#if WAKE_ON_MOTION_ENABLED
  pinMode(PIN_MPU_INT, INPUT_PULLUP);
#endif

  Battery::begin();
  Pointer::reset();

  if (g_savedSpeed > 2) g_savedSpeed = SPEED_DEFAULT;
  Pointer::setSpeed(static_cast<Pointer::Speed>(g_savedSpeed));

  startSensor();

  BleMouse::begin(DEVICE_NAME, DEVICE_MANUFACTURER);

  if (g_sensorReady) RgbLed::setState(RgbLed::State::kAdvertising);

  g_mode = Mode::kActive;
  g_lastFrameUs = micros();
  g_lastActivityMs = millis();
  g_lastBatteryMs = millis();

  LOG("Anunciando como \"%s\". CPU %d MHz, light sleep %s, TX %d dBm.\n", DEVICE_NAME,
      CPU_FREQ_MHZ, Power::lightSleepActive() ? "ativo" : "indisponivel", BLE_TX_POWER_DBM);
  LOG("Velocidade: %s\n", speedName(Pointer::speed()));

  if (Power::wokeFromMotion()) LOG("Despertou por movimento.\n");
}

void loop() {
  RgbLed::update();

  g_left.update();
  g_right.update();
  g_middle.update();
  g_center.update();
  g_speed.update();

  const bool connected = BleMouse::connected();

  if (g_sensorReady) {
    RgbLed::setState(connected ? RgbLed::State::kConnected : RgbLed::State::kAdvertising);
  }

  // Ao conectar, mostra em que velocidade o aparelho esta - a escolha
  // sobreviveu ao sono profundo e o usuario pode nao lembrar dela.
  if (connected && !g_wasConnected) {
    RgbLed::flash(speedColor(Pointer::speed()));
  }
  g_wasConnected = connected;

  // Sem ninguem conectado, o anuncio rapido so vale durante a janela em que se
  // espera alguem procurando; depois dela o custo nao se paga.
  if (!connected) {
    if (!g_advSlowed && BleMouse::millisSinceConnected() > ADV_FAST_MS) {
      BleMouse::slowDownAdvertising();
      g_advSlowed = true;
      LOG("Anuncio lento.\n");
    }
    if (BleMouse::millisSinceConnected() > ADV_TIMEOUT_MS) {
      LOG("Ninguem conectou, dormindo.\n");
#if DEBUG_SERIAL
      Serial.flush();
#endif
      Power::deepSleep();
    }
  } else {
    g_advSlowed = false;
  }

  const uint32_t periodUs =
      1000000UL / (g_mode == Mode::kActive ? REPORT_HZ_ACTIVE : REPORT_HZ_IDLE);

  waitForNextFrame(periodUs);

  const uint32_t nowUs = micros();
  if (nowUs - g_lastFrameUs < periodUs) return;

  const float dt = (nowUs - g_lastFrameUs) / 1000000.0f;
  g_lastFrameUs = nowUs;

  // Os botoes de velocidade e centralizacao funcionam em qualquer estado.
  if (justPressed(g_speed, g_speedWasPressed)) {
    g_lastActivityMs = millis();
    cycleSpeed();
  }

  if (justPressed(g_center, g_centerWasPressed)) {
    g_lastActivityMs = millis();
    if (g_mode == Mode::kIdle) enterActive();
    Recenter::start();
    LOG("Centralizando.\n");
  }

  // Enquanto a sequencia roda, o giroscopio e ignorado: misturar o movimento
  // da mao com a caminhada ate o centro erraria o alvo.
  if (Recenter::running()) {
    if (!connected) {
      Recenter::cancel();
    } else {
      Recenter::update();
      return;
    }
  }

  if (!g_sensorReady) return;

  const uint8_t buttons =
      (g_left.pressed() ? BleMouse::kButtonLeft : 0) |
      (g_right.pressed() ? BleMouse::kButtonRight : 0);

  if (g_mode == Mode::kIdle) {
    if (motionDetected() || anyButtonPressed()) {
      g_lastActivityMs = millis();
      enterActive();
    } else {
      // Os botoes continuam sendo atendidos, so que na cadencia reduzida.
      if (buttons != g_lastButtons) {
        BleMouse::sendReport(buttons, 0, 0, 0, 0);
        g_lastButtons = buttons;
      }
      refreshBattery();
      if (millis() - g_lastActivityMs > IDLE_SLEEP_MS) {
        LOG("Inatividade prolongada, sono profundo.\n");
#if DEBUG_SERIAL
        Serial.flush();
#endif
        Power::deepSleep();
      }
    }
    return;
  }

  ImuSample sample;
  if (!Mpu6050::read(sample)) return;

  const PointerOutput motion = Pointer::update(sample, dt, g_middle.pressed());

  if (Pointer::speedDps() > IDLE_MOTION_DPS || anyButtonPressed()) {
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

  if (millis() - g_lastActivityMs > IDLE_ENTER_MS) {
    enterIdle();
  }
}
