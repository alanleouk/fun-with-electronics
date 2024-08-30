#include <Arduino.h>
#include <BLEDevice.h>
#include <BLECharacteristic.h>
#include <BLE2902.h>

#define BLE_DEVICE_NAME "Midi BLE"
#define MIDI_SERVICE_UUID BLEUUID("03B80E5A-EDE8-4B33-A751-6CE34EC4C700")
#define MIDI_CHARACTERISTIC_UUID BLEUUID("7772E5DB-3868-4112-A1A9-F2669D106BF3")
#define PEDAL_COUNT 4

// Debug Constants
const static bool debugMode = true;
const static bool debugPedals = true;
const static bool debugQueue = false;

// Pins
const static int resistancePin = 34;

// Pedals
uint8_t _pedalValuesCurrent[PEDAL_COUNT] = {0, 0, 0, 0};
uint8_t _pedalValuesPrev[PEDAL_COUNT] = {0, 0, 0, 0};
uint8_t _pedalController[PEDAL_COUNT] = {64, 64, 64, 64};

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
  analogReadResolution(12);

  // Communication
  Serial.begin(115200);

  // Pin Modes
  pinMode(resistancePin, ANALOG);

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
  _midiBuffer[0] = 0;
  _midiBuffer[1] = 0;
  _midiBuffer[2] = 0;
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
}

void reset()
{
  for (size_t i = 0; i < PEDAL_COUNT; i++)
  {
    _pedalValuesCurrent[i] = 0;
    _pedalValuesPrev[i] = 0;
  }
}

void process()
{
  // uint16_t newResistanceValue = floor(analogRead(resistancePin) * RESISTANCE_FACTOR);

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

      midiCharacteristic.setValue(_midiBuffer, _midiBufferLength);
      midiCharacteristic.notify();
    }
  }
}
