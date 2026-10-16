#include "app.h"
#include <esp_timer.h>

static bool running = false;
static bool lapFrozen = false;
static int64_t accumulatedUs = 0;
static int64_t startedUs = 0;
static int64_t lapUs = 0;
static char lastDisplayed[32] = {};

struct StopwatchButton
{
    int x, y, w, h;
    const char *label;
    uint16_t color;
};

static const StopwatchButton STOPWATCH_BUTTONS[] = {
    {10, 100, 300, 42, "Start", COLOR_BUTTON},
    {10, 150, 145, 42, "Lap", COLOR_BUTTON_OPT},
    {165, 150, 145, 42, "Reset", COLOR_BUTTON_OPT},
};

static int64_t elapsedUs()
{
    return accumulatedUs + (running ? esp_timer_get_time() - startedUs : 0);
}

static void drawStartStop()
{
    const StopwatchButton &b = STOPWATCH_BUTTONS[0];
    drawButton(b.x, b.y, b.w, b.h, running ? "Stop" : "Start",
               running ? COLOR_BUTTON_ALT : COLOR_BUTTON, COLOR_BUTTON_TEXT);
}

static void drawStopwatchTime(bool force)
{
    const int64_t hundredths = (lapFrozen ? lapUs : elapsedUs()) / 10000;
    char text[32];
    snprintf(text, sizeof(text), "%02llu:%02u:%02u",
             static_cast<unsigned long long>(hundredths / 6000),
             static_cast<unsigned>((hundredths / 100) % 60),
             static_cast<unsigned>(hundredths % 100));

    if (!force && strcmp(text, lastDisplayed) == 0)
        return;
    strcpy(lastDisplayed, text);

    gfx->fillRect(0, HEADER_HEIGHT, SCREEN_WIDTH, 66, COLOR_BACKGROUND);
    gfx->setFont(FONT_EXTRA_LARGE);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_TEXT);

    int16_t x1, y1;
    uint16_t width, height;
    gfx->getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
    gfx->setCursor(
        (SCREEN_WIDTH - (int)width) / 2 - x1,
        HEADER_HEIGHT + (66 - (int)height) / 2 - y1);
    gfx->print(text);

    setDefaultFont();
    gfx->setTextSize(2);
}

bool stopwatchRunning()
{
    return running;
}

void updateStopwatch()
{
    if (state.currentScreen == SCREEN_STOPWATCH)
        drawStopwatchTime(false);
}

void drawStopwatchScreen()
{
    clearScreen(COLOR_BACKGROUND);
    titleBar("Stop Watch");
    drawStopwatchTime(true);

    drawStartStop();
    for (unsigned i = 1; i < sizeof(STOPWATCH_BUTTONS) / sizeof(STOPWATCH_BUTTONS[0]); ++i)
    {
        const StopwatchButton &b = STOPWATCH_BUTTONS[i];
        drawButton(b.x, b.y, b.w, b.h, b.label, b.color, COLOR_BUTTON_TEXT);
    }

    drawNav1("Back");
    drawTimerBeepsButton();
    drawNav4("Home");
}

void enterStopwatchScreen()
{
    state.currentScreen = SCREEN_STOPWATCH;
    drawCurrentScreen();
}

void handleStopwatchTouch(int x, int y)
{
    if (handleTimerBeepsTouch(x, y))
        return;

    if (navPressed1(x, y))
    {
        playTimerButtonBeep();
        enterTimersScreen();
        return;
    }

    for (unsigned i = 0; i < sizeof(STOPWATCH_BUTTONS) / sizeof(STOPWATCH_BUTTONS[0]); ++i)
    {
        const StopwatchButton &b = STOPWATCH_BUTTONS[i];
        if (!touchInRect(x, y, b.x, b.y, b.w, b.h))
            continue;

        playTimerButtonBeep();
        switch (i)
        {
        case 0:
            if (running)
            {
                accumulatedUs = elapsedUs();
                running = false;
                lapFrozen = false;
            }
            else
            {
                startedUs = esp_timer_get_time();
                running = true;
            }
            break;
        case 1:
            if (running)
            {
                if (!lapFrozen)
                    lapUs = elapsedUs();
                lapFrozen = !lapFrozen;
            }
            break;
        case 2:
            running = false;
            lapFrozen = false;
            accumulatedUs = 0;
            lapUs = 0;
            break;
        }

        drawStopwatchTime(true);
        if (i != 1)
            drawStartStop();
        return;
    }
}
