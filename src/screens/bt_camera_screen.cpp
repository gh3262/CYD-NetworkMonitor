#include "app.h"

static const unsigned long SHUTTER_FEEDBACK_MS = 300;

static CameraSettings cameraSettings = DEFAULT_CAMERA_SETTINGS;

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
static bool shutterFeedbackActive = false;
static unsigned long shutterFeedbackStartedMs = 0;

const CameraSettings &btCameraSettings()
{
    return cameraSettings;
}

void setBtCameraSettings(const CameraSettings &settings)
{
    cameraSettings = settings;
}

static unsigned long delayCountdownMs()
{
    return static_cast<unsigned long>(cameraSettings.delaySeconds) * 1000UL;
}

static unsigned long seriesShotIntervalMs()
{
    return static_cast<unsigned long>(cameraSettings.secondsBetween) * 1000UL;
}

static void drawShutterButton()
{
    drawButton(
        20, 45, 120, 40, "Shutter",
        shutterFeedbackActive ? COLOR_BUTTON : COLOR_BUTTON_ALT,
        COLOR_BUTTON_TEXT);
}

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

void drawBtCameraScreen()
{
    if (activeSequence != HID_SEQUENCE_IDLE)
    {
        if (activeSequence == HID_SEQUENCE_SERIES_SHOTS)
        {
            char shotText[8];
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

    titleBar("BT CAMERA");

    drawShutterButton();
    drawButton(20, 90, 120, 40, "Delay", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);
    drawButton(20, 135, 120, 40, "Series", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);

    setBodyFont();
    gfx->setTextSize(1);

    gfx->setTextColor(COLOR_TEXT);
    char text[32];
    gfx->setCursor(150, 70);
    gfx->print("Single Shot");

    gfx->setCursor(150, 115);
    snprintf(text, sizeof(text), "%d Second Delay", cameraSettings.delaySeconds);
    gfx->print(text);

    gfx->setCursor(150, 160);
    snprintf(
        text,
        sizeof(text),
        "%d Shots %d Sec Apart",
        cameraSettings.shotCount,
        cameraSettings.secondsBetween);
    int16_t x1, y1;
    uint16_t width, height;
    gfx->getTextBounds(text, 150, 160, &x1, &y1, &width, &height);
    if (x1 + width > SCREEN_WIDTH)
        snprintf(
            text,
            sizeof(text),
            "%d Shots %ds Apart",
            cameraSettings.shotCount,
            cameraSettings.secondsBetween);
    gfx->print(text);

    drawNav1("Back");
    drawNav2("Settings");
    drawNav4("Home");
}

void enterBtCameraScreen()
{
    state.currentScreen = SCREEN_BT_CAMERA;
    drawCurrentScreen();
}

void handleBtCameraTouch(int x, int y)
{
    if (activeSequence != HID_SEQUENCE_IDLE)
        return;

    // Back button
    if (navPressed1(x, y))
    {
        enterBtHidScreen();
    }
    else if (navPressed2(x, y))
    {
        enterBtCameraSettingsScreen();
    }
    else if (touchInRect(x, y, 20, 45, 120, 40))
    {
        shutterFeedbackActive = bleSendVolumeUp();
        if (shutterFeedbackActive)
            shutterFeedbackStartedMs = millis();
        else
            Serial.println("BLE HID: volume-up report was not sent.");
        drawShutterButton();
    }
    else if (touchInRect(x, y, 20, 90, 120, 40))
    {
        activeSequence = HID_SEQUENCE_DELAY_COUNTDOWN;
        sequenceStartedMs = millis();
        displayedCountdownValue = cameraSettings.delaySeconds;
        drawCurrentScreen();
    }
    else if (touchInRect(x, y, 20, 135, 120, 40))
    {
        activeSequence = HID_SEQUENCE_SERIES_COUNTDOWN;
        sequenceStartedMs = millis();
        displayedCountdownValue = cameraSettings.delaySeconds;
        drawCurrentScreen();
    }
}

void updateBtCamera()
{
    const unsigned long now = millis();
    if (shutterFeedbackActive &&
        now - shutterFeedbackStartedMs >= SHUTTER_FEEDBACK_MS)
    {
        shutterFeedbackActive = false;
        if (state.currentScreen == SCREEN_BT_CAMERA &&
            activeSequence == HID_SEQUENCE_IDLE)
            drawShutterButton();
    }

    if (activeSequence == HID_SEQUENCE_IDLE)
        return;

    if (activeSequence == HID_SEQUENCE_DELAY_COUNTDOWN ||
        activeSequence == HID_SEQUENCE_SERIES_COUNTDOWN)
    {
        const unsigned long elapsed = now - sequenceStartedMs;
        if (elapsed >= delayCountdownMs())
        {
            if (activeSequence == HID_SEQUENCE_DELAY_COUNTDOWN)
            {
                activeSequence = HID_SEQUENCE_IDLE;
                if (!bleSendVolumeUp())
                    Serial.println("BLE HID: delayed volume-up report was not sent.");

                if (state.currentScreen == SCREEN_BT_CAMERA)
                    drawCurrentScreen();
                return;
            }

            activeSequence = HID_SEQUENCE_SERIES_SHOTS;
            displayedShotNumber = 1;
            if (!bleSendVolumeUp())
                Serial.println("BLE HID: series volume-up report was not sent.");
            nextSeriesShotMs = millis() + seriesShotIntervalMs();

            if (state.currentScreen == SCREEN_BT_CAMERA)
                drawCurrentScreen();
            return;
        }

        const int remainingSeconds =
            static_cast<int>((delayCountdownMs() - elapsed + 999) / 1000);
        if (remainingSeconds != displayedCountdownValue)
        {
            displayedCountdownValue = remainingSeconds;
            if (state.currentScreen == SCREEN_BT_CAMERA)
                drawCurrentScreen();
        }
        return;
    }

    if (now >= nextSeriesShotMs)
    {
        if (displayedShotNumber >= cameraSettings.shotCount)
        {
            activeSequence = HID_SEQUENCE_IDLE;
        }
        else
        {
            ++displayedShotNumber;
            if (!bleSendVolumeUp())
                Serial.println("BLE HID: series volume-up report was not sent.");
            nextSeriesShotMs = millis() + seriesShotIntervalMs();
        }

        if (state.currentScreen == SCREEN_BT_CAMERA)
            drawCurrentScreen();
    }
}

bool btCameraSequenceActive()
{
    return activeSequence != HID_SEQUENCE_IDLE;
}
