#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Web Server
WebServer server(80);
const char *ssid = "AWNet_2G_Guest";
const char *password = "EwKTBBVFL61gR3nMjJF9ey7qoP";

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

#define TWO_PI 6.283185307179586476925286766559
#define TIMER_ALARM_COUNT 1000
#define CONST_PERIOD_2_PI 6.2832 // 2 * PI
#define ARRAY_LEN 400
#define DAC_AMPLITUDE 255
#define SIGNAL_CHANNELS 2

// Pins
const static int _signalPin[SIGNAL_CHANNELS] = {25, 26};

WaveType _waveType[SIGNAL_CHANNELS] = {WaveType::Cadence, WaveType::Max};
uint8_t _sin_wav[ARRAY_LEN];
uint8_t _tri_wav[ARRAY_LEN];
uint8_t _saw_wav[ARRAY_LEN];
uint8_t _squ_wav[ARRAY_LEN];
uint8_t _cad_wav[ARRAY_LEN];
uint8_t _max_wav[ARRAY_LEN];

uint32_t _dacAmplitude = 255;
uint64_t _cycleTimeMicros = 1000000;
uint32_t _amplitudeInverseFactor = 0;
uint64_t _reportTimePerWrite = 0;
uint64_t _lastWriteTime = 0;

// Methods
void configureWave();
void handleWaveOutput();
uint8_t *currentWaveform(uint8_t channelIndex);
WaveType currentWaveType(uint8_t channelIndex);
const char *currentWaveTypeChars(uint8_t channelIndex);
const char *waveTypeChars(WaveType waveType);
String generateHtml(uint8_t channelIndex);
void handle_root();
void handle_NotFound();

void setup()
{
  // Analog
  analogReadResolution(12);

  // Pins
  pinMode(2, OUTPUT);

  // Random
  srand(0);

  // Communication
  Serial.begin(115200);
  Serial.println("Starting Signal Generator");
  delay(1000);

  // Connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  int connectCount = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    connectCount++;
    if (connectCount >= 16)
    {
      ESP.restart();
    }
  }
  Serial.println();
  Serial.print("WiFi connected...");
  Serial.print(" Got IP: ");
  Serial.println(WiFi.localIP());

  // Generate Waves
  Serial.println("Generating waves");
  uint32_t pnt_num = ARRAY_LEN;
  for (int i = 0; i < pnt_num; i++)
  {
    _sin_wav[i] = (uint8_t)((sin(i * CONST_PERIOD_2_PI / pnt_num) + 1) * (double)(DAC_AMPLITUDE) / 2 + 0.5);
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

  configureWave();

  // Web Server
  server.on("/", handle_root);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");

  // LED
  digitalWrite(2, HIGH);
}

void handleWaveOutput()
{
  static uint32_t previousWaveFormIndex = 0;
  uint32_t waveFormIndex = (esp_timer_get_time() % _cycleTimeMicros) / _amplitudeInverseFactor;
  if (waveFormIndex != previousWaveFormIndex)
  {
    for (uint8_t channelIndex = 0; channelIndex < SIGNAL_CHANNELS; channelIndex++)
    {
      dacWrite(_signalPin[channelIndex], currentWaveform(channelIndex)[waveFormIndex]);
    }

    previousWaveFormIndex = waveFormIndex;
  }

  if (_lastWriteTime > 0)
  {
    _reportTimePerWrite = (esp_timer_get_time() - _lastWriteTime);
  }
  _lastWriteTime = esp_timer_get_time();
}

void loop()
{
  server.handleClient();
  handleWaveOutput();
}

void configureWave()
{
  _amplitudeInverseFactor = _cycleTimeMicros / ARRAY_LEN;
}

uint8_t *currentWaveform(uint8_t channelIndex)
{
  switch (currentWaveType(channelIndex))
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

WaveType currentWaveType(uint8_t channelIndex)
{
  return _waveType[channelIndex];
}

const char *currentWaveTypeChars(uint8_t channelIndex)
{
  return waveTypeChars(currentWaveType(channelIndex));
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

void appendHeader(String &ptr, uint8_t channelIndex)
{
  ptr += "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Signal Generator</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 30px;} h1 {color: #444444;margin: 20px auto 10px;} h3 {color: #444444;margin-bottom: 10px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #3498db;}\n";
  ptr += ".button-on:active {background-color: #2980b9;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "<script src=\"http://ajax.aspnetcdn.com/ajax/jQuery/jquery-1.6.1.min.js\"></script>\n";
  ptr += "<script>\n";
  ptr += "var graph,xPadding=30,yPadding=30,data={values:[";
  for (int i = 0; i < ARRAY_LEN; i++)
  {
    char buffer[50];
    sprintf(buffer, "{X:%d,Y:%d},", i, currentWaveform(channelIndex)[i]);
    ptr += buffer;
  }
  ptr += "]};function getMaxY(){for(var e=0,t=0;t<data.values.length;t++)data.values[t].Y>e&&(e=data.values[t].Y);return e+=10-e%10}function getXPixel(e){return(graph.width()-xPadding)/data.values.length*e+1.5*xPadding}function getYPixel(e){return graph.height()-(graph.height()-yPadding)/getMaxY()*e-yPadding}$(document).ready(function(){var e=(graph=$(\"#graph\"))[0].getContext(\"2d\");e.lineWidth=2,e.strokeStyle=\"#333\",e.font=\"italic 8pt sans-serif\",e.textAlign=\"center\",e.beginPath(),e.moveTo(xPadding,0),e.lineTo(xPadding,graph.height()-yPadding),e.lineTo(graph.width(),graph.height()-yPadding),e.stroke(),e.textAlign=\"right\",e.textBaseline=\"middle\";for(var t=0;t<getMaxY();t+=10)e.fillText(t,xPadding-10,getYPixel(t));e.strokeStyle=\"#f00\",e.beginPath(),e.moveTo(getXPixel(0),getYPixel(data.values[0].Y));for(t=1;t<data.values.length;t++)e.lineTo(getXPixel(t),getYPixel(data.values[t].Y));e.stroke()});\n";
  ptr += "</script>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
}

void appendFooter(String &ptr, uint8_t channelIndex)
{
  ptr += "</body>\n";
  ptr += "</html>\n";
}

void appendLink(String &ptr, uint8_t channelIndex, WaveType waveType, uint64_t cycleTimeMicros, String title, bool divWrap)
{
  if (cycleTimeMicros == 0)
  {
    cycleTimeMicros = _cycleTimeMicros;
  }

  if (divWrap)
  {
    ptr += "<div>";
  }
  ptr += "<a href=\"./?v=1";
  ptr += "&channelIndex=";
  ptr += channelIndex;
  ptr += "&waveType=";
  ptr += waveTypeChars(waveType);
  ptr += "&cycleTimeMicros=";
  ptr += std::to_string(cycleTimeMicros).c_str();
  ptr += "\">";
  ptr += title;
  ptr += "</a>\n";
  if (divWrap)
  {
    ptr += "</div>";
  }
}

String generateHtml(uint8_t channelIndex)
{
  String ptr = "";
  appendHeader(ptr, channelIndex);

  // Title
  ptr += "<h1>Signal Generator</h1>\n";

  // Channel Links
  ptr += "<h3>Frequency Links</h3>\n";
  appendLink(ptr, 0, _waveType[0], _cycleTimeMicros, "Channel 1", true);
  appendLink(ptr, 1, _waveType[1], _cycleTimeMicros, "Channel 2", true);

  // Frequency Links
  ptr += "<h3>Frequency Links</h3>\n";
  appendLink(ptr, channelIndex, currentWaveType(channelIndex), (_cycleTimeMicros * 2), "Half", true);
  appendLink(ptr, channelIndex, currentWaveType(channelIndex), (_cycleTimeMicros / 2), "Double", true);

  // Settings

  ptr += "<h3>Settings</h3>\n";

  ptr += "<div>Channel: \n";
  ptr += (channelIndex + 1);
  ptr += "</div>\n";

  ptr += "<div>Wave type: \n";
  ptr += currentWaveTypeChars(channelIndex);
  ptr += "</div>\n";

  ptr += "<div>Output Pin: \n";
  ptr += std::to_string(_signalPin[channelIndex]).c_str();
  ptr += "</div>\n";

  ptr += "<div>Time per write (micros): \n";
  ptr += std::to_string(_reportTimePerWrite).c_str();
  ptr += "</div>\n";

  ptr += "<div>Wave Time (microseconds): \n";
  ptr += std::to_string(_cycleTimeMicros).c_str();
  ptr += "</div>\n";

  ptr += "<div>Frequency (Hz): \n";
  ptr += std::to_string(1000000.0F / _cycleTimeMicros).c_str();
  ptr += "</div>\n";

  ptr += "<div>Frequency (cpm): \n";
  ptr += std::to_string(1000000.0F * 60.0 / _cycleTimeMicros).c_str();
  ptr += "</div>\n";

  // Wave Links

  ptr += "<h3>Wave Links</h3>\n";
  appendLink(ptr, channelIndex, WaveType::Sin, _cycleTimeMicros, "Sine Wave", true);
  appendLink(ptr, channelIndex, WaveType::Square, _cycleTimeMicros, "Square Wave", true);
  appendLink(ptr, channelIndex, WaveType::Triangle, _cycleTimeMicros, "Triangle Wave", true);
  appendLink(ptr, channelIndex, WaveType::Sawtooth, _cycleTimeMicros, "Sawtooth Wave", true);
  appendLink(ptr, channelIndex, WaveType::Cadence, _cycleTimeMicros, "Cadence Wave", true);
  appendLink(ptr, channelIndex, WaveType::Max, _cycleTimeMicros, "Max Wave", true);

  // Chart Canvas

  ptr += "<h3>Chart</h3>\n";
  ptr += "<canvas id=\"graph\" width=\"960\" height=\"600\"></canvas>\n";

  appendFooter(ptr, channelIndex);

  return ptr;
}

void handle_root()
{
  int argumentCount = server.args();
  uint8_t channelIndex = 0;
  for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++)
  {
    String argName = server.argName(argumentIndex);
    String argValue = server.arg(argumentIndex);
    if (argName == "channelIndex")
    {
      channelIndex = argValue.toInt();
    }
  }

  for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++)
  {
    String argName = server.argName(argumentIndex);
    String argValue = server.arg(argumentIndex);

    if (argName == "cycleTimeMicros")
    {
      uint32_t newCycleTimeMicros = argValue.toInt();
      if (newCycleTimeMicros > 0)
      {
        _cycleTimeMicros = newCycleTimeMicros;
        configureWave();
      }
    }
    else if (argName == "waveType")
    {
      if (argValue == "sine" || argValue == "sin")
      {
        _waveType[channelIndex] = WaveType::Sin;
      }
      else if (argValue == "triangle" || argValue == "tri")
      {
        _waveType[channelIndex] = WaveType::Triangle;
      }
      else if (argValue == "sawtooth" || argValue == "saw")
      {
        _waveType[channelIndex] = WaveType::Sawtooth;
      }
      else if (argValue == "square" || argValue == "squ")
      {
        _waveType[channelIndex] = WaveType::Square;
      }
      else if (argValue == "cadence" || argValue == "cad")
      {
        _waveType[channelIndex] = WaveType::Cadence;
      }
    }
  }

  server.send(200, "text/html", generateHtml(channelIndex));
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}
