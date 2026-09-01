// Scanner I2C - use quando o MPU6050 nao responder.
// Grave este sketch sozinho (Arduino IDE) e abra o monitor a 115200.
//
// Esperado: "encontrado em 0x68" (ou 0x69, se o pino AD0 estiver em 3V3).
// Se nao achar nada, o problema e ligacao: confira SDA/SCL, 3V3 e GND.

#include <Wire.h>

#define PIN_SDA 5
#define PIN_SCL 6

void setup() {
  Serial.begin(115200);
  delay(500);
  Wire.begin(PIN_SDA, PIN_SCL, 100000);   // 100 kHz: mais tolerante a fio ruim
  Serial.println("\nScanner I2C");
}

void loop() {
  int achados = 0;

  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Dispositivo encontrado em 0x%02X\n", addr);
      achados++;
    }
  }

  if (achados == 0) {
    Serial.println("Nada encontrado. Confira SDA (GPIO 5), SCL (GPIO 6), 3V3 e GND.");
  }

  delay(3000);
}
