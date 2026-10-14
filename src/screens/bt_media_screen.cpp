#include "app.h"

struct MediaButton
{
    int x, y, w, h;
    const char *label;
    BleConsumerKey key;
    uint16_t color;
};

// Left two-thirds: Play/Pause over Previous | Next, then the status line.
// Right third: Volume Up over Volume Down.
static const MediaButton MEDIA_BUTTONS[] = {
    {10,  40,  193, 71, "Play/Pause", BLE_KEY_PLAY_PAUSE,  COLOR_BUTTON},
    {10,  116, 94,  52, "Previous",   BLE_KEY_PREVIOUS,    COLOR_BUTTON_ALT},
    {109, 116, 94,  52, "Next",       BLE_KEY_NEXT,        COLOR_BUTTON_ALT},
    {220, 40,  90,  74, "Vol Up",     BLE_KEY_VOLUME_UP,   COLOR_BUTTON_ALT},
    {220, 118, 90,  74, "Vol Down",   BLE_KEY_VOLUME_DOWN, COLOR_BUTTON_ALT},
};

static const int MEDIA_BUTTON_COUNT = sizeof(MEDIA_BUTTONS) / sizeof(MEDIA_BUTTONS[0]);

// Status line below Previous | Next, centered under the left-side buttons.
static const int STATUS_X = 10;
static const int STATUS_Y = 170;
static const int STATUS_W = 193;
static const int STATUS_H = NAV_Y - STATUS_Y - 2;

void refreshBtMediaStatus()
{
    const char *status = "Disconnected";
    uint16_t color = COLOR_ERROR;

    if (!bleInitialized())
    {
        status = "Starting BLE";
        color = COLOR_WARNING;
    }
    else
    {
        switch (bleState())
        {
        case BT_CONNECTED:
            status = "Connected";
            color = COLOR_OK;
            break;
        case BT_ADVERTISING:
            status = "Reconnecting...";
            color = COLOR_WARNING;
            break;
        case BT_PAIRING:
            status = "Pairing...";
            color = COLOR_WARNING;
            break;
        case BT_UNAVAILABLE:
            status = "BT Unavailable";
            break;
        default:
            break;
        }
    }

    gfx->fillRect(STATUS_X, STATUS_Y, STATUS_W, STATUS_H, COLOR_BACKGROUND);
    gfx->setFont(FONT_SANS_SMALL);
    // Custom GFX fonts are multiplied by text size; other helpers leave it at 2.
    gfx->setTextSize(1);

    int16_t x1, y1;
    uint16_t w, h;
    gfx->getTextBounds(status, 0, 0, &x1, &y1, &w, &h);
    gfx->setTextColor(color);
    // Fixed baseline so strings with descenders don't shift vertically.
    gfx->setCursor(STATUS_X + (STATUS_W - (int)w) / 2 - x1, STATUS_Y + 19);
    gfx->print(status);

    setDefaultFont();
    gfx->setTextSize(2);
}

void drawBtMediaScreen()
{
    clearScreen(COLOR_BACKGROUND);
    titleBar("Media Control");

    for (int i = 0; i < MEDIA_BUTTON_COUNT; ++i)
    {
        const MediaButton &b = MEDIA_BUTTONS[i];
        drawButton(b.x, b.y, b.w, b.h, b.label, b.color, COLOR_BUTTON_TEXT);
    }

    drawNav1("Back");
    drawNav4("Home");
    refreshBtMediaStatus();
}

void enterBtMediaScreen()
{
    state.currentScreen = SCREEN_BT_MEDIA;
    drawCurrentScreen();
}

void handleBtMediaTouch(int x, int y)
{
    if (navPressed1(x, y))
    {
        enterBtHidScreen();
        return;
    }

    for (int i = 0; i < MEDIA_BUTTON_COUNT; ++i)
    {
        const MediaButton &b = MEDIA_BUTTONS[i];
        if (!touchInRect(x, y, b.x, b.y, b.w, b.h))
            continue;

        if (bleSendConsumerKey(b.key))
            playSound(SOUND_CLICK);
        else
            Serial.printf("BLE HID: %s report was not sent.\n", b.label);
        return;
    }
}
