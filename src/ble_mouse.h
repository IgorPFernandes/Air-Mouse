#pragma once

#include <stdint.h>

// Mouse HID sobre Bluetooth LE. O host enxerga um mouse padrao de 5 botoes,
// com deslocamento em 16 bits, roda e scroll horizontal.
namespace BleMouse {

enum Button : uint8_t {
  kButtonLeft   = 0x01,
  kButtonRight  = 0x02,
  kButtonMiddle = 0x04,
};

void begin(const char *deviceName, const char *manufacturer);

bool connected();

void sendReport(uint8_t buttons, int16_t dx, int16_t dy, int8_t wheel, int8_t pan);

void setBatteryLevel(uint8_t percent);

// Alterna o slave latency da conexao. Com latencia alta o radio deixa de
// acordar a cada evento de conexao quando nao ha nada a enviar, sem perder a
// capacidade de responder no primeiro evento seguinte quando houver.
// Chamar so na transicao de estado: renegociar parametros custa tempo de radio.
void setLowLatency(bool active);

// Reduz o intervalo de anuncio depois da janela inicial de busca.
void slowDownAdvertising();

// Tempo desde o ultimo evento de conexao ou inicio do anuncio.
uint32_t millisSinceConnected();

}  // namespace BleMouse
