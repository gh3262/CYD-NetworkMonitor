#include <Arduino.h>
#include <algorithm>
#include <vector>
#include <WiFi.h>
#include <ESP32Ping.h>
#include "cyd.h"
#include "credentials.h"
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>


const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

enum Screen
{
    SCREEN_STATUS,
    SCREEN_SCAN,
    SCREEN_SYSTEM,
    SCREEN_TOOLS
};

struct AppState
{
    Screen currentScreen;
    int networkCount;
    int scanPage;
    int gatewayPingMs;
    int internetPingMs;
};

const int PING_RUNNING = -3;
const int PING_SLOW_MS = 100;
const int PING_NOT_RUN = -2;
const int PING_FAILED = -1;

const char* INTERNET_PING_HOST = "8.8.8.8";

AppState state = {SCREEN_STATUS, 0, 0, PING_NOT_RUN, PING_NOT_RUN};

// Scan list layout
const int SCAN_LINE_HEIGHT = 12;
const int SCAN_FIRST_LINE_Y = 50;
const int SCAN_ROWS_PER_PAGE = (NAV_Y - 5 - SCAN_FIRST_LINE_Y) / SCAN_LINE_HEIGHT;

int scanPageCount()
{
    const int count = state.networkCount < 0 ? 0 : state.networkCount;
    return (count + SCAN_ROWS_PER_PAGE - 1) / SCAN_ROWS_PER_PAGE;
}

// Scan result indices sorted by signal strength, strongest first
std::vector<int> scanOrder;

// Connection
void connectWiFi();

// Status Screen
void drawNetworkScreen();

// Scan Screen
void performScan();
void drawScanScreen();

// System Screen
void drawSystemScreen();

// Tools Screen
void drawToolsScreen();
void enterToolsScreen();
void performGatewayPing();
void performInternetPing();

// Diagnostics
void colorTest();

// Navigation
void drawCurrentScreen();
void enterScanScreen();
void enterSystemScreen();

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

    showSplash("Network Monitor");

    connectWiFi();

    syncTime();

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

    // gfx->setTextSize(1);
    // gfx->setTextColor(COLOR_WARNING);

    // gfx->setCursor(195, 10);
    // gfx->println(currentTimeString());
    // gfx->setCursor(255, 10);
    // gfx->println(currentDateString());

    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_TEXT);

    gfx->setCursor(10, 36);
    gfx->println("SSID:");

    gfx->setCursor(10, 52);
    gfx->println(WiFi.SSID());

    gfx->setCursor(10, 74);
    gfx->println("IP:");

    gfx->setCursor(10, 90);
    gfx->println(WiFi.localIP());

    gfx->setCursor(10, 114);
    gfx->print("RSSI: ");

    const int rssi = WiFi.RSSI();
    if (rssi > -65)
        gfx->setTextColor(COLOR_OK);
    else if (rssi > -80)
        gfx->setTextColor(COLOR_WARNING);
    else
        gfx->setTextColor(COLOR_ERROR);

    gfx->print(rssi);
    gfx->println(" dBm");
    gfx->setTextColor(COLOR_TEXT);

    gfx->setCursor(10, 138);
    gfx->print("Uptime: ");
    gfx->println(formatUptime());

    // Kept above NAV_Y so it doesn't collide with the buttons
    gfx->setCursor(10, 162);
    gfx->print("Health: ");

    if (state.internetPingMs == PING_NOT_RUN && state.gatewayPingMs == PING_NOT_RUN)
    {
        gfx->println("--");
    }
    else if (state.internetPingMs >= 0)
    {
        gfx->setTextColor(COLOR_OK);
        gfx->println("ONLINE");
    }
    else if (state.gatewayPingMs >= 0)
    {
        gfx->setTextColor(COLOR_WARNING);
        gfx->println("LOCAL ONLY");
    }
    else
    {
        gfx->setTextColor(COLOR_ERROR);
        gfx->println("OFFLINE");
    }

    gfx->setTextColor(COLOR_TEXT);

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

        case SCREEN_TOOLS:
            drawToolsScreen();
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
    state.scanPage = 0;

    scanOrder.clear();
    for (int i = 0; i < state.networkCount; i++)
    {
        scanOrder.push_back(i);
    }

    std::sort(scanOrder.begin(), scanOrder.end(), [](int a, int b)
    {
        return WiFi.RSSI(a) > WiFi.RSSI(b);
    });
}

void drawScanScreen()
{
   
    clearScreen(COLOR_BACKGROUND);

    titleBar("WIFI SCAN");

    // titleBar() leaves the text size at 2, so reset it for the list
    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(1);
    // gfx->setFont(&FreeSans9pt7b); 

    const int pageCount = scanPageCount();

    // y=25 would sit inside the 30 px title bar, so draw the count just below it
    char buffer[40];
    if (pageCount > 1)
    {
        snprintf(buffer, sizeof(buffer), "Found %d networks (page %d/%d)",
                 state.networkCount, state.scanPage + 1, pageCount);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "Found %d networks", state.networkCount < 0 ? 0 : state.networkCount);
    }
    gfx->setCursor(10, 36);
    gfx->println(buffer);

    const String connectedBssid = WiFi.BSSIDstr();
    const int firstRow = state.scanPage * SCAN_ROWS_PER_PAGE;

    for (int row = 0; row < SCAN_ROWS_PER_PAGE && firstRow + row < (int)scanOrder.size(); row++)
    {
        const int i = scanOrder[firstRow + row];

        gfx->setCursor(10, SCAN_FIRST_LINE_Y + (row * SCAN_LINE_HEIGHT));

        const bool connected = (WiFi.status() == WL_CONNECTED) && (WiFi.BSSIDstr(i) == connectedBssid);
        gfx->setTextColor(connected ? COLOR_OK : COLOR_TEXT);

        gfx->print(WiFi.SSID(i));

        gfx->print(" ");

        gfx->print(WiFi.RSSI(i));

        gfx->println(" dBm");
    }
    gfx->setFont(); // Reset to default font

    drawButton(
        NAV_LEFT_X,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT,
        "Back");

    if (pageCount > 1)
    {
        drawButton(
            NAV_CENTER_X,
            NAV_Y,
            NAV_WIDTH,
            NAV_HEIGHT,
            "Next");
    }

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

    // gfx->setCursor(10, 80);
    // gfx->print("Light Level: ");
    // gfx->print(lightPercent());
    // gfx->println("%");

    gfx->setCursor(10, 110);
    gfx->print("Uptime: ");
    gfx->println(formatUptime());
    // gfx->println(" sec");

    // gfx->setCursor(10, 140);
    // gfx->print("ADC34: ");
    // gfx->println(analogRead(34));

    drawButton(
        NAV_LEFT_X,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT,
        "Back");

    drawButton(
        NAV_CENTER_X,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT,
        "Tools");

    drawButton(
        NAV_RIGHT_X,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT,
        "Refresh");
}

void enterSystemScreen()
{
    state.currentScreen = SCREEN_SYSTEM;
    drawCurrentScreen();
}

void drawPingResult(int pingMs)
{
    if (pingMs == PING_NOT_RUN)
    {
        gfx->setTextColor(COLOR_TEXT);
        gfx->println("--");
    }
    else if (pingMs == PING_RUNNING)
    {
        gfx->setTextColor(COLOR_WARNING);
        gfx->println("Pinging...");
    }
    else if (pingMs == PING_FAILED)
    {
        gfx->setTextColor(COLOR_ERROR);
        gfx->println("No reply");
    }
    else
    {
        gfx->setTextColor(pingMs > PING_SLOW_MS ? COLOR_WARNING : COLOR_OK);
        gfx->print(pingMs);
        gfx->println(" ms");
    }

    gfx->setTextColor(COLOR_TEXT);
}

void drawToolsScreen()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar("TOOLS");

    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(FONT_NORMAL);

    gfx->setCursor(10, 42);
    gfx->print("Gateway ");
    gfx->println(WiFi.gatewayIP());

    gfx->setCursor(10, 62);
    gfx->print("Ping: ");
    drawPingResult(state.gatewayPingMs);

    gfx->setCursor(10, 92);
    gfx->print("Internet ");
    gfx->println(INTERNET_PING_HOST);

    gfx->setCursor(10, 112);
    gfx->print("Ping: ");
    drawPingResult(state.internetPingMs);

    gfx->setCursor(10, 150);
    gfx->print("RSSI: ");
    gfx->print(WiFi.RSSI());
    gfx->println(" dBm");

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
        "Ping");
}

int pingHost(const IPAddress& host)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return PING_FAILED;
    }

    if (Ping.ping(host, 3))
    {
        return (int)Ping.averageTime();
    }

    return PING_FAILED;
}

void performGatewayPing()
{
    state.gatewayPingMs = pingHost(WiFi.gatewayIP());
}

void performInternetPing()
{
    state.internetPingMs = pingHost(IPAddress(8, 8, 8, 8));
}
void enterToolsScreen()
{
    state.currentScreen = SCREEN_TOOLS;
    drawCurrentScreen();
}

void loop()
{
    int x, y;

    // static unsigned long lastClockUpdate = 0;

    // if (millis() - lastClockUpdate > 15000)
    // {
    //     drawClock();
    //     lastClockUpdate = millis();
    // }

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

            // Next page button (only present when there is more than one page)
            else if (scanPageCount() > 1 && touchInRect(
                    x, y,
                    NAV_CENTER_X, NAV_Y,
                    NAV_WIDTH, NAV_HEIGHT))
            {
                state.scanPage = (state.scanPage + 1) % scanPageCount();
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

            // Tools button
            else if (touchInRect(
                    x, y,
                    NAV_CENTER_X, NAV_Y,
                    NAV_WIDTH, NAV_HEIGHT))
            {
                enterToolsScreen();
                delay(300);
            }

            // Refresh button
            else if (touchInRect(
                    x, y,
                    NAV_RIGHT_X, NAV_Y,
                    NAV_WIDTH, NAV_HEIGHT))
            {
                drawCurrentScreen();
                delay(300);
            }
        }
        else if (state.currentScreen == SCREEN_TOOLS)
        {
            // Back button returns to the System screen
            if (touchInRect(
                    x, y,
                    NAV_LEFT_X, NAV_Y,
                    NAV_WIDTH, NAV_HEIGHT))
            {
                enterSystemScreen();
                delay(300);
            }

            // Ping button
            else if (touchInRect(
                    x, y,
                    NAV_RIGHT_X, NAV_Y,
                    NAV_WIDTH, NAV_HEIGHT))
            {
                state.gatewayPingMs = PING_RUNNING;
                state.internetPingMs = PING_RUNNING;
                drawCurrentScreen();

                performGatewayPing();
                performInternetPing();
                drawCurrentScreen();
                delay(300);
            }
        }
    }
}
