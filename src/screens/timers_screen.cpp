#include "app.h"

static bool lastHeaderRunning = false;
static uint16_t headerBackground = COLOR_HEADER_BG;

static bool anyTimerRunning()
{
    return stopwatchRunning() || countdownRunning() || elapsedRunning();
}

void drawHeaderTimerIndicator(uint16_t bgColor)
{
    headerBackground = bgColor;
    lastHeaderRunning = anyTimerRunning();
    gfx->fillCircle(235, 15, 5,
                    lastHeaderRunning ? COLOR_BUTTON_ALT : headerBackground);
}

static void drawTimerIndicators(bool force)
{
    const bool running[] = {stopwatchRunning(), countdownRunning(), elapsedRunning()};
    static bool lastRunning[3] = {};

    for (unsigned i = 0; i < sizeof(running) / sizeof(running[0]); ++i)
    {
        if (force || running[i] != lastRunning[i])
        {
            gfx->fillCircle(250, 65 + 45 * i, 15,
                            running[i] ? COLOR_BUTTON_ALT : COLOR_BUTTON_OPT);
            lastRunning[i] = running[i];
        }
    }
}

void updateTimers()
{
    if ((state.currentScreen != SCREEN_BT_CAMERA || !btCameraSequenceActive()) &&
        anyTimerRunning() != lastHeaderRunning)
        drawHeaderTimerIndicator(headerBackground);

    if (state.currentScreen == SCREEN_TIMERS)
        drawTimerIndicators(false);
}

void drawTimersScreen()
{
    clearScreen(COLOR_BACKGROUND);
    titleBar("TIMERS");

    drawButton(20, 45, 200, 40, "Stop Watch", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);
    drawButton(20, 90, 200, 40, "Count Down", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);
    drawButton(20, 135, 200, 40, "Elapsed Time", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);
    drawTimerIndicators(true);

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
