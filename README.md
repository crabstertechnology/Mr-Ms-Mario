# 🌙 Mr.&Ms Luna: The Ultimate Interactive Desktop Companion Robots

Welcome to the future of interactive desktop companions! Meet **Mr. Luna** and **Ms. Luna**, a pair of smart, expressive, and connected desk toy robots designed to bring personality, utility, and joy to your workspace. 

Whether you want a charming digital assistant to relay your phone's notifications, a whimsical alarm clock, or a real-time emotional bridge to a loved one's desk, Mr.&Ms Luna are here to light up your day.

---

## 🚀 What is Mr.&Ms Luna? (The Use Cases)

Mr.&Ms Luna are more than just static ornaments; they are dynamic companions designed for several key purposes:

*   **Desk Companionship & Stress Relief:** With over 63 unique facial expressions rendered on a crisp OLED display and synchronized classic sound effects, they react dynamically to your touch, keeping your workspace lively and interactive.
*   **Smart Desktop Assistant:** Connect them to your phone to receive real-time, glanceable updates. Read scrolling text notifications, track meetings, manage alarms, and even view turn-by-turn Google Maps navigation without touching your phone.
*   **Duo Sync (Long-Distance Connection):** Pair a Mr. Luna and a Ms. Luna together. They can communicate over Wi-Fi, mirroring expressions and sending romantic or friendly reactions across desks—whether in the same room or across the globe.
*   **Hacker-Friendly Open Source Project:** Built on the powerful **ESP32-C3 SuperMini** microcontroller, they are completely open-source and customizable. Easily flash new firmware, design custom bitmap animations, or program new interactive behaviors.

---

## 👥 Who are They For?

*   **Couples & Partners (Couple Mode):** The ultimate gift for long-distance or co-working couples. In Couple Mode, the robots share a heartbeat, trigger romantic expressions (like heart-eyes and winks), and play sweet chimes. When you tap your robot, your partner's robot responds!
*   **Best Friends & Besties (Friends Mode):** Perfect for sharing with a close friend. Sync your robots to coordinate friendly greetings, send messages, and share chimes.
*   **Tech Enthusiasts & Makers:** A dream project for anyone interested in IoT, Arduino, Flutter, BLE, or hardware hacking. The clean separation of hardware firmware and mobile app provides a great learning playground.
*   **Professionals & Work-from-Home Heroes:** Anyone looking to spruce up their desk setup. It helps you stay focused by filtering important phone notifications right to your peripheral vision.

---

## 🌟 Feature Breakdown

### 1. Expressive OLED Display (63+ Animations)
Equipped with an SSD1306 128x64 display, the robots render smooth, high-speed procedural and bitmap eye animations:
*   **Dynamic Expressions:** Happy, Sad, Angry, Surprised, Wink, and Sleeping states.
*   **Preloaded Library:** A built-in table of 63 detailed GIF-style animations (e.g., eye roll, looking around, squinting).
*   **Custom Labels:** Display custom status messages alongside expressions.

### 2. Multi-Gesture Touch Sensor
Interact naturally with a simple touch! The capacitive touch sensor supports:
*   **Single Tap:** Wink reaction, quick audio chirp, or skip to a random animation.
*   **Double Tap:** Plays a classic coin chime and triggers a Happy face reaction.
*   **Long Press:** Manually toggle the robot's sleep cycle.
*   **Alarm Dismissal:** Silence active alarms or reminders with a single tap.

### 3. Integrated Audio Synthesizer
A non-blocking piezo buzzer synthesizer outputs charming melodies tailored to the robot's state:
*   Includes chimes for *Power-up*, *Power-down*, *Jump*, *Coin*, *Game Over*, *Chirp*, and *Startup*.

### 4. Smart Notification & Navigation Mirroring
Relay phone alerts directly to your desk companion:
*   **App Notifications:** View incoming messages (WhatsApp, Slack, SMS) in marquee-scrolling format.
*   **Turn-by-Turn Navigation:** Displays custom direction icons (L/R arrows) and remaining distance from Google Maps.
*   **Calendar Reminders:** Sounds alerts for upcoming meetings and birthdays.
*   **Alarms:** Synchronized desktop alarm clock that rings and flashes when your phone alarm goes off.

### 5. Companion Pairing & Relationship Engine
Pair Mr.&Ms Luna through the mobile app and configure:
*   **Besties (Friends Mode):** For synchronized play and shared notifications.
*   **Partners (Couple Mode):** For intimate sharing, romantic chimes, and shared heart animations.

### 6. AI Companion Chat
Use the Flutter companion app to send messages or ask questions to your AI Companion. The robot will display responses on its screen.

### 7. Smart Power Management
*   **Auto-Sleep:** Automatically enters sleep mode after 45 seconds of inactivity to conserve battery power.
*   **Instant Wake:** Tapping the touch sensor instantly wakes the robot up.

---

## 🛠️ How It Works (Technical Overview)

The ecosystem is split into three main components:
1.  **Firmware (`luna_firmware/`):** C++ Arduino code running on the ESP32-C3 SuperMini. Controls the SSD1306 screen, Buzzer, Touch Pin, BLE server, and Wi-Fi WebSockets client.
2.  **Companion App (`mobile_flutter/`):** A beautiful cross-platform Flutter application (iOS/Android) featuring Glassmorphic styling, BLE device management, Wi-Fi configuration, notification listener, and calendar/chat integration.
3.  **Local broker server (`server.py`):** A Python-based WebSocket and HTTP broker to route messages, bridge Wi-Fi-enabled robots, and support compilation workflows.

---

## ⚡ Quick Start Guide

### 1. Flash the Firmware
*   **Arduino IDE:** Open `luna_firmware/luna_firmware.ino`. Select `ESP32C3 Dev Module` as your board, ensure `Adafruit SSD1306` and `Adafruit GFX` libraries are installed, and hit upload.
*   **PlatformIO:** Use the provided configurations to compile and upload via VS Code.
*   **Web Flashing:** Open the companion web dashboard, navigate to the Flasher tab, connect the ESP32 via USB, and write the binary partitions directly.

### 2. Connect the Mobile App
1.  Launch the **Flutter Mobile App** on your smartphone.
2.  Turn on Bluetooth and pair with **"Mr. Luna Robot"** or **"Ms. Luna Robot"**.
3.  Use the App Dashboard to configure your home Wi-Fi SSID and password. Once saved, the robot will automatically transition to Wi-Fi/WebSocket control.
4.  Activate the **Notification Listener Service** inside the app settings to start forwarding notifications and navigation events!

### 3. Pair Companions
*   Pair a second robot in the app.
*   Go to **Robot Relationship Settings** and select **Friends Mode** or **Couple Mode** to sync Mr.&Ms Luna!

---

## 📂 Project Structure

```
├── luna_firmware/       # C++ Arduino firmware for ESP32-C3 SuperMini
│   ├── config.h          # Pins, UUIDs, sound/expression definitions
│   ├── expressions.h     # OLED render loop & bitmap helpers
│   ├── audio.h           # Non-blocking buzzer melody player
│   ├── bluetooth.h       # BLE GATT Server callbacks
│   └── luna_network.h   # Wi-Fi WebSockets and HTTP client
├── mobile_flutter/       # Cross-platform Flutter companion application
│   ├── lib/screens/      # Dashboard and custom UI widgets
│   └── lib/services/     # BLE, local SQLite database, and Notification relayer
├── server.py             # Python-based web app host & WebSocket router
└── README.md             # This document!
```

---

*Enjoy interacting with Mr.&Ms Luna! 🌙✨*