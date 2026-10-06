#ifndef APP_H
#define APP_H

#include <Arduino.h>
#include <WiFi.h>
#include "cyd.h"

// ====================================================
// Application State
// ====================================================

enum Screen
{
    SCREEN_HOME,
    SCREEN_WIFI_STATUS,
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
    String currentSSID;
    int currentRSSI;
};

const int PING_RUNNING = -3;
const int PING_SLOW_MS = 100;
const int PING_NOT_RUN = -2;
const int PING_FAILED = -1;

const char* const INTERNET_PING_HOST = "8.8.8.8";

// Defined in main.cpp
extern AppState state;

// ====================================================
// Navigation (main.cpp)
// ====================================================

void drawCurrentScreen();

// ====================================================
// Screens (src/screens/)
//   draw*   - render the screen
//   enter*  - make it the current screen and draw it
//   handle* - process a touch while it is the current screen
// ====================================================

// home_screen.cpp
void drawHomeScreen();
void enterHomeScreen();
void handleHomeTouch(int x, int y);

// wifi_status_screen.cpp
void drawWifiStatusScreen();
void enterWifiStatusScreen();
void handleWifiStatusTouch(int x, int y);

// wifi_scan_screen.cpp
void performScan();
void drawScanScreen();
void enterScanScreen();
void handleScanTouch(int x, int y);

// system_screen.cpp
void drawSystemScreen();
void enterSystemScreen();
void handleSystemTouch(int x, int y);

// tools_screen.cpp
void drawToolsScreen();
void enterToolsScreen();
void handleToolsTouch(int x, int y);
void performGatewayPing();
void performInternetPing();

#endif
