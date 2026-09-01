#pragma once

// Ponto unico de configuracao do firmware. Ver docs/AJUSTES.md para o efeito
// pratico de cada parametro.

// ---------------------------------------------------------------------------
// Identificacao
// ---------------------------------------------------------------------------
#define DEVICE_NAME         "Air Mouse"
#define DEVICE_MANUFACTURER "IgorPFernandes"

// ---------------------------------------------------------------------------
// Pinos (ESP32-C3)
//
// GPIO 2, 8 e 9 sao pinos de strapping e GPIO 18 e 19 sao o USB nativo; por
// isso ficam fora, com excecao do GPIO 8, usado so como saida para o LED.
// PIN_BTN_LEFT precisa estar entre GPIO 0 e 5, faixa que acorda o chip do
// sono profundo no ESP32-C3.
// ---------------------------------------------------------------------------
#define PIN_SDA        5
#define PIN_SCL        6

#define PIN_BTN_LEFT   3
#define PIN_BTN_RIGHT  4
#define PIN_BTN_MID   10

#define PIN_BAT_ADC    1
#define PIN_LED        8
#define LED_ACTIVE_LOW 1

// ---------------------------------------------------------------------------
// Bateria
// ---------------------------------------------------------------------------
// Desligue se o divisor resistivo nao estiver montado: o firmware passa a
// reportar 100% em vez de ler o ADC.
#define BATTERY_SENSE_ENABLED 1

// (R1 + R2) / R2. Com dois resistores de 100 k, o valor e 2.0.
#define BAT_DIVIDER_RATIO 2.0f

// ---------------------------------------------------------------------------
// Orientacao
// ---------------------------------------------------------------------------
#define SWAP_AXES         0
#define INVERT_X          1
#define INVERT_Y          1

// Mantem os eixos da tela corretos com o aparelho inclinado de lado.
#define ROLL_COMPENSATION 1

// ---------------------------------------------------------------------------
// Resposta do cursor
// ---------------------------------------------------------------------------
#define REPORT_HZ         125
#define GYRO_DEADZONE_DPS 1.2f     // abaixo disso e tremor de mao
#define SMOOTHING         0.35f    // 0 = sem filtro, 0.9 = bem lento
#define SENSITIVITY       0.085f   // ganho linear, px por deg/s
#define ACCEL_GAIN        0.0016f  // ganho quadratico
#define MAX_STEP          90       // teto de px por relatorio

#define SCROLL_DIVISOR    18.0f    // maior = scroll mais lento
#define SCROLL_INVERT     0

// ---------------------------------------------------------------------------
// Botoes
// ---------------------------------------------------------------------------
#define DEBOUNCE_MS 25

// ---------------------------------------------------------------------------
// Energia
// ---------------------------------------------------------------------------
#define IDLE_SLEEP_MS     (5UL * 60UL * 1000UL)
#define IDLE_MOTION_DPS   6.0f
#define BATTERY_UPDATE_MS 60000UL

// ---------------------------------------------------------------------------
// Depuracao
// ---------------------------------------------------------------------------
#define DEBUG_SERIAL  1
#define DEBUG_PLOT_MS 0   // > 0 imprime dx, dy e bateria a cada N ms
