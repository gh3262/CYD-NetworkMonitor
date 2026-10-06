#include "cyd.h"

// Single definitions of the hardware objects declared extern in the CYD
// headers, so every source file shares the same display and touch instances.

static Arduino_DataBus *displayBus =
    new Arduino_ESP32SPI(
        TFT_DC,
        TFT_CS,
        TFT_SCK,
        TFT_MOSI,
        TFT_MISO);

Arduino_GFX *gfx =
    new Arduino_ILI9341(
        displayBus,
        TFT_RST,
        1);

XPT2046_Touchscreen touch(TOUCH_CS);
