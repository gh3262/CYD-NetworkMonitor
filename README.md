# CYD Network Monitor

A touch-screen network monitor for the ESP32 "Cheap Yellow Display" (CYD, ESP32-2432S028R: 320x240 TFT with XPT2046 touch). It shows WiFi status, scans for nearby networks, measures gateway and internet latency, and keeps its clock in sync over NTP. This was my first PlatformIO project.

## Features

- **Home screen** as the landing page, with navigation to each branch.
- **WiFi Status**: SSID, IP, color-coded signal strength (RSSI), uptime and start time, and a `Ping: gateway / internet ms` line with an overall Health indicator (ONLINE / LOCAL ONLY / OFFLINE). Refresh runs the pings.
- **WiFi Scan**: lists nearby networks sorted by signal strength, paged, with the connected network highlighted. It also works while disconnected.
- **System**: free heap, minimum heap, sketch size and uptime.
- **Tools**: gateway and internet ping with green/yellow/red results, and an NTP button that re-syncs the clock and reports Synced / Sync failed.
- **Clock** in the title bar, updated every minute.
- **Status LED** shows connection state and signal quality, and WiFi reconnects automatically in the background.

## Screens and navigation

Every screen has four buttons along the bottom. Button 4 is always **Home** (except on the Home screen itself).

| Screen | Button 1 | Button 2 | Button 3 | Button 4 |
|---|---|---|---|---|
| Home | WiFi | | | |
| WiFi Status | System | Scan | Refresh | Home |
| WiFi Scan | Back | Next (when paged) | Rescan | Home |
| System | Back | Tools | Refresh | Home |
| Tools | Back | NTP | Ping | Home |

## Hardware

- ESP32-2432S028R (CYD) board, as defined in `include/cyd_pins.h`.
- A 2.4 GHz WiFi network.

## Setup

1. Install [PlatformIO](https://platformio.org/).
2. Copy `include/credentials_template.h` to `include/credentials.h` (it is gitignored) and fill in:
   - `WIFI_SSID` and `WIFI_PASSWORD`
   - `NTP_SERVER_1` and `NTP_SERVER_2`
   - `TZ_STRING`, a POSIX time zone string, for example `CST6CDT,M3.2.0,M11.1.0`
3. Build and upload: `pio run -t upload`. Open the serial monitor at 115200 baud.

Library and platform versions are pinned in `platformio.ini` (espressif32 7.1.3, GFX Library for Arduino 1.5.7, Adafruit GFX 1.12.6, ESP32Ping 1.7, and XPT2046_Touchscreen at a fixed commit).

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
  screens/
    home_screen.cpp
    wifi_status_screen.cpp
    wifi_scan_screen.cpp
    system_screen.cpp
    tools_screen.cpp
```

Each screen file provides `draw*`, `enter*` and `handle*Touch` functions. `main.cpp` holds a table with one entry per screen, so adding a screen means adding an enum value, a source file and one table row.

## Design notes

- **Non-blocking UI:** pings run in a FreeRTOS background task, the WiFi scan is asynchronous, and NTP sync is polled from `loop()`. Touches stay responsive while they run. Only the boot-time WiFi connect and NTP sync block.
- **Touch handling:** presses are edge-triggered with a short release debounce, so one tap triggers one action.
- **Partial redraws:** result rows (pings, NTP status) are redrawn in place instead of repainting the whole screen.
- **WiFi resilience:** the connect has a 15 s timeout, and the app continues without WiFi if it fails. A non-blocking loop retries every 10 s and updates the LED. A scan while disconnected drops the pending connection attempt first so it can complete.
- **Memory:** time and uptime strings use static buffers instead of heap `String`s, and scan results are copied out of the WiFi driver and freed immediately.
