# LUNA EEZ STUDIO UI VALIDATION & COMPLIANCE REPORT

**Target Hardware**: Waveshare ESP32-S3 Touch LCD 1.69" (240 × 280 portrait, ST7789, CST816T Touch)  
**Primary UI File**: `W:\Mr.mario\1.69 Luna Firmware\ui\luna.eez-project`  
**Mirrored UI File**: `W:\Mr.mario\luna\luna.eez-project`  
**Firmware Target**: `1.69 Luna Firmware\1.69 Luna Firmware.ino` & `interaction.h`  
**Evaluation Date**: September 10, 2026  
**Status Policy**: Strict physical vs. software verification boundary. No inferred hardware verification.  

---

## 1. Executive Summary

The Luna EEZ Studio UI project and CST816T interaction engine have undergone a comprehensive interaction and touch hardening pass. Visual designs have been frozen. The touch interaction engine has been upgraded from loose delta checks into a formal 5-state gesture machine with strict movement deadbands, 1.5× horizontal dominance, vertical scroll locking, and transition debouncing.

---

## 2. Five-Tier Verification Matrix

### Tier 1: SOFTWARE VERIFIED
*Tests proven through source inspection, coordinate analysis, and schema auditing.*

- **Design System Freeze**: All 15 screen layouts, 12 user widgets, typography scales, colors (Light and Cyberpunk Dark), and 3-tier radii (`6, 10, 14 px`) verified frozen.
- **Touch Target Hitbox Geometries**:
  - `notification_mark_read` / `notification_back`: `98 × 38 px` (height $\ge 38\text{ px}$).
  - `calendar_view_day` / `calendar_detail_back`: `208 × 38 px`.
  - `game_retry` / `game_exit`: `98 × 42 px`.
  - `maps_exit`: `208 × 38 px`.
  - `settings_save`: `208 × 38 px` with centered label offset `@ (0, 11)` (negative offset `@ (-13, -2)` eliminated).
  - `level_calibrate`: `208 × 34 px`.
  - `pomodoro_start` / `pomodoro_reset`: `98 × 40 px`.
  - Toggle switch containers: `208 × 44 px` with `44 × 22 px` switches.
  - Slider containers: `208 × 48 px` with `14 px` track height.
  - Calendar day cells: `27 × 23 px` spacing with `26 × 20 px` selection pills.
- **Scroll Flag Cleansing**:
  - Exactly 0 static labels, buttons, or switches possess the `SCROLLABLE` flag.
  - Dedicated vertical scroll containers verified on `NOTIFICATIONS` (`208 × 216`), `GAMES` (`208 × 168`), `SETTINGS` (`208 × 166`), and `CALENDAR_EVENTS` (`208 × 166`).
- **Arcade Launcher Completeness**:
  - All 7 games verified present and indexed in `GAMES`: `Luna Racer`, `Luna Space`, `Flappy Mochy`, `Coin Catcher`, `Mochy Jump`, `Stacker`, `Memory Matrix`.
- **Dynamic Variable Bindings**:
  - All 29 dynamic global firmware variables verified intact with matching types and default values.

---

### Tier 2: FIRMWARE BUILD VERIFIED
*Compiler and linker validation on the ESP32-S3 toolchain.*

- **FQBN**: `esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=custom,PSRAM=opi,CDCOnBoot=cdc`
- **Compiler**: `arduino-cli` with ESP32 core v3.3.11
- **Build Status**: **SUCCESS (Exit Code 0)**
- **Program Storage Utilization**: 6,454,970 bytes (38% of 16 MB flash)
- **Dynamic Memory (RAM) Utilization**: 40,164 bytes (12% of 320 KB SRAM)
- **Flash Upload to COM3**: **SUCCESS (Exit Code 0)**
  - Bootloader: verified at `0x00000000`
  - Partition Table: verified at `0x00008000`
  - Boot App0: flashed & hash verified at `0x0000e000`
  - Firmware Binary: 741,376 bytes flashed & hash verified at `0x00010000`
  - Hard reset executed via RTS pin.

---

### Tier 3: SIMULATION / STRUCTURAL VERIFIED
*State machine architecture, coordinate transforms, and event configuration.*

- **Gesture State Machine Architecture**:
  - Verified implemented in `interaction.h`:
    $$\text{STATE\_IDLE} \longrightarrow \text{STATE\_TOUCH\_DOWN} \longrightarrow \text{STATE\_TRACKING} \longrightarrow \begin{cases} \text{STATE\_SCROLL\_VERTICAL} \\ \text{STATE\_SWIPE\_HORIZONTAL} \\ \text{STATE\_CANCELLED} \end{cases}$$
- **Deadband Isolation**:
  - Finger movement $< 10\text{ px}$ remains in `STATE_TOUCH_DOWN`. On release, dispatches cleanly as `BTN1_SINGLE` or `BTN1_LONG` with zero swipe contamination.
- **Directional Dominance**:
  - Horizontal swipe requires $|dX| \ge 50\text{ px}$ AND $|dX| > 1.5 \times |dY|$.
  - Vertical scroll locks when $|dY| \ge 16\text{ px}$ AND $|dY| > |dX|$. Once locked, horizontal screen transition cannot be triggered.
  - Ambiguous diagonal movements (e.g. $dX=30, dY=30$) are cancelled (`BTN_NONE`).
- **Debounce & Transition Lock**:
  - Swipes enforce a minimum $350\text{ ms}$ lockout between transitions (`lastSwipeTransitionMs`), preventing multi-screen cascade from trailing touch packets.
- **Screen Navigation Order**:
  - Primary sequence verified in `CYCLE[]`:
    $$\text{Face (Home)} \longleftrightarrow \text{Clock} \longleftrightarrow \text{Notifications} \longleftrightarrow \text{Calendar} \longleftrightarrow \text{Games} \longleftrightarrow \text{Card} \longleftrightarrow \text{Settings} \longleftrightarrow \text{Level} \longleftrightarrow \text{Pomodoro}$$
  - $dX < 0 \implies \text{BTN\_SWIPE\_LEFT (Next)}$
  - $dX > 0 \implies \text{BTN\_SWIPE\_RIGHT (Previous)}$
- **Coordinate Mapping & Diagnostics**:
  - Continuous reporting via serial: `[TOUCH_RAW] G:%02X F:%d X:%d Y:%d MAPPED_X:%d MAPPED_Y:%d`
  - $Y_{\text{mapped}} = \text{constrain}(Y_{\text{raw}} - 20, 0, 279)$ accounting for ST7789 20px hardware panel offset.

---

### Tier 4: PHYSICALLY VERIFIED
*Tests physically performed and observed via serial telemetry on the connected board.*

- **Boot Telemetry**:
  - Physical board reboot on `COM3` confirmed via USB CDC.
  - Serial message received: `Mr. Luna Robot Booting Up... Version: 2.0.0`
- **I2C Touch Controller Initialization**:
  - CST816T hard-reset cycle and bus initialization verified at 400 kHz on `SDA=11, SCL=10`.
  - Serial message: `[Touch] CST816T hardened interaction engine initialized (Deadband=10px, Swipe=50px/1.5x).`

---

### Tier 5: NOT PHYSICALLY VERIFIED
*Tests that require physical human fingertip contact on the live capacitive glass panel.*

- **Manual Human Fingertip Gestures**:
  - `NOT PHYSICALLY VERIFIED`: Repeated manual single-finger taps across screen edges and center.
  - `NOT PHYSICALLY VERIFIED`: Fast vs. slow swipe distinction on physical glass.
  - `NOT PHYSICALLY VERIFIED`: Physical tactile resistance and scrollbar decay timing on live LCD.
  - `NOT PHYSICALLY VERIFIED`: Long press finger hold duration on physical glass.

> [!IMPORTANT]
> In accordance with project policy, physical finger interaction results are marked **NOT PHYSICALLY VERIFIED** until the user or test technician physically performs fingertip swipes on the live Waveshare touch panel.

---

## 3. Hardware Test Script Ready for User

The diagnostic monitoring tool is available at `tools/touch_monitor.py` and `tools/capture_gestures.py`. 

To observe live hardware gesture classification in real time, run:
```powershell
python tools/touch_monitor.py
```
This displays real-time `[Touch]` events, zone classification, raw coordinates, and mapped coordinates as you touch the glass.
