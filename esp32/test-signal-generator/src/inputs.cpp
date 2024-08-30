#include "inputs.h"

void setupInputs()
{
  for (uint8_t channelIndex = 0; channelIndex < INPUT_SIGNAL_CHANNELS; channelIndex++)
  {
    pinMode(_inputSignalPins[channelIndex], INPUT);
  }
}

void configureInputWaveFactor()
{
  for (uint8_t channelIndex = 0; channelIndex < INPUT_SIGNAL_CHANNELS; channelIndex++)
  {
    _inputFactorX[channelIndex] = _inputCycleTimeMicros[channelIndex] / INPUT_BUFFER_LEN;
  }
}

void clearInputBuffers()
{
  for (uint8_t channelIndex = 0; channelIndex < INPUT_SIGNAL_CHANNELS; channelIndex++)
  {
    for (uint32_t bufferIndex = 0; bufferIndex < INPUT_BUFFER_LEN; bufferIndex++)
    {
      _inputBuffer[channelIndex][bufferIndex] = analogRead(_inputSignalPins[channelIndex]);
    }
  }
}

void readInputs()
{
  for (uint8_t channelIndex = 0; channelIndex < INPUT_SIGNAL_CHANNELS; channelIndex++)
  {
    if (_inputsEnabled[channelIndex])
    {
      uint32_t bufferIndex = (esp_timer_get_time() % _inputCycleTimeMicros[channelIndex]) / _inputFactorX[channelIndex];
      if (bufferIndex != _inputBufferIndex_Prev[channelIndex])
      {
        for (uint32_t index = _inputBufferIndex_Prev[channelIndex] + 1; index <= bufferIndex; index++)
        {
          _inputBuffer[channelIndex][index] = analogRead(_inputSignalPins[channelIndex]);
          /*
          Serial.print("Index: ");
          Serial.print(index);
          Serial.print("; Value: ");
          Serial.print(_inputBuffer[channelIndex][index]);
          Serial.println();
          */
        }
        _inputBufferIndex_Prev[channelIndex] = bufferIndex;
      }
    }
  }
}
