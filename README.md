# 1.69" ESP32-S3 Modular Touch Device: Hardware Specifications

---

## 1. Modular Detach Architecture

The hardware platform utilizes a **Modular Detach Design** dividing the system into a **Docked Desktop Base** and a **Detachable Portable Core**:

```
 ┌─────────────────────────────────────────────────────────────┐
 │                    ATTACHED / DOCKED STATE                  │
 │                     Stationary Desk Base                    │
 │  • Continuously powered via multi-pin pogo dock contacts    │
 │  • Internal LiPo battery maintained under constant charge   │
 │  • Upright desktop viewing angle                            │
 └──────────────────────────────┬──────────────────────────────┘
                                │
                        [ DETACH ACTION ]
                (Quick-release magnetic / pogo dock)
                                │
                                ▼
 ┌─────────────────────────────────────────────────────────────┐
 │                    DETACHED / PORTABLE STATE                │
 │               Standalone Wearable / Pocket Unit             │
 │  • Powered autonomously by onboard LiPo battery             │
 │  • Power-hold latch (GPIO 41) prevents accidental shutdown  │
 │  • Calibrated ADC voltage divider (GPIO 1) battery sensing  │
 │  • 10 kHz PWM backlight dimming (GPIO 15) for power saving  │
 │  • Full capacitive touch, 6-axis IMU, and RTC operational   │
 └─────────────────────────────────────────────────────────────┘
```

### 1.1 Attached Mode (Docked Desktop Base)
* **Interface**: High-durability spring-loaded gold pogo pins and alignment magnets.
* **Power Delivery**: Directly supplied via external USB-C (5V VBUS converted to system 3.3V rail).
* **Battery Charging**: Constant-current / constant-voltage charging path charges the internal LiPo cell while powering the main board.

### 1.2 Detached Mode (Portable / Wearable Unit)
* **Form Factor**: Self-contained, ultra-thin touch module (usable as a wrist wearable or portable pocket device).
* **Power Source**: Internal 3.7V single-cell Lithium-Polymer (LiPo) battery.
* **Power Latch (`PWR_HOLD`)**: GPIO 41 is driven continuously `HIGH` by the microcontroller to latch the power supply circuit and maintain active state.
* **Battery Monitoring**: ADC1_CH0 (GPIO 1) reads battery level across a precision resistor divider with a calibrated multiplier of $3.10\times$.
* **Power Optimization**: High-frequency 10 kHz PWM backlight control (GPIO 15) provides fine-grained display brightness dimming to extend battery runtime.

---

## 2. Hardware Specifications

| Component | Technical Specification | Details / Remarks |
| :--- | :--- | :--- |
| **Microcontroller (MCU)** | **ESP32-S3** (Xtensa® 32-bit dual-core LX7) | Clock frequency up to **240 MHz** |
| **System Memory** | **16 MB** Quad SPI Flash, **8 MB** Octal (OPI) PSRAM | High-bandwidth PSRAM supports off-screen framebuffers |
| **Wireless Connectivity** | 2.4 GHz Wi-Fi (802.11 b/g/n) & Bluetooth 5.0 (BLE) | Onboard ceramic antenna |
| **Display Panel** | **1.69" IPS TFT LCD** | Resolution: **$240 \times 280\text{ pixels}$**, 16-bit RGB565 color depth |
| **Display Driver IC** | Sitronix **ST7789** | High-speed SPI interface up to **40 MHz** (hardware offset: $Y = 20\text{ px}$) |
| **Backlight Control** | High-efficiency LED Driver | Driven via **GPIO 15** with **10 kHz PWM** dimming |
| **Capacitive Touch IC** | Hynitron **CST816T** Single-point Capacitive Touch | I2C address: `0x15` @ 400 kHz, interrupt-driven (`FALLING` edge) |
| **Real-Time Clock (RTC)**| NXP **PCF85063A** Precision RTC | I2C address: `0x51` @ 400 kHz, ultra-low power timekeeping |
| **Inertial Sensor (IMU)**| QST **QMI8658** 6-Axis Motion Sensor | I2C address: `0x6B` (fallback `0x6A`), 3-axis accelerometer + 3-axis gyroscope |
| **Audio / Haptics** | Passive Piezoelectric Buzzer | Driven via **GPIO 42** using square-wave PWM frequencies |
| **Battery Monitoring** | Internal Resistor Voltage Divider to ADC | Connected to **GPIO 1** (ADC1_CH0), calibrated multiplier $3.10\times$ |
| **Power Hold Latch** | Active Power Retention Circuit | **GPIO 41** driven `HIGH` to maintain device power |
| **Dock Interface** | Surface Pogo-Pin Contacts | $V_{BUS}$, $3.3\text{V}$, `GND`, and battery charge rails |
| **Operating Voltage** | 3.3V Logic Level / 3.7V–4.2V LiPo Battery Input | Dual-path power routing (USB / Battery) |

---

## 3. Complete Pinout & GPIO Assignment

| Peripheral | Signal Label | ESP32-S3 GPIO | Bus / Protocol | Functional Description |
| :--- | :--- | :--- | :--- | :--- |
| **Display (ST7789)** | `MOSI` | **GPIO 7** | SPI Master Out | Pixel data stream via SPI DMA |
| | `SCK` | **GPIO 6** | SPI Clock | 40 MHz SPI serial clock line |
| | `CS` | **GPIO 5** | SPI Chip Select | Active LOW panel select |
| | `DC` | **GPIO 4** | Data / Command | Command: `LOW`, Data: `HIGH` |
| | `RST` | **GPIO 8** | Panel Reset | Active LOW hardware reset |
| | `BLK` | **GPIO 15** | Backlight Control | 10 kHz PWM backlight brightness control |
| **Touch (CST816T)** | `SDA` | **GPIO 11** | I2C Data | Shared I2C data line (400 kHz) |
| | `SCL` | **GPIO 10** | I2C Clock | Shared I2C clock line (400 kHz) |
| | `INT` | **GPIO 14** | Touch Interrupt | Falling-edge hardware touch detection |
| | `RST` | **GPIO 13** | Touch Reset | Hardware reset for touch controller |
| **RTC (PCF85063A)** | `SDA` / `SCL` | **GPIO 11** / **GPIO 10** | I2C (`0x51`) | Shared I2C bus for time/date registers |
| **IMU (QMI8658)** | `SDA` / `SCL` | **GPIO 11** / **GPIO 10** | I2C (`0x6B`) | Shared I2C bus for 6-axis acceleration/gyro |
| **Audio Buzzer** | `BUZZER` | **GPIO 42** | PWM / Tone | Square-wave audio and haptic chime output |
| **Battery Sensing** | `BAT_ADC` | **GPIO 1** | ADC1_CH0 | Analog voltage reading for LiPo charge level |
| **Power Management**| `PWR_HOLD` | **GPIO 41** | Digital Output | Must be driven `HIGH` to maintain power |

---

## 4. Subsystem Electrical & Interface Details

### 4.1 Display & Backlight Subsystem
- **Bus**: 4-line Serial Peripheral Interface (SPI).
- **Pixel Clock**: Configured for 40 MHz (operational up to 80 MHz).
- **Physical Resolution**: $240 \times 280\text{ pixels}$ (active display area offset vertically by 20 pixels relative to the controller RAM window).
- **Backlight Driver**: Controlled via GPIO 15 using a 10 kHz PWM carrier frequency to prevent audible coil whine and display flicker.

### 4.2 Capacitive Touch Subsystem
- **Controller**: CST816T single-point capacitive touch processor.
- **Interface**: I2C slave at address `0x15` on GPIO 11 (SDA) and GPIO 10 (SCL).
- **Interrupt Line**: Connected to GPIO 14, triggering on `FALLING` edge to instantly wake the CPU from sleep or trigger immediate event polling.

### 4.3 Motion & Real-Time Clock Subsystems
- **RTC (PCF85063A)**: Connected to the primary I2C bus at address `0x51`, providing year, month, day, weekday, hours, minutes, and seconds.
- **IMU (QMI8658)**: 6-axis motion tracking unit on the primary I2C bus at address `0x6B`, providing 3-axis linear acceleration and 3-axis rotational angular velocity.

### 4.4 Power Management & Detach Circuitry
- **Power-Hold Mechanism**: When detached from the dock, the device power latch requires GPIO 41 to remain driven `HIGH`. If driven `LOW`, the power rail cuts off to provide true zero-quiescent off-state.
- **Battery Divider**: An internal resistive voltage divider scales the battery voltage (up to 4.20V) down into the ESP32-S3 ADC1 range ($0\text{V} - 3.1\text{V}$), read via GPIO 1 and scaled by a $3.10\times$ calibration factor.
