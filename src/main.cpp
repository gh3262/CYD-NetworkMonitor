#include <Arduino.h>
#include <WiFi.h>
#include "cyd.h"

const char* ssid = "VirusServer_2232";
const char* password = "Girraffe151!";

enum Screen
{
    SCREEN_STATUS,
    SCREEN_SCAN,
    SCREEN_SYSTEM
};

struct AppState
{
    Screen currentScreen;
    int networkCount;
};

AppState state = {SCREEN_STATUS, 0};

// Connection
void connectWiFi();

// Status Screen
void drawNetworkScreen();

// Scan Screen
void performScan();
void drawScanScreen();

// System Screen
void drawSystemScreen();

// Diagnostics
void colorTest();

// Navigation
void drawCurrentScreen();
void enterScanScreen();
void enterSystemScreen();

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

    //colorTest();
    // while (true);
    // while (true);

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

    gfx->setCursor(10, 40);
    gfx->println("SSID:");

    gfx->setCursor(10, 60);
    gfx->println(WiFi.SSID());

    gfx->setCursor(10, 85);
    gfx->println("IP:");

    gfx->setCursor(10, 105);
    gfx->println(WiFi.localIP());

    gfx->setCursor(10, 135);
    gfx->print("RSSI: ");
    gfx->print(WiFi.RSSI());
    gfx->println(" dBm");

    // Kept above NAV_Y so it doesn't collide with the buttons
    gfx->setCursor(10, 165);
    gfx->print("Uptime: ");
    gfx->println(formatUptime());

    drawButton(
        NAV_LEFT_X,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT,
        "System");

    drawButton(
        NAV_CENTER_X,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT,
        "Scan");

    drawButton(
        NAV_RIGHT_X,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT,
        "Refresh");
}

void drawCurrentScreen()
{
    switch (state.currentScreen)
    {
        case SCREEN_STATUS:
            drawNetworkScreen();
            break;

        case SCREEN_SCAN:
            drawScanScreen();
            break;

        case SCREEN_SYSTEM:
            drawSystemScreen();
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

    state.networkCount = WiFi.scanNetworks();
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
    snprintf(buffer, sizeof(buffer), "Found %d networks", state.networkCount < 0 ? 0 : state.networkCount);
    gfx->setCursor(10, 36);
    gfx->println(buffer);

    const int lineHeight = 12;
    const int firstLineY = 50;
    const int maxLines = (NAV_Y - 5 - firstLineY) / lineHeight;

    for (int i = 0; i < state.networkCount && i < maxLines; i++)
    {
        gfx->setCursor(10, firstLineY + (i * lineHeight));

        gfx->print(WiFi.SSID(i));

        gfx->print(" ");

        gfx->print(WiFi.RSSI(i));

        gfx->println(" dBm");
    }

    drawButton(
        NAV_LEFT_X,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT,
        "Back");

    drawButton(
        NAV_RIGHT_X,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT,
        "Rescan");
}

void enterScanScreen()
{
    performScan();

    state.currentScreen = SCREEN_SCAN;
    drawCurrentScreen();
}

void colorTest()
{
    gfx->fillRect(0,   0, 80, 80, CYD_BLACK);
    gfx->fillRect(80,  0, 80, 80, CYD_WHITE);
    gfx->fillRect(160, 0, 80, 80, CYD_RED);
    gfx->fillRect(240, 0, 80, 80, CYD_GREEN);

    gfx->fillRect(0,   80, 80, 80, CYD_BLUE);
    gfx->fillRect(80,  80, 80, 80, CYD_YELLOW);
    gfx->fillRect(160, 80, 80, 80, CYD_CYAN);
    gfx->fillRect(240, 80, 80, 80, CYD_MAGENTA);
}

void drawSystemScreen()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar("SYSTEM");

    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(FONT_NORMAL);

    gfx->setCursor(10, 50);
    gfx->print("Free Heap: ");
    gfx->print(ESP.getFreeHeap() / 1024);
    gfx->println(" KB");

    gfx->setCursor(10, 80);
    gfx->print("Light Level: ");
    gfx->print(lightPercent());
    gfx->println("%");

    gfx->setCursor(10, 110);
    gfx->print("Uptime: ");
    gfx->println(formatUptime());
    // gfx->println(" sec");

    drawButton(
        NAV_LEFT_X,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT,
        "Back");
}

void enterSystemScreen()
{
    state.currentScreen = SCREEN_SYSTEM;
    drawCurrentScreen();
}

void loop()
{
    int x, y;

    if (getTouch(x, y))
    {
        Serial.print("Touch: ");
        Serial.print(x);
        Serial.print(",");
        Serial.println(y);

        if (state.currentScreen == SCREEN_STATUS)
        {
            // System button
            if (touchInRect(
                    x, y,
                    NAV_LEFT_X, NAV_Y,
                    NAV_WIDTH, NAV_HEIGHT))
            {
                enterSystemScreen();
                delay(300);
            }

            // Scan button
            else if (touchInRect(
                    x, y,
                    NAV_CENTER_X, NAV_Y,
                    NAV_WIDTH, NAV_HEIGHT))
            {
                enterScanScreen();
                delay(300);
            }

            // Refresh button
            else if (touchInRect(
                    x, y,
                    NAV_RIGHT_X, NAV_Y,
                    NAV_WIDTH, NAV_HEIGHT))
            {
                drawCurrentScreen();
                networkSignalLED(WiFi.RSSI());
                delay(300);
            }
        }
        else if (state.currentScreen == SCREEN_SCAN)
        {
            // Back button
            if (touchInRect(
                    x, y,
                    NAV_LEFT_X, NAV_Y,
                    NAV_WIDTH, NAV_HEIGHT))
            {
                state.currentScreen = SCREEN_STATUS;
                drawCurrentScreen();
                delay(300);
            }

            // Rescan button
            else if (touchInRect(
                    x, y,
                    NAV_RIGHT_X, NAV_Y,
                    NAV_WIDTH, NAV_HEIGHT))
            {
                performScan();
                drawCurrentScreen();
                delay(300);
            }
        }
        else if (state.currentScreen == SCREEN_SYSTEM)
        {
            // Back button
            if (touchInRect(
                    x, y,
                    NAV_LEFT_X, NAV_Y,
                    NAV_WIDTH, NAV_HEIGHT))
            {
                state.currentScreen = SCREEN_STATUS;
                drawCurrentScreen();
                delay(300);
            }
        }
    }
}
