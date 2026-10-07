#include "app.h"

void drawBtHidScreen()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar("BLUETOOTH HID");

    centerText("NOT IMPLEMENTED", 120, COLOR_TEXT, 1);

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
    // Back button
    if (navPressed1(x, y))
    {
        enterBtLeScreen();
    }
}
