#include "ble_mouse.h"

#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>

#include <string>

namespace {

// Mouse de 5 botoes. X e Y em 16 bits: em 8 bits o deslocamento satura em 127
// e movimentos rapidos ficam truncados, o que aparece como um cursor que
// "engasga" quando se vira a mao depressa.
const uint8_t kReportMap[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (Button 1)
    0x29, 0x05,        //     Usage Maximum (Button 5)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x05,        //     Report Count (5)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data, Variable, Absolute)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x03,        //     Report Size (3)
    0x81, 0x03,        //     Input (Constant) - completa o byte dos botoes
    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x16, 0x01, 0x80,  //     Logical Minimum (-32767)
    0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x06,        //     Input (Data, Variable, Relative)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x06,        //     Input (Data, Variable, Relative)
    0x05, 0x0C,        //     Usage Page (Consumer)
    0x0A, 0x38, 0x02,  //     Usage (AC Pan)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x06,        //     Input (Data, Variable, Relative)
    0xC0,              //   End Collection
    0xC0               // End Collection
};

NimBLEHIDDevice      *g_hid       = nullptr;
NimBLECharacteristic *g_input     = nullptr;
volatile bool         g_connected = false;

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *) override {
    g_connected = true;
  }

  void onDisconnect(NimBLEServer *) override {
    g_connected = false;
    NimBLEDevice::startAdvertising();
  }
};

}  // namespace

namespace BleMouse {

void begin(const char *deviceName, const char *manufacturer) {
  NimBLEDevice::init(deviceName);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  // Bonding habilitado para o host reconectar sozinho apos o primeiro pareamento.
  NimBLEDevice::setSecurityAuth(true, false, true);

  NimBLEServer *server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  g_hid = new NimBLEHIDDevice(server);
  g_input = g_hid->inputReport(1);

  // Sobrecarga com std::string de proposito: um const char* passado para
  // setValue() casa com o template generico e grava o ponteiro, nao o texto.
  g_hid->manufacturer(std::string(manufacturer));
  g_hid->pnp(0x02, 0xE502, 0xA111, 0x0210);
  g_hid->hidInfo(0x00, 0x01);
  g_hid->reportMap(const_cast<uint8_t *>(kReportMap), sizeof(kReportMap));
  g_hid->setBatteryLevel(100);
  g_hid->startServices();

  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
  advertising->setAppearance(HID_MOUSE);
  advertising->addServiceUUID(g_hid->hidService()->getUUID());
  advertising->setScanResponse(true);
  advertising->start();
}

bool connected() {
  return g_connected;
}

void sendReport(uint8_t buttons, int16_t dx, int16_t dy, int8_t wheel, int8_t pan) {
  if (!g_connected || g_input == nullptr) return;

  const uint8_t report[7] = {
      buttons,
      static_cast<uint8_t>(dx & 0xFF),
      static_cast<uint8_t>((dx >> 8) & 0xFF),
      static_cast<uint8_t>(dy & 0xFF),
      static_cast<uint8_t>((dy >> 8) & 0xFF),
      static_cast<uint8_t>(wheel),
      static_cast<uint8_t>(pan),
  };

  g_input->setValue(report, sizeof(report));
  g_input->notify();
}

void setBatteryLevel(uint8_t percent) {
  if (g_hid != nullptr) g_hid->setBatteryLevel(percent);
}

}  // namespace BleMouse
