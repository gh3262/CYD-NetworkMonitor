#include "app.h"

void drawBtHidScreen()
{
    clearScreen(COLOR_BACKGROUND);
    titleBar("BLUETOOTH HID");

    drawButton(20, 45, 120, 40, "Camera", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);
    drawButton(20, 90, 120, 40, "Media", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);
    drawButton(20, 135, 120, 40, "KB", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);

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
    if (navPressed1(x, y))
    {
        enterBtLeScreen();
    }
    else if (touchInRect(x, y, 20, 45, 120, 40))
    {
        enterBtCameraScreen();
    }
    else if (touchInRect(x, y, 20, 90, 120, 40))
    {
        enterBtMediaScreen();
    }
    else if (touchInRect(x, y, 20, 135, 120, 40))
    {
        enterBtKbScreen();
    }
}

void refreshBtHidStatus(int x, int y, int w, int h)
{
    const char *status = "Disconnected";
    uint16_t color = COLOR_ERROR;

    if (bleState() == BT_UNAVAILABLE)
        status = "BT Unavailable";
    else if (!bleInitialized())
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
        default:
            break;
        }
    }

    gfx->fillRect(x, y, w, h, COLOR_BACKGROUND);
    gfx->setFont(FONT_SANS_SMALL);
    gfx->setTextSize(1);

    int16_t x1, y1;
    uint16_t tw, th;
    gfx->getTextBounds(status, 0, 0, &x1, &y1, &tw, &th);
    gfx->setTextColor(color);
    // Fixed baseline so strings with descenders don't shift vertically.
    gfx->setCursor(x + (w - (int)tw) / 2 - x1, y + 19);
    gfx->print(status);

    setDefaultFont();
    gfx->setTextSize(2);
}
