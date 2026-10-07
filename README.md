# CYD Network Monitor

A touch-screen network monitor and Bluetooth LE tool for the ESP32 "Cheap Yellow Display" (CYD, ESP32-2432S028R: 320x240 TFT with XPT2046 touch). It shows WiFi status, scans for nearby networks, measures gateway and internet latency, keeps its clock in sync over NTP, and pairs with a phone over Bluetooth LE. This was my first PlatformIO project.

## Status

| Area | State |
|---|---|
| WiFi status, scan, tools, system info | Working |
| BLE pairing, bonding, reconnect (NimBLE) | Working, verified with a phone |
| BLE HID (keyboard/mouse) | Not implemented; the HID page is a placeholder |

**Current milestone:** pairing works. The phone pairs, bonds and then drops the link (disconnect reason `0x213`, remote user terminated) because the CYD exposes no HID service yet. The HID service is the next step.

## Features

- **Home screen** as the landing page, with navigation to each branch.
- **WiFi Status**: SSID, IP, color-coded signal strength (RSSI), uptime and start time, and a `Ping: gateway / internet ms` line with an overall Health indicator (ONLINE / LOCAL ONLY / OFFLINE). Refresh runs the pings.
- **WiFi Scan**: lists nearby networks sorted by signal strength, paged, with the connected network highlighted. It also works while disconnected.
- **System**: free heap, minimum heap, sketch size and uptime, with a link to Tools.
- **Tools**: gateway and internet ping with green/yellow/red results, and an NTP button that re-syncs the clock and reports Synced / Sync failed.
- **BT LE**: Bluetooth connection manager showing status and the paired device, with Pair / Connect / Stop / Cancel / Disconnect and Forget.
- **BT HID**: placeholder page ("NOT IMPLEMENTED").
- **Clock** in the title bar, updated every minute.
- **Status LED** shows connection state and signal quality, and WiFi reconnects automatically in the background.

## Screens and navigation

Every screen has four buttons along the bottom. Button 4 is **Home** on every screen except Home itself, where it is **System**.

| Screen | Button 1 | Button 2 | Button 3 | Button 4 |
|---|---|---|---|---|
| Home | WiFi | BT LE | | System |
| WiFi Status | Scan | Refresh | | Home |
| WiFi Scan | Back | Next (when paged) | Rescan | Home |
| System | Back (Home) | Tools | Refresh | Home |
| Tools | Back (System) | NTP | Ping | Home |
| BT LE | Pair / Connect / Stop / Cancel / Disconnect | Forget | HID | Home |
| BT HID | Back (BT LE) | | | Home |

```
HOME
+-- WiFi
¦   +-- Status
¦   +-- Scan
+-- BT LE
¦   +-- HID
+-- System
    +-- Tools
```

## Bluetooth LE

All Bluetooth state and logic lives in `src/bluetooth/ble_manager.*`, built on [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino). Screens only read state (`bleState()`, `bleHasBond()`, `bleDeviceName()`, `bleInitialized()`) and request actions (`bleStartPairing()`, `bleStartAdvertising()`, `bleStop()`, `bleDisconnect()`, `bleForgetBond()`).

States: `BT_DISCONNECTED`, `BT_ADVERTISING` (bonded device saved, waiting for it), `BT_PAIRING` (no bond, open to a new device), `BT_CONNECTED`, `BT_UNAVAILABLE` (stack failed to start).

Rules:

- **One device at a time.** Forget the saved device before pairing a new one. A second phone that bonds while one is saved is rejected and its bond deleted.
- **Reconnect.** A saved device starts advertising at boot and after a dropped link. Stop and Disconnect leave the radio idle until the user presses Connect.
- **Pairing** is Just Works with bonding and LE Secure Connections, and times out after 120 s. The device advertises as `CYD-HID` with the keyboard appearance.
- **Device name.** The phone's name is not sent to the CYD, so the Device row shows the bonded Bluetooth address.
- **Connected** is reported only after the link is encrypted.
- Connect, authentication and disconnect events are logged to the serial monitor with a `BLE:` prefix.

## Hardware

- ESP32-2432S028R (CYD) board, as defined in `include/cyd_pins.h`.
- A 2.4 GHz WiFi network.
- For Bluetooth: a phone or other BLE host.

## Setup

1. Install [PlatformIO](https://platformio.org/).
2. Copy `include/credentials_template.h` to `include/credentials.h` (it is gitignored) and fill in:
   - `WIFI_SSID` and `WIFI_PASSWORD`
   - `NTP_SERVER_1` and `NTP_SERVER_2`
   - `TZ_STRING`, a POSIX time zone string, for example `CST6CDT,M3.2.0,M11.1.0`
3. Build and upload: `pio run -t upload`. Open the serial monitor at 115200 baud.

Library and platform versions are pinned in `platformio.ini` (espressif32 7.1.3, GFX Library for Arduino 1.5.7, Adafruit GFX 1.12.6, ESP32Ping 1.7, NimBLE-Arduino 2.5.1, and XPT2046_Touchscreen at a fixed commit).

If a phone keeps stale pairing data, press Forget on the CYD and also remove `CYD-HID` in the phone's Bluetooth settings.

## Project layout

```
include/
  app.h               Screen enum, shared state, function declarations
  cyd.h               Umbrella include for the CYD helper headers
  cyd_pins.h          Board pin and touch-calibration definitions
  cyd_theme.h         Colors, fonts, screen size, nav button layout, touch constants
  cyd_display.h       Display init, title bar, clock, buttons
  cyd_touch.h         Touch init, coordinate mapping, nav button hit tests
  cyd_time.h          NTP sync and time/date formatting
  cyd_helpers.h       LED, backlight, light sensor, uptime helpers
  credentials*.h      WiFi/NTP settings (template is committed, real file is not)
src/
  main.cpp            setup, WiFi connect/maintain, screen table, touch loop
  cyd_globals.cpp     Single definitions of the display and touch objects
  bluetooth/
    ble_manager.h/.cpp  BLE state, advertising, bonding (NimBLE)
  screens/
    home_screen.cpp
    wifi_status_screen.cpp
    wifi_scan_screen.cpp
    system_screen.cpp
    tools_screen.cpp
    bt_le_screen.cpp
    bt_hid_screen.cpp
```

Each screen file provides `draw*`, `enter*` and `handle*Touch` functions. `main.cpp` holds a table with one entry per screen, so adding a screen means adding an enum value, a source file and one table row.

## Design notes

- **Non-blocking UI:** pings run in a FreeRTOS background task, the WiFi scan is asynchronous, and NTP sync is polled from `loop()`. Touches stay responsive while they run. Only the boot-time WiFi connect and NTP sync block.
- **Touch handling:** presses are edge-triggered with a short release debounce, so one tap triggers one action.
- **Partial redraws:** result rows (pings, NTP status) are redrawn in place instead of repainting the whole screen.
- **WiFi resilience:** the connect has a 15 s timeout, and the app continues without WiFi if it fails. A non-blocking loop retries every 10 s and updates the LED. A scan while disconnected drops the pending connection attempt first so it can complete.
- **Memory:** time and uptime strings use static buffers instead of heap `String`s, and scan results are copied out of the WiFi driver and freed immediately. Flash use is about 84% with NimBLE, so a larger-app partition table may be needed as features grow.
- **BLE threading:** NimBLE callbacks run on the BLE host task and only update volatile state; the UI reads it from `loop()`.

## Roadmap

- BLE HID service (report map, input reports) so the phone keeps the connection
- HID functions: keyboard, mouse, consumer/media keys, camera remote
- MQTT and Adafruit IO integration
- Additional diagnostics
