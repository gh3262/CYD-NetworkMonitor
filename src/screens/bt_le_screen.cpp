#include "app.h"

// Placeholder BLE state; real NimBLE calls replace the fake transitions later
static BtState btState = BT_DISCONNECTED;
static char bondedName[24] = "";
static unsigned long pairingStartMs = 0;

static const unsigned long FAKE_PAIRING_MS = 3000;
static const char *const FAKE_DEVICE_NAME = "Pixel 10";
static const uint16_t COLOR_DISABLED = 0x7BEF;

BtState btLeState()
{
    return btState;
}

static bool hasBond()
{
    return bondedName[0] != '\0';
}

static void drawStatusRows()
{
    const char *status = "Disconnected";
    uint16_t statusColor = COLOR_ERROR;

    if (btState == BT_PAIRING)
    {
        status = "Pairing...";
        statusColor = COLOR_WARNING;
    }
    else if (btState == BT_CONNECTED)
    {
        status = "Connected";
        statusColor = COLOR_OK;
    }

    gfx->fillRect(0, 50, SCREEN_WIDTH, 100, COLOR_BACKGROUND);
    setBodyFont();

    gfx->setTextColor(COLOR_TEXT);
    gfx->setCursor(10, 69);
    gfx->print("Status:");
    gfx->setTextColor(statusColor);
    gfx->setCursor(30, 89);
    gfx->print(status);

    gfx->setTextColor(COLOR_TEXT);
    gfx->setCursor(10, 119);
    gfx->print("Device:");
    gfx->setCursor(30, 139);
    gfx->print(hasBond() ? bondedName : "None");

    setDefaultFont();
    gfx->setTextSize(2);
}

static void drawButtons()
{
    const char *primary = "Pair";

    if (btState == BT_PAIRING)
        primary = "Cancel";
    else if (btState == BT_CONNECTED)
        primary = "Disconnect";

    if (btState == BT_DISCONNECTED && hasBond())
        drawButton(NAV_X1, NAV_Y, NAV_WIDTH, NAV_HEIGHT, "Pair", COLOR_DISABLED, CYD_BLACK);
    else
        drawNav1(primary);

    if (hasBond())
        drawNav2("Forget");
    else
        drawButton(NAV_X2, NAV_Y, NAV_WIDTH, NAV_HEIGHT, "Forget", COLOR_DISABLED, CYD_BLACK);

    drawNav3("HID");
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
    // Pair / Cancel / Disconnect
    if (navPressed1(x, y))
    {
        if (btState == BT_DISCONNECTED)
        {
            // One device at a time: Forget must come first
            if (hasBond())
                return;

            btState = BT_PAIRING;
            pairingStartMs = millis();
        }
        else
        {
            btState = BT_DISCONNECTED;
        }
        drawCurrentScreen();
    }

    // Forget
    else if (navPressed2(x, y))
    {
        if (hasBond())
        {
            bondedName[0] = '\0';
            btState = BT_DISCONNECTED;
            drawCurrentScreen();
        }
    }

    // BT HID button
    else if (navPressed3(x, y))
    {
        enterBtHidScreen();
    }
}

// Called from loop(): completes the fake pairing after a delay
void updateBtLe()
{
    if (btState == BT_PAIRING && millis() - pairingStartMs > FAKE_PAIRING_MS)
    {
        btState = BT_CONNECTED;
        strncpy(bondedName, FAKE_DEVICE_NAME, sizeof(bondedName) - 1);

        if (state.currentScreen == SCREEN_BT_LE)
            drawCurrentScreen();
    }
}
