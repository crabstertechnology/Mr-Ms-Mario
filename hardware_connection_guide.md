# Hardware Connection Guide: ESP32-S3 Mini with ST7789 TFT, MAX98357 DAC, INMP441 Mic & Touch Sensor

This document outlines the wiring schemas and hardware considerations for building a setup containing an **ESP32-S3 Mini**, a **1.3" 240x240 ST7789 TFT LCD Display**, a **MAX98357 I2S Class D Amplifier**, an **INMP441 I2S MEMS Microphone**, and a **capacitive Touch Sensor (e.g., TTP223)**.

---

## 🔌 Power Delivery & Grounding

Providing clean power is critical for reliable performance, especially for audio components which are sensitive to high-frequency digital noise.

1. **3.3V Power Line (3V3):**
   * **ST7789 Display** must be powered by **3.3V**.
   * **INMP441 Microphone** must be powered by **3.3V** (do not connect to 5V; it will permanently damage the sensor).
   * **Touch Sensor** must be powered by **3.3V**.
2. **5.0V Power Line (5V / VIN / VBUS):**
   * **MAX98357 DAC & Amplifier** should ideally be powered by **5V** to output the full **3W** of audio power. While it can run on 3.3V, the maximum output volume will be significantly lower, and audio distortion will occur at higher volume levels.
3. **Common Ground (GND):**
   * All ground pins from all modules must connect to a shared Ground (GND) pin on the ESP32-S3 Mini to establish a common reference voltage.

---

## 🗺️ Pin Mappings

The ESP32-S3 features an internal GPIO matrix, allowing most peripherals (SPI, I2S) to be mapped to any free GPIO. However, we have chosen safe, high-speed pins that avoid strapping conflicts (GPIO 0, 45, 46) and native USB pins (GPIO 19, 20).

*Note: Pins between **GPIO 14 and GPIO 45** are excluded to avoid conflicts or broken hardware pins.*

### Option A: Shared-Clock Mode (Recommended for Full-Duplex I2S)
*Use this option if you are using a single I2S peripheral (duplex mode) in your firmware. The microphone and DAC will share the serial clock (`BCLK/SCK`) and word select (`LRC/WS`) lines, saving 2 GPIO pins.*

| ST7789 Display Pin | Label | ESP32-S3 Mini Pin | Description |
| :--- | :--- | :--- | :--- |
| Pin 1 | **GND** | **GND** | Common Ground |
| Pin 2 | **VCC** | **3V3** | 3.3V Power |
| Pin 3 | **SCL** | **GPIO 12** | SPI Serial Clock (SCLK/SCK) |
| Pin 4 | **SDA** | **GPIO 11** | SPI Master Out Slave In (MOSI) |
| Pin 5 | **RES** | **GPIO 10** | TFT Reset (Active LOW) |
| Pin 6 | **DC** | **GPIO 13** | TFT Data / Command Selection |
| Pin 7 | **BLK** | **GPIO 3** or **3V3** | Backlight control (GPIO 3 allows PWM dimming) |

| MAX98357 DAC Pin | Label | ESP32-S3 Mini Pin | Description |
| :--- | :--- | :--- | :--- |
| Pin 1 | **GND** | **GND** | Common Ground |
| Pin 2 | **Vin / VCC** | **5V (VIN/VBUS)** | 5V Power (highly recommended for 3W output) |
| Pin 3 | **LRC** | **GPIO 5** | I2S Word Select / Frame Clock (WS) |
| Pin 4 | **BCLK** | **GPIO 4** | I2S Bit Clock (BCLK) |
| Pin 5 | **DIN** | **GPIO 6** | I2S Serial Data Out from ESP32-S3 |
| Pin 6 | **GAIN** | *Unconnected* | Default 9dB gain (leave open) |
| Pin 7 | **SD** | *Unconnected* | Shutdown/Mode. Leaving open mixes Left & Right channels |

| INMP441 Mic Pin | Label | ESP32-S3 Mini Pin | Description |
| :--- | :--- | :--- | :--- |
| Pin 1 | **GND** | **GND** | Common Ground |
| Pin 2 | **VDD** | **3V3** | 3.3V Power (Do not use 5V!) |
| Pin 3 | **L/R** | **GND** | Left/Right Select. Grounding selects the Left channel |
| Pin 4 | **SCK** | **GPIO 4** | I2S Bit Clock (Shared with MAX98357 BCLK) |
| Pin 5 | **WS** | **GPIO 5** | I2S Word Select (Shared with MAX98357 LRC) |
| Pin 6 | **SD** | **GPIO 7** | I2S Serial Data In to ESP32-S3 |

| Touch Sensor Pin | Label | ESP32-S3 Mini Pin | Description |
| :--- | :--- | :--- | :--- |
| Pin 1 | **GND** | **GND** | Common Ground |
| Pin 2 | **VCC** | **3V3** | 3.3V Power |
| Pin 3 | **OUT** | **GPIO 1** | Touch signal output (matches TOUCH_PIN in config.h) |

---

### Option B: Independent-Clock Mode (Alternative Layout)
*Note: This configuration requires pins 15, 16, and 17. Since you cannot use pins 14 to 45, Option A is highly recommended.*

| Peripheral | Module Pin | ESP32-S3 Mini Pin | Description |
| :--- | :--- | :--- | :--- |
| **ST7789 Display** | **SCL** | **GPIO 12** | SPI Clock |
| | **SDA** | **GPIO 11** | SPI MOSI |
| | **RES** | **GPIO 10** | TFT Reset |
| | **DC** | **GPIO 13** | TFT Data/Command |
| | **BLK** | **GPIO 3** | TFT Backlight (or tie to 3V3) |
| **MAX98357 (DAC)** | **LRC** | **GPIO 5** | I2S0 Word Select (WS) |
| | **BCLK** | **GPIO 4** | I2S0 Bit Clock (BCLK) |
| | **DIN** | **GPIO 6** | I2S0 Serial Data Out |
| **INMP441 (Mic)** | **WS** | **GPIO 16** | I2S1 Word Select (WS) |
| | **SCK** | **GPIO 15** | I2S1 Bit Clock (BCLK) |
| | **SD** | **GPIO 17** | I2S1 Serial Data In |
| **Touch Sensor** | **OUT** | **GPIO 1** | Touch signal output |

---

## 🎨 Block Diagram / Wiring Representation

```text
                  +-----------------------------------------+
                  |              ESP32-S3 Mini              |
                  +-----------------------------------------+
                  | GND   3V3   5V   G12   G11   G10   G1   |
                  +--+-----+-----+----+-----+-----+---+-----+
                     |     |     |    |     |     |   |
                     |     |     +----+-----+-----+---+-----+
                     |     |          |     |     |   |     |
      +--------------+     |          |     |     |   |     |
      |                    +-----+    |     |     |   |     |
      |                          |    |     |     |   |     |
+-----+-----+                    |    |     |     |   |     |
|   INMP441 |                    |    |     |     |   |     |
|   (Mic)   |                    |    |     |     |   |     |
|           |                    |    |     |     |   |     |
| 1. GND    +--------------------+    |     |     |   |     |
| 2. VDD    +--------------------+    |     |     |   |     |
| 3. L/R    +----+               |    |     |     |   |     |
| 4. SCK    +----+---------+     |    |     |     |   |     |
| 5. WS     +----+-----+   |     |    |     |     |   |     |
| 6. SD     +----+---+ |   |     |    |     |     |   |     |
+-----------+    |   | |   |     |    |     |     |   |     |
                 v   v v   v     v    v     v     v   v     v
             (GND) (G7)(G5)(G4)(3V3)(G12)(G11)(G10)(G1)(G03)
                 ^   ^ ^   ^     ^    ^     ^     ^   ^     ^
+-----------+    |   | |   |     |    |     |     |   |     |
|  ST7789   |    |   | |   |     |    |     |     |   |     |
|  Display  |    |   | |   |     |    |     |     |   |     |
|           |    |   | |   |     |    |     |     |   |     |
| 1. GND    +----+   | |   |     |    |     |     |   |     |
| 2. VCC    +--------+------------+    |     |     |   |     |
| 3. SCL    +--------------------------+     |     |   |     |
| 4. SDA    +--------------------------------+     |   |     |
| 5. RES    +--------------------------------------+   |     |
| 6. DC     +------------------------------------------+     |
| 7. BLK    +------------------------------------------------+
+-----------+
                 ^   ^ ^   ^     ^                     ^
                 |   | |   |     |                     |
+-----------+    |   | |   |     |                     |
| MAX98357  |    |   | |   |     |                     |
| (DAC)     |    |   | |   |     |                     |
|           |    |   | |   |     |                     |
| 1. GND    +----+   | |   |     |                     |
| 2. Vin    +--------+---+-------+------> (5V / VIN Rail)
| 3. LRC    +--------+---+ |     |
| 4. BCLK   +------------+       |
| 5. DIN    +--------------------+------> (GPIO 6)
+-----------+
                 ^               ^                     ^
                 |               |                     |
+-----------+    |               |                     |
| Touch     |    |               |                     |
| Sensor    |    |               |                     |
|           |    |               |                     |
| 1. GND    +----+               |                     |
| 2. VCC    +--------------------+                     |
| 3. OUT    +------------------------------------------+
+-----------+
```

---

## 💻 Code Configurations

Here are standard configuration snippets for configuring libraries inside the Arduino IDE or PlatformIO.

### 1. Display Configuration (`TFT_eSPI` library - `User_Setup.h`)
If using the popular `TFT_eSPI` library, define the following in your config headers:

```cpp
#define TFT_MISO             -1 // Not connected (7-pin display has no MISO/SO line)
#define TFT_MOSI             11 // SDA Pin
#define TFT_SCLK             12 // SCL Pin
#define TFT_CS               -1 // Not connected (7-pin ST7789 ties CS low internally)
#define TFT_DC               13 // DC Pin
#define TFT_RST              10 // RES Pin
#define TFT_BL               3  // BLK Pin
#define TFT_BACKLIGHT_ON   HIGH // Backlight turns on when BL pin is HIGH
```

### 2. Touch Sensor Configuration
In your `config.h` file, ensure you have the touch pin configured on GPIO 1:

```cpp
#define TOUCH_PIN            1  // Capacitive Touch Sensor signal input pin
```

### 3. Duplex I2S Configuration (Standard Arduino `I2S.h` Library)
If you are initializing I2S for both playback and recording (Option A - Shared Clocks), use the following configuration in your Arduino sketch:

```cpp
#include <I2S.h>

const int i2s_bclk = 4; // Shared Bit Clock
const int i2s_ws   = 5; // Shared Word Select
const int i2s_dout = 6; // Data out to MAX98357
const int i2s_din  = 7; // Data in from INMP441

void setupI2S() {
  // Initialize I2S in full-duplex (playback & record)
  I2S.setAllPins(i2s_bclk, i2s_ws, i2s_dout, i2s_din, -1);
  
  // Start I2S at 16kHz sample rate, 16-bit depth
  if (!I2S.begin(I2S_PHILIPS_MODE, 16000, 16)) {
    Serial.println("Failed to initialize I2S duplex!");
    while (1);
  }
}
```

---

## ⚠️ Troubleshooting & Design Best Practices

1. **Audio Noise & Hissing (DAC):**
   * I2S modules are sensitive to electrical noise. Try putting a **100µF capacitor** across the `Vin` and `GND` pins of the MAX98357 module as close as possible to the board to filter out supply ripple.
   * Keep I2S signal wires as short as possible to minimize inductive crosstalk.
2. **Microphone DC Offset (INMP441):**
   * The INMP441 output signal often has a tiny DC offset. In software, it is highly recommended to implement a digital **High-Pass Filter (HPF)** at ~100Hz on the input buffer to clear the DC bias and low-frequency room noise.
3. **ST7789 Visual Glitches:**
   * ST7789 displays can support high SPI clocks up to 40MHz. However, if you experience visual glitches or missing lines, reduce the SPI clock speed in your code to **27MHz** or **20MHz** to ensure signal integrity.
   * If your module lacks a **CS (Chip Select)** pin, standard SPI libraries might expect a CS pin to toggle. Ensure your library is configured with `TFT_CS = -1` or similar setting to prevent it from trying to toggle a non-existent pin.
