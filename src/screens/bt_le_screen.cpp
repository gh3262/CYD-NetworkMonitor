#include "app.h"

static const uint16_t COLOR_DISABLED = 0x7BEF;

static void drawStatusRows()
{
    const char *status = "Disconnected";
    uint16_t statusColor = COLOR_ERROR;

    switch (bleState())
    {
    case BT_ADVERTISING:
        status = "Advertising...";
        statusColor = COLOR_WARNING;
        break;
    case BT_PAIRING:
        status = "Pairing...";
        statusColor = COLOR_WARNING;
        break;
    case BT_CONNECTED:
        status = "Connected";
        statusColor = COLOR_OK;
        break;
    default:
        break;
    }

    if (bleState() == BT_UNAVAILABLE)
    {
        status = "Bluetooth unavailable";
        statusColor = COLOR_ERROR;
    }
    else if (!bleInitialized())
    {
        status = "Starting BLE...";
        statusColor = COLOR_WARNING;
    }

    gfx->fillRect(0, 50, SCREEN_WIDTH, 100, COLOR_BACKGROUND);
    setBodyFont();

    gfx->setCursor(10, 69);
    gfx->setTextColor(COLOR_LABEL_TEXT);
    gfx->print("Status:");
    gfx->setTextColor(statusColor);
    gfx->setCursor(30, 89);
    gfx->print(status);

    gfx->setCursor(10, 119);
    gfx->setTextColor(COLOR_LABEL_TEXT);
    gfx->print("Device:");
    gfx->setTextColor(COLOR_TEXT);
    gfx->setCursor(30, 139);
    gfx->print(bleHasBond() ? bleDeviceName().c_str() : "None");

    setDefaultFont();
    gfx->setTextSize(2);
}

static void drawButtons()
{
    const char *primary = bleHasBond() ? "Connect" : "Pair";

    switch (bleState())
    {
    case BT_ADVERTISING: primary = "Stop";       break;
    case BT_PAIRING:     primary = "Cancel";     break;
    case BT_CONNECTED:   primary = "Unlink"; break;
    default:             break;
    }

    if (bleInitialized())
        drawNav1(primary);
    else
        drawButton(NAV_X1, NAV_Y, NAV_WIDTH, NAV_HEIGHT, primary, COLOR_DISABLED, CYD_BLACK);

    if (bleHasBond())
        drawNav2("Forget");
    else
        drawButton(NAV_X2, NAV_Y, NAV_WIDTH, NAV_HEIGHT, "Forget", COLOR_DISABLED, CYD_BLACK);

    if (bleState() == BT_CONNECTED)
        drawNav3("HID");
    else
        drawButton(NAV_X3, NAV_Y, NAV_WIDTH, NAV_HEIGHT, "HID", COLOR_DISABLED, CYD_BLACK);
    drawNav4("Home");
}

void drawBtLeScreen()
{
    clearScreen(COLOR_BACKGROUND);
    titleBar("BT LE");
    drawStatusRows();
    drawButtons();
}

void enterBtLeScreen()
{
    state.currentScreen = SCREEN_BT_LE;
    drawCurrentScreen();
}

void handleBtLeTouch(int x, int y)
{
    // Pair / Connect / Stop / Cancel / Disconnect
    if (navPressed1(x, y))
    {
        if (!bleInitialized())
            return;

        switch (bleState())
        {
        case BT_DISCONNECTED:
            if (bleHasBond())
                bleStartAdvertising();
            else
                bleStartPairing();
            break;
        case BT_CONNECTED:
            bleDisconnect();
            break;
        default:
            bleStop();
            break;
        }
        drawCurrentScreen();
    }

    // Forget
    else if (navPressed2(x, y))
    {
        if (bleInitialized() && bleHasBond())
        {
            bleForgetBond();
            drawCurrentScreen();
        }
    }

    // HID button
    else if (navPressed3(x, y))
    {
        if (bleState() == BT_CONNECTED)
            enterBtHidScreen();
    }
}

void updateBtLe()
{
    if (bleUpdate() && state.currentScreen == SCREEN_BT_LE)
        drawCurrentScreen();
}
