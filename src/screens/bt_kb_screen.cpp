#include "app.h"

struct KeyboardButton
{
    int x, y, w, h;
    const char *label;
    bool consumer;
    BleConsumerKey consumerKey;
    BleKeyboardKey keyboardKey;
    BleKeyboardModifier modifier;
};

static const KeyboardButton KEYBOARD_BUTTONS[] = {
    {6,   34, 96, 36, "Vol Up",   true,  BLE_KEY_VOLUME_UP,   BLE_KEY_TAB,         BLE_MOD_NONE},
    {112, 34, 96, 36, "Vol Down", true,  BLE_KEY_VOLUME_DOWN, BLE_KEY_TAB,         BLE_MOD_NONE},
    {218, 34, 96, 36, "Tab",      false, BLE_KEY_VOLUME_UP,   BLE_KEY_TAB,         BLE_MOD_NONE},
    {6,   76, 96, 36, "All",      false, BLE_KEY_VOLUME_UP,   BLE_KEY_A,           BLE_MOD_LEFT_CTRL},
    {112, 76, 96, 36, "Copy",     false, BLE_KEY_VOLUME_UP,   BLE_KEY_C,           BLE_MOD_LEFT_CTRL},
    {218, 76, 96, 36, "Paste",    false, BLE_KEY_VOLUME_UP,   BLE_KEY_V,           BLE_MOD_LEFT_CTRL},
    {6,  118, 96, 36, "Left",     false, BLE_KEY_VOLUME_UP,   BLE_KEY_LEFT_ARROW,  BLE_MOD_NONE},
    {112,118, 96, 36, "Right",    false, BLE_KEY_VOLUME_UP,   BLE_KEY_RIGHT_ARROW, BLE_MOD_NONE},
    {218,118, 96, 36, "Pg Up",    false, BLE_KEY_VOLUME_UP,   BLE_KEY_PAGE_UP,     BLE_MOD_NONE},
    {6,  160, 96, 36, "Pg Down",  false, BLE_KEY_VOLUME_UP,   BLE_KEY_PAGE_DOWN,   BLE_MOD_NONE},
};

void refreshBtKbStatus()
{
    refreshBtHidStatus(112, 168, 202, 28);
}

void drawBtKbScreen()
{
    clearScreen(COLOR_BACKGROUND);
    titleBar("KB Control");

    for (const KeyboardButton &b : KEYBOARD_BUTTONS)
        drawButton(b.x, b.y, b.w, b.h, b.label, COLOR_BUTTON_ALT, COLOR_BUTTON_TEXT);

    drawNav1("Back");
    drawNav4("Home");
    refreshBtKbStatus();
}

void enterBtKbScreen()
{
    state.currentScreen = SCREEN_BT_KB;
    drawCurrentScreen();
}

void handleBtKbTouch(int x, int y)
{
    if (navPressed1(x, y))
    {
        enterBtHidScreen();
        return;
    }

    for (const KeyboardButton &b : KEYBOARD_BUTTONS)
    {
        if (!touchInRect(x, y, b.x, b.y, b.w, b.h))
            continue;

        const bool sent = b.consumer ? bleSendConsumerKey(b.consumerKey)
                                     : bleSendKeyboardKey(b.keyboardKey, b.modifier);
        if (sent)
            playSound(SOUND_CLICK);
        else
            Serial.printf("BLE HID: %s report was not sent.\n", b.label);
        return;
    }
}
