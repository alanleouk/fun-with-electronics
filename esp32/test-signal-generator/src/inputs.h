#ifndef ALANLEOUK_MAIN_INPUTS_H_
#define ALANLEOUK_MAIN_INPUTS_H_

#include <Arduino.h>

// Definitions
// #define ADC_AMPLITUDE 1023
#define INPUT_SIGNAL_CHANNELS 2
#define INPUT_BUFFER_LEN 160

// Pins
const static int _inputSignalPins[INPUT_SIGNAL_CHANNELS] = {34, 35};

// Buffers
static uint16_t _inputBuffer[INPUT_SIGNAL_CHANNELS][INPUT_BUFFER_LEN];

// State
static bool _inputsEnabled[INPUT_SIGNAL_CHANNELS] = {true, false};
static uint32_t _inputBufferIndex_Prev[INPUT_SIGNAL_CHANNELS] = {0, 0};
static uint64_t _inputCycleTimeMicros[INPUT_SIGNAL_CHANNELS] = {1000000, 1000000};
static uint32_t _inputFactorX[INPUT_SIGNAL_CHANNELS] = {0, 0};

// Methods
void setupInputs();
void configureInputWaveFactor();
void clearInputBuffers();
void readInputs();

#endif /* ALANLEOUK_MAIN_INPUTS_H_ */
