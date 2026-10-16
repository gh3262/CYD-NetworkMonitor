#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>

// ====================================================
// BLE manager: owns all Bluetooth state and logic.
// Screens only read state and request actions.
// ====================================================

enum BtState
{
    BT_DISCONNECTED,
    BT_ADVERTISING,   // bonded device saved, waiting for it to connect
    BT_PAIRING,       // no bond, open to a new device
    BT_CONNECTED,
    BT_UNAVAILABLE    // BLE stack failed to start
};

// One-shot connection outcomes for the UI (sounds, alerts)
enum BtEvent
{
    BT_EVENT_NONE,
    BT_EVENT_CONNECTED,   // encrypted link established
    BT_EVENT_FAILED,      // pairing timed out, encryption failed, or device rejected
    BT_EVENT_LOST         // connected link dropped without a user request
};

// Consumer-control keys; values are bits in the HID input report
enum BleConsumerKey : uint8_t
{
    BLE_KEY_VOLUME_UP   = 0x01,
    BLE_KEY_VOLUME_DOWN = 0x02,
    BLE_KEY_PLAY_PAUSE  = 0x04,
    BLE_KEY_NEXT        = 0x08,
    BLE_KEY_PREVIOUS    = 0x10
};

enum BleKeyboardKey : uint8_t
{
    BLE_KEY_A           = 0x04,
    BLE_KEY_C           = 0x06,
    BLE_KEY_V           = 0x19,
    BLE_KEY_TAB         = 0x2B,
    BLE_KEY_PAGE_UP     = 0x4B,
    BLE_KEY_PAGE_DOWN   = 0x4E,
    BLE_KEY_RIGHT_ARROW = 0x4F,
    BLE_KEY_LEFT_ARROW  = 0x50
};

enum BleKeyboardModifier : uint8_t
{
    BLE_MOD_NONE      = 0x00,
    BLE_MOD_LEFT_CTRL = 0x01
};

// Call once from setup(); returns false (state BT_UNAVAILABLE) on failure
bool bleInit();

// State
bool bleInitialized();   // false until bleInit() succeeds
BtState bleState();
bool bleHasBond();
String bleDeviceName();

// Actions
void bleStartPairing();       // no bond: become discoverable
void bleStartAdvertising();   // bond: wait for the saved device
void bleStop();               // cancel pairing / stop advertising
void bleDisconnect();
void bleForgetBond();
// Press and release one key; false when not connected or the send fails
bool bleSendConsumerKey(BleConsumerKey key);
bool bleSendKeyboardKey(BleKeyboardKey key, BleKeyboardModifier modifier = BLE_MOD_NONE);
bool bleSendVolumeUp();

// Call from loop(); returns true when the state or device changed
bool bleUpdate();

// Returns the latest connection outcome once, then BT_EVENT_NONE
BtEvent bleTakeEvent();

#endif
