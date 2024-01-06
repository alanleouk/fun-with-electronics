#include <Arduino.h>

#define TWO_PI 6.283185307179586476925286766559

// Simulation
#define timerDivider 80 // (80) timer frequency divider. timer runs at 80MHz by default.
#define simulationTimeId 0
hw_timer_t *simulationTimer = NULL;
void IRAM_ATTR onSimulationTimer();

// Pins
const static int simulationResistancePin = 25;
const static int simulationCadencePin = 26;

// Wave Tables
#define EXAMPLE_TIMER_RESOLUTION 1000000                                               // 1MHz, 1 tick = 1us
#define EXAMPLE_WAVE_FREQ_HZ 0.5                                                       // Default wave frequency 50 Hz, it can't be too high
#define EXAMPLE_CONVERT_FREQ_HZ (EXAMPLE_ARRAY_LEN * EXAMPLE_WAVE_FREQ_HZ)             // The frequency that DAC convert every data in the wave array
#define EXAMPLE_TIMER_ALARM_COUNT (EXAMPLE_TIMER_RESOLUTION / EXAMPLE_CONVERT_FREQ_HZ) // The count value that trigger the timer alarm callback
#define CONST_PERIOD_2_PI 6.2832                                                       // 2 * PI
#define EXAMPLE_ARRAY_LEN 400                                                          // Length of wave array
#define EXAMPLE_DAC_AMPLITUDE 255                                                      // Amplitude of DAC voltage. If it's more than 256 will causes dac_output_voltage() output 0.
uint8_t sin_wav[EXAMPLE_ARRAY_LEN];
uint8_t tri_wav[EXAMPLE_ARRAY_LEN];
uint8_t saw_wav[EXAMPLE_ARRAY_LEN];
uint8_t squ_wav[EXAMPLE_ARRAY_LEN];

void setup()
{
  // Analog
  analogReadResolution(12);

  // Communication
  Serial.begin(115200);
  Serial.println("Starting Signal Generator");

  // Resistance
  dacWrite(simulationResistancePin, 80);

  // Generate Waves
  uint32_t pnt_num = EXAMPLE_ARRAY_LEN;
  for (int i = 0; i < pnt_num; i++)
  {
    sin_wav[i] = (uint8_t)((sin(i * CONST_PERIOD_2_PI / pnt_num) + 1) * (double)(EXAMPLE_DAC_AMPLITUDE) / 2 + 0.5);
    tri_wav[i] = (i > (pnt_num / 2)) ? (2 * EXAMPLE_DAC_AMPLITUDE * (pnt_num - i) / pnt_num) : (2 * EXAMPLE_DAC_AMPLITUDE * i / pnt_num);
    saw_wav[i] = (i == pnt_num) ? 0 : (i * EXAMPLE_DAC_AMPLITUDE / pnt_num);
    squ_wav[i] = (i < (pnt_num / 2)) ? EXAMPLE_DAC_AMPLITUDE : 0;
  }

  // Debug Timer
  simulationTimer = timerBegin(simulationTimeId, timerDivider, true);
  timerAttachInterrupt(simulationTimer, &onSimulationTimer, true);
  timerAlarmWrite(simulationTimer, EXAMPLE_TIMER_ALARM_COUNT, true);
  timerAlarmEnable(simulationTimer);
}

void loop()
{
  delay(900);
}

void IRAM_ATTR onSimulationTimer()
{
  static uint32_t index = 0;
  dacWrite(simulationCadencePin, sin_wav[index]);
  index++;
  index %= EXAMPLE_ARRAY_LEN;
}
