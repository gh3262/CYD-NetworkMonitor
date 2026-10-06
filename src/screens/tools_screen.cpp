#include "app.h"
#include <ESP32Ping.h>

static const char* timeSyncStatus = "--";

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

// ---- Pings run in a background task so the UI stays responsive ----

static bool pingActive = false;
static volatile bool pingFinished = false;

static void pingTask(void*)
{
    state.gatewayPingMs = pingHost(WiFi.gatewayIP());

    IPAddress host;
    state.internetPingMs = host.fromString(INTERNET_PING_HOST)
        ? pingHost(host)
        : PING_FAILED;

    pingFinished = true;
    vTaskDelete(NULL);
}

void startPings()
{
    if (pingActive)
        return;

    state.gatewayPingMs = PING_RUNNING;
    state.internetPingMs = PING_RUNNING;
    pingFinished = false;

    pingActive = (xTaskCreate(pingTask, "ping", 6144, NULL, 1, NULL) == pdPASS);

    if (!pingActive)
    {
        state.gatewayPingMs = PING_FAILED;
        state.internetPingMs = PING_FAILED;
    }
}

// ---- NTP sync is polled instead of waited on ----

static const unsigned long NTP_TIMEOUT_MS = 10000;

static bool ntpActive = false;
static unsigned long ntpStartMs = 0;

void startNtpSync()
{
    if (ntpActive)
        return;

    timeSyncStatus = "Syncing...";
    ntpActive = true;
    ntpStartMs = millis();
    startTimeSync();
}

// Called from loop(): redraws when a background job finishes
void updateBackgroundJobs()
{
    if (pingActive && pingFinished)
    {
        pingActive = false;

        if (WiFi.status() == WL_CONNECTED)
            networkSignalLED(WiFi.RSSI());

        refreshCurrentScreenResults();
    }

    if (ntpActive)
    {
        const bool replied = ntpReplyReceived();

        if (replied || millis() - ntpStartMs > NTP_TIMEOUT_MS)
        {
            ntpActive = false;
            timeSyncStatus = (replied && finishTimeSync()) ? "Synced" : "Sync failed";

            refreshCurrentScreenResults();
            drawClock();
        }
    }
}
static void drawPingRows()
{
    setBodyFont();
    gfx->setTextSize(1);
    gfx->fillRect(0, 35, SCREEN_WIDTH, 50, COLOR_BACKGROUND);
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
}

static void drawTimeRow()
{
    setBodyFont();
    gfx->setTextSize(1);
    gfx->fillRect(0, 124, SCREEN_WIDTH, 20, COLOR_BACKGROUND);
    gfx->setTextColor(COLOR_TEXT);

    gfx->setCursor(10, 139);
    gfx->print("Time: ");
    gfx->println(timeSyncStatus);
}

void refreshToolsResults()
{
    drawPingRows();
    drawTimeRow();
    setDefaultFont();
    gfx->setTextSize(2);
}

void drawToolsScreen()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar("TOOLS");

    setBodyFont();
    gfx->setTextColor(COLOR_TEXT);

    drawPingRows();

    drawTimeRow();

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
    }

    // NTP sync button
    else if (navPressed2(x, y))
    {
        startNtpSync();
        refreshToolsResults();
    }

    // Ping button
    else if (navPressed3(x, y))
    {
        startPings();
        refreshToolsResults();
    }
}
