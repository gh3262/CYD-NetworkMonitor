#include "app.h"

void drawHomeScreen()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar(
        "HOME",
        COLOR_HEADER_BG,
        COLOR_HEADER_TEXT);

    drawNav1("WiFi");
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
        delay(300);
    }
}
