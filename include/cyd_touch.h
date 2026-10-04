#ifndef CYD_TOUCH_H
#define CYD_TOUCH_H

#include <SPI.h>
#include <XPT2046_Touchscreen.h>

#include "cyd_pins.h"

// ====================================================
// Touch Object
// ====================================================

static XPT2046_Touchscreen touch(TOUCH_CS);

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

inline bool touchAvailable()
{
    TS_Point p = touch.getPoint();
    return (p.z > 100);
}

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
            320),
        0,
        319);
}

inline int rawToScreenY(int rawY)
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

    if (p.z < 100)
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

#endif