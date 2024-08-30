#include "outputs.h"

void setupOutputs()
{
  for (uint8_t channelIndex = 0; channelIndex < OUTPUT_SIGNAL_CHANNELS; channelIndex++)
  {
    pinMode(_outputSignalPins[channelIndex], OUTPUT);
  }
}

void configureOutputWaveFactor()
{
  for (uint8_t channelIndex = 0; channelIndex < OUTPUT_SIGNAL_CHANNELS; channelIndex++)
  {
    _outputFactorX[channelIndex] = _outputCycleTimeMicros[channelIndex] / OUTPUT_ARRAY_LEN;
  }
}

void generateOutputWaves()
{
  Serial.println("Generating output waves");
  uint32_t pnt_num = OUTPUT_ARRAY_LEN;
  for (int i = 0; i < pnt_num; i++)
  {
    _sin_wav[i] = (uint8_t)((sin(i * TWO_PI / pnt_num) + 1) * (double)(DAC_AMPLITUDE) / 2 + 0.5);
    _tri_wav[i] = (i > (pnt_num / 2)) ? (2 * DAC_AMPLITUDE * (pnt_num - i) / pnt_num) : (2 * DAC_AMPLITUDE * i / pnt_num);
    _saw_wav[i] = (i == pnt_num) ? 0 : (i * DAC_AMPLITUDE / pnt_num);
    _squ_wav[i] = (i < (pnt_num / 2)) ? DAC_AMPLITUDE : 0;
    _cad_wav[i] = DAC_AMPLITUDE;
    _max_wav[i] = DAC_AMPLITUDE;
  }
  _cad_wav[1] = 240;
  _cad_wav[2] = 210;
  _cad_wav[3] = 190;
  _cad_wav[4] = 170;
  _cad_wav[5] = 150;
  _cad_wav[6] = 130;
  _cad_wav[7] = 110;
  _cad_wav[8] = 90;
  _cad_wav[9] = 70;
  _cad_wav[10] = 50;
  _cad_wav[11] = 30;
  _cad_wav[12] = 10;
  _cad_wav[13] = 0;
  _cad_wav[14] = 0;
  _cad_wav[15] = 0;
  _cad_wav[16] = 0;
  _cad_wav[17] = 0;
  _cad_wav[18] = 0;
  _cad_wav[19] = 0;
  _cad_wav[20] = 0;
  _cad_wav[21] = 0;
  _cad_wav[22] = 0;
  _cad_wav[23] = 0;
  _cad_wav[24] = 0;
  _cad_wav[25] = 0;
  _cad_wav[26] = 0;
  _cad_wav[27] = 0;
  _cad_wav[28] = 0;
  _cad_wav[29] = 10;
  _cad_wav[30] = 30;
  _cad_wav[31] = 50;
  _cad_wav[32] = 70;
  _cad_wav[33] = 90;
  _cad_wav[34] = 110;
  _cad_wav[35] = 130;
  _cad_wav[36] = 150;
  _cad_wav[37] = 170;
  _cad_wav[38] = 190;
  _cad_wav[39] = 210;
  _cad_wav[40] = 240;
}

void writeOutputs()
{
  for (uint8_t channelIndex = 0; channelIndex < OUTPUT_SIGNAL_CHANNELS; channelIndex++)
  {
    if (_outputsEnabled[channelIndex])
    {
      uint32_t bufferIndex = (esp_timer_get_time() % _outputCycleTimeMicros[channelIndex]) / _outputFactorX[channelIndex];
      if (bufferIndex != _outputBufferIndex_Prev[channelIndex])
      {
        if (_customDacAmplitude[channelIndex] == _dacAmplitude[channelIndex])
        {
          dacWrite(_outputSignalPins[channelIndex], currentOutputWaveform(channelIndex)[bufferIndex]);
        }
        else
        {
          uint8_t waveFormValue = currentOutputWaveform(channelIndex)[bufferIndex] * (static_cast<float>(_customDacAmplitude[channelIndex]) / _dacAmplitude[channelIndex]);
          dacWrite(_outputSignalPins[channelIndex], waveFormValue);
        }

        _outputBufferIndex_Prev[channelIndex] = bufferIndex;
      }
    }
  }

  if (_lastWriteTime > 0)
  {
    _reportTimePerWrite = (esp_timer_get_time() - _lastWriteTime);
  }
  _lastWriteTime = esp_timer_get_time();
}

uint8_t *currentOutputWaveform(uint8_t channelIndex)
{
  switch (currentOutputWaveType(channelIndex))
  {
  case WaveType::Sin:
    return _sin_wav;

  case WaveType::Square:
    return _squ_wav;

  case WaveType::Triangle:
    return _tri_wav;

  case WaveType::Sawtooth:
    return _saw_wav;

  case WaveType::Max:
    return _max_wav;

  case WaveType::Cadence:
  default:
    return _cad_wav;
  }
}

WaveType currentOutputWaveType(uint8_t channelIndex)
{
  return _waveType[channelIndex];
}

const char *currentOutputWaveTypeChars(uint8_t channelIndex)
{
  return waveTypeChars(currentOutputWaveType(channelIndex));
}

const char *waveTypeChars(WaveType waveType)
{
  switch (waveType)
  {
  case WaveType::Sin:
    return "sin";

  case WaveType::Square:
    return "squ";

  case WaveType::Triangle:
    return "tri";

  case WaveType::Sawtooth:
    return "saw";

  case WaveType::Max:
    return "max";

  case WaveType::Cadence:
  default:
    return "cad";
  }
}
