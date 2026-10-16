# CYD Network Monitor

A touch-screen network monitor and Bluetooth LE tool for the ESP32 "Cheap Yellow Display" (CYD, ESP32-2432S028R: 320x240 TFT with XPT2046 touch). It shows WiFi status, scans for nearby networks, measures gateway and internet latency, keeps its clock in sync over NTP, and pairs with a phone over Bluetooth LE. This was my first PlatformIO project.

## Status

| Area | State |
|---|---|
| WiFi status, scan, tools, system info | Working |
| BLE pairing, bonding, reconnect (NimBLE) | Working, verified with a phone |
| BLE HID consumer control | Implemented; camera shutter support depends on the phone and app |
| BLE HID media control | Working, verified with an Android phone; track skip support can vary by app |
| BLE HID keyboard control | Implemented; device/app behavior needs hardware verification |

## New Features

### Bluetooth HID Camera Remote

- Camera shutter release with a confirmation beep
- Countdown timer mode
- Burst photography mode
- Configurable camera settings
- Persistent Bluetooth bonding
- Automatic reconnect

### Bluetooth HID Media Control

- Play/Pause, Previous Track, and Next Track
- Volume Up and Volume Down
- Click sound when a key is sent
- Live connection status on the page
- See [Media Control](#media-control) for setup and re-pairing

### Bluetooth Architecture

- NimBLE integration
- BLE Manager abstraction
- BT LE management screen
- BT HID application layer

### Bluetooth HID Keyboard Control

- KB is the third button on the Bluetooth HID page
- Volume Up, Volume Down, Tab, Left Arrow, Right Arrow, Page Up, and Page Down
- All (Ctrl-A), Copy (Ctrl-C), and Paste (Ctrl-V)
- One press and release per tap, with a click on successful send
- Live connection status; see [Keyboard Control](#keyboard-control) for pairing notes

## Existing Features

- WiFi diagnostics
- WiFi scanner
- Network tools
- NTP time synchronization
- System diagnostics

## Features

- **Home screen** as the landing page, with navigation to each branch.
- **WiFi Status**: SSID, IP, color-coded signal strength (RSSI), uptime and start time, and a `Ping: gateway / internet ms` line with an overall Health indicator (ONLINE / LOCAL ONLY / OFFLINE). Refresh runs the pings.
- **WiFi connection**: scans for configured networks and connects to the strongest signal. If none are in range, startup continues offline and the device retries scans periodically.
- **WiFi Scan**: lists nearby networks sorted by signal strength, paged, with the connected network highlighted. It also works while disconnected.
- **System**: free heap, minimum heap, sketch size, uptime, and the device's WiFi MAC address, with a link to Tools.
- **Tools**: gateway and internet ping with green/yellow/red results, and an NTP button that re-syncs the clock and reports Synced / Sync failed.
- **BT LE**: Bluetooth connection manager showing status and the paired device, with Pair / Connect / Stop / Cancel / Disconnect and Forget. HID stays disabled until a phone is connected.
- **BT HID**: function menu with Camera, Media, and KB buttons.
- **BT Camera**: sends a Volume Up consumer-control report for a single shot, after a configurable countdown, or as a configurable shot series. Countdown and shot progress are shown full-screen.
- **Camera Settings**: adjusts Delay Seconds, Nbr Shots, and Time Between from 1 to 30. SET applies the values to BT Camera; Reset restores the editor to 5 / 5 / 2. Values are kept until reboot.
- **Media Control**: Play/Pause, Previous, Next, Volume Up, and Volume Down media keys for the connected phone, with a click sound when a key is sent and a live Bluetooth status line below the track buttons.
- **KB Control**: Volume Up, Volume Down, Tab, All (Ctrl-A), Copy (Ctrl-C), Paste (Ctrl-V), Left Arrow, Right Arrow, Page Up, and Page Down for the connected device, with a click sound when a key is sent and a live Bluetooth status line.
- **Clock** in the title bar, updated every minute.
- **Status LED** shows connection state and signal quality, and WiFi reconnects automatically in the background.
- **Sound** plays non-blocking tones through the speaker amplifier on IO26. Any screen can use it; the camera plays a low 800 Hz tick each second of a countdown and a shutter beep after each successfully sent shutter report. A success sound plays when WiFi connects (at boot or on reconnect) or a phone connects over Bluetooth. An error sound plays when the boot-time WiFi connect fails, when an established WiFi or Bluetooth connection is lost, or when Bluetooth pairing times out, encryption fails, or a second device is rejected. User-requested Bluetooth disconnects are silent.

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
| BT Camera | Back (BT HID) | Settings | | Home |
| Camera Settings | Back (BT Camera) | SET | Reset | Home |
| Media Control | Back (BT HID) | | | Home |
| KB Control | Back (BT HID) | | | Home |

BT HID has three body buttons: **Camera**, **Media**, **KB**. Camera opens BT Camera, whose body buttons are **Shutter**, **Delay**, and **Series**. Media opens Media Control: a large **Play/Pause** with **Previous** and **Next** below it and a Bluetooth status line under those on the left two-thirds, and **Vol Up** over **Vol Down** on the right third. KB opens KB Control with a three-column, four-row grid of 96 x 36 pixel buttons: Vol Up/Vol Down/Tab, All/Copy/Paste, Left/Right/Pg Up, then Pg Down. Connection status occupies the two unused bottom-row cells.

```
HOME
+-- WiFi
�   +-- Status
�   +-- Scan
+-- BT LE
�   +-- HID
        +-- Camera
        �   +-- Settings
        +-- Media
        +-- KB
+-- System
    +-- Tools
```

## Bluetooth LE

All Bluetooth state and logic lives in `src/bluetooth/ble_manager.*`, built on [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino). Screens only read state (`bleState()`, `bleHasBond()`, `bleDeviceName()`, `bleInitialized()`) and request actions (`bleStartPairing()`, `bleStartAdvertising()`, `bleStop()`, `bleDisconnect()`, `bleForgetBond()`).

States: `BT_DISCONNECTED`, `BT_ADVERTISING` (bonded device saved, waiting for it), `BT_PAIRING` (no bond, open to a new device), `BT_CONNECTED`, `BT_UNAVAILABLE` (stack failed to start).

Rules:

- **One device at a time.** Forget the saved device before pairing a new one. A second phone that bonds while one is saved is rejected and its bond deleted.
- **Reconnect.** A saved device starts advertising at boot and after a dropped link. Stop and Disconnect leave the radio idle until the user presses Connect.
- **Pairing** is Just Works with bonding and LE Secure Connections, and times out after 120 s. The device advertises as `CYD-HID-XX` with the generic HID appearance, where `XX` is the last byte of the board's WiFi MAC address in hex (the last two characters of the MAC on the System screen; for MAC `24:6F:28:AB:CD:EF` the name is `CYD-HID-EF`). This keeps multiple boards distinguishable in a phone's Bluetooth list. The name is also printed to the serial monitor at boot.
- **HID controls.** Open HID, then Camera. The Camera screen sends a Bluetooth Consumer Control Volume Increment report when Shutter is tapped. Shutter flashes green for 300 ms when the report is sent successfully, then returns to red; this confirms sending, not that the phone took a photo. Delay waits the configured number of seconds before sending; Series waits the same delay, then sends the configured number of reports with the configured spacing (default 5 seconds, 5 shots, 2 seconds apart). The countdown and shot number are shown without buttons while a sequence runs. A connected phone and a camera app that treats Volume Up as the shutter are required.
- **Media keys.** The consumer-control report carries Volume Up, Volume Down, Play/Pause, Next Track, and Previous Track; `bleSendConsumerKey()` presses and releases one. See [Media Control](#media-control).
- **Keyboard keys.** A separate eight-byte keyboard input report carries Tab, Left/Right Arrow, Page Up/Down, and Ctrl-A/C/V; `bleSendKeyboardKey()` presses and releases a key with an optional modifier (none by default). The release clears both the key and modifier. KB uses the existing consumer-control report for volume. See [Keyboard Control](#keyboard-control).
- **Device name.** The phone's name is not sent to the CYD, so the Device row shows the bonded Bluetooth address.
- **Connected** is reported only after the link is encrypted.
- Connect, authentication and disconnect events are logged to the serial monitor with a `BLE:` prefix.

## Media Control

The Media Control screen turns the CYD into a Bluetooth remote for whatever is playing on the connected phone (music, podcasts, video). The media controls have been tested and work with an Android phone.

Open it from **Home > BT LE > HID > Media**. The phone must be connected (BT LE shows Connected).

| Button | Action |
|---|---|
| Play/Pause (large, top left) | Toggles playback |
| Previous (bottom left) | Previous track |
| Next (bottom middle) | Next track |
| Vol Up (top right) | Raises the phone volume |
| Vol Down (bottom right) | Lowers the phone volume |

Each tap sends one key press and release. A click sound confirms the key was sent; this means the CYD sent it, not that the phone acted on it. If sending fails (for example, no phone connected), nothing plays and a message is logged to the serial monitor. Back returns to BT HID; Home returns to the Home screen.

The status line below Previous and Next shows the Bluetooth status and updates as it changes, so the reason a button does nothing is visible right away:

| Status | Color | Meaning |
|---|---|---|
| Connected | Green | Keys are sent to the phone |
| Reconnecting... | Yellow | A saved phone is out of range or its Bluetooth is off; the CYD is waiting for it |
| Pairing... | Yellow | Pairing is open on the BT LE screen |
| Disconnected | Red | Bluetooth is idle; press Connect or Pair on the BT LE screen |
| Starting BLE / BT Unavailable | Yellow / Red | The Bluetooth stack is starting or failed to start |

"Connected" means the link works, not that the phone recognizes the media keys. If it shows Connected and the buttons still do nothing, follow the re-pair steps below.

Previous and Next depend on the phone and the media app. Some apps ignore them, and some treat Previous as "restart track" when a track is partway through.

### If the media buttons don't work the first time

Phones save the CYD's list of supported keys (the HID report descriptor) when they first pair. A phone paired with firmware from before Media Control was added only knows about Volume Up, so Play/Pause, Previous, Next, and Vol Down do nothing. Unpair and re-pair to make the phone read the new key list:

1. **Forget on the CYD:** go to **Home > BT LE**, press **Disconnect** if connected, then press **Forget**.
2. **Forget on the phone:** open the phone's Bluetooth settings and remove `CYD-HID-XX` (the name ends with the last two characters of the MAC on the System screen; older firmware used plain `CYD-HID`).
   - **iPhone:** Settings > Bluetooth > tap the (i) next to `CYD-HID-XX` > **Forget This Device**.
   - **Android:** Settings > Connected devices (or Bluetooth) > tap the gear next to `CYD-HID-XX` > **Unpair** / **Forget**.
3. **Turn the phone's Bluetooth off and back on.** This clears any cached service data.
4. **Pair again:** on the CYD press **Pair** on the BT LE screen, then select `CYD-HID-XX` in the phone's Bluetooth list within 120 seconds. Accept the pairing prompt.
5. **Test:** wait until BT LE shows Connected, start some audio on the phone, then open **HID > Media** and press Play/Pause.

If the keys still don't work, restart the phone and repeat steps 1–4. The same steps fix a camera shutter that stops working after a firmware update.

## Keyboard Control

Open **Home > BT LE > HID > KB** while connected to a BLE host.

| Button | Action |
|---|---|
| Volume Up / Volume Down | Raises / lowers the host's volume using consumer-control keys |
| Tab | Sends Tab (usually advances focus) |
| All | Sends Ctrl-A (select all) |
| Copy | Sends Ctrl-C (copy selection) |
| Paste | Sends Ctrl-V (paste clipboard) |
| Left Arrow / Right Arrow | Sends the corresponding arrow key |
| Page Up / Page Down | Sends the corresponding page navigation key |

Each tap sends one press followed by a release. A click confirms successful sending, not that the host acted on the key. Failed sends are logged to the serial monitor without a click. Connection status updates live; Back returns to BT HID and Home returns to Home.

All, Copy, and Paste send Ctrl shortcuts, not Command shortcuts. The host/app must support them and have the intended field focused. Adding these shortcuts does not change the keyboard report descriptor, so devices already paired with KB-capable firmware do not need to re-pair for this update.

**After upgrading from firmware without KB, forget the pairing on both the CYD and the host, then pair again.** The HID descriptor now includes a keyboard report, and hosts may cache the old descriptor. Follow the [re-pair steps](#if-the-media-buttons-dont-work-the-first-time), then test KB in an app with a text field or scrollable document. Key behavior depends on the host, app, and current focus; mobile hosts may hide the on-screen keyboard while a Bluetooth keyboard is connected.

## Sound

`src/audio/sound_manager.*` drives the speaker amplifier on `SPEAKER_PIN` (IO26) with LEDC channel 6 PWM. It is included through `app.h`, so every screen can use it. Sounds are non-blocking: `soundUpdate()` advances them from `loop()`, and a new sound replaces the one playing.

- `playSound(SOUND_SHUTTER)` plays a built-in sound: `SOUND_CLICK`, `SOUND_BEEP`, `SOUND_SHUTTER`, `SOUND_COUNTDOWN`, `SOUND_SUCCESS`, or `SOUND_ERROR`.
- `playTone(frequencyHz, durationMs)` plays one tone.
- `playToneSequence(steps, count)` plays up to 16 `ToneStep` entries; frequency 0 is a rest.
- `stopSound()`, `setSoundEnabled()`, and `setSoundVolume(0-100)` control playback. Volume defaults to 50% and is not saved across reboots.

## Hardware

- ESP32-2432S028R (CYD) board, as defined in `include/cyd_pins.h`.
- A 2.4 GHz WiFi network.
- For Bluetooth: a phone or other BLE host.

## Setup

1. Install [PlatformIO](https://platformio.org/).
2. Copy `include/credentials_template.h` to `include/credentials.h` (it is gitignored) and fill in:
   - Add each known WiFi network's SSID and password to the `wifiNetworks` array. The device scans and connects to the strongest one in range.
   - `NTP_SERVER_1` and `NTP_SERVER_2`
   - `TZ_STRING`, a POSIX time zone string, for example `CST6CDT,M3.2.0,M11.1.0`
3. Build and upload: `pio run -t upload`. Open the serial monitor at 115200 baud.

Library and platform versions are pinned in `platformio.ini` (espressif32 7.1.3, GFX Library for Arduino 1.5.7, Adafruit GFX 1.12.6, ESP32Ping 1.7, NimBLE-Arduino 2.5.1, and XPT2046_Touchscreen at a fixed commit).

If a phone keeps stale pairing or HID service data, press Forget on the CYD and also remove `CYD-HID-XX` in the phone's Bluetooth settings, then pair again. See [If the media buttons don't work the first time](#if-the-media-buttons-dont-work-the-first-time) for the full steps.

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
  audio/
    sound_manager.h/.cpp  Non-blocking speaker tones and built-in sounds
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
    bt_camera_screen.cpp
    bt_camera_settings_screen.cpp
    bt_media_screen.cpp
    bt_kb_screen.cpp
```

Each screen file provides `draw*`, `enter*` and `handle*Touch` functions. `main.cpp` holds a table with one entry per screen, so adding a screen means adding an enum value, a source file and one table row.

## Design notes

- **Non-blocking UI:** pings run in a FreeRTOS background task, the WiFi scan is asynchronous, and NTP sync is polled from `loop()`. Touches stay responsive while they run. Only the boot-time WiFi connect and NTP sync block.
- **Touch handling:** presses are edge-triggered with a short release debounce, so one tap triggers one action.
- **Partial redraws:** result rows (pings, NTP status) are redrawn in place instead of repainting the whole screen.
- **WiFi resilience:** the connect has a 15 s timeout, and the app continues without WiFi if it fails. A non-blocking loop retries every 10 s and updates the LED. A scan while disconnected drops the pending connection attempt first so it can complete.
- **Memory:** time and uptime strings use static buffers instead of heap `String`s, and scan results are copied out of the WiFi driver and freed immediately. Flash use is about 88% with NimBLE, so a larger-app partition table may be needed as features grow.
- **BLE threading:** NimBLE callbacks run on the BLE host task and only update volatile state; the UI reads it from `loop()`.

## Roadmap

- Additional HID keyboard keys and mouse reports
- MQTT and Adafruit IO integration
- Additional diagnostics
