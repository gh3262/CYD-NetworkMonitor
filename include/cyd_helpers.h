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
// Touch Conversion
// ====================================================
inline int touchToScreenX(int rawX)
{
    return constrain(
        map(rawX,
            TOUCH_X_MIN,
            TOUCH_X_MAX,
            0,
            320),
        0,
        319);
}
inline int touchToScreenY(int rawY)
{
    return constrain(
        map(rawY,
            TOUCH_Y_MIN,
            TOUCH_Y_MAX,
            0,
            240),
        0,
        239);
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
inline unsigned long uptimeSeconds()
{
    return millis() / 1000;
}
inline unsigned long uptimeMinutes()
{
    return millis() / 60000;
}
inline unsigned long uptimeHours()
{
    return millis() / 3600000;
}
#endif
