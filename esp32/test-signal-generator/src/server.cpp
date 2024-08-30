#include "server.h"
#include "screen.h"

// Web Server

void MyServer::connectWifi()
{
  Serial.printf("Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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
}

void MyServer::startServer()
{
  if (server == nullptr)
  {
    server = new WebServer(80);
  }

  server->on("/", std::bind(&MyServer::handle_root, this));
  server->on("/updateScreen", std::bind(&MyServer::handle_updateScreen, this));
  server->onNotFound(std::bind(&MyServer::handle_NotFound, this));
  server->begin();
  Serial.println("HTTP server started");
}

void MyServer::handleClient()
{
  server->handleClient();
}

void MyServer::appendHeader(String &ptr, uint8_t channelIndex)
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
  for (int bufferIndex = 0; bufferIndex < OUTPUT_ARRAY_LEN; bufferIndex++)
  {
    char buffer[50];
    if (_customDacAmplitude[channelIndex] == _dacAmplitude[channelIndex])
    {
      sprintf(buffer, "{X:%d,Y:%d},", bufferIndex, currentOutputWaveform(channelIndex)[bufferIndex]);
    }
    else
    {
      uint8_t waveFormValue = currentOutputWaveform(channelIndex)[bufferIndex] * (static_cast<float>(_customDacAmplitude[channelIndex]) / _dacAmplitude[channelIndex]);
      sprintf(buffer, "{X:%d,Y:%d},", bufferIndex, waveFormValue);
    }
    ptr += buffer;
  }

  ptr += "]};function getMaxY(){for(var e=0,t=0;t<data.values.length;t++)data.values[t].Y>e&&(e=data.values[t].Y);return e+=10-e%10}function getXPixel(e){return(graph.width()-xPadding)/data.values.length*e+1.5*xPadding}function getYPixel(e){return graph.height()-(graph.height()-yPadding)/getMaxY()*e-yPadding}$(document).ready(function(){var e=(graph=$(\"#graph\"))[0].getContext(\"2d\");e.lineWidth=2,e.strokeStyle=\"#333\",e.font=\"italic 8pt sans-serif\",e.textAlign=\"center\",e.beginPath(),e.moveTo(xPadding,0),e.lineTo(xPadding,graph.height()-yPadding),e.lineTo(graph.width(),graph.height()-yPadding),e.stroke(),e.textAlign=\"right\",e.textBaseline=\"middle\";for(var t=0;t<getMaxY();t+=10)e.fillText(t,xPadding-10,getYPixel(t));e.strokeStyle=\"#f00\",e.beginPath(),e.moveTo(getXPixel(0),getYPixel(data.values[0].Y));for(t=1;t<data.values.length;t++)e.lineTo(getXPixel(t),getYPixel(data.values[t].Y));e.stroke()});\n";
  ptr += "</script>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
}

void MyServer::appendFooter(String &ptr, uint8_t channelIndex)
{
  ptr += "</body>\n";
  ptr += "</html>\n";
}

void MyServer::appendLink(String &ptr, uint8_t channelIndex, WaveType waveType, uint64_t cycleTimeMicros, String title, bool divWrap)
{
  if (cycleTimeMicros == 0)
  {
    cycleTimeMicros = _outputCycleTimeMicros[channelIndex];
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

String MyServer::generateHtml(uint8_t channelIndex)
{
  String ptr = "";
  appendHeader(ptr, channelIndex);

  // Title
  ptr += "<h1>Signal Generator</h1>\n";

  // Channel Links
  ptr += "<h3>Frequency Links</h3>\n";
  appendLink(ptr, 0, _waveType[0], _outputCycleTimeMicros[channelIndex], "Channel 1", true);
  appendLink(ptr, 1, _waveType[1], _outputCycleTimeMicros[channelIndex], "Channel 2", true);

  // Frequency Links
  ptr += "<h3>Frequency Links</h3>\n";
  appendLink(ptr, channelIndex, currentOutputWaveType(channelIndex), (_outputCycleTimeMicros[channelIndex] * 2), "Half", true);
  appendLink(ptr, channelIndex, currentOutputWaveType(channelIndex), (_outputCycleTimeMicros[channelIndex] / 2), "Double", true);

  // Settings

  ptr += "<h3>Settings</h3>\n";

  ptr += "<div>Channel: \n";
  ptr += (channelIndex + 1);
  ptr += "</div>\n";

  ptr += "<div>Wave type: \n";
  ptr += currentOutputWaveTypeChars(channelIndex);
  ptr += "</div>\n";

  ptr += "<div>Input Pin: \n";
  ptr += std::to_string(_inputSignalPins[channelIndex]).c_str();
  ptr += "</div>\n";

  ptr += "<div>Output Pin: \n";
  ptr += std::to_string(_outputSignalPins[channelIndex]).c_str();
  ptr += "</div>\n";

  if (_waveType[channelIndex] != WaveType::Max)
  {
    ptr += "<div>Time per write (micros): \n";
    ptr += std::to_string(_reportTimePerWrite).c_str();
    ptr += "</div>\n";

    ptr += "<div>Wave Time (microseconds): \n";
    ptr += std::to_string(_outputCycleTimeMicros[channelIndex]).c_str();
    ptr += "</div>\n";

    ptr += "<div>Frequency (Hz): \n";
    ptr += std::to_string(1000000.0F / _outputCycleTimeMicros[channelIndex]).c_str();
    ptr += "</div>\n";

    ptr += "<div>Frequency (cpm): \n";
    ptr += std::to_string(1000000.0F * 60.0 / _outputCycleTimeMicros[channelIndex]).c_str();
    ptr += "</div>\n";
  }

  if (_customDacAmplitude[channelIndex] != _dacAmplitude[channelIndex])
  {
    ptr += "<div>Custom Amplitude: \n";
    ptr += std::to_string(_customDacAmplitude[channelIndex]).c_str();
    ptr += "</div>\n";
  }

  // Wave Links

  ptr += "<h3>Wave Links</h3>\n";
  appendLink(ptr, channelIndex, WaveType::Sin, _outputCycleTimeMicros[channelIndex], "Sine Wave", true);
  appendLink(ptr, channelIndex, WaveType::Square, _outputCycleTimeMicros[channelIndex], "Square Wave", true);
  appendLink(ptr, channelIndex, WaveType::Triangle, _outputCycleTimeMicros[channelIndex], "Triangle Wave", true);
  appendLink(ptr, channelIndex, WaveType::Sawtooth, _outputCycleTimeMicros[channelIndex], "Sawtooth Wave", true);
  appendLink(ptr, channelIndex, WaveType::Cadence, _outputCycleTimeMicros[channelIndex], "Cadence Wave", true);
  appendLink(ptr, channelIndex, WaveType::Max, _outputCycleTimeMicros[channelIndex], "Max Wave", true);

  // Chart Canvas

  ptr += "<h3>Chart</h3>\n";
  ptr += "<canvas id=\"graph\" width=\"960\" height=\"600\"></canvas>\n";

  appendFooter(ptr, channelIndex);

  return ptr;
}

void MyServer::handle_root()
{
  Serial.println("handle_root");

  int argumentCount = server->args();
  uint8_t channelIndex = 0;
  for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++)
  {
    String argName = server->argName(argumentIndex);
    String argValue = server->arg(argumentIndex);
    if (argName == "channelIndex")
    {
      channelIndex = argValue.toInt();
    }
  }

  for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++)
  {
    String argName = server->argName(argumentIndex);
    String argValue = server->arg(argumentIndex);

    if (argName == "cycleTimeMicros")
    {
      uint32_t newCycleTimeMicros = argValue.toInt();
      if (newCycleTimeMicros > 0)
      {
        _outputCycleTimeMicros[channelIndex] = newCycleTimeMicros;
        configureOutputWaveFactor();
      }
    }
    else if (argName == "customDacAmplitude")
    {
      uint32_t newCustomDacAmplitude = argValue.toInt();
      if (newCustomDacAmplitude > 0)
      {
        _customDacAmplitude[channelIndex] = newCustomDacAmplitude;
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

  server->send(200, "text/html", generateHtml(channelIndex));
}

void MyServer::handle_updateScreen()
{
  Serial.println("handle_updateScreen");
  updateScreen();
  server->send(200, "text/html", "Success");
}

void MyServer::handle_NotFound()
{
  Serial.println("handle_NotFound");
  server->send(404, "text/plain", "Not found");
}
