#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Web Server
WebServer server(80);
const char *ssid = "AWNet_2G_Guest";
const char *password = "EwKTBBVFL61gR3nMjJF9ey7qoP";

// Pins
const static int simulationResistancePin = 25;
const static int signalPin = 26;

// Wave Tables
enum WaveType
{
  Sin = 1,
  Triangle = 2,
  Saw = 3,
  Square = 4,
  Cadence = 5
};

#define TWO_PI 6.283185307179586476925286766559
#define TIMER_ALARM_COUNT 1000
#define CONST_PERIOD_2_PI 6.2832 // 2 * PI
#define ARRAY_LEN 400
#define DAC_AMPLITUDE 255
WaveType waveType = WaveType::Cadence;
uint8_t sin_wav[ARRAY_LEN];
uint8_t tri_wav[ARRAY_LEN];
uint8_t saw_wav[ARRAY_LEN];
uint8_t squ_wav[ARRAY_LEN];
uint8_t cad_wav[ARRAY_LEN];

uint32_t dacAmplitude = 255;
uint64_t cycleTimeMicros = 1000000;
uint32_t amplitudeInverseFactor = 0;
uint64_t reportTimePerWrite = 0;
uint64_t lastWriteTime = 0;

// Methods
void configureWave();
void handleWaveOutput();
uint8_t *currentWaveform();
const char *currentWaveType();
String generateHtml();
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

  // Resistance
  dacWrite(simulationResistancePin, 80);

  // Generate Waves
  Serial.println("Generating waves");
  uint32_t pnt_num = ARRAY_LEN;
  for (int i = 0; i < pnt_num; i++)
  {
    sin_wav[i] = (uint8_t)((sin(i * CONST_PERIOD_2_PI / pnt_num) + 1) * (double)(DAC_AMPLITUDE) / 2 + 0.5);
    tri_wav[i] = (i > (pnt_num / 2)) ? (2 * DAC_AMPLITUDE * (pnt_num - i) / pnt_num) : (2 * DAC_AMPLITUDE * i / pnt_num);
    saw_wav[i] = (i == pnt_num) ? 0 : (i * DAC_AMPLITUDE / pnt_num);
    squ_wav[i] = (i < (pnt_num / 2)) ? DAC_AMPLITUDE : 0;
    cad_wav[i] = 255;
  }
  cad_wav[1] = 240;
  cad_wav[2] = 210;
  cad_wav[3] = 190;
  cad_wav[4] = 170;
  cad_wav[5] = 150;
  cad_wav[6] = 130;
  cad_wav[7] = 110;
  cad_wav[8] = 90;
  cad_wav[9] = 70;
  cad_wav[10] = 50;
  cad_wav[11] = 30;
  cad_wav[12] = 10;
  cad_wav[13] = 0;
  cad_wav[14] = 0;
  cad_wav[15] = 0;
  cad_wav[16] = 0;
  cad_wav[17] = 0;
  cad_wav[18] = 0;
  cad_wav[19] = 0;
  cad_wav[20] = 0;
  cad_wav[21] = 0;
  cad_wav[22] = 0;
  cad_wav[23] = 0;
  cad_wav[24] = 0;
  cad_wav[25] = 0;
  cad_wav[26] = 0;
  cad_wav[27] = 0;
  cad_wav[28] = 0;
  cad_wav[29] = 10;
  cad_wav[30] = 30;
  cad_wav[31] = 50;
  cad_wav[32] = 70;
  cad_wav[33] = 90;
  cad_wav[34] = 110;
  cad_wav[35] = 130;
  cad_wav[36] = 150;
  cad_wav[37] = 170;
  cad_wav[38] = 190;
  cad_wav[39] = 210;
  cad_wav[40] = 240;

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
  static uint32_t previousIndex = 0;
  uint32_t index = (esp_timer_get_time() % cycleTimeMicros) / amplitudeInverseFactor;
  if (index != previousIndex)
  {
    dacWrite(signalPin, currentWaveform()[index]);
    previousIndex = index;
  }

  if (lastWriteTime > 0)
  {
    reportTimePerWrite = (esp_timer_get_time() - lastWriteTime);
  }
  lastWriteTime = esp_timer_get_time();
}

void loop()
{
  server.handleClient();
  handleWaveOutput();
}

void configureWave()
{
  amplitudeInverseFactor = cycleTimeMicros / ARRAY_LEN;
}

uint8_t *currentWaveform()
{
  switch (waveType)
  {
  case WaveType::Sin:
    return sin_wav;

  case WaveType::Square:
    return squ_wav;

  case WaveType::Triangle:
    return tri_wav;

  case WaveType::Saw:
    return saw_wav;

  case WaveType::Cadence:
  default:
    return cad_wav;
  }
}

const char *currentWaveType()
{
  switch (waveType)
  {
  case WaveType::Sin:
    return "sin";

  case WaveType::Square:
    return "squ";

  case WaveType::Triangle:
    return "tri";

  case WaveType::Saw:
    return "saw";

  case WaveType::Cadence:
  default:
    return "cad";
  }
}

String generateHtml()
{
  String ptr = "<!DOCTYPE html> <html>\n";
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
    sprintf(buffer, "{X:%d,Y:%d},", i, currentWaveform()[i]);
    ptr += buffer;
  }
  ptr += "]};function getMaxY(){for(var e=0,t=0;t<data.values.length;t++)data.values[t].Y>e&&(e=data.values[t].Y);return e+=10-e%10}function getXPixel(e){return(graph.width()-xPadding)/data.values.length*e+1.5*xPadding}function getYPixel(e){return graph.height()-(graph.height()-yPadding)/getMaxY()*e-yPadding}$(document).ready(function(){var e=(graph=$(\"#graph\"))[0].getContext(\"2d\");e.lineWidth=2,e.strokeStyle=\"#333\",e.font=\"italic 8pt sans-serif\",e.textAlign=\"center\",e.beginPath(),e.moveTo(xPadding,0),e.lineTo(xPadding,graph.height()-yPadding),e.lineTo(graph.width(),graph.height()-yPadding),e.stroke(),e.textAlign=\"right\",e.textBaseline=\"middle\";for(var t=0;t<getMaxY();t+=10)e.fillText(t,xPadding-10,getYPixel(t));e.strokeStyle=\"#f00\",e.beginPath(),e.moveTo(getXPixel(0),getYPixel(data.values[0].Y));for(t=1;t<data.values.length;t++)e.lineTo(getXPixel(t),getYPixel(data.values[t].Y));e.stroke()});\n";
  ptr += "</script>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Signal Generator</h1>\n";

  // Frequency Links

  ptr += "<h3>Frequency Links</h3>\n";
  ptr += "<div><a href=\"./?waveType=";
  ptr += currentWaveType();
  ptr += "&cycleTimeMicros=";
  ptr += std::to_string(cycleTimeMicros * 2).c_str();
  ptr += "\">Half</a></div>\n";

  ptr += "<div><a href=\"./?waveType=";
  ptr += currentWaveType();
  ptr += "&cycleTimeMicros=";
  ptr += std::to_string(cycleTimeMicros / 2).c_str();
  ptr += "\">Double</a></div>\n";

  // Settings

  ptr += "<h3>Settings</h3>\n";
  ptr += "<div>Wave type: \n";
  ptr += currentWaveType();
  ptr += "</div>\n";

  ptr += "<div>Output Pin: \n";
  ptr += std::to_string(signalPin).c_str();
  ptr += "</div>\n";

  ptr += "<div>Time per write (micros): \n";
  ptr += std::to_string(reportTimePerWrite).c_str();
  ptr += "</div>\n";

  ptr += "<div>Wave Time (microseconds): \n";
  ptr += std::to_string(cycleTimeMicros).c_str();
  ptr += "</div>\n";

  ptr += "<div>Frequency (Hz): \n";
  ptr += std::to_string(1000000.0F / cycleTimeMicros).c_str();
  ptr += "</div>\n";

  ptr += "<div>Frequency (cpm): \n";
  ptr += std::to_string(1000000.0F * 60.0 / cycleTimeMicros).c_str();
  ptr += "</div>\n";

  // Wave Links

  ptr += "<h3>Wave Links</h3>\n";

  ptr += "<div><a href=\"./?waveType=sin&cycleTimeMicros=";
  ptr += std::to_string(cycleTimeMicros).c_str();
  ptr += "\">Sine Wave</a></div>\n";

  ptr += "<div><a href=\"./?waveType=squ&cycleTimeMicros=";
  ptr += std::to_string(cycleTimeMicros).c_str();
  ptr += "\">Square Wave</a></div>\n";

  ptr += "<div><a href=\"./?waveType=tri&cycleTimeMicros=";
  ptr += std::to_string(cycleTimeMicros).c_str();
  ptr += "\">Triangle Wave</a></div>\n";

  ptr += "<div><a href=\"./?waveType=saw&cycleTimeMicros=";
  ptr += std::to_string(cycleTimeMicros).c_str();
  ptr += "\">Sawtooth Wave</a></div>\n";

  ptr += "<div><a href=\"./?waveType=cad&cycleTimeMicros=";
  ptr += std::to_string(cycleTimeMicros).c_str();
  ptr += "\">Cadence Wave</a></div>\n";

  // Chart

  ptr += "<h3>Chart</h3>\n";
  ptr += "<canvas id=\"graph\" width=\"960\" height=\"600\"></canvas>\n";

  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

void handle_root()
{
  int argumentCount = server.args();
  for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++)
  {
    String argName = server.argName(argumentIndex);
    String argValue = server.arg(argumentIndex);

    if (argName == "cycleTimeMicros")
    {
      uint32_t newCycleTimeMicros = argValue.toInt();
      if (newCycleTimeMicros > 0)
      {
        cycleTimeMicros = newCycleTimeMicros;
        configureWave();
      }
    }
    else if (argName == "waveType")
    {
      if (argValue == "sine" || argValue == "sin")
      {
        waveType = WaveType::Sin;
      }
      else if (argValue == "triangle" || argValue == "tri")
      {
        waveType = WaveType::Triangle;
      }
      else if (argValue == "sawtooth" || argValue == "saw")
      {
        waveType = WaveType::Saw;
      }
      else if (argValue == "square" || argValue == "squ")
      {
        waveType = WaveType::Square;
      }
      else if (argValue == "cadence" || argValue == "cad")
      {
        waveType = WaveType::Cadence;
      }
    }
  }

  server.send(200, "text/html", generateHtml());
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}
