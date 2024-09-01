#include <Arduino.h>
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32.h>

BLEMIDI_CREATE_DEFAULT_INSTANCE()

#define BLE_DEVICE_NAME "Midi BLE"
#define PEDAL_COUNT 2
#define MIDI_MIN 0
#define MIDI_MAX 127
#define MAX_ANALOG_VALUE 4095
#define CHANGE_MARGIN 2

struct PedalInfo
{
  int gpioPin;
  uint16_t rawValue;
  uint16_t rawValueMin;
  uint16_t rawValueMax;
  uint8_t currentValue;
  uint8_t previousValue;
  uint8_t controlChangeNumber;
  bool invert;
};

// Debug Constants
const static bool debugMode = false;
const static bool debugPedals = true;

// Pedals
PedalInfo _pedals[PEDAL_COUNT] = {
    {
      gpioPin : 34,
      controlChangeNumber : 64,
      invert : false
    },
    {
      gpioPin : 35,
      controlChangeNumber : 65,
      invert : false
    },
};

bool isConnected = false;

// Methods
void reset();
void process();

void setup()
{
  Serial.println("Program Setup (Started)");

  // Analog
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  // Communication
  Serial.begin(115200);

  // Reset
  reset();

  // BLE
  MIDI.begin();

  BLEMIDI.setHandleConnected(
      []()
      {
        isConnected = true;
        // digitalWrite(LED_BUILTIN, HIGH);
      });

  BLEMIDI.setHandleDisconnected(
      []()
      {
        isConnected = false;
        // digitalWrite(LED_BUILTIN, LOW);
      });

  MIDI.setHandleNoteOn(
      [](byte channel, byte note, byte velocity)
      {
        // digitalWrite(LED_BUILTIN, LOW);
      });
  MIDI.setHandleNoteOff(
      [](byte channel, byte note, byte velocity)
      {
        // digitalWrite(LED_BUILTIN, HIGH);
      });

  Serial.println("Program Setup (Complete)");
}

void loop()
{
  if (isConnected)
  {
    // Process
    process();

    // Debug
    if (debugMode)
    {
      delay(2000);
    }
  }

  vTaskDelay(500 / (portTICK_PERIOD_MS * 1000)); // Nanoseconds
}

void reset()
{
  for (size_t i = 0; i < PEDAL_COUNT; i++)
  {
    PedalInfo *pedal = &_pedals[i];
    pedal->rawValue = 0;
    pedal->rawValueMin = (MAX_ANALOG_VALUE / 2) - 1;
    pedal->rawValueMax = (MAX_ANALOG_VALUE / 2) + 1;
    pedal->currentValue = MIDI_MIN;
    pedal->previousValue = MIDI_MIN;
  }
}

void process()
{
  // uint16_t newResistanceValue = floor(analogRead(resistancePin) * RESISTANCE_FACTOR);

  for (size_t i = 0; i < PEDAL_COUNT; i++)
  {
    PedalInfo *pedal = &_pedals[i];

    // Raw Value
    pedal->rawValue = analogRead(pedal->gpioPin);

    // Calibration
    if (pedal->rawValueMin > pedal->rawValue)
    {
      pedal->rawValueMin = pedal->rawValue;
    }
    if (pedal->rawValueMax < pedal->rawValue)
    {
      pedal->rawValueMax = pedal->rawValue;
    }

    // Value
    pedal->currentValue = map(pedal->rawValue, pedal->rawValueMin, pedal->rawValueMax, MIDI_MIN, MIDI_MAX);

    if (abs(pedal->currentValue - pedal->previousValue) > CHANGE_MARGIN)
    {
      pedal->previousValue = pedal->currentValue;
      MIDI.sendControlChange(pedal->controlChangeNumber, pedal->invert ? MIDI_MAX - pedal->currentValue : pedal->currentValue, 1);

      // Debug
      if (debugMode)
      {
        if (debugPedals)
        {
          Serial.printf("Pedal %u: GPIO = %u; Raw = %u; Current = %u;\r\n", i, pedal->gpioPin, pedal->rawValue, pedal->currentValue);
        }
      }
    }
  }
}
