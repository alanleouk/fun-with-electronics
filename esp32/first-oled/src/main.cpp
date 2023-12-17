#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

// https://www.youtube.com/watch?v=Rw_BSWku54E

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

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);

void writeHelloWorld()
{
  int w = tft.width();
  int h = tft.height();

  // Text with wrapping
  tft.setTextWrap(true);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(1);

  char hello[] = "Hello World, what is out there?";
  for (int i = 0; i < 32; i++)
  {
    tft.print(hello[i]);
    delay(100);
  }

  tft.invertDisplay(true);
  delay(1500);
  tft.invertDisplay(false);
  delay(1500);
}

void setup()
{
  Serial.begin(115200);
  tft.initR(INITR_BLACKTAB); // Init ST7735S chip, black tab
  Serial.println("Initialized");

  tft.fillScreen(ST77XX_BLACK);
  int w = tft.width();
  int h = tft.height();

  Serial.print("width =");
  Serial.println(w);
  Serial.print("height =");
  Serial.println(h);

  writeHelloWorld();
}

void loop()
{
  delay(1000);
}
