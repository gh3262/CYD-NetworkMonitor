#include "app.h"

void drawTimersScreen()
{
    clearScreen(COLOR_BACKGROUND);
    titleBar("TIMERS");

    drawButton(20, 45, 200, 40, "Stop Watch", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);
    drawButton(20, 90, 200, 40, "Count Down", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);
    drawButton(20, 135, 200, 40, "Elapsed Time", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);

    drawNav1("Back");
    drawNav4("Home");
}

void enterTimersScreen()
{
    state.currentScreen = SCREEN_TIMERS;
    drawCurrentScreen();
}

void handleTimersTouch(int x, int y)
{
    if (navPressed1(x, y))
    {
        enterHomeScreen();
    }
    else if (touchInRect(x, y, 20, 45, 200, 40))
    {
        enterStopwatchScreen();
    }
    else if (touchInRect(x, y, 20, 90, 200, 40))
    {
        enterCountdownScreen();
    }
    else if (touchInRect(x, y, 20, 135, 200, 40))
    {
        enterElapsedScreen();
    }
}
