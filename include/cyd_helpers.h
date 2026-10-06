#ifndef CYD_HELPERS_H
#define CYD_HELPERS_H
#include <Arduino.h>
#include "cyd_pins.h"
// ====================================================
// RGB LED Helpers
//
// LED is Common-Anode:
// LOW  = ON
// HIGH = OFF
// ====================================================
inline void ledInit()
{
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, HIGH);
}
inline void ledOff()
{
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, HIGH);
}
inline void ledRed()
{
    ledOff();
    digitalWrite(LED_RED, LOW);
}
inline void ledGreen()
{
    ledOff();
    digitalWrite(LED_GREEN, LOW);
}
inline void ledBlue()
{
    ledOff();
    digitalWrite(LED_BLUE, LOW);
}
inline void ledYellow()
{
    ledOff();
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
}
inline void ledMagenta()
{
    ledOff();
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BLUE, LOW);
}
inline void ledCyan()
{
    ledOff();
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);
}
inline void ledWhite()
{
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);
}
// ====================================================
// Display Backlight
// ====================================================
inline void backlightInit()
{
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
}
inline void backlightOn()
{
    digitalWrite(TFT_BL, HIGH);
}
inline void backlightOff()
{
    digitalWrite(TFT_BL, LOW);
}
// ====================================================
// Light Sensor
// ====================================================
inline int lightLevel()
{
    return analogRead(LIGHT_ADC);
}
// Returns percentage (rough estimate)
inline int lightPercent()
{
    return map(analogRead(LIGHT_ADC),
               0,
               4095,
               0,
               100);
}
// ====================================================
// Uptime Helpers
// ====================================================
// Returns a static buffer: use it right away
inline const char* formatUptime()
{
    unsigned long totalSeconds = millis() / 1000;

    unsigned long hours =
        totalSeconds / 3600;

    unsigned long minutes =
        (totalSeconds % 3600) / 60;

    unsigned long seconds =
        totalSeconds % 60;

    static char buffer[16];

    snprintf(
        buffer,
        sizeof(buffer),
        "%02lu:%02lu:%02lu",
        hours,
        minutes,
        seconds);

    return buffer;
}
#endif
