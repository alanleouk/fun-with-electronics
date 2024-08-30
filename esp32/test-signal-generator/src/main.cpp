#include <Arduino.h>
#include "server.h"
#include "inputs.h"
#include "outputs.h"
#include "screen.h"

// Definitions
MyServer *_server = nullptr;

// Methods

void setup()
{
  // Communication
  Serial.begin(115200);
  Serial.println("Starting Signal Generator");
  delay(1000);

  // Setups
  setupInputs();
  setupOutputs();

  // Server
  _server = new MyServer();

  // Analog
  analogReadResolution(12);

  // Pins
  pinMode(2, OUTPUT);

  // Screen
  initialiseScreen();
  clearScreen();

  // Random
  srand(0);

  // Connect to WiFi
  _server->connectWifi();

  // Setup Inputs
  clearInputBuffers();
  configureInputWaveFactor();

  // Setup Outputs
  generateOutputWaves();
  configureOutputWaveFactor();

  // Web Server
  _server->startServer();

  // LED
  digitalWrite(2, HIGH);
}

void loop()
{
  _server->handleClient();
  readInputs();
  writeOutputs();
  // updateScreen();
}
