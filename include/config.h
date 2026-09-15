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
// A SuperMini expoe GPIO 0 a 10, 20 e 21: treze pinos, doze deles usados aqui.
// GPIO 2 e o unico que sobra, deixado livre de proposito por ser pino de
// strapping - se estiver em nivel baixo no reset, a placa nao arranca.
//
// GPIO 20 e 21 sao a UART0, livres porque a serial usa o USB nativo.
// GPIO 9 e o pino BOOT: segura-lo durante o reset entra em modo de gravacao,
// o que e inofensivo mas confunde quem nao espera.
//
// PIN_BTN_LEFT e PIN_MPU_INT precisam estar entre GPIO 0 e 5: e a unica faixa
// que acorda o ESP32-C3 do sono profundo. Verificado em power.cpp.
// ---------------------------------------------------------------------------
#define PIN_SDA         5
#define PIN_SCL         6

#define PIN_BTN_LEFT    3
#define PIN_BTN_RIGHT   4
#define PIN_BTN_MID    10
#define PIN_BTN_CENTER  7   // centraliza o cursor na tela
#define PIN_BTN_SPEED   9   // percorre lento -> medio -> rapido

#define PIN_MPU_INT     0   // INT do MPU6050, desperta por movimento
#define PIN_BAT_ADC     1

// LED RGB de anodo comum: cada catodo vai ao GPIO por um resistor de 220 ohm
// e o anodo comum ao 3V3. Anodo comum de proposito - os pinos ficam em nivel
// alto em repouso, o que mantem GPIO 8 seguro como pino de strapping.
#define PIN_RGB_R       8
#define PIN_RGB_G      20
#define PIN_RGB_B      21
#define RGB_COMMON_ANODE 1

// ---------------------------------------------------------------------------
// Bateria
// ---------------------------------------------------------------------------
#define BATTERY_SENSE_ENABLED 1

// (R1 + R2) / R2. Com dois resistores de 100 k, o valor e 2.0.
#define BAT_DIVIDER_RATIO 2.0f

// O pe do divisor ligado direto ao GND drena cerca de 20 uA continuos. Com uma
// celula pequena isso pesava; com 18650 em paralelo sao 0,3% da capacidade em
// um ano, e o GPIO que fazia o chaveamento vale mais como botao. Para voltar a
// chavear, defina 1 e escolha um pino livre para PIN_BAT_GND.
#define BATTERY_GATED 0

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
// Niveis de velocidade
//
// O botao de velocidade percorre os tres. O multiplicador incide sobre
// SENSITIVITY e ACCEL_GAIN ao mesmo tempo, para que a curva mantenha o mesmo
// formato e so mude de escala.
// ---------------------------------------------------------------------------
#define SPEED_SLOW_SCALE   0.55f   // vermelho
#define SPEED_MEDIUM_SCALE 1.00f   // amarelo, os valores acima como estao
#define SPEED_FAST_SCALE   1.60f   // verde

#define SPEED_DEFAULT 1            // 0 lento, 1 medio, 2 rapido

// ---------------------------------------------------------------------------
// Centralizacao do cursor
//
// O HID reporta deslocamento relativo, entao nao existe "ir para a posicao X".
// O firmware encosta o cursor no canto superior esquerdo, onde o sistema o
// prende, e de la caminha meia tela. Precisa saber o tamanho da tela.
// ---------------------------------------------------------------------------
#define SCREEN_WIDTH   1920
#define SCREEN_HEIGHT  1080

// Passos em que a caminhada ate o centro e dividida. Muitos passos pequenos
// sofrem menos com a aceleracao de ponteiro do sistema do que um salto unico.
#define RECENTER_STEPS 24

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
#define BLE_TX_POWER_DBM 0

// Intervalo de conexao, em unidades de 1,25 ms.
#define CONN_INTERVAL_MIN 6    // 7,5 ms
#define CONN_INTERVAL_MAX 12   // 15 ms

// Slave latency: quantos eventos de conexao o periferico pode ignorar quando
// nao tem nada a enviar.
#define CONN_LATENCY_ACTIVE 0
#define CONN_LATENCY_IDLE   40   // ~480 ms efetivos entre despertares

// Supervision timeout, em unidades de 10 ms.
#define CONN_TIMEOUT 600

#define IDLE_ENTER_MS 4000UL

// Vinte minutos ate o sono profundo, e nao dois. Sair do sono profundo custa
// uma reconexao BLE de 1 a 2 s, o que no meio de uma aula aparece como um
// aparelho que nao responde quando se aponta para a tela.
//
// O estado ocioso custa 2 mA: uma jornada inteira de 8 h sem dormir nenhuma vez
// gasta 16 mAh, meio por cento de uma 18650. Nao vale trocar isso por latencia
// na frente de uma turma. Ver docs/ENERGIA.md.
#define IDLE_SLEEP_MS (20UL * 60UL * 1000UL)
#define ADV_TIMEOUT_MS (180UL * 1000UL)

// Intervalo de anuncio, em unidades de 0,625 ms.
#define ADV_INTERVAL_FAST 32     // 20 ms
#define ADV_INTERVAL_SLOW 1600   // 1 s
#define ADV_FAST_MS 30000UL

#define IDLE_MOTION_DPS 6.0f

#define WAKE_ON_MOTION_ENABLED 1
#define WAKE_ON_MOTION_THRESHOLD 2

// Um LED aceso o tempo todo consome de 2 a 5 mA, mais que o aparelho inteiro
// em repouso. A cor da velocidade aparece so por este tempo apos uma troca ou
// apos conectar, e depois apaga.
#define RGB_CONFIRM_MS 1500

// ---------------------------------------------------------------------------
// Botoes
// ---------------------------------------------------------------------------
#define DEBOUNCE_MS 25

// ---------------------------------------------------------------------------
// Depuracao
// ---------------------------------------------------------------------------
#define DEBUG_SERIAL  1
#define DEBUG_PLOT_MS 0   // > 0 imprime dx, dy e bateria a cada N ms
