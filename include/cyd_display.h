#ifndef CYD_DISPLAY_H
#define CYD_DISPLAY_H
#include <Arduino_GFX_Library.h>
#include "cyd_pins.h"
#include "cyd_helpers.h"
#include "cyd_theme.h"
// ====================================================
// Display Objects
// ====================================================
static Arduino_DataBus *displayBus =
    new Arduino_ESP32SPI(
        TFT_DC,
        TFT_CS,
        TFT_SCK,
        TFT_MOSI,
        TFT_MISO);
static Arduino_GFX *gfx =
    new Arduino_ILI9341(
        displayBus,
        TFT_RST,
        1);
// ====================================================
// Initialization
// ====================================================
inline bool initDisplay()
{
    backlightInit();
    if (!gfx->begin())
        return false;

    gfx->invertDisplay(true);

    return true;
}
// ====================================================
// Convenience Functions
// ====================================================
inline void clearScreen(uint16_t color = COLOR_BACKGROUND)
{
    gfx->fillScreen(color);
}
inline void titleBar(
    const char *title,
    uint16_t bgColor = COLOR_HEADER_BG,
    uint16_t textColor = COLOR_HEADER_TEXT)
{
    gfx->fillRect(0, 0, 320, 30, bgColor);
    gfx->setTextColor(textColor);
    gfx->setTextSize(2);
    gfx->setCursor(5, 8);
    gfx->print(title);
}
inline void centerText(
    const char *text,
    int y,
    uint16_t color = COLOR_TEXT,
    int textSize = 2)
{
    gfx->setTextSize(textSize);
    gfx->setTextColor(color);
    int16_t x1, y1;
    uint16_t w, h;
    gfx->getTextBounds(
        text,
        0,
        0,
        &x1,
        &y1,
        &w,
        &h);
    int x = (320 - w) / 2;
    gfx->setCursor(x, y);
    gfx->print(text);
}
// ====================================================
// Simple Button
// ====================================================
inline void drawButton(
    int x,
    int y,
    int w,
    int h,
    const char *label,
    uint16_t buttonColor = COLOR_BUTTON,
    uint16_t textColor = COLOR_BUTTON_TEXT)
{
    gfx->fillRoundRect(
        x,
        y,
        w,
        h,
        6,
        buttonColor);
    gfx->setTextColor(textColor);
    gfx->setTextSize(2);
    int16_t x1, y1;
    uint16_t tw, th;
    gfx->getTextBounds(
        label,
        0,
        0,
        &x1,
        &y1,
        &tw,
        &th);
    gfx->setCursor(
        x + (w - tw) / 2,
        y + (h - th) / 2 + 2);
    gfx->print(label);
}
inline bool pointInButton(
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
