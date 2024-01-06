#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

const char *ssid = "AWNet_2G_Guest";
const char *password = "EwKTBBVFL61gR3nMjJF9ey7qoP";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

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

void writeLocalTime();
void printLocalTime();

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);

void writeLocalTime()
{
  tft.setTextWrap(true);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3);

  struct tm timeInfo;
  char buffer[12];

  int length = 0;
  if (!getLocalTime(&timeInfo))
  {
    length = sprintf(buffer, "Failed");
  }
  else
  {
    char buffer[12];
    // length = sprintf(buffer, "%H:%M:%S", &timeInfo);
    length = strftime(buffer, 12, "%H:%M:%S", &timeInfo);
  }

  for (int i = 0; i < length; i++)
  {
    tft.print(buffer[i]);
    delay(100);
  }
}

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

void setup()
{
  Serial.begin(115200);

  // Connect to WiFi
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

  // Initialise Time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  // Disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // Initialise Display
  Serial.println("Initialing Display");
  tft.initR(INITR_BLACKTAB); // Init ST7735S chip, black tab
  Serial.println("...Initialized");

  // Fill Screen
  tft.fillScreen(ST77XX_BLACK);
  int w = tft.width();
  int h = tft.height();

  // Debug Output
  Serial.print("width=");
  Serial.println(w);
  Serial.print("height=");
  Serial.println(h);
}

void loop()
{
  writeLocalTime();
  delay(10000);
}
