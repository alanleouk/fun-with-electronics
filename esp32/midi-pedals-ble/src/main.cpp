#include <Arduino.h>
#include <BLEDevice.h>
#include <BLECharacteristic.h>
#include <BLE2902.h>

#define BLE_DEVICE_NAME "Midi BLE"
#define MIDI_SERVICE_UUID BLEUUID("03B80E5A-EDE8-4B33-A751-6CE34EC4C700")
#define MIDI_CHARACTERISTIC_UUID BLEUUID("7772E5DB-3868-4112-A1A9-F2669D106BF3")
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

// Bluetooth
BLEServer *p_bleServer;

// Bluetooth Cycle Power Service (cps)
BLEService *p_cpsService;
BLEAdvertising *p_cpsAdvertising = NULL;
BLEAdvertisementData _cpsAdvertisementData;

BLECharacteristic midiCharacteristic(MIDI_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_NOTIFY);

static const uint _midiBufferLength = 5;

unsigned char _midiBuffer[_midiBufferLength];

// Classes
class ServerCallbacks : public BLEServerCallbacks
{
public:
  void onConnect(BLEServer *pServer)
  {
    // _deviceConnected = true;
    if (p_cpsAdvertising != NULL)
    {
      p_cpsAdvertising->start();
    }
  };
  void onDisconnect(BLEServer *pServer)
  {
    // _deviceConnected = false;
  }
};

// Methods
void reset();
void process();

void setup()
{
  // Analog
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  // Communication
  Serial.begin(115200);

  // Reset
  reset();

  // Pin Modes
  for (size_t i = 0; i < PEDAL_COUNT; i++)
  {
    pinMode(_pedals[i].gpioPin, ANALOG);
  }

  // Bluetooth
  BLEDevice::init(BLE_DEVICE_NAME);
  p_bleServer = BLEDevice::createServer();
  p_bleServer->setCallbacks(new ServerCallbacks());

  // Service
  p_cpsService = p_bleServer->createService(MIDI_SERVICE_UUID);

  // Characteristics
  p_cpsService->addCharacteristic(&midiCharacteristic);
  midiCharacteristic.addDescriptor(new BLE2902());

  // Midi Buffer
  _midiBuffer[0] = 0x80;
  _midiBuffer[1] = 0x80;
  _midiBuffer[2] = 0xB0;
  _midiBuffer[3] = 0;
  _midiBuffer[4] = 0;

  // Defaults
  midiCharacteristic.setValue(_midiBuffer, _midiBufferLength);

  // Start Services
  p_cpsService->start();

  // Setup Advertising
  p_cpsAdvertising = BLEDevice::getAdvertising();
  p_cpsAdvertising->addServiceUUID(MIDI_SERVICE_UUID);
  p_cpsAdvertising->setScanResponse(true);

  // Advertise
  p_cpsAdvertising->start();

  // Core Info
  Serial.print("Setup: Executing on core ");
  Serial.println(xPortGetCoreID());
}

void loop()
{
  unsigned long time = millis();

  // Process
  process();

  // Debug
  if (debugMode)
  {
    delay(2000);
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

      _midiBuffer[3] = pedal->controlChangeNumber;
      _midiBuffer[4] = pedal->invert ? MIDI_MAX - pedal->currentValue : pedal->currentValue;

      midiCharacteristic.setValue(_midiBuffer, _midiBufferLength);
      midiCharacteristic.notify();

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
