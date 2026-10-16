#include "ble_manager.h"
#include <atomic>
#include <esp_system.h>
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>

// The BLE manager is the single owner of connection state, advertising
// state, bond information and the device name. NimBLE callbacks run on the
// BLE host task, so they only touch the volatile fields below; screens read
// state from the main loop through the accessors.

static const char *const ADV_NAME_PREFIX = "CYD-HID";
static char advName[16] = "CYD-HID";
static const unsigned long PAIRING_TIMEOUT_MS = 120000;
static const uint16_t NO_CONN = 0xFFFF;
static const uint8_t CONSUMER_REPORT_ID = 1;
static const uint8_t KEYBOARD_REPORT_ID = 2;

static volatile BtState btState = BT_DISCONNECTED;
static volatile bool initialized = false;
static volatile bool changed = false;
static volatile uint16_t connHandle = NO_CONN;
static volatile bool bondPresentAtConnect = false;
static volatile bool userDisconnect = false;
static std::atomic<BtEvent> pendingEvent(BT_EVENT_NONE);
static unsigned long pairingStartMs = 0;

static NimBLEServer *server = nullptr;
static NimBLEAdvertising *advertising = nullptr;
static NimBLEHIDDevice *hidDevice = nullptr;
static NimBLECharacteristic *consumerInputReport = nullptr;
static NimBLECharacteristic *keyboardInputReport = nullptr;

static uint8_t hidReportMap[] = {
    0x05, 0x0C,       // Usage Page (Consumer)
    0x09, 0x01,       // Usage (Consumer Control)
    0xA1, 0x01,       // Collection (Application)
    0x85, CONSUMER_REPORT_ID,
    0x15, 0x00,       // Logical Minimum (0)
    0x25, 0x01,       // Logical Maximum (1)
    0x09, 0xE9,       // Usage (Volume Increment)   bit 0
    0x09, 0xEA,       // Usage (Volume Decrement)   bit 1
    0x09, 0xCD,       // Usage (Play/Pause)         bit 2
    0x09, 0xB5,       // Usage (Scan Next Track)    bit 3
    0x09, 0xB6,       // Usage (Scan Previous Track) bit 4
    0x75, 0x01,       // Report Size (1)
    0x95, 0x05,       // Report Count (5)
    0x81, 0x02,       // Input (Data, Variable, Absolute)
    0x75, 0x03,       // Report Size (3)
    0x95, 0x01,       // Report Count (1)
    0x81, 0x03,       // Input (Constant, Variable, Absolute)
    0xC0,            // End Collection

    0x05, 0x01,      // Usage Page (Generic Desktop)
    0x09, 0x06,      // Usage (Keyboard)
    0xA1, 0x01,      // Collection (Application)
    0x85, KEYBOARD_REPORT_ID,
    0x05, 0x07,      // Usage Page (Keyboard)
    0x19, 0xE0,      // Usage Minimum (Left Control)
    0x29, 0xE7,      // Usage Maximum (Right GUI)
    0x15, 0x00,      // Logical Minimum (0)
    0x25, 0x01,      // Logical Maximum (1)
    0x75, 0x01,      // Report Size (1)
    0x95, 0x08,      // Report Count (8)
    0x81, 0x02,      // Input (Data, Variable, Absolute): modifiers
    0x75, 0x08,      // Report Size (8)
    0x95, 0x01,      // Report Count (1)
    0x81, 0x03,      // Input (Constant): reserved byte
    0x19, 0x00,      // Usage Minimum (No event)
    0x29, 0x65,      // Usage Maximum (Keyboard Application)
    0x15, 0x00,      // Logical Minimum (0)
    0x25, 0x65,      // Logical Maximum (101)
    0x75, 0x08,      // Report Size (8)
    0x95, 0x06,      // Report Count (6)
    0x81, 0x00,      // Input (Data, Array, Absolute): key codes
    0xC0             // End Collection
};

static void setState(BtState next)
{
    if (btState != next)
    {
        btState = next;
        changed = true;
    }
}

static void startRadio()
{
    if (advertising && !advertising->isAdvertising())
        advertising->start();
}

static void stopRadio()
{
    if (advertising && advertising->isAdvertising())
        advertising->stop();
}

static void dropConnection()
{
    const uint16_t handle = connHandle;

    if (server && handle != NO_CONN)
        server->disconnect(handle);
}

class ServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *, NimBLEConnInfo &connInfo) override
    {
        Serial.printf("BLE: connect %s handle=%u\n",
                      connInfo.getIdAddress().toString().c_str(), connInfo.getConnHandle());
        connHandle = connInfo.getConnHandle();
        bondPresentAtConnect = NimBLEDevice::getNumBonds() > 0;
        userDisconnect = false;

        // Ask the phone to encrypt/bond; we are Connected only once it does
        NimBLEDevice::startSecurity(connInfo.getConnHandle());
    }

    void onAuthenticationComplete(NimBLEConnInfo &connInfo) override
    {
        Serial.printf("BLE: auth complete encrypted=%d bonded=%d authenticated=%d\n",
                      connInfo.isEncrypted(), connInfo.isBonded(), connInfo.isAuthenticated());

        if (!connInfo.isEncrypted())
        {
            pendingEvent = BT_EVENT_FAILED;
            server->disconnect(connInfo);
            return;
        }

        // One device at a time: a stranger that paired while we were bonded
        if (bondPresentAtConnect &&
            NimBLEDevice::getBondedAddress(0) != connInfo.getIdAddress())
        {
            NimBLEDevice::deleteBond(connInfo.getIdAddress());
            pendingEvent = BT_EVENT_FAILED;
            server->disconnect(connInfo);
            return;
        }

        pendingEvent = BT_EVENT_CONNECTED;
        setState(BT_CONNECTED);
    }

    void onDisconnect(NimBLEServer *, NimBLEConnInfo &, int reason) override
    {
        Serial.printf("BLE: disconnect reason=0x%X userRequested=%d state=%d\n",
                      reason, (int)userDisconnect, (int)btState);

        connHandle = NO_CONN;

        if (userDisconnect || btState == BT_DISCONNECTED)
        {
            setState(BT_DISCONNECTED);
            return;
        }

        // Link lost or intruder rejected: keep waiting
        if (btState == BT_CONNECTED)
        {
            pendingEvent = BT_EVENT_LOST;
            setState(NimBLEDevice::getNumBonds() > 0 ? BT_ADVERTISING : BT_PAIRING);
        }

        pairingStartMs = millis();
        changed = true;
        startRadio();
    }
};

static ServerCallbacks serverCallbacks;

bool bleInit()
{
    if (initialized)
        return true;

    // Suffix the last byte of the WiFi MAC (as shown on the System screen)
    // so multiple boards advertise distinct names, e.g. "CYD-HID-EF".
    uint8_t mac[6];
    if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK)
        snprintf(advName, sizeof(advName), "%s-%02X", ADV_NAME_PREFIX, mac[5]);
    Serial.printf("BLE: advertising as %s\n", advName);

    if (!NimBLEDevice::init(advName))
    {
        setState(BT_UNAVAILABLE);
        return false;
    }

    // Just Works pairing with bonding and LE Secure Connections
    NimBLEDevice::setSecurityAuth(true, false, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

    server = NimBLEDevice::createServer();
    server->setCallbacks(&serverCallbacks);
    server->advertiseOnDisconnect(false);

    hidDevice = new NimBLEHIDDevice(server);
    hidDevice->setManufacturer("CYD-NetworkMonitor");
    hidDevice->setHidInfo(0, 1);
    hidDevice->setReportMap(hidReportMap, sizeof(hidReportMap));
    consumerInputReport = hidDevice->getInputReport(CONSUMER_REPORT_ID);
    if (!consumerInputReport)
    {
        Serial.println("BLE: failed to create consumer-control input report.");
        setState(BT_UNAVAILABLE);
        return false;
    }
    keyboardInputReport = hidDevice->getInputReport(KEYBOARD_REPORT_ID);
    if (!keyboardInputReport)
    {
        Serial.println("BLE: failed to create keyboard input report.");
        setState(BT_UNAVAILABLE);
        return false;
    }

    advertising = NimBLEDevice::getAdvertising();
    advertising->setName(advName);
    advertising->setAppearance(GENERIC_HID);
    if (!advertising->addServiceUUID(hidDevice->getHidService()->getUUID()))
    {
        Serial.println("BLE: failed to add HID service to advertising data.");
        setState(BT_UNAVAILABLE);
        return false;
    }

    if (!server->start())
    {
        Serial.println("BLE: failed to start GATT server.");
        setState(BT_UNAVAILABLE);
        return false;
    }

    initialized = true;
    changed = true;

    // A saved device reconnects by itself after boot
    if (bleHasBond())
        bleStartAdvertising();

    return true;
}

bool bleInitialized()
{
    return initialized;
}

BtState bleState()
{
    return btState;
}

bool bleHasBond()
{
    return initialized && NimBLEDevice::getNumBonds() > 0;
}

String bleDeviceName()
{
    if (!bleHasBond())
        return "";

    // The phone's name is not sent to us; the address identifies the bond
    return String(NimBLEDevice::getBondedAddress(0).toString().c_str());
}

void bleStartPairing()
{
    // One device at a time: Forget must come first
    if (!initialized || btState != BT_DISCONNECTED || bleHasBond())
        return;

    pairingStartMs = millis();
    setState(BT_PAIRING);
    startRadio();
}

void bleStartAdvertising()
{
    if (!initialized || btState != BT_DISCONNECTED || !bleHasBond())
        return;

    setState(BT_ADVERTISING);
    startRadio();
}

void bleStop()
{
    if (btState != BT_PAIRING && btState != BT_ADVERTISING)
        return;

    userDisconnect = true;
    stopRadio();
    dropConnection();
    setState(BT_DISCONNECTED);
}

void bleDisconnect()
{
    if (btState != BT_CONNECTED)
        return;

    userDisconnect = true;
    dropConnection();
    setState(BT_DISCONNECTED);
}

void bleForgetBond()
{
    if (!initialized)
        return;

    userDisconnect = true;
    stopRadio();
    dropConnection();
    NimBLEDevice::deleteAllBonds();
    setState(BT_DISCONNECTED);
}

bool bleSendConsumerKey(BleConsumerKey key)
{
    if (!initialized || btState != BT_CONNECTED || !consumerInputReport)
        return false;

    const uint8_t pressed = static_cast<uint8_t>(key);
    const uint8_t released = 0x00;

    if (!consumerInputReport->notify(&pressed, sizeof(pressed)))
        return false;

    delay(20);
    return consumerInputReport->notify(&released, sizeof(released));
}

bool bleSendVolumeUp()
{
    return bleSendConsumerKey(BLE_KEY_VOLUME_UP);
}

bool bleSendKeyboardKey(BleKeyboardKey key, BleKeyboardModifier modifier)
{
    if (!initialized || btState != BT_CONNECTED || !keyboardInputReport)
        return false;

    const uint8_t pressed[8] = {
        static_cast<uint8_t>(modifier), 0, static_cast<uint8_t>(key), 0, 0, 0, 0, 0
    };
    const uint8_t released[8] = {};

    if (!keyboardInputReport->notify(pressed, sizeof(pressed)))
        return false;

    delay(20);
    return keyboardInputReport->notify(released, sizeof(released));
}

bool bleUpdate()
{
    if (btState == BT_PAIRING && millis() - pairingStartMs > PAIRING_TIMEOUT_MS)
    {
        bleStop();
        pendingEvent = BT_EVENT_FAILED;
    }

    if (changed)
    {
        changed = false;
        return true;
    }

    return false;
}

BtEvent bleTakeEvent()
{
    return pendingEvent.exchange(BT_EVENT_NONE);
}
