#include "app.h"

void drawBtLeScreen()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar("BT LE");

    centerText("NOT IMPLEMENTED", 120, COLOR_TEXT, 1);

    drawNav1("Back");
    drawNav2("HID");
    drawNav4("Home");
}

void enterBtLeScreen()
{
    state.currentScreen = SCREEN_BT_LE;
    drawCurrentScreen();
}

void handleBtLeTouch(int x, int y)
{
    // Back button
    if (navPressed1(x, y))
    {
        enterHomeScreen();
    }

    // HID button
    else if (navPressed2(x, y))
    {
        enterBtHidScreen();
    }
}
