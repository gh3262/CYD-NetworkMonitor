#ifndef CYD_H
#define CYD_H

//
// ESP32-2432S028R CYD Support Package
//
// Includes:
//   - Pin definitions
//   - Helper functions
//   - Display support
//   - Touch support
//

#include "cyd_pins.h"
#include "cyd_helpers.h"
#include "cyd_display.h"
#include "cyd_touch.h"

// ====================================================
// Complete Hardware Initialization
// ====================================================

inline bool initCYD()
{
    ledInit();
    backlightInit();

    if (!initDisplay())
    {
        ledRed();
        return false;
    }

    initTouch();

    ledGreen();

    return true;
}

// ====================================================
// Startup Screen
// ====================================================

inline void showSplash(
    const char* title = "ESP32 CYD",
    uint16_t bgColor = BLACK)
{
    clearScreen(bgColor);

    centerText(
        title,
        90,
        WHITE,
        3);
}

// ====================================================
// Network Status LED Helpers
// ====================================================

inline void networkDisconnected()
{
    ledRed();
}

inline void networkConnecting()
{
    ledBlue();
}

inline void networkConnected()
{
    ledGreen();
}

inline void networkWeakSignal()
{
    ledYellow();
}

// ====================================================
// WiFi Signal Quality
// ====================================================

inline void networkSignalLED(int rssi)
{
    if (rssi > -65)
    {
        networkConnected();
    }
    else if (rssi > -80)
    {
        networkWeakSignal();
    }
    else
    {
        networkDisconnected();
    }
}

#endif