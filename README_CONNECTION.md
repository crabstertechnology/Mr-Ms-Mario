# 🌙 Hardware Connection Guide: ESP32-S3 Mini & ST7735 (1.8" TFT Display)

This document provides the complete wiring map and `TFT_eSPI` configuration required when upgrading your companion robot to the **ESP32-S3 Mini** microcontroller and **ST7735 1.8-inch TFT Display (128x160)**.

Using hardware SPI pins on the ESP32-S3 ensures the fastest possible rendering and frame rates for smooth screen updates and animations.

---

## 🔌 1. Wiring & Connection Table

Connect the ST7735 TFT display module to the ESP32-S3 Mini headers according to the table below:

| ST7735 TFT Pin | ESP32-S3 Mini Pin | Description | Notes |
| :--- | :--- | :--- | :--- |
| **VCC** / **VDD** | **5V** or **3V3** | Power Input | Check your display module voltage (usually 5V for red-PCB modules with regulator, 3.3V for blue/black PCBs). |
| **GND** | **G** / **GND** | System Ground | |
| **CS** | **GPIO 10** | Chip Select | SPI SS / CS Pin |
| **RESET** / **RST** | **GPIO 3** | Reset | Screen reset pin (changed from GPIO 14 because it is broken) |
| **A0** / **DC** | **GPIO 9** | Data/Command | Command selection pin |
| **SDA** / **MOSI** | **GPIO 11** | SPI Data Out | Hardware SPI MOSI |
| **SCK** / **SCL** | **GPIO 12** | SPI Clock | Hardware SPI SCLK |
| **LED** / **BL** / **K** | **3V3** or **GPIO 18** | Screen Backlight | Connect to **3V3** for always-on max brightness, or **GPIO 18** if you want PWM dimming. |

> [!WARNING]
> Ensure you verify the pin labels on your specific display module's silkscreen, as names like `A0` / `DC` / `RS` and `SDA` / `MOSI` / `SDI` can vary between manufacturers.

---

## 🛠️ 2. TFT_eSPI Library Setup (`User_Setup.h`)

Replace the contents of your `TFT_eSPI` library setup file (located at your active document path: `c:\Users\sasit\OneDrive\Documents\Arduino\libraries\TFT_eSPI\User_Setup.h`) with the following block:

```cpp
// User_Setup.h configuration for ESP32-S3 Mini & ST7735 1.8" Display
#define USER_SETUP_INFO "ESP32-S3-Mini_ST7735"

// Driver definition
#define ST7735_DRIVER

// Color definition adjustments for standard ST7735 displays.
// Select the correct tab variant if screen colors are inverted or swapped.
// Common options for 1.8" screens:
#define ST7735_BLACKTAB
// #define ST7735_REDTAB
// #define ST7735_GREENTAB
// #define ST7735_GREENTAB2
// #define ST7735_GREENTAB3

// Display resolution
#define TFT_WIDTH  128
#define TFT_HEIGHT 160

// Color inversion settings (if colors look negative/inverted)
// #define TFT_INVERSION_ON
// #define TFT_INVERSION_OFF

// SPI Pin definitions for ESP32-S3 Mini (Hardware FSPI / SPI2)
#define TFT_MOSI 11  // SPI MOSI (SDA)
#define TFT_SCLK 12  // SPI SCK (SCL)
#define TFT_CS   10  // Chip select control pin
#define TFT_DC    9  // Data Command control pin (A0 / RS)
#define TFT_RST  3  // Reset pin (RES) - changed from 14 because GPIO 14 is broken

// Backlight control (Optional)
// Uncomment if you connected the LED pin to GPIO 18
// #define TFT_BL           18
// #define TFT_BACKLIGHT_ON HIGH

// Font selections (enable small fonts to save memory)
#define LOAD_GLCD   // Standard 8x8 font
#define LOAD_FONT2  // Small 16-pixel font
#define LOAD_FONT4  // Medium 26-pixel font

// SPI Frequency (27MHz is standard and highly stable for ST7735)
#define SPI_FREQUENCY  27000000 
```

---

## 🔍 3. Troubleshooting & Fine-Tuning

### 🎨 Inverted Colors or Red/Blue Swap
If the text or graphics appear in negative colors (e.g., black is white, orange is cyan):
1. Open `User_Setup.h`.
2. Toggle the color inversion define:
   ```cpp
   #define TFT_INVERSION_ON
   ```
   If it is already on, comment it out:
   ```cpp
   // #define TFT_INVERSION_ON
   ```
3. If colors are still incorrect (e.g., Red and Blue are swapped), uncomment one of the other tab definitions in `User_Setup.h`, such as `#define ST7735_REDTAB` or `#define ST7735_GREENTAB3`, and comment out `#define ST7735_BLACKTAB`.

### ⚡ Screen Glitches or White Screen
1. **Check Power:** The ST7735 backlight requires significant current. If it displays a white screen, check that the display's `VCC` is supplied with 5V (if it has a 3.3V regulator on the back) or a stable 3.3V.
2. **Double check SPI Pins:** Make sure `MOSI` goes to the display's `SDA` pin and `SCLK` goes to the display's `SCK`/`CLK` pin.
3. **SPI Frequency:** If you see pixel noise or unstable images, try lowering the SPI speed by changing `#define SPI_FREQUENCY 27000000` to `#define SPI_FREQUENCY 20000000` or `#define SPI_FREQUENCY 16000000` in `User_Setup.h`.

---

## 🚀 4. Firmware Versions Comparison (v1 vs v2)

The repository maintains support for both display configurations:

| Firmware Version | Targeted Microcontroller | Display Type | Display Driver Library | Communication Compatibility |
| :--- | :--- | :--- | :--- | :--- |
| **`luna_firmware` (v1)** | ESP32-C3 SuperMini | SSD1306 OLED (128x64) | `Adafruit_SSD1306` (I2C) | Full Companion App Support |
| **`luna_firmware_v2` (v2)** | ESP32-S3 Mini | ST7735 TFT 1.8" (128x160) | `TFT_eSPI` (SPI) | Full Companion App Support |

### ⚡ Flashing the V2 Firmware
1. Ensure the `TFT_eSPI` library is installed in your Arduino IDE (`Sketch` > `Include Library` > `Manage Libraries...`).
2. Make sure your local `User_Setup.h` has been updated with the S3 Mini pin configurations (as detailed in **Section 2**).
3. Open the `luna_firmware_v2.ino` sketch inside the [luna_firmware_v2](file:///w:/Mr.mario/luna_firmware_v2) directory.
4. Select board **LOLIN S3 Mini** (or **ESP32S3 Dev Module**) under `Tools` > `Board`.
5. Select the correct COM port.
6. Verify and upload!

