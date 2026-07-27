# 📅 Detailed Work Report (AM / PM Timeline Format)

**Project:** Mr. / Ms. Mario (Luna Firmware & Mobile Application)  
**Author / Developer:** crabstertechnology  
**Time Format:** 12-Hour AM/PM (IST - Indian Standard Time)  

---

## 🕒 Chronological Work Activity Timeline (AM / PM)


### 📅 July 24, 2026

* 🕑 **02:28:52 PM** | `commit e139473`
  * **Feature**: Implemented real-time `OLED Simulator` widget in Flutter (`lib/widgets/oled_simulator.dart`), added OLED display inversion toggle in `BluetoothService`, and updated ESP32 main loop display rendering logic.
* 🕙 **10:05:28 AM** | `commit 68a570a`
  * **Build & Hardware**: Compiled firmware binaries (`luna_firmware.ino.bin`, `.elf`, `.map`), generated 4MB merged flash binary for ESP32-C3, updated partition tables and SDK configuration.

---

### 📅 July 19, 2026

* 🕒 **03:08:00 AM** | `commit b75f456`
  * **Feature**: Added native Kotlin Android platform channels (`MainActivity.kt`) for background audio streaming, PCM sound buffering, updated Flutter database services and main dashboard UI.

---

### 📅 July 18, 2026

* 🕚 **11:07:19 PM** | `commit 39a3ea6`
  * **Feature & Maintenance**: Created user `LoginScreen` in Flutter, revised `MainDashboard` interface, added USB auto-flashing Python script (`identify_and_flash.py`), and removed 10+ obsolete test/server files.
* 🕘 **09:55:10 PM** | `commit 415b4d4`
  * **Feature**: Integrated `FirebaseService` into mobile app for authentication and live pairing. Added multi-target build configurations for **ESP32-C3** and **LOLIN S3 Mini** hardware variants.

---

### 📅 July 16, 2026 (Firmware v2 Refactor Session)

* 🕓 **04:06:44 PM** | `commit 7109c9b`
  * **Firmware**: Updated `LunaFace` class with expression state handling and smartwatch UI modes.
* 🕓 **04:02:52 PM** | `commit be7d965`
  * **Build**: Compiled `luna_firmware_v2` project binary objects and updated compiler database.
* 🕓 **04:02:37 PM** | `commit 56806f2`
  * **Firmware**: Added `expressions.h` header for rendering eyes, facial expressions, and status icons.
* 🕒 **03:58:21 PM** | `commit 77c80e2`
  * **Firmware**: Implemented `LunaFace` class to manage smartwatch UI states, expression transitions, and notification history.
* 🕒 **03:53:15 PM** | `commit fba7e93`
  * **Firmware**: Created bitmap definitions in `expressions.h` for hardware OLED/TFT display.
* 🕒 **03:43:02 PM** | `commit 32a0667`
  * **Build**: Generated new firmware release binaries and memory link map for `luna_firmware_v2`.
* 🕒 **03:40:17 PM** | `commit de92b60`
  * **Build**: Created compilation database (`compile_commands.json`).
* 🕒 **03:34:53 PM** | `commit 9a71948`
  * **Build**: Executed full firmware compilation cycle and linked hardware objects.
* 🕒 **03:30:17 PM** | `commit 400e198`
  * **Firmware**: Built base architecture for Luna robot supporting BLE GATT server, I2S audio driver, and WiFi networking.
* 🕒 **03:16:20 PM** | `commit 805fe0c`
  * **Firmware**: Initialized hardware abstraction layers for BLE stack, OLED driver, and network protocols.
* 🕒 **03:09:18 PM** | `commit fd88bbc`
  * **Mobile App**: Updated Flutter `BluetoothService` to send characteristic commands matching new firmware headers.
* 🕛 **12:25:24 PM** | `commit 8cde7ef`
  * **Firmware**: Created `LunaAudio` class for handling ESP32 I2S hardware playback, sound synthesis, and stream tasks.
* 🕛 **12:25:13 PM** | `commit 3b30f0d`
  * **Mobile App**: Implemented low-latency audio streaming service transferring raw sound packets over BLE to Luna hardware.

---

### 📅 July 12, 2026

* 🕔 **05:39:59 PM** | `commit ab215ab`
  * **Firmware**: Established early firmware v2 folder structure, modularizing audio, bluetooth, and system configuration modules.

---

### 📅 July 11, 2026

* 🕛 **12:08:59 PM** | `commit 9cce489`
  * **Android**: Built Kotlin audio streaming channel and local filesystem access in `MainActivity.kt`.
* 🕚 **11:57:17 AM** | `commit 003b80e`
  * **Android**: Created native audio stream handler and background service orchestrator.
* 🕚 **11:41:32 AM** | `commit 53bfb33`
  * **Firmware**: Updated ESP32-S3 firmware build setup and audio streaming handlers.
* 🕚 **11:13:45 AM** | `commit 6778317`
  * **Mobile App**: Added `BLEService` device management and cross-platform plugin registration.
* 🕚 **11:09:35 AM** | `commit dc84cf6`
  * **Mobile App**: Created main Bluetooth dashboard with live device scanning and connection status.

---

## 📊 Summary Breakdown (AM vs PM Activities)

* **Morning Sessions (10:00 AM – 12:00 PM)**: Firmware binary compilation, ESP32 build option setups, and initial BLE service architecture.
* **Afternoon Sessions (12:00 PM – 05:00 PM)**: Core firmware development (`LunaAudio`, `LunaFace`, `expressions.h`), memory mapping, OLED simulator, and BLE GATT protocol integration.
* **Night Sessions (09:00 PM – 04:00 AM)**: Firebase integration, Android Kotlin native channels, login UI screens, background audio streaming, and test script cleanup.
