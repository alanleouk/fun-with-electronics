#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "time.h"

const char *ssid = "AWNet_2G_Guest";
const char *password = "EwKTBBVFL61gR3nMjJF9ey7qoP";

WebServer server(80);

const int _ledCount = 4;
uint8_t _ledPin[_ledCount] = {2, 13, 12, 14};
bool _ledStatus[_ledCount] = {LOW, LOW, LOW, LOW};

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

uint8_t _rgbLedPinRed = 5;
uint8_t _rgbLedPinGreen = 18;
uint8_t _rgbLedPinBlue = 19;

void printLocalTime()
{
  struct tm timeInfo;
  if (!getLocalTime(&timeInfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeInfo, "%A, %B %d %Y %H:%M:%S");
}

String SendHTML()
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>LED Control</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #3498db;}\n";
  ptr += ".button-on:active {background-color: #2980b9;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>ESP32 Web Server</h1>\n";
  ptr += "<h3>Using Station(STA) Mode</h3>\n";

  for (int i = 0; i < _ledCount; i++)
  {
    if (_ledStatus[i])
    {
      ptr += "<p>LED ";
      ptr += std::to_string(i).c_str();
      ptr += " Status: ON</p><a class=\"button button-off\" href=\"/led";
      ptr += std::to_string(i).c_str();
      ptr += "off\">OFF</a>\n";
    }
    else
    {
      ptr += "<p>LED ";
      ptr += std::to_string(i).c_str();
      ptr += " Status: OFF</p><a class=\"button button-on\" href=\"/led";
      ptr += std::to_string(i).c_str();
      ptr += "on\">ON</a>\n";
    }
  }

  /*
  struct tm timeInfo;
  if (getLocalTime(&timeInfo))
  {

  }
  */

  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

void handle_OnConnect()
{
  for (int i = 0; i < _ledCount; i++)
  {
    _ledStatus[i] = LOW;
  }
  server.send(200, "text/html", SendHTML());
}

void handle_led0on()
{
  _ledStatus[0] = HIGH;
  server.send(200, "text/html", SendHTML());
}

void handle_led0off()
{
  _ledStatus[0] = LOW;
  server.send(200, "text/html", SendHTML());
}

void handle_led1on()
{
  _ledStatus[1] = HIGH;
  server.send(200, "text/html", SendHTML());
}

void handle_led1off()
{
  _ledStatus[1] = LOW;
  server.send(200, "text/html", SendHTML());
}

void handle_led2on()
{
  _ledStatus[2] = HIGH;
  server.send(200, "text/html", SendHTML());
}

void handle_led2off()
{
  _ledStatus[2] = LOW;
  server.send(200, "text/html", SendHTML());
}

void handle_led3on()
{
  _ledStatus[3] = HIGH;
  server.send(200, "text/html", SendHTML());
}

void handle_led3off()
{
  _ledStatus[3] = LOW;
  server.send(200, "text/html", SendHTML());
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

void setup()
{
  Serial.begin(115200);
  delay(100);

  // Pin Modes
  for (int i = 0; i < _ledCount; i++)
  {
    pinMode(_ledPin[i], OUTPUT);
  }

  pinMode(_rgbLedPinRed, OUTPUT);
  pinMode(_rgbLedPinGreen, OUTPUT);
  pinMode(_rgbLedPinBlue, OUTPUT);

  // connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  // init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  // disconnect WiFi as it's no longer needed
  // WiFi.disconnect(true);
  // WiFi.mode(WIFI_OFF);

  server.on("/", handle_OnConnect);
  server.on("/led0on", handle_led0on);
  server.on("/led0off", handle_led0off);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.on("/led2on", handle_led2on);
  server.on("/led2off", handle_led2off);
  server.on("/led3on", handle_led3on);
  server.on("/led3off", handle_led3off);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");

  // Default RGB Colour
  analogWrite(_rgbLedPinRed, 25);
  analogWrite(_rgbLedPinGreen, 0);
  analogWrite(_rgbLedPinBlue, 25);
  Serial.println("Default colour for RGB LED set to 0, 151, 157");
}

void loop()
{
  // delay(1000);
  // printLocalTime();

  server.handleClient();

  for (int i = 0; i < _ledCount; i++)
  {
    if (_ledStatus[i])
    {
      digitalWrite(_ledPin[i], HIGH);
    }
    else
    {
      digitalWrite(_ledPin[i], LOW);
    }
  }
}
