#ifndef CYD_TOUCH_H
#define CYD_TOUCH_H

#include <SPI.h>
#include <XPT2046_Touchscreen.h>

#include "cyd_pins.h"
#include "cyd_theme.h"

// ====================================================
// Touch Object
// ====================================================

extern XPT2046_Touchscreen touch;

// ====================================================
// Initialization
// ====================================================

inline bool initTouch()
{
    SPI.begin(
        TOUCH_SCK,
        TOUCH_MISO,
        TOUCH_MOSI);

    touch.begin();

    return true;
}

// ====================================================
// Raw Touch Access
// ====================================================

inline TS_Point getRawTouch()
{
    return touch.getPoint();
}

// ====================================================
// Screen Coordinate Conversion
// ====================================================

inline int rawToScreenX(int rawX)
{
    return constrain(
        map(rawX,
            TOUCH_X_MIN,
            TOUCH_X_MAX,
            0,
            SCREEN_WIDTH),
        0,
        SCREEN_WIDTH - 1);
}

inline int rawToScreenY(int rawY)
{
    return constrain(
        map(rawY,
            TOUCH_Y_MIN,
            TOUCH_Y_MAX,
            0,
            SCREEN_HEIGHT),
        0,
        SCREEN_HEIGHT - 1);
}

// ====================================================
// Touch Position
// ====================================================

inline int getTouchX()
{
    TS_Point p = touch.getPoint();
    return rawToScreenX(p.x);
}

inline int getTouchY()
{
    TS_Point p = touch.getPoint();
    return rawToScreenY(p.y);
}

inline bool getTouch(int &x, int &y)
{
    TS_Point p = touch.getPoint();

    if (p.z < TOUCH_MIN_PRESSURE)
        return false;

    x = rawToScreenX(p.x);
    y = rawToScreenY(p.y);

    return true;
}

// ====================================================
// Touch Pressure
// ====================================================

inline int getTouchPressure()
{
    return touch.getPoint().z;
}

// ====================================================
// Button Helper
// ====================================================

inline bool touchInRect(
    int touchX,
    int touchY,
    int x,
    int y,
    int w,
    int h)
{
    return (
        touchX >= x &&
        touchX <= (x + w) &&
        touchY >= y &&
        touchY <= (y + h));
}

// ====================================================
// Navigation Buttons
// ====================================================

inline bool navPressed1(int x, int y)
{
    return touchInRect(
        x,
        y,
        NAV_X1,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT);
}

inline bool navPressed2(int x, int y)
{
    return touchInRect(
        x,
        y,
        NAV_X2,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT);
}

inline bool navPressed3(int x, int y)
{
    return touchInRect(
        x,
        y,
        NAV_X3,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT);
}

inline bool navPressed4(int x, int y)
{
    return touchInRect(
        x,
        y,
        NAV_X4,
        NAV_Y,
        NAV_WIDTH,
        NAV_HEIGHT);
}

#endif