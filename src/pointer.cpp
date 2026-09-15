#include "pointer.h"

#include <math.h>

#include "config.h"

namespace {

constexpr float kRadToDeg = 57.2957795f;
constexpr float kDegToRad = 0.0174532925f;

float g_smoothX = 0.0f;
float g_smoothY = 0.0f;

// Fracao de pixel que sobrou do relatorio anterior. Sem acumular isso, todo
// deslocamento menor que 1 px seria truncado para zero e o cursor ficaria
// imovel em movimentos lentos.
float g_residualX = 0.0f;
float g_residualY = 0.0f;

float g_scrollAccumulator = 0.0f;
float g_rollDeg = 0.0f;
float g_speedDps = 0.0f;

Pointer::Speed g_speed = static_cast<Pointer::Speed>(SPEED_DEFAULT);
float          g_speedScale = SPEED_MEDIUM_SCALE;

float scaleFor(Pointer::Speed speed) {
  switch (speed) {
    case Pointer::Speed::kSlow: return SPEED_SLOW_SCALE;
    case Pointer::Speed::kFast: return SPEED_FAST_SCALE;
    default:                    return SPEED_MEDIUM_SCALE;
  }
}

// Subtrai o limiar em vez de zerar. Zerar cria um degrau: o cursor salta de
// parado para a velocidade cheia ao cruzar a borda da zona morta.
float applyDeadzone(float v) {
  if (v > GYRO_DEADZONE_DPS) return v - GYRO_DEADZONE_DPS;
  if (v < -GYRO_DEADZONE_DPS) return v + GYRO_DEADZONE_DPS;
  return 0.0f;
}

// Termo linear para precisao no movimento lento, termo quadratico para
// atravessar a tela sem girar o braco inteiro. O nivel de velocidade escala os
// dois termos igualmente, de modo que so o tamanho do gesto muda.
float applyAccelerationCurve(float v) {
  const float magnitude = fabsf(v);
  const float result = (magnitude * SENSITIVITY + magnitude * magnitude * ACCEL_GAIN) *
                       g_speedScale;
  return v < 0.0f ? -result : result;
}

float clampToMaxStep(float v) {
  if (v > MAX_STEP) return MAX_STEP;
  if (v < -MAX_STEP) return -MAX_STEP;
  return v;
}

}  // namespace

namespace Pointer {

void reset() {
  g_smoothX = g_smoothY = 0.0f;
  g_residualX = g_residualY = 0.0f;
  g_scrollAccumulator = 0.0f;
  g_rollDeg = 0.0f;
  g_speedDps = 0.0f;
}

void setSpeed(Speed speed) {
  g_speed = speed;
  g_speedScale = scaleFor(speed);

  // O residuo esta na escala antiga; mante-lo faria o cursor dar um tranco
  // no primeiro quadro apos a troca.
  g_residualX = g_residualY = 0.0f;
}

Speed speed() {
  return g_speed;
}

Speed nextSpeed() {
  switch (g_speed) {
    case Speed::kSlow:   return Speed::kMedium;
    case Speed::kMedium: return Speed::kFast;
    default:             return Speed::kSlow;
  }
}

PointerOutput update(const ImuSample &sample, float dt, bool scrollMode) {
  PointerOutput out{0, 0, 0};

  float x = sample.gz;   // guinada -> horizontal
  float y = sample.gy;   // arfagem -> vertical

#if ROLL_COMPENSATION
  // Filtro complementar: o giroscopio da a variacao rapida do angulo e o
  // acelerometro corrige a deriva de longo prazo usando a gravidade. Girar o
  // vetor pelo angulo resultante mantem os eixos da tela corretos mesmo com o
  // aparelho inclinado de lado.
  const float accelRoll = atan2f(sample.ay, sample.az) * kRadToDeg;
  g_rollDeg = 0.98f * (g_rollDeg + sample.gx * dt) + 0.02f * accelRoll;

  const float c = cosf(g_rollDeg * kDegToRad);
  const float s = sinf(g_rollDeg * kDegToRad);
  const float rotatedX = x * c - y * s;
  const float rotatedY = y * c + x * s;
  x = rotatedX;
  y = rotatedY;
#else
  (void)dt;
#endif

#if SWAP_AXES
  const float swap = x;
  x = y;
  y = swap;
#endif
#if INVERT_X
  x = -x;
#endif
#if INVERT_Y
  y = -y;
#endif

  g_speedDps = fmaxf(fabsf(x), fabsf(y));

  const float dzX = applyDeadzone(x);
  const float dzY = applyDeadzone(y);

  g_smoothX = g_smoothX * SMOOTHING + dzX * (1.0f - SMOOTHING);
  g_smoothY = g_smoothY * SMOOTHING + dzY * (1.0f - SMOOTHING);

  if (scrollMode) {
    float step = g_smoothY / SCROLL_DIVISOR;
#if SCROLL_INVERT
    step = -step;
#endif
    g_scrollAccumulator += step;

    if (g_scrollAccumulator >= 1.0f || g_scrollAccumulator <= -1.0f) {
      const float ticks = truncf(g_scrollAccumulator);
      g_scrollAccumulator -= ticks;
      out.wheel = static_cast<int8_t>(fmaxf(-8.0f, fminf(8.0f, ticks)));
    }

    g_smoothX = 0.0f;
    return out;
  }

  g_scrollAccumulator = 0.0f;

  g_residualX += applyAccelerationCurve(g_smoothX);
  g_residualY += applyAccelerationCurve(g_smoothY);

  const float stepX = truncf(g_residualX);
  const float stepY = truncf(g_residualY);
  g_residualX -= stepX;
  g_residualY -= stepY;

  out.dx = static_cast<int16_t>(clampToMaxStep(stepX));
  out.dy = static_cast<int16_t>(clampToMaxStep(stepY));
  return out;
}

float speedDps() {
  return g_speedDps;
}

}  // namespace Pointer
