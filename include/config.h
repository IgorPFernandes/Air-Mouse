#pragma once

// Ponto unico de configuracao do firmware. Ver docs/AJUSTES.md para o efeito
// pratico de cada parametro e docs/ENERGIA.md para o orcamento de consumo.

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
//
// PIN_BTN_LEFT e PIN_MPU_INT precisam estar entre GPIO 0 e 5: e a unica faixa
// que acorda o ESP32-C3 do sono profundo. Verificado em power.cpp.
// ---------------------------------------------------------------------------
#define PIN_SDA        5
#define PIN_SCL        6

#define PIN_BTN_LEFT   3
#define PIN_BTN_RIGHT  4
#define PIN_BTN_MID   10

#define PIN_MPU_INT    0   // INT do MPU6050, desperta por movimento
#define PIN_BAT_ADC    1
#define PIN_BAT_GND    7   // terra chaveado do divisor, ver BATTERY_GATED
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

// Com 1, o pe do divisor vai para PIN_BAT_GND em vez do terra, e o pino so vai
// a nivel baixo durante a medicao. Um divisor de 100 k + 100 k ligado
// permanentemente drena cerca de 20 uA, o que sozinho superaria o consumo do
// aparelho dormindo. Com 0, ligue o divisor direto no GND.
#define BATTERY_GATED 1

#define BATTERY_UPDATE_MS 120000UL

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
#define REPORT_HZ_ACTIVE  125
#define REPORT_HZ_IDLE    50       // cadencia enquanto nao ha movimento

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
//
// Tres estados: ATIVO (em uso), OCIOSO (parado, conexao mantida) e SONO
// PROFUNDO. Ver docs/ENERGIA.md para o consumo de cada um.
// ---------------------------------------------------------------------------

// 80 MHz e o minimo com o radio BLE estavel no ESP32-C3, e corta cerca de
// metade da corrente de CPU em relacao aos 160 MHz padrao.
#define CPU_FREQ_MHZ 80

// Light sleep automatico entre iteracoes do laco. Exige suporte a power
// management no core; o firmware detecta a ausencia e segue sem ele.
#define LIGHT_SLEEP_ENABLED 1

// Potencia de transmissao, em dBm: -12, -9, -6, -3, 0, 3, 6 ou 9.
// Para uso em mesa, 0 dBm cobre a sala inteira. Cada degrau acima custa
// corrente nas rajadas de transmissao sem ganho pratico de alcance.
#define BLE_TX_POWER_DBM 0

// Intervalo de conexao, em unidades de 1,25 ms.
#define CONN_INTERVAL_MIN 6    // 7,5 ms
#define CONN_INTERVAL_MAX 12   // 15 ms

// Slave latency: quantos eventos de conexao o periferico pode ignorar quando
// nao tem nada a enviar. E o maior ganho de energia do projeto com a conexao
// ativa, porque o radio deixa de acordar a cada 7,5 ms sem deixar de responder
// imediatamente quando ha movimento.
#define CONN_LATENCY_ACTIVE 0
#define CONN_LATENCY_IDLE   40   // ~480 ms efetivos entre despertares

// Supervision timeout, em unidades de 10 ms. Precisa ser maior que
// intervalo * (latency + 1); 6 s da folga confortavel.
#define CONN_TIMEOUT 600

// Tempo parado ate reduzir cadencia, subir a latencia e por o giroscopio em
// espera. Sair desse estado custa cerca de 50 ms e nao derruba a conexao.
#define IDLE_ENTER_MS 4000UL

// Tempo parado ate o sono profundo. Sair custa uma reconexao BLE (1 a 2 s).
#define IDLE_SLEEP_MS (120UL * 1000UL)

// Tempo anunciando sem ninguem conectar ate dormir.
#define ADV_TIMEOUT_MS (180UL * 1000UL)

// Intervalo de anuncio, em unidades de 0,625 ms.
#define ADV_INTERVAL_FAST 32     // 20 ms, nos primeiros ADV_FAST_MS
#define ADV_INTERVAL_SLOW 1600   // 1 s, depois disso
#define ADV_FAST_MS 30000UL

// Velocidade angular acima da qual o aparelho e considerado em uso.
#define IDLE_MOTION_DPS 6.0f

// Despertar por movimento: o MPU6050 fica em modo de baixo consumo, so com o
// acelerometro, e puxa PIN_MPU_INT a nivel baixo ao detectar movimento.
// Exige o pino INT do modulo ligado. Com 0, so o botao desperta.
#define WAKE_ON_MOTION_ENABLED 1

// Limiar de movimento, em unidades de 32 mg. 2 = 64 mg, sensivel o bastante
// para reagir ao aparelho ser pego e alto o bastante para ignorar vibracao
// de mesa.
#define WAKE_ON_MOTION_THRESHOLD 2

// ---------------------------------------------------------------------------
// Depuracao
//
// A serial USB mantem o CDC ativo e custa corrente. Desligue para medir
// consumo de verdade ou para uso normal com bateria.
// ---------------------------------------------------------------------------
#define DEBUG_SERIAL  1
#define DEBUG_PLOT_MS 0   // > 0 imprime dx, dy e bateria a cada N ms
