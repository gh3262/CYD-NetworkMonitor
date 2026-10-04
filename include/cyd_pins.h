#ifndef CYD_PINS_H
#define CYD_PINS_H
//
// ESP32-2432S028R CYD Pin Definitions
//
// Verified against hardware testing and schematic
//
// ====================================================
// TFT Display (ILI9341)
// ====================================================
#define TFT_BL      21
#define TFT_DC      2
#define TFT_CS      15
#define TFT_SCK     14
#define TFT_MOSI    13
#define TFT_MISO    12
// Not connected
#define TFT_RST     -1
// ====================================================
// Touch Controller (XPT2046)
// ====================================================
#define TOUCH_CS    33
#define TOUCH_IRQ   36
#define TOUCH_SCK   25
#define TOUCH_MOSI  32
#define TOUCH_MISO  39
// Raw calibration values
#define TOUCH_X_MIN 250
#define TOUCH_X_MAX 3760
#define TOUCH_Y_MIN 250
#define TOUCH_Y_MAX 3700
// ====================================================
// microSD Card
// ====================================================
#define SD_CS       5
#define SD_SCK      18
#define SD_MOSI     23
#define SD_MISO     19
// ====================================================
// Audio
// ====================================================
#define SPEAKER_PIN 26
// ====================================================
// Ambient Light Sensor
// ====================================================
#define LIGHT_ADC   34
// ====================================================
// RGB Status LED
// Common-Anode
// LOW  = ON
// HIGH = OFF
// ====================================================
#define LED_RED      4
#define LED_GREEN   16
#define LED_BLUE    17
// ====================================================
// Expansion Connector
// ====================================================
#define CN1_PIN     27
// ====================================================
// Common Colors (RGB565)
// ====================================================
#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define YELLOW      0xFFE0
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define ORANGE      0xFD20
#define GRAY        0x8410
#endif
