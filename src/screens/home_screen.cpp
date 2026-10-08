#include "app.h"

void refreshHomeTime()
{
    const int areaTop = HEADER_HEIGHT;
    const int areaBottom = NAV_Y;
    const int centerY = (areaTop + areaBottom) / 2;

    gfx->fillRect(
        0,
        areaTop,
        SCREEN_WIDTH,
        areaBottom - areaTop,
        COLOR_BACKGROUND);

    const char *time = currentTimeString();
    gfx->setFont(FONT_EXTRA_LARGE);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_TEXT);

    int16_t x1, y1;
    uint16_t width, height;
    gfx->getTextBounds(time, 0, 0, &x1, &y1, &width, &height);
    gfx->setCursor(
        (SCREEN_WIDTH - width) / 2 - x1,
        centerY - (height / 2) - y1);
    gfx->print(time);

    setDefaultFont();
    gfx->setTextSize(2);
}

void drawHomeScreen()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar(
        "HOME",
        COLOR_HEADER_BG,
        COLOR_HEADER_TEXT);

    refreshHomeTime();

    drawNav1("WiFi");
    drawNav2("BT LE");
    drawNav4("System");
}

void enterHomeScreen()
{
    state.currentScreen = SCREEN_HOME;
    drawCurrentScreen();
}

void handleHomeTouch(int x, int y)
{
    // WiFi button
    if (navPressed1(x, y))
    {
        enterWifiStatusScreen();
    }

    // BT LE button
    else if (navPressed2(x, y))
    {
        enterBtLeScreen();
    }

    // System button
    else if (navPressed4(x, y))
    {
        enterSystemScreen();
    }
}
