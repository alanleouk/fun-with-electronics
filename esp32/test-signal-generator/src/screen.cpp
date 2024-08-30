#include "screen.h"
#include "inputs.h"

void initialiseScreen()
{
    tft.initR(INITR_BLACKTAB); // Init ST7735S chip, black tab
    Serial.println("Screen Initialized");
}

void clearScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setRotation(1);

    // Debug
    /*
    tft.setCursor(0, 30);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(1);

    char hello[] = "Hello World, what is out there?";
    for (int i = 0; i < 32; i++)
    {
        tft.print(hello[i]);
        delay(100);
    }
    */
}

void updateScreen()
{
    Serial.println("updateScreen");

    clearScreen();

    int16_t width = tft.width();
    int16_t height = tft.height();

    // TODO: No hard coded channel

    /*
        for (uint32_t bufferIndex = 0; bufferIndex < INPUT_BUFFER_LEN; bufferIndex++)
        {
            Serial.print("Value: ");
            Serial.print(_inputBuffer[0][bufferIndex]);
            Serial.println();
        }
    */

    for (int16_t x = 0; x < width; x++)
    {

        uint16_t xScaled = static_cast<float>(x * INPUT_BUFFER_LEN) / static_cast<float>(width);
        uint16_t y = static_cast<float>(_inputBuffer[0][xScaled]) * 4096.0F / static_cast<float>(height);
        // Serial.print("Value: ");
        // Serial.print(x);
        // Serial.print(",");
        // Serial.print(y);
        // Serial.println();
        tft.drawPixel(x, y, ST77XX_RED);
    }
}