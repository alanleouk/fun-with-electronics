#include <Arduino.h>
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32.h>

BLEMIDI_CREATE_DEFAULT_INSTANCE()

#define BLE_DEVICE_NAME "Midi BLE"
#define PEDAL_COUNT 4

// Debug Constants
const static bool debugMode = true;
const static bool debugPedals = true;

// Pedals
uint8_t _pedalValuesCurrent[PEDAL_COUNT] = {0, 0, 0, 0};
uint8_t _pedalValuesPrev[PEDAL_COUNT] = {0, 0, 0, 0};
uint8_t _pedalController[PEDAL_COUNT] = {64, 64, 64, 64};

bool isConnected = false;

void setup()
{
  // Communication
  Serial.begin(115200);

  pinMode(2, OUTPUT);
  Serial.println("Program Setup (Started)");

  // BLE
  MIDI.begin();

  BLEMIDI.setHandleConnected([]()
                             {
                               isConnected = true;
                               // digitalWrite(LED_BUILTIN, HIGH);
                             });

  BLEMIDI.setHandleDisconnected([]()
                                {
                                  isConnected = false;
                                  // digitalWrite(LED_BUILTIN, LOW);
                                });

  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity)
                       {
                         // digitalWrite(LED_BUILTIN, LOW);
                       });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity)
                        {
                          // digitalWrite(LED_BUILTIN, HIGH);
                        });

  Serial.println("Program Setup (Complete)");
}

void notifyMeasurement()
{
  for (size_t i = 0; i < PEDAL_COUNT; i++)
  {
    if (_pedalValuesCurrent[i] != _pedalValuesPrev[i])
    {
      MIDI.sendControlChange(_pedalController[i], _pedalValuesCurrent[i], 1);
    }
  }
}

void loop()
{
  if (isConnected)
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
}
