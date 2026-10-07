#include "app.h"

void drawSystemScreen()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar("SYSTEM");

    setBodyFont();
    gfx->setTextColor(COLOR_TEXT);

    gfx->setCursor(10, 59);
    gfx->print("Free Heap: ");
    gfx->print(ESP.getFreeHeap() / 1024);
    gfx->println(" KB");

    gfx->setCursor(10, 79);
    gfx->print("Sketch Size: ");
    gfx->print(ESP.getSketchSize() / 1024);
    gfx->println(" KB");

    gfx->setCursor(10, 99);
    gfx->print("Min Heap: ");
    gfx->print(ESP.getMinFreeHeap() / 1024);
    gfx->println(" KB");

    gfx->setCursor(10, 139);
    gfx->println("Uptime:");
    gfx->setCursor(20, 159);
    gfx->print(formatUptime());
    gfx->print("   ");
    gfx->println(startDateTimeString());

    drawNav1("Back");
    drawNav2("Tools");
    drawNav3("Refresh");
    drawNav4("Home");
}

void enterSystemScreen()
{
    state.currentScreen = SCREEN_SYSTEM;
    drawCurrentScreen();
}

void handleSystemTouch(int x, int y)
{
    // Back button
    if (navPressed1(x, y))
    {
        enterHomeScreen();
    }

    // Tools button
    else if (navPressed2(x, y))
    {
        enterToolsScreen();
    }

    // Refresh button
    else if (navPressed3(x, y))
    {
        drawCurrentScreen();
    }
}
