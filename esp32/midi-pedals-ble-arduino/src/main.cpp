#include "Arduino.h"
#include <ArduinoBLE.h>

#define BLE_DEVICE_NAME "Midi BLE"
#define MIDI_SERVICE_UUID "03B80E5A-EDE8-4B33-A751-6CE34EC4C700"
#define MIDI_CHARACTERISTIC_UUID "7772E5DB-3868-4112-A1A9-F2669D106BF3"
#define PEDAL_COUNT 4

// Debug Constants
const static bool debugMode = true;
const static bool debugPedals = true;

// Methods
void setupBufferDefault();
void notifyMeasurement();

// Pedals
uint8_t _pedalValuesCurrent[PEDAL_COUNT] = {0, 0, 0, 0};
uint8_t _pedalValuesPrev[PEDAL_COUNT] = {0, 0, 0, 0};
uint8_t _pedalController[PEDAL_COUNT] = {64, 64, 64, 64};

static const uint _midiBufferLength = 5;
unsigned char _midiBuffer[_midiBufferLength];

BLEService _service(MIDI_SERVICE_UUID);
BLECharacteristic _midiCharacteristic(MIDI_CHARACTERISTIC_UUID, BLERead | BLEWriteWithoutResponse | BLENotify, _midiBufferLength);

void setupBufferDefault()
{
  // Midi Buffer
  _midiBuffer[0] = 0;
  _midiBuffer[1] = 0;
  _midiBuffer[2] = 0;
  _midiBuffer[3] = 0;
  _midiBuffer[4] = 0;
}

void notifyMeasurement()
{
  for (size_t i = 0; i < PEDAL_COUNT; i++)
  {
    if (_pedalValuesCurrent[i] != _pedalValuesPrev[i])
    {
      _pedalValuesPrev[i] = _pedalValuesCurrent[i];

      _midiBuffer[0] = 0x80;
      _midiBuffer[1] = 0x80;
      _midiBuffer[2] = 0x0B;
      _midiBuffer[3] = _pedalController[i];
      _midiBuffer[4] = _pedalValuesCurrent[i];

      _midiCharacteristic.writeValue(_midiBuffer, _midiBufferLength);
    }
  }
}

void setup()
{
  // Buffers
  setupBufferDefault();

  // Communication
  Serial.begin(115200);

  pinMode(2, OUTPUT);
  Serial.println("Program Setup (Started)");

  // BLE
  BLE.begin();
  BLE.setDeviceName(BLE_DEVICE_NAME);
  BLE.setLocalName(BLE_DEVICE_NAME); // TODO
  BLE.setAdvertisedService(_service);
  _service.addCharacteristic(_midiCharacteristic);
  BLE.addService(_service);
  BLE.advertise();

  Serial.println("Program Setup (Complete)");
}

void loop()
{
  BLEDevice central = BLE.central();
  if (central)
  {
    while (central.connected())
    {
      notifyMeasurement();

      // Debug
      if (debugMode)
      {
        if (debugPedals)
        {

          for (size_t i = 0; i < PEDAL_COUNT; i++)
          {
            Serial.printf("Pedal %u: ", i);
            Serial.println(_pedalValuesCurrent[i]);
            _pedalValuesCurrent[i]++;
          }
        }

        delay(2000);
      }

      delay(5);
    }

    delay(1000);
  }
}
