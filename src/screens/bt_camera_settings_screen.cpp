#include "app.h"

static const int SETTING_COUNT = 3;
static const int ROW_Y[SETTING_COUNT] = {45, 95, 145};
static const int MINUS_X = 160;
static const int VALUE_X = 205;
static const int VALUE_WIDTH = 60;
static const int PLUS_X = 270;
static const int SQUARE_SIZE = 40;

static CameraSettings editedSettings = DEFAULT_CAMERA_SETTINGS;

static int *settingValue(int row)
{
    switch (row)
    {
    case 0: return &editedSettings.delaySeconds;
    case 1: return &editedSettings.shotCount;
    default: return &editedSettings.secondsBetween;
    }
}

static void drawValue(int row)
{
    char text[4];
    snprintf(text, sizeof(text), "%d", *settingValue(row));
    drawButton(VALUE_X, ROW_Y[row], VALUE_WIDTH, SQUARE_SIZE, text, COLOR_BACKGROUND, COLOR_TEXT);
    gfx->drawRoundRect(VALUE_X, ROW_Y[row], VALUE_WIDTH, SQUARE_SIZE, 8, COLOR_TEXT);
}

static void drawSettingRow(int row, const char *label)
{
    setBodyFont();
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_TEXT);
    gfx->setCursor(8, ROW_Y[row] + 26);
    gfx->print(label);

    drawButton(MINUS_X, ROW_Y[row], SQUARE_SIZE, SQUARE_SIZE, "-", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);
    drawValue(row);
    drawButton(PLUS_X, ROW_Y[row], SQUARE_SIZE, SQUARE_SIZE, "+", COLOR_BUTTON, COLOR_BUTTON_TEXT);
}

static void changeValue(int row, int delta)
{
    int *value = settingValue(row);
    const int updated = constrain(*value + delta, CAMERA_SETTING_MIN, CAMERA_SETTING_MAX);
    if (updated == *value)
        return;

    *value = updated;
    drawValue(row);
}

void drawBtCameraSettingsScreen()
{
    clearScreen(COLOR_BACKGROUND);
    titleBar("CAMERA SETTINGS");

    drawSettingRow(0, "Delay Seconds");
    drawSettingRow(1, "Nbr Shots");
    drawSettingRow(2, "Time Between");

    drawNav1("Back");
    drawNav2("SET");
    drawNav3("Reset");
    drawNav4("Home");
}

void enterBtCameraSettingsScreen()
{
    editedSettings = btCameraSettings();
    state.currentScreen = SCREEN_BT_CAMERA_SETTINGS;
    drawCurrentScreen();
}

void handleBtCameraSettingsTouch(int x, int y)
{
    if (navPressed1(x, y))
    {
        enterBtCameraScreen();
        return;
    }

    if (navPressed2(x, y))
    {
        setBtCameraSettings(editedSettings);
        enterBtCameraScreen();
        return;
    }

    if (navPressed3(x, y))
    {
        editedSettings = DEFAULT_CAMERA_SETTINGS;
        for (int row = 0; row < SETTING_COUNT; ++row)
            drawValue(row);
        return;
    }

    for (int row = 0; row < SETTING_COUNT; ++row)
    {
        if (touchInRect(x, y, MINUS_X, ROW_Y[row], SQUARE_SIZE, SQUARE_SIZE))
        {
            changeValue(row, -1);
            return;
        }
        if (touchInRect(x, y, PLUS_X, ROW_Y[row], SQUARE_SIZE, SQUARE_SIZE))
        {
            changeValue(row, 1);
            return;
        }
    }
}
