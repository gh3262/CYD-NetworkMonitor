#include <Arduino.h>
#include <WiFi.h>
#include "app.h"
#include "credentials.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

AppState state = {SCREEN_HOME, 0, 0, PING_NOT_RUN, PING_NOT_RUN};

// Connection
void connectWiFi();

// Diagnostics
void colorTest();

void setup()
{
    Serial.begin(115200);

    // analogReadResolution(12);
    // analogSetAttenuation(ADC_11db);

    // Serial.println("ADC test");

    // while(true)
    // {
    //     Serial.print("34: ");
    //     Serial.println(analogRead(34));
    //     Serial.println("-----");
    //     Serial.print("32: ");
    //     Serial.println(analogRead(32));
    //     Serial.print("33: ");
    //     Serial.println(analogRead(33));
    //     Serial.print("25: ");
    //     Serial.println(analogRead(25));
    //     Serial.print("26: ");
    //     Serial.println(analogRead(26));
    //     Serial.print("27: ");
    //     Serial.println(analogRead(27));
    //     Serial.println("-----");
    //     //Serial.println(analogRead(34));
    //     delay(1000);
    // }

    if (!initCYD())
    {
        while (true)
        {
        }
    }

    showSplash("Network Status");

    connectWiFi();

    if (!syncTime())
        Serial.println("Time synchronization failed; start time unavailable.");

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

void drawCurrentScreen()
{
    switch (state.currentScreen)
    {
        case SCREEN_HOME:
            drawHomeScreen();
            break;

        case SCREEN_WIFI_STATUS:
            drawWifiStatusScreen();
            break;

        case SCREEN_SCAN:
            drawScanScreen();
            break;

        case SCREEN_SYSTEM:
            drawSystemScreen();
            break;

        case SCREEN_TOOLS:
            drawToolsScreen();
            break;
    }
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

static void handleTouch(int x, int y)
{
    if (state.currentScreen == SCREEN_HOME)
    {
        handleHomeTouch(x, y);
        return;
    }

    // Home button (present on every screen except Home)
    if (navPressed4(x, y))
    {
        enterHomeScreen();
        delay(300);
        return;
    }

    switch (state.currentScreen)
    {
        case SCREEN_WIFI_STATUS:
            handleWifiStatusTouch(x, y);
            break;

        case SCREEN_SCAN:
            handleScanTouch(x, y);
            break;

        case SCREEN_SYSTEM:
            handleSystemTouch(x, y);
            break;

        case SCREEN_TOOLS:
            handleToolsTouch(x, y);
            break;

        default:
            break;
    }
}

void loop()
{
    int x, y;

    static int lastMinute = -1;

    struct tm timeinfo;

    if (getLocalTime(&timeinfo))
    {
        if (timeinfo.tm_min != lastMinute)
        {
            drawClock();

            lastMinute = timeinfo.tm_min;
        }
    }

    if (getTouch(x, y))
    {
        Serial.print("Touch: ");
        Serial.print(x);
        Serial.print(",");
        Serial.println(y);

        handleTouch(x, y);
    }
}
