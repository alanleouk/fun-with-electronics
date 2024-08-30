#ifndef ALANLEOUK_MAIN_OUTPUTS_H_
#define ALANLEOUK_MAIN_OUTPUTS_H_

#include <Arduino.h>

// Definitions
#define TWO_PI 6.283185307179586476925286766559
#define DAC_AMPLITUDE 255
#define OUTPUT_SIGNAL_CHANNELS 2
#define OUTPUT_ARRAY_LEN 400

// Wave Tables
enum WaveType
{
  Sin = 1,
  Triangle = 2,
  Sawtooth = 3,
  Square = 4,
  Cadence = 5,
  Max = 6
};

// Pins
const static int _outputSignalPins[OUTPUT_SIGNAL_CHANNELS] = {25, 26};

// Buffers
static WaveType _waveType[OUTPUT_SIGNAL_CHANNELS] = {WaveType::Cadence, WaveType::Max};
static uint8_t _sin_wav[OUTPUT_ARRAY_LEN];
static uint8_t _tri_wav[OUTPUT_ARRAY_LEN];
static uint8_t _saw_wav[OUTPUT_ARRAY_LEN];
static uint8_t _squ_wav[OUTPUT_ARRAY_LEN];
static uint8_t _cad_wav[OUTPUT_ARRAY_LEN];
static uint8_t _max_wav[OUTPUT_ARRAY_LEN];

// State
static bool _outputsEnabled[OUTPUT_SIGNAL_CHANNELS] = {true, false};
static uint32_t _outputBufferIndex_Prev[OUTPUT_SIGNAL_CHANNELS] = {0, 0};
static const uint8_t _dacAmplitude[OUTPUT_SIGNAL_CHANNELS] = {DAC_AMPLITUDE, DAC_AMPLITUDE};
static uint8_t _customDacAmplitude[OUTPUT_SIGNAL_CHANNELS] = {DAC_AMPLITUDE, 80};
static uint64_t _outputCycleTimeMicros[OUTPUT_SIGNAL_CHANNELS] = {1000000, 1000000};
static uint32_t _outputFactorX[OUTPUT_SIGNAL_CHANNELS] = {0, 0};
//
static uint64_t _reportTimePerWrite = 0;
static uint64_t _lastWriteTime = 0;

// Methods
void setupOutputs();
void writeOutputs();
void configureOutputWaveFactor();
void generateOutputWaves();
uint8_t *currentOutputWaveform(uint8_t channelIndex);
WaveType currentOutputWaveType(uint8_t channelIndex);
const char *currentOutputWaveTypeChars(uint8_t channelIndex);
const char *waveTypeChars(WaveType waveType);

#endif /* ALANLEOUK_MAIN_OUTPUTS_H_ */
