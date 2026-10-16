#include "app.h"
#include <esp_timer.h>

static const int MIN_SET_MINUTES = 1;
static const int MAX_SET_MINUTES = 240;
static const int64_t US_PER_MINUTE = 60000000;
static int setMinutes = 10;
static int64_t remainingUs = 10 * US_PER_MINUTE;
static int64_t deadlineUs = 0;
static bool running = false;
static int64_t lastBeepMinute = 10;
static int64_t lastDisplayedSeconds = -1;

struct CountdownButton
{
    int x, y, w, h;
    const char *label;
    uint16_t color;
    int minuteDelta;
};

static const CountdownButton SETTING_BUTTONS[] = {
    {10, 108, 60, 38, "+1", COLOR_BUTTON, 1},
    {78, 108, 60, 38, "+10", COLOR_BUTTON, 10},
    {10, 154, 60, 38, "-1", COLOR_BUTTON_ALT, -1},
    {78, 154, 60, 38, "-10", COLOR_BUTTON_ALT, -10},
};

static int64_t timeRemainingUs()
{
    if (!running)
        return remainingUs;

    const int64_t left = deadlineUs - esp_timer_get_time();
    return left > 0 ? left : 0;
}

static void formatTime(int64_t seconds, char *text, size_t size)
{
    snprintf(text, size, "%02u:%02u:%02u",
             static_cast<unsigned>(seconds / 3600),
             static_cast<unsigned>((seconds / 60) % 60),
             static_cast<unsigned>(seconds % 60));
}

static void drawMainTime(bool force)
{
    // Round up so a newly started minute still reads 00:01:00, not 00:00:59.
    const int64_t seconds = (timeRemainingUs() + 999999) / 1000000;
    if (!force && seconds == lastDisplayedSeconds)
        return;
    lastDisplayedSeconds = seconds;

    char text[16];
    formatTime(seconds, text, sizeof(text));
    gfx->fillRect(0, HEADER_HEIGHT, SCREEN_WIDTH, 48, COLOR_BACKGROUND);
    gfx->setFont(FONT_EXTRA_LARGE);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_TEXT);

    int16_t x1, y1;
    uint16_t w, h;
    gfx->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    gfx->setCursor((SCREEN_WIDTH - (int)w) / 2 - x1,
                   HEADER_HEIGHT + (48 - (int)h) / 2 - y1);
    gfx->print(text);
    setDefaultFont();
    gfx->setTextSize(2);
}

static void drawSetTime()
{
    char time[16];
    char text[24];
    formatTime(setMinutes * 60, time, sizeof(time));
    snprintf(text, sizeof(text), "Set: %s", time);
    gfx->fillRect(0, 80, SCREEN_WIDTH, 24, COLOR_BACKGROUND);
    setBodyFont();
    centerText(text, 82, COLOR_LABEL_TEXT, 1);
    setDefaultFont();
    gfx->setTextSize(2);
}

static void drawStartStop()
{
    drawButton(222, 108, 88, 84, running ? "Stop" : "Start",
               running ? COLOR_BUTTON_ALT : COLOR_BUTTON, COLOR_BUTTON_TEXT);
}

bool countdownRunning()
{
    return running;
}

void updateCountdown()
{
    const int64_t left = timeRemainingUs();
    if (running && left == 0)
    {
        running = false;
        remainingUs = 0;
        if (timerBeepsEnabled() && soundEnabled() && soundVolume() > 0 &&
            !playTimerSound(SOUND_SUCCESS, 10, 1000))
            Serial.println("Countdown timer: completion alarm could not be played.");

        if (state.currentScreen == SCREEN_COUNTDOWN)
            drawStartStop();
    }
    else if (running)
    {
        const int64_t minute = (left + US_PER_MINUTE - 1) / US_PER_MINUTE;
        if (minute < lastBeepMinute)
        {
            // Coalesce missed minute boundaries into a single notification.
            lastBeepMinute = minute;
            if (timerBeepsEnabled() && soundEnabled() && soundVolume() > 0 &&
                !playTimerSound(SOUND_BEEP))
                Serial.println("Countdown timer: minute beep could not be played.");
        }
    }

    if (state.currentScreen == SCREEN_COUNTDOWN)
        drawMainTime(false);
}

void drawCountdownScreen()
{
    clearScreen(COLOR_BACKGROUND);
    titleBar("Count Down");
    drawMainTime(true);
    drawSetTime();
    for (const CountdownButton &b : SETTING_BUTTONS)
        drawButton(b.x, b.y, b.w, b.h, b.label, b.color, COLOR_BUTTON_TEXT);
    drawButton(146, 131, 68, 38, "Set", COLOR_BUTTON_OPT, COLOR_BUTTON_TEXT);
    drawStartStop();

    drawNav1("Back");
    drawTimerBeepsButton();
    drawNav4("Home");
}

void enterCountdownScreen()
{
    state.currentScreen = SCREEN_COUNTDOWN;
    drawCurrentScreen();
}

void handleCountdownTouch(int x, int y)
{
    if (handleTimerBeepsTouch(x, y))
        return;

    if (navPressed1(x, y))
    {
        playTimerButtonBeep();
        enterTimersScreen();
        return;
    }

    for (const CountdownButton &b : SETTING_BUTTONS)
    {
        if (!touchInRect(x, y, b.x, b.y, b.w, b.h))
            continue;

        playTimerButtonBeep();
        setMinutes = constrain(setMinutes + b.minuteDelta, MIN_SET_MINUTES, MAX_SET_MINUTES);
        drawSetTime();
        return;
    }

    if (touchInRect(x, y, 146, 131, 68, 38))
    {
        playTimerButtonBeep();
        running = false;
        remainingUs = setMinutes * US_PER_MINUTE;
        lastBeepMinute = setMinutes;
        drawMainTime(true);
        drawStartStop();
    }
    else if (touchInRect(x, y, 222, 108, 88, 84))
    {
        playTimerButtonBeep();
        if (running)
        {
            remainingUs = timeRemainingUs();
            running = false;
        }
        else if (remainingUs > 0)
        {
            deadlineUs = esp_timer_get_time() + remainingUs;
            running = true;
        }
        drawMainTime(true);
        drawStartStop();
    }
}
