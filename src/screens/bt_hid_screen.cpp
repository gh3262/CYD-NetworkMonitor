#include "app.h"

static const unsigned long DELAY_COUNTDOWN_MS = 5000;
static const unsigned long SERIES_SHOT_INTERVAL_MS = 2000;
static const int SERIES_SHOT_COUNT = 5;

enum HidSequence
{
    HID_SEQUENCE_IDLE,
    HID_SEQUENCE_DELAY_COUNTDOWN,
    HID_SEQUENCE_SERIES_COUNTDOWN,
    HID_SEQUENCE_SERIES_SHOTS
};

static HidSequence activeSequence = HID_SEQUENCE_IDLE;
static unsigned long sequenceStartedMs = 0;
static unsigned long nextSeriesShotMs = 0;
static int displayedCountdownValue = 0;
static int displayedShotNumber = 0;

static void drawSequenceStatus(const char *text)
{
    clearScreen(COLOR_BACKGROUND);

    gfx->setFont(FONT_EXTRA_LARGE);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_TEXT);

    int16_t x1, y1;
    uint16_t width, height;
    gfx->getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
    gfx->setCursor(
        (SCREEN_WIDTH - width) / 2 - x1,
        (SCREEN_HEIGHT - height) / 2 - y1);
    gfx->print(text);

    setDefaultFont();
    gfx->setTextSize(2);
}

void drawBtHidScreen()
{
    if (activeSequence != HID_SEQUENCE_IDLE)
    {
        if (activeSequence == HID_SEQUENCE_SERIES_SHOTS)
        {
            char shotText[5];
            snprintf(shotText, sizeof(shotText), "# %d", displayedShotNumber);
            drawSequenceStatus(shotText);
        }
        else
        {
            char countdownText[12];
            snprintf(
                countdownText,
                sizeof(countdownText),
                "%d",
                displayedCountdownValue);
            drawSequenceStatus(countdownText);
        }
        return;
    }

    clearScreen(COLOR_BACKGROUND);

    titleBar("BLUETOOTH HID");

    drawButton(20, 45, 120, 40, "Shutter", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);
    drawButton(20, 90, 120, 40, "Delay", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);
    drawButton(20, 135, 120, 40, "Series", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);

    setBodyFont();
    gfx->setTextSize(1);

    gfx->setTextColor(COLOR_TEXT);
    gfx->setCursor(150, 70);
    gfx->print("Single Shot");

    gfx->setCursor(150, 115);
    gfx->print("5 Second Delay");

    gfx->setCursor(150, 160);
    gfx->print("5 Shots 2 Sec Apart");

    drawNav1("Back");
    drawNav4("Home");
}

void enterBtHidScreen()
{
    state.currentScreen = SCREEN_BT_HID;
    drawCurrentScreen();
}

void handleBtHidTouch(int x, int y)
{
    if (activeSequence != HID_SEQUENCE_IDLE)
        return;

    // Back button
    if (navPressed1(x, y))
    {
        enterBtLeScreen();
    }
    else if (touchInRect(x, y, 20, 45, 120, 40))
    {
        if (!bleSendVolumeUp())
            Serial.println("BLE HID: volume-up report was not sent.");
    }
    else if (touchInRect(x, y, 20, 90, 120, 40))
    {
        activeSequence = HID_SEQUENCE_DELAY_COUNTDOWN;
        sequenceStartedMs = millis();
        displayedCountdownValue = 5;
        drawCurrentScreen();
    }
    else if (touchInRect(x, y, 20, 135, 120, 40))
    {
        activeSequence = HID_SEQUENCE_SERIES_COUNTDOWN;
        sequenceStartedMs = millis();
        displayedCountdownValue = 5;
        drawCurrentScreen();
    }
}

void updateBtHid()
{
    if (activeSequence == HID_SEQUENCE_IDLE)
        return;

    const unsigned long now = millis();
    if (activeSequence == HID_SEQUENCE_DELAY_COUNTDOWN ||
        activeSequence == HID_SEQUENCE_SERIES_COUNTDOWN)
    {
        const unsigned long elapsed = now - sequenceStartedMs;
        if (elapsed >= DELAY_COUNTDOWN_MS)
        {
            if (activeSequence == HID_SEQUENCE_DELAY_COUNTDOWN)
            {
                activeSequence = HID_SEQUENCE_IDLE;
                if (!bleSendVolumeUp())
                    Serial.println("BLE HID: delayed volume-up report was not sent.");

                if (state.currentScreen == SCREEN_BT_HID)
                    drawCurrentScreen();
                return;
            }

            activeSequence = HID_SEQUENCE_SERIES_SHOTS;
            displayedShotNumber = 1;
            if (!bleSendVolumeUp())
                Serial.println("BLE HID: series volume-up report was not sent.");
            nextSeriesShotMs = millis() + SERIES_SHOT_INTERVAL_MS;

            if (state.currentScreen == SCREEN_BT_HID)
                drawCurrentScreen();
            return;
        }

        const int remainingSeconds =
            static_cast<int>((DELAY_COUNTDOWN_MS - elapsed + 999) / 1000);
        if (remainingSeconds != displayedCountdownValue)
        {
            displayedCountdownValue = remainingSeconds;
            if (state.currentScreen == SCREEN_BT_HID)
                drawCurrentScreen();
        }
        return;
    }

    if (now >= nextSeriesShotMs)
    {
        if (displayedShotNumber == SERIES_SHOT_COUNT)
        {
            activeSequence = HID_SEQUENCE_IDLE;
        }
        else
        {
            ++displayedShotNumber;
            if (!bleSendVolumeUp())
                Serial.println("BLE HID: series volume-up report was not sent.");
            nextSeriesShotMs = millis() + SERIES_SHOT_INTERVAL_MS;
        }

        if (state.currentScreen == SCREEN_BT_HID)
            drawCurrentScreen();
    }
}

bool btHidSequenceActive()
{
    return activeSequence != HID_SEQUENCE_IDLE;
}
