#include <Arduino.h>
#include <WiFi.h>
#include "cyd.h"

const char* ssid = "VirusServer_2232";
const char* password = "Girraffe151!";

void connectWiFi();
void drawNetworkScreen();

void setup()
{
    Serial.begin(115200);

    if (!initCYD())
    {
        while (true)
        {
        }
    }

    showSplash("Network Monitor");

    connectWiFi();
    drawNetworkScreen();
}

void connectWiFi()
{
    networkConnecting();

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    networkConnected();

    Serial.println();
    Serial.println("WiFi Connected");
}

void drawNetworkScreen()
{
    clearScreen(BLACK);

    titleBar("NETWORK MONITOR");

    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);

    gfx->setCursor(10, 50);
    gfx->println("SSID:");

    gfx->setCursor(10, 75);
    gfx->println(WiFi.SSID());

    gfx->setCursor(10, 110);
    gfx->println("IP:");

    gfx->setCursor(10, 135);
    gfx->println(WiFi.localIP());

    gfx->setCursor(10, 170);
    gfx->print("RSSI: ");
    gfx->print(WiFi.RSSI());
    gfx->println(" dBm");
}

void loop()
{
}