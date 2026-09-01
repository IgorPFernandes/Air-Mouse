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

}  // namespace BleMouse
