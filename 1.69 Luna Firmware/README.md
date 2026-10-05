# 🌙 Waveshare 1.69" Luna Smartwatch & Companion Firmware

A comprehensive embedded firmware, UI engine, and companion OS engineered for the **Waveshare ESP32-S3 Touch LCD 1.69"** smartwatch and desktop companion hardware.

---

## 🛠️ 1. Hardware Specifications

| Component | Specification / Part Number | Pinout & Bus Details | Function / Remarks |
| :--- | :--- | :--- | :--- |
| **Microcontroller (MCU)** | ESP32-S3 (Xtensa® 32-bit dual-core LX7 @ 240 MHz) | Internal SoC | 16 MB Flash, 8 MB Octal SPI (OPI) PSRAM |
| **Display Panel** | 1.69" IPS TFT LCD (240 × 280 pixels, 16-bit RGB565) | `MOSI`: GPIO 7<br>`SCK`: GPIO 6<br>`CS`: GPIO 5<br>`DC`: GPIO 4<br>`RST`: GPIO 8 | Controller: **ST7789**<br>Bus: SPI @ **40 MHz**<br>Hardware panel offset: $Y = 20\text{ px}$ |
| **Backlight Control** | High-efficiency LED Driver | `BLK`: GPIO 15 | 10 kHz PWM dimming (eliminates optical flicker and coil whine) |
| **Capacitive Touch** | Hynitron **CST816T** Single-point Capacitive Touch | `SDA`: GPIO 11<br>`SCL`: GPIO 10<br>`INT`: GPIO 14<br>`RST`: GPIO 13 | I2C address: `0x15` @ 400 kHz<br>Hardware interrupt-driven (`FALLING` edge) |
| **Real-Time Clock (RTC)** | NXP **PCF85063A** Precision RTC | `SDA`: GPIO 11<br>`SCL`: GPIO 10 | I2C address: `0x51`<br>Battery-backed timekeeping with auto-build timestamp seed |
| **Motion Sensor (IMU)** | QST **QMI8658** 6-Axis Inertial Measurement Unit | `SDA`: GPIO 11<br>`SCL`: GPIO 10 | I2C address: `0x6B` (fallback `0x6A`)<br>3-axis Accelerometer & 3-axis Gyroscope |
| **Audio & Haptics** | Passive Piezoelectric Buzzer | `BUZZER`: GPIO 42 | Polyphonic & retro square-wave tone synthesizer (`LunaAudio`) |
| **Battery Monitor** | Internal Resistor Voltage Divider | `BAT_ADC`: GPIO 1 | Calibrated multiplier: `3.10f`<br>Calculates LiPo charge percentage (up to 4.20V) |
| **Power Retention** | Power Hold Pin | `PWR_HOLD`: GPIO 41 | Driven HIGH continuously to prevent system sleep/cut-off |

---

## ⚡ 2. Software Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Luna Smartwatch OS                     │
├───────────────┬────────────────────────────┬────────────────┤
│ Interaction   │ UI Engine & Rendering      │ Subsystems     │
├───────────────┼────────────────────────────┼────────────────┤
│ CST816T I2C   │ Double-buffered PSRAM      │ PCF85063A RTC  │
│ State Machine │ GFXcanvas16 (240x280)      │ QMI8658 IMU    │
│ Touch Columns │ Adafruit GFX + Custom Path │ LunaAudio      │
│ Kinetic Drag  │ DMA Blit to ST7789 @ 40MHz │ BLE Peripheral │
│ Debounce/Slop │ Status Bar & Popup Layer   │ NVS Storage    │
└───────────────┴────────────────────────────┴────────────────┘
```

1. **Double-Buffered PSRAM Canvas**:
   - High-speed `GFXcanvas16` allocated directly in 8MB PSRAM.
   - All visual elements, vector fonts, cards, and animations render off-screen first and push via 40 MHz SPI DMA, delivering tear-free, crisp 30+ FPS graphics.
2. **Kinetic Touch Engine (`interaction.h`)**:
   - Multi-phase gesture recognition: Tap, Double Tap, Long Press, Horizontal Screen Swipes, and Vertical Scrolling.
   - Touch Slop (10 px threshold) & Scroll Latch: suppresses accidental button clicks when the user intends to scroll.
   - Smooth momentum & velocity decay physics for Settings and Arcade list browsing.
3. **Persistent Non-Volatile Storage (NVS)**:
   - Uses ESP32 `Preferences.h` (`"luna"` and `"luna_qr"` namespaces).
   - Preserves user settings (brightness, silent mode, clock style, animation speed, BLE status), up to 20 calendar events/alarms, game high scores, and digital business card URLs across power cycles.
4. **Bluetooth Low Energy (BLE)**:
   - Custom GATT service (`4fafc201-1fb5-459e-8fcc-c5c9c331914b`) with dedicated characteristics for bidirectional synchronization:
     - Real-time notification pushes (with app name, title, message preview).
     - Clock time & calendar event syncing.
     - Live turn-by-turn navigation data forwarding.
     - Remote expression triggering & companion cloud matchmaking.

---

## 🎨 3. UI Component Library

The firmware features a tailored, high-contrast vector design system:

| UI Component | Description & Visual Layout |
| :--- | :--- |
| **Status Bar Header** | Persistent 22 px top header displaying: current time (`HH:MM`), screen category pill (e.g. `CLOCK`, `NOTIFS`, `ARCADE`), battery level icon with live fill & percentage, BLE connection indicator, and silent mode mute icon. |
| **Notification Card** | High-contrast rounded cards (10 px radius) featuring app source tags (e.g. WhatsApp, System), timestamp, truncated preview, and unread dot indicator. |
| **WhatsApp Quick Reply Sheet** | Modal action sheet overlay offering predefined quick responses (`"On my way!"`, `"Call you later"`, `"OK"`) sent directly back to the phone via BLE. |
| **Monthly Calendar Grid** | 7-column monthly grid tailored for 240 px width with day labels (`M`, `T`, `W`, `T`, `F`, `S`, `S`) and highlighted active date indicator. |
| **Event Item Card** | Agenda entry card displaying event type pill (`ALARM`, `MEETING`, `REMINDER`, `BDAY`), event time, and title string. |
| **Radial Progress Arc** | Anti-aliased circular gauge widget with dynamic color fill used for elapsed vs. remaining Pomodoro focus intervals. |
| **Settings List Item** | Interactive scrollable rows with category icons, title, status readout (`LOW/MED/HIGH`, `ON/OFF`), and right-chevron indicator. |
| **High-Contrast 2D QR Code** | On-the-fly QR code generator using Richard Moore's embedded QR library. Dynamically scales modules (Versions 2–10) with quiet zones for optical scannability. |
| **Notification Popup Toast** | Real-time overlay banner interrupting screens to alert incoming calls or urgent messages without exiting current background activity. |

---

## 📱 4. Screen-by-Screen Documentation

The firmware includes **9 distinct screen modes** arranged in an intuitive navigation loop:

```
[Clock Home] ⇄ [Notifications] ⇄ [Calendar] ⇄ [Turn-by-Turn Maps] ⇄ [Pomodoro Focus]
     ⇅
[Companion/Pet] ⇄ [Digital Business Card] ⇄ [System Settings] ⇄ [Arcade Games]
```

### 1. Clock Screen (`SCREEN_CLOCK`)
- **Primary Function**: Main watch home screen providing glanceable time, date, and device health.
- **Features**:
  - Displays large format hours and minutes, live pulsing seconds counter, current weekday, and calendar date.
  - Features **2 Distinct Visual Clock Styles**:
    - *Style 0 (Luna OS Digital)*: Clean minimalist modern sans-serif typography with vertical layout.
    - *Style 1 (Aerospace Chrono)*: Technical instrumentation layout with circular dials and telemetry details.
- **Interactions**:
  - **Center Tap**: Toggles between Clock Style 0 and Style 1.
  - **Swipe Left / Right Tap**: Advances to **Notifications Screen**.
  - **Swipe Right / Left Tap**: Returns to **Companion/Pet Screen**.

---

### 2. Notifications Screen (`SCREEN_NOTIFICATIONS`)
- **Primary Function**: Smartwatch message notification inbox.
- **Features**:
  - Stores incoming notifications synced over BLE from the companion smartphone app.
  - List View: Shows up to 3 notification cards simultaneously with app name, sender, and preview text.
  - Detail View: Full message reader with word-wrap formatting.
  - WhatsApp Integration: When viewing a WhatsApp notification, provides a dedicated **Quick Reply** action sheet.
- **Interactions**:
  - **Tap Card**: Opens full Notification Detail View.
  - **Tap Quick Reply (in WhatsApp detail)**: Opens canned quick responses and transmits selection back to phone.
  - **Tap Return / Dismiss**: Exits Detail View back to Notifications list.
  - **Long Press (>= 500 ms)**: Clears all unread notifications.
  - **Swipe Left / Edge Right Tap**: Moves to **Calendar Screen**.
  - **Swipe Right / Edge Left Tap**: Returns to **Clock Screen**.

---

### 3. Calendar & Schedule Screen (`SCREEN_CALENDAR`)
- **Primary Function**: Personal agenda, schedule viewer, and monthly calendar.
- **Features**:
  - Dual-mode visualization:
    1. *Monthly Calendar View*: 7×5 grid view highlighting the current day.
    2. *Agenda Events View*: Chronological list of saved alarms, reminders, meetings, and birthday events retrieved from NVS memory.
  - Active alarm ringing overlay when an event matches the RTC timestamp.
- **Interactions**:
  - **Center Tap**: Toggles view mode (Monthly Grid $\leftrightarrow$ Event Schedule List).
  - **Swipe Left / Edge Right Tap**: Advances to **Turn-by-Turn Maps Screen** (if navigation active) or **Pomodoro Focus Screen**.
  - **Swipe Right / Edge Left Tap**: Returns to **Notifications Screen**.

---

### 4. Turn-by-Turn Navigation Screen (`SCREEN_MAPS`)
- **Primary Function**: Live GPS heads-up navigation display forwarded from phone.
- **Features**:
  - Automatically activates when navigation route starts on companion app.
  - Displays vector maneuver arrows: Turn Left, Turn Right, Keep Left/Right, Roundabout, U-Turn, Straight.
  - Large remaining distance readout (e.g. `250 m`, `1.2 km`), estimated time of arrival (ETA), and target street name.
- **Interactions**:
  - **Tap Screen**: Acknowledges maneuver instruction / toggles detail mode.
  - **Swipe Left / Edge Right Tap**: Moves to **Pomodoro Focus Screen**.
  - **Swipe Right / Edge Left Tap**: Returns to **Calendar Screen**.

---

### 5. Focus & Pomodoro Timer Screen (`SCREEN_POMODORO`)
- **Primary Function**: Productivity focus timer using the Pomodoro technique.
- **Features**:
  - **3 Focus Modes**:
    - `FOCUS`: 25 minutes work interval.
    - `SHORT BREAK`: 5 minutes rest interval.
    - `LONG BREAK`: 15 minutes recovery interval.
  - Dynamic circular progress arc showing visual countdown.
  - Completed sessions counter indicator.
  - Buzzer alarm chiming at session transition.
- **Interactions**:
  - **Tap Top Mode Pill**: Cycles between Focus, Short Break, and Long Break.
  - **Tap Center / Timer**: Starts, Pauses, or Resumes the countdown timer.
  - **Double Tap / Long Press**: Resets timer back to initial interval duration.
  - **Swipe Left / Edge Right Tap**: Advances to **Arcade Games Screen**.
  - **Swipe Right / Edge Left Tap**: Returns to previous screen.

---

### 6. Arcade Games Screen (`SCREEN_GAMES`)
- **Primary Function**: Embedded retro gaming suite with **7 playable titles**.
- **Features**:
  - **Arcade Launcher Menu**: Vector gamepad illustration with game title selection and high scores saved to NVS.
  - **7 Built-in Games**:
    1. **Luna Racer**: High-speed highway obstacle avoidance game with nitro boost and acceleration mechanics.
    2. **Space Defender**: Classic retro space shooter with laser cannons, scrolling starfield, and enemy waves.
    3. **Flappy Luna**: Physics-based flapping bird game with pipe obstacles and collision detection.
    4. **Luna Catcher**: Falling items catching game with speed progression.
    5. **Luna Jump**: Vertical jumping platformer using accelerometer or touch controls.
    6. **Luna Stacker**: Precision timing block-stacking arcade tower builder.
    7. **Luna Memory**: Simon-style visual and auditory sequence memory challenge.
- **Interactions**:
  - **Tap "PLAY GAMES >"**: Launches the Arcade selection carousel.
  - **Vertical Swipe / Scroll**: Browses through the game catalog with inertia.
  - **Tap Selected Game**: Launches game into full-screen active play.
  - **During Gameplay**: Touch left/right or tap according to specific game mechanics.
  - **Exit Game**: Touch exit trigger or long press to return to Arcade Menu.
  - **Swipe Left / Edge Right Tap**: Moves to **Settings Screen**.
  - **Swipe Right / Edge Left Tap**: Returns to **Pomodoro Focus Screen**.

---

### 7. System Settings Screen (`SCREEN_SETTINGS`)
- **Primary Function**: Device configuration dashboard.
- **Features**:
  - Smooth-scrolling list of 7 interactive system options:
    1. **Display Brightness**: Adjusts backlight between Low, Medium, and High (10 kHz PWM).
    2. **Sound FX**: Toggles synthesizer buzzer between Mute (Silent) and Sound Active.
    3. **Clock Face Style**: Toggles default clock theme (Minimal Digital vs. Aerospace Chrono).
    4. **Animation Speed**: Configures frame timing delay (50 ms, 100 ms, 150 ms).
    5. **Bluetooth LE**: Enables or disables BLE advertising and radio server.
    6. **Save Settings**: Commits all modified parameters permanently to NVS flash memory.
    7. **Exit Settings**: Exits menu directly back to Clock Home.
- **Interactions**:
  - **Vertical Swipe / Drag**: Smoothly scrolls the settings list with inertia physics.
  - **Tap Option Row**: Cycles or adjusts the tapped setting value immediately.
  - **Tap "Save Settings"**: Stores preferences and emits confirmation chime.
  - **Tap "Exit Settings"**: Returns directly to **Clock Screen**.
  - **Swipe Left / Edge Right Tap**: Moves to **Digital Business Card Screen**.
  - **Swipe Right / Edge Left Tap**: Returns to **Arcade Games Screen**.

---

### 8. Digital Business Card Screen (`SCREEN_CARD`)
- **Primary Function**: Contact sharing, social profile, or website quick-link via scannable QR code.
- **Features**:
  - Dynamically encodes any URL (up to 255 characters) stored in NVS into an on-screen QR Code.
  - Configurable over BLE via command: `QRCARD:<your_url>`.
  - High-contrast pure black-on-white rendering with safe optical quiet margins for instant smartphone scanning.
  - Displays registered owner / company label below the QR code.
- **Interactions**:
  - **Single Tap Anywhere**: Exits card view and returns to **Clock Screen**.
  - **Swipe Left / Edge Right Tap**: Advances to **Companion/Pet Screen**.
  - **Swipe Right / Edge Left Tap**: Returns to **Settings Screen**.

---

### 9. Companion / Pet Screen (`SCREEN_FACE`)
- **Primary Function**: Expressive robot desktop companion with interactive pet animations.
- **Features**:
  - Dynamic robotic eye expressions (Happy, Sad, Angry, Surprised, Sleeping, Winking, Thinking, Sick).
  - **Interactive Feeding Menu**: Center 3 touch circles:
    - Left Circle: Feed Salad (`BTN_CIRCLE_LEFT`)
    - Middle Circle: Drink Milk (`BTN_CIRCLE_MID`)
    - Right Circle: Feed Fish (`BTN_CIRCLE_RIGHT`)
  - Remote synchronization with partner devices via Firebase cloud triggers.
- **Interactions**:
  - **Center Tap**: Immediately navigates to **Clock Screen**.
  - **Tap Feeding Circles**: Triggers corresponding feeding animation and synth sound effect.
  - **Swipe Right / Left Tap**: Moves to **Digital Business Card Screen**.

---

## 🕹️ 5. Navigation & Gesture Summary

| Gesture / Action | Screen Context | Triggered Result |
| :--- | :--- | :--- |
| **Swipe Left** ($\leftarrow$) | Any Screen (except during active game) | Advances to **Next Screen** in the rotation loop |
| **Swipe Right** ($\rightarrow$) | Any Screen (except during active game) | Returns to **Previous Screen** in the rotation loop |
| **Right Edge Tap** ($X \ge 225\text{ px}$) | Any Non-Clock Screen | Alternative single-touch trigger for **Next Screen** |
| **Left Edge Tap** ($X < 15\text{ px}$) | Any Non-Clock Screen | Alternative single-touch trigger for **Previous Screen** |
| **Right Side Tap** ($X \ge 190\text{ px}$) | Clock Screen | Advances to **Notifications Screen** |
| **Left Side Tap** ($X < 40\text{ px}$) | Clock Screen | Returns to **Companion/Pet Screen** |
| **Center Tap** ($40 \le X < 190\text{ px}$) | Clock Screen | Cycles **Clock Visual Style** (0 $\leftrightarrow$ 1) |
| **Center Tap** | Companion Screen | Navigates directly to **Clock Screen** |
| **Center Tap** | Notifications Screen | Opens selected card in **Detail View** |
| **Center Tap** | Calendar Screen | Toggles between **Month Grid** and **Event List** |
| **Center Tap** | Pomodoro Screen | Starts, pauses, or resumes the focus timer |
| **Vertical Drag / Flick** | Settings / Arcade screens | Smooth kinetic scroll with momentum & velocity decay |
| **Long Press** ($\ge 500\text{ ms}$) | Any Screen | Global shortcut: **Return to Clock Home Screen** |
| **Double Tap** | Any Screen | Returns immediately to **Clock Home Screen** |

---

## 💾 6. Build & Flashing Instructions

### Prerequisites
- [Arduino CLI](https://arduino.github.io/arduino-cli/) or Arduino IDE 2.x
- ESP32 Board Core: `esp32:esp32` (v2.0.14+ or v3.x)

### Target Board Configuration
- **FQBN**: `esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=custom,PSRAM=opi,CDCOnBoot=cdc`
- **Flash Size**: 16 MB
- **Partition Scheme**: Custom (`partitions.csv` with SPIFFS / LittleFS allocation)
- **PSRAM**: Octal SPI (OPI) enabled

### Command-Line Compilation & Flash
```powershell
# Compile firmware
arduino-cli compile --fqbn esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=custom,PSRAM=opi,CDCOnBoot=cdc "1.69 Luna Firmware.ino"

# Flash firmware over USB (COM3 or detected port)
arduino-cli upload -p COM3 --fqbn esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=custom,PSRAM=opi,CDCOnBoot=cdc "1.69 Luna Firmware.ino"
```
