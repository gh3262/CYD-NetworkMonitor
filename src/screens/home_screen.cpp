#include "app.h"

void drawHomeScreen()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar(
        "HOME",
        COLOR_HEADER_BG,
        COLOR_HEADER_TEXT);

    drawNav1("WiFi");
    drawNav2("BT LE");
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
}
