#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

const char *ssid = "AWNet_2G_Guest";
const char *password = "EwKTBBVFL61gR3nMjJF9ey7qoP";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

const bool debugMode = false;

char timeLength = 8;
char timeBuffer[12] = "00:00:00";

char dateLength = 11;
char dateBuffer[12] = "01 Jan 2024";

unsigned long timeUpdatePeriod = 43200000;
unsigned long lastTimeUpdate = 0;

// ESP32-WROOM
#define TFT_DC 12   // A0
#define TFT_CS 13   // CS
#define TFT_MOSI 14 // SDA
#define TFT_CLK 27  // SCK
#define TFT_RST 0
#define TFT_MISO 0

// Powering the TFT display...
// GND is GND  that is the easy one
// LED pin on TFT will control brightness.  Using DAC on the ESP32 can control brightness by connecting
// to this pin.  See code below.  If this pin is high, VCC on the TFT does not need to be connected, but
// things seem to work better when it is connected.  VCC can go to either the 3.3 volt or the 5 volt (Vin)
// of the ESP32.  There does not appear to be any appreciable brightness difference when 5v is used instead of 3.3
// But the screen is brightest when 3.3 volts for VCC and 5volts for LED.

/* Possible colors that are predefined...
ST7735_BLACK ST77XX_BLACK
ST7735_WHITE ST77XX_WHITE
ST7735_RED ST77XX_RED
ST7735_GREEN ST77XX_GREEN
ST7735_BLUE ST77XX_BLUE
ST7735_CYAN ST77XX_CYAN
ST7735_MAGENTA ST77XX_MAGENTA
ST7735_YELLOW ST77XX_YELLOW
ST7735_ORANGE ST77XX_ORANGE
*/

void updateTimeBuffer();
void printLocalTime();
void writeDisplay();
void updateTime();

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);

void updateTimeBuffer()
{
  if (!debugMode)
  {
    struct tm timeInfo;

    if (!getLocalTime(&timeInfo))
    {
      timeLength = sprintf(timeBuffer, "00:00:00");
      dateLength = sprintf(dateBuffer, "ERROR");
    }
    else
    {
      timeLength = strftime(timeBuffer, 12, "%H:%M:%S", &timeInfo);
      dateLength = strftime(dateBuffer, 12, "%d %b %Y", &timeInfo);
    }
  }
}

void writeDisplay()
{
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  tft.setCursor(10, 10);
  for (int i = 0; i < timeLength; i++)
  {
    tft.print(timeBuffer[i]);
  }

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  tft.setCursor(15, 40);
  for (int i = 0; i < dateLength; i++)
  {
    tft.print(dateBuffer[i]);
  }
}

void updateTime()
{
  bool success = true;

  if (!debugMode)
  {
    // Connect to WiFi
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.setCursor(10, 10);
    tft.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    tft.println("WiFi connected..!");
    tft.print("Got IP: ");
    tft.println(WiFi.localIP());

    // Initialise Time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeInfo;
    if (!getLocalTime(&timeInfo))
    {
      tft.println("Failed to obtain time");
      success = false;
    }

    // Disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }

  if (success)
  {
    delay(3000);
    tft.fillScreen(ST77XX_BLACK);
    lastTimeUpdate = millis();
  }
}

void setup()
{
  Serial.begin(115200);

  // Initialise Display
  Serial.println("Initialing Display");
  tft.initR(INITR_BLACKTAB); // Init ST7735S chip, black tab
  Serial.println("...Initialized");

  // Fill Screen
  tft.setTextWrap(true);
  tft.setTextSize(3);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  int w = tft.width();
  int h = tft.height();

  // Clear Screen
  tft.fillScreen(ST77XX_BLACK);

  // Debug Output
  if (debugMode)
  {
    Serial.print("width=");
    Serial.println(w);
    Serial.print("height=");
    Serial.println(h);
  }
}

void loop()
{
  if (lastTimeUpdate == 0 || millis() - lastTimeUpdate > timeUpdatePeriod)
  {
    updateTime();
  }
  updateTimeBuffer();
  writeDisplay();
  delay(500);
}
