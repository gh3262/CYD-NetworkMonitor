#include "app.h"
#include <ESP32Ping.h>

static String timeSyncStatus = "--";

static void drawPingResult(int pingMs)
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

static int pingHost(const IPAddress& host)
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

void drawToolsScreen()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar("TOOLS");

    setBodyFont();
    gfx->setTextColor(COLOR_TEXT);

    gfx->setCursor(10, 51);
    gfx->print("Gateway ");
    gfx->print(WiFi.gatewayIP());
    gfx->print(" - "); // Add spacing before the ping result
    drawPingResult(state.gatewayPingMs);

    gfx->setCursor(10, 75);
    gfx->print("Internet ");
    gfx->print(INTERNET_PING_HOST);
    gfx->print(" - "); // Add spacing before the ping result
    drawPingResult(state.internetPingMs);

    gfx->setCursor(10, 139);
    gfx->print("Time: ");
    gfx->println(timeSyncStatus);

    gfx->setCursor(10, 159);
    gfx->print("RSSI: ");
    gfx->print(WiFi.RSSI());
    gfx->println(" dBm");

    drawNav1("Back");
    drawNav2("NTP");
    drawNav3("Ping");
    drawNav4("Home");
}

void enterToolsScreen()
{
    state.currentScreen = SCREEN_TOOLS;
    drawCurrentScreen();
}

void handleToolsTouch(int x, int y)
{
    // Back button returns to the System screen
    if (navPressed1(x, y))
    {
        enterSystemScreen();
        delay(300);
    }

    // NTP sync button
    else if (navPressed2(x, y))
    {
        timeSyncStatus = "Syncing...";
        drawCurrentScreen();

        timeSyncStatus = syncTime() ? "Synced" : "Sync failed";
        drawCurrentScreen();
        drawClock();
        delay(300);
    }

    // Ping button
    else if (navPressed3(x, y))
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
