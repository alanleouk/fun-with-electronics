#ifndef ALANLEOUK_MAIN_SERVER_H_
#define ALANLEOUK_MAIN_SERVER_H_

#define WIFI_SSID "AWNet_2G_Guest"
#define WIFI_PASSWORD "EwKTBBVFL61gR3nMjJF9ey7qoP"

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "inputs.h"
#include "outputs.h"

class MyServer
{
public:
    void connectWifi();
    void startServer();
    void handleClient();

private:
    WebServer *server = nullptr;

    void appendHeader(String &ptr, uint8_t channelIndex);
    void appendFooter(String &ptr, uint8_t channelIndex);
    void appendLink(String &ptr, uint8_t channelIndex, WaveType waveType, uint64_t cycleTimeMicros, String title, bool divWrap);
    String generateHtml(uint8_t channelIndex);
    void handle_root();
    void handle_updateScreen();
    void handle_NotFound();
};

#endif /* ALANLEOUK_MAIN_SERVER_H_ */
