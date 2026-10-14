#include "app.h"

static const uint16_t COLOR_DISABLED = 0x7BEF;

void drawBtHidScreen()
{
    clearScreen(COLOR_BACKGROUND);
    titleBar("BLUETOOTH HID");

    drawButton(20, 45, 120, 40, "Camera", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);
    drawButton(20, 90, 120, 40, "Media", COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);
    drawButton(20, 135, 120, 40, "tbd", COLOR_DISABLED, CYD_BLACK);

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
}
