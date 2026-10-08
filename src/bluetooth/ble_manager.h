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
bool bleSendVolumeUp();

// Call from loop(); returns true when the state or device changed
bool bleUpdate();

#endif
