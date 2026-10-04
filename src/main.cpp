#include <Arduino.h>
#include <WiFi.h>
#include "cyd.h"

const char* ssid = "VirusServer_2232";
const char* password = "Girraffe151!";

enum Screen
{
    SCREEN_STATUS,
    SCREEN_SCAN
};

Screen currentScreen = SCREEN_STATUS;
int networkCount = 0;

// Connection
void connectWiFi();

// Status Screen
void drawNetworkScreen();

// Scan Screen
void performScan();
void drawScanScreen();

// Navigation
void drawCurrentScreen();
void enterScanScreen();

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
    drawCurrentScreen();
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
    clearScreen(COLOR_BACKGROUND);

    titleBar(
    "NETWORK MONITOR",
    COLOR_HEADER_BG,
    COLOR_HEADER_TEXT);

    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_TEXT);

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

    gfx->setCursor(10, 200);
    gfx->print("Uptime: ");
    gfx->print(millis() / 1000);
    gfx->println(" sec");

    drawButton(
        200,   // x
        200,   // y
        120,   // width
        30,    // height
        "Refresh");

    drawButton(
        200,
        160,
        120,
        30,
        "Scan");
}

void drawCurrentScreen()
{
    switch (currentScreen)
    {
        case SCREEN_STATUS:
            drawNetworkScreen();
            break;

        case SCREEN_SCAN:
            drawScanScreen();
            break;
    }
}

void performScan()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar("WIFI SCAN");

    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(FONT_SMALL);

    gfx->setCursor(10, 50);
    gfx->println("Scanning...");

    networkCount = WiFi.scanNetworks();
}

void drawScanScreen()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar("WIFI SCAN");

    // titleBar() leaves the text size at 2, so reset it for the list
    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(1);

    // y=25 would sit inside the 30 px title bar, so draw the count just below it
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Found %d networks", networkCount < 0 ? 0 : networkCount);
    gfx->setCursor(10, 36);
    gfx->println(buffer);

    const int lineHeight = 12;
    const int firstLineY = 50;
    const int maxLines = (195 - firstLineY) / lineHeight;

    for (int i = 0; i < networkCount && i < maxLines; i++)
    {
        gfx->setCursor(10, firstLineY + (i * lineHeight));

        gfx->print(WiFi.SSID(i));

        gfx->print(" ");

        gfx->print(WiFi.RSSI(i));

        gfx->println(" dBm");
    }

    drawButton(
        10,
        200,
        120,
        30,
        "Back");

    drawButton(
        200,
        200,
        120,
        30,
        "Rescan");
}

void enterScanScreen()
{
    performScan();

    currentScreen = SCREEN_SCAN;
    drawCurrentScreen();
}

unsigned long lastUpdate = 0;

void loop()
{
    int x, y;

    if (getTouch(x, y))
    {
        Serial.print("Touch: ");
        Serial.print(x);
        Serial.print(",");
        Serial.println(y);

        if (currentScreen == SCREEN_STATUS)
        {
            // Scan button
            if (touchInRect(
                    x, y,
                    200, 160,
                    120, 30))
            {
                Serial.println("Scan pressed");
                enterScanScreen();
                delay(300);
            }

            // Refresh button
            if (touchInRect(
                    x, y,
                    200, 200,
                    120, 30))
            {
                Serial.println("Refresh pressed");

                ledBlue();

                drawCurrentScreen();

                networkSignalLED(WiFi.RSSI());

                delay(300);
            }
        }
        else if (currentScreen == SCREEN_SCAN)
        {
            // Back button
            if (touchInRect(
                    x, y,
                    10, 200,
                    120, 30))
            {
                Serial.println("Back pressed");

                currentScreen = SCREEN_STATUS;
                drawCurrentScreen();

                delay(300);
            }

            // Rescan button
            if (touchInRect(
                    x, y,
                    200, 200,
                    120, 30))
            {
                Serial.println("Rescan pressed");

                performScan();
                drawCurrentScreen();

                delay(300);
            }
        }
    }
}