#include <Arduino.h>

// Simulation
#define timerDivider 80 // (80) timer frequency divider. timer runs at 80MHz by default.
#define simulationTimeId 0
hw_timer_t *simulationTimer = NULL;
void IRAM_ATTR onSimulationTimer();

// Pins
const static int simulationResistancePin = 25;
const static int simulationCadencePin = 26;

void setup()
{
  // Debug Timer
  simulationTimer = timerBegin(simulationTimeId, timerDivider, true);
  timerAttachInterrupt(simulationTimer, &onSimulationTimer, true);
  timerAlarmWrite(simulationTimer, 10000, true);
  timerAlarmEnable(simulationTimer);
}

void loop()
{
  delay(900);
}

void IRAM_ATTR onSimulationTimer()
{
  if (millis() % 1000 < 100)
  {
    dacWrite(simulationCadencePin, 10);
  }
  else
  {
    dacWrite(simulationCadencePin, 220);
  }

  if (millis() % 8000 < 4000)
  {
    dacWrite(simulationResistancePin, 80);
  }
  else
  {
    dacWrite(simulationResistancePin, 96);
  }
}