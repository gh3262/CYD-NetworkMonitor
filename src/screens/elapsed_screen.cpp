#include "app.h"
#include <esp_timer.h>

static bool running = false;
static int64_t accumulatedUs = 0;
static int64_t startedUs = 0;
static int64_t lastBeepMinute = 0;
static char lastDisplayed[32] = {};

struct ElapsedButton
{
    int x, y, w, h;
    const char *label;
    uint16_t color;
};

static const ElapsedButton ELAPSED_BUTTONS[] = {
    {10, 100, 300, 42, "Start", COLOR_BUTTON},
    {165, 150, 145, 42, "Reset", COLOR_BUTTON_OPT},
};

static int64_t elapsedUs()
{
    return accumulatedUs + (running ? esp_timer_get_time() - startedUs : 0);
}

static void drawStartStop()
{
    const ElapsedButton &b = ELAPSED_BUTTONS[0];
    drawButton(b.x, b.y, b.w, b.h, running ? "Stop" : "Start",
               running ? COLOR_BUTTON_ALT : COLOR_BUTTON, COLOR_BUTTON_TEXT);
}

static void drawElapsedTime(bool force)
{
    const int64_t seconds = elapsedUs() / 1000000;
    char text[32];
    snprintf(text, sizeof(text), "%02llu:%02u:%02u",
             static_cast<unsigned long long>(seconds / 3600),
             static_cast<unsigned>((seconds / 60) % 60),
             static_cast<unsigned>(seconds % 60));

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

bool elapsedRunning()
{
    return running;
}

void updateElapsed()
{
    const int64_t minute = elapsedUs() / 60000000;
    if (running && minute > lastBeepMinute)
    {
        // Coalesce missed boundaries if the main loop was temporarily blocked.
        lastBeepMinute = minute;
        if (timerBeepsEnabled() && soundEnabled() && soundVolume() > 0 &&
            !playTimerSound(SOUND_BEEP))
            Serial.println("Elapsed timer: minute beep could not be played.");
    }

    if (state.currentScreen == SCREEN_ELAPSED)
        drawElapsedTime(false);
}

void drawElapsedScreen()
{
    clearScreen(COLOR_BACKGROUND);
    titleBar("Elapsed Time");
    drawElapsedTime(true);

    drawStartStop();
    for (unsigned i = 1; i < sizeof(ELAPSED_BUTTONS) / sizeof(ELAPSED_BUTTONS[0]); ++i)
    {
        const ElapsedButton &b = ELAPSED_BUTTONS[i];
        drawButton(b.x, b.y, b.w, b.h, b.label, b.color, COLOR_BUTTON_TEXT);
    }

    drawNav1("Back");
    drawTimerBeepsButton();
    drawNav4("Home");
}

void enterElapsedScreen()
{
    state.currentScreen = SCREEN_ELAPSED;
    drawCurrentScreen();
}

void handleElapsedTouch(int x, int y)
{
    if (handleTimerBeepsTouch(x, y))
        return;

    if (navPressed1(x, y))
    {
        playTimerButtonBeep();
        enterTimersScreen();
        return;
    }

    for (unsigned i = 0; i < sizeof(ELAPSED_BUTTONS) / sizeof(ELAPSED_BUTTONS[0]); ++i)
    {
        const ElapsedButton &b = ELAPSED_BUTTONS[i];
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
            }
            else
            {
                startedUs = esp_timer_get_time();
                running = true;
            }
            break;
        case 1:
            running = false;
            accumulatedUs = 0;
            lastBeepMinute = 0;
            break;
        }

        drawElapsedTime(true);
        drawStartStop();
        return;
    }
}
