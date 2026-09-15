#include "recenter.h"

#include "ble_mouse.h"
#include "config.h"

namespace {

// Empurroes contra o canto. O sistema prende o cursor em 0,0, entao o excesso
// e descartado e o ponto de partida fica conhecido. Alguns relatorios seguidos
// cobrem o caso de um deles se perder no radio.
constexpr uint8_t kCornerPushes = 6;
constexpr int16_t kCornerStep = -32000;

enum class Phase : uint8_t { kIdle, kCorner, kWalk };

Phase   g_phase = Phase::kIdle;
uint8_t g_step = 0;

}  // namespace

namespace Recenter {

void start() {
  g_phase = Phase::kCorner;
  g_step = 0;
}

bool running() {
  return g_phase != Phase::kIdle;
}

void cancel() {
  g_phase = Phase::kIdle;
  g_step = 0;
}

bool update() {
  switch (g_phase) {
    case Phase::kIdle:
      return false;

    case Phase::kCorner:
      BleMouse::sendReport(0, kCornerStep, kCornerStep, 0, 0);
      if (++g_step >= kCornerPushes) {
        g_phase = Phase::kWalk;
        g_step = 0;
      }
      return true;

    case Phase::kWalk: {
      // Passos pequenos e espalhados no tempo sofrem menos com a aceleracao de
      // ponteiro do que um salto unico de meia tela.
      const int16_t dx = (SCREEN_WIDTH / 2) / RECENTER_STEPS;
      const int16_t dy = (SCREEN_HEIGHT / 2) / RECENTER_STEPS;

      BleMouse::sendReport(0, dx, dy, 0, 0);

      if (++g_step >= RECENTER_STEPS) {
        g_phase = Phase::kIdle;
        g_step = 0;
        return false;
      }
      return true;
    }
  }

  return false;
}

}  // namespace Recenter
