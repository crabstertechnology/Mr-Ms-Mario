// =============================================================================
// smart_home.h — Luna Smart Home Relay Controller (ESP32-S3 Touch LCD 1.69")
// High-performance background TCP client & interactive 2x2 touch screen
// =============================================================================
#ifndef SMART_HOME_H
#define SMART_HOME_H

#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include "config.h"

// Smart Home Connection Status
enum SmartHomeStatus {
  SH_STATUS_DISCONNECTED = 0,
  SH_STATUS_CONNECTING,
  SH_STATUS_CONNECTED,
  SH_STATUS_READY,
  SH_STATUS_ERROR
};

class LunaSmartHome {
private:
  // AP and Server Configuration
  const char* ssid = "ESP32_Relay_AP";
  const char* password = "12345678";
  IPAddress receiverIP = IPAddress(192, 168, 4, 1);
  const uint16_t receiverPort = 8080;

  // Relay State & Control
  volatile bool relayState[4];
  volatile int8_t pendingCmd[4]; // 0: None, 1: ON, 2: OFF
  volatile bool requestSync;
  volatile SmartHomeStatus currentStatus;
  String statusMsg;
  unsigned long lastStatusPoll;
  volatile bool activeMode;
  TaskHandle_t networkTaskHandle;
  SemaphoreHandle_t stateMutex;

  // Internal TCP helper
  bool sendTcpCommand(int relay, bool turnOn) {
    if (!activeMode || WiFi.status() != WL_CONNECTED) {
      currentStatus = SH_STATUS_DISCONNECTED;
      statusMsg = "WIFI OFFLINE";
      return false;
    }

    WiFiClient client;
    client.setTimeout(250);
    if (!client.connect(receiverIP, receiverPort)) {
      currentStatus = SH_STATUS_ERROR;
      statusMsg = "RECEIVER OFFLINE";
      return false;
    }

    client.setNoDelay(true);
    String cmd = "R" + String(relay + 1) + (turnOn ? "_ON\n" : "_OFF\n");
    client.print(cmd);

    unsigned long t = millis();
    while (millis() - t < 150) {
      if (client.available()) {
        String ack = client.readStringUntil('\n');
        parseAck(ack);
        client.stop();
        currentStatus = SH_STATUS_READY;
        statusMsg = "READY";
        return true;
      }
      vTaskDelay(pdMS_TO_TICKS(5));
    }

    client.stop();
    currentStatus = SH_STATUS_READY;
    statusMsg = "READY";
    return true;
  }

  // Internal Status Poll
  bool fetchStatus() {
    if (!activeMode || WiFi.status() != WL_CONNECTED) {
      currentStatus = SH_STATUS_DISCONNECTED;
      statusMsg = "WIFI OFFLINE";
      return false;
    }

    WiFiClient client;
    client.setTimeout(250);
    if (!client.connect(receiverIP, receiverPort)) {
      statusMsg = "NO RECEIVER";
      return false;
    }

    client.setNoDelay(true);
    client.print("STATUS\n");

    unsigned long t = millis();
    while (millis() - t < 200) {
      if (client.available()) {
        String ack = client.readStringUntil('\n');
        parseAck(ack);
        client.stop();
        currentStatus = SH_STATUS_READY;
        statusMsg = "READY";
        return true;
      }
      vTaskDelay(pdMS_TO_TICKS(5));
    }

    client.stop();
    return false;
  }

  // Parse ACK string: e.g. "R1=ON R2=OFF R3=OFF R4=ON"
  void parseAck(const String& ack) {
    for (int i = 0; i < 4; i++) {
      int idx = ack.indexOf("R" + String(i + 1) + "=");
      if (idx >= 0) {
        if (ack.startsWith("ON", idx + 3)) {
          relayState[i] = true;
        } else if (ack.startsWith("OFF", idx + 3)) {
          relayState[i] = false;
        }
      }
    }
  }

  // FreeRTOS background task function
  static void taskTrampoline(void* param) {
    LunaSmartHome* self = (LunaSmartHome*)param;
    self->networkLoop();
  }

  void networkLoop() {
    while (true) {
      if (!activeMode) {
        // If Wi-Fi radio is currently ON, disconnect and power down completely to save battery
        if (WiFi.getMode() != WIFI_OFF) {
          WiFi.disconnect(true, true);
          WiFi.mode(WIFI_OFF);
          currentStatus = SH_STATUS_DISCONNECTED;
          statusMsg = "STANDBY (OFF)";
        }
        // Sleep in low power until user navigates to SCREEN_SMART_HOME
        vTaskDelay(pdMS_TO_TICKS(150));
        continue;
      }

      // activeMode is true: User is viewing SCREEN_SMART_HOME!
      // 1. Maintain Wi-Fi Connection
      if (WiFi.status() != WL_CONNECTED) {
        if (WiFi.getMode() != WIFI_STA) {
          WiFi.mode(WIFI_STA);
          WiFi.setSleep(false);
          WiFi.setAutoReconnect(true);
        }
        currentStatus = SH_STATUS_CONNECTING;
        statusMsg = "CONNECTING AP...";
        WiFi.begin(ssid, password);
        WiFi.setTxPower(WIFI_POWER_8_5dBm);
        WiFi.setSleep(false);

        unsigned long startConnect = millis();
        while (activeMode && WiFi.status() != WL_CONNECTED && (millis() - startConnect < 8000)) {
          vTaskDelay(pdMS_TO_TICKS(200));
        }

        if (!activeMode) continue; // User swiped away while connecting!

        if (WiFi.status() == WL_CONNECTED) {
          currentStatus = SH_STATUS_CONNECTED;
          statusMsg = "CONNECTED";
          fetchStatus();
        } else {
          currentStatus = SH_STATUS_DISCONNECTED;
          statusMsg = "AP NOT FOUND";
          vTaskDelay(pdMS_TO_TICKS(2000));
          continue;
        }
      }

      // 2. Process Pending Relay Commands
      bool processedCommand = false;
      for (int i = 0; i < 4; i++) {
        if (!activeMode) break;
        if (pendingCmd[i] != 0) {
          int cmd = pendingCmd[i];
          pendingCmd[i] = 0; // Consume
          bool turnOn = (cmd == 1);
          sendTcpCommand(i, turnOn);
          processedCommand = true;
        }
      }

      // 3. Periodic Status Sync
      unsigned long now = millis();
      if (activeMode && !processedCommand && (requestSync || (now - lastStatusPoll >= 2500))) {
        requestSync = false;
        fetchStatus();
        lastStatusPoll = now;
      }

      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }

public:
  const char* relayNames[4] = {"Fan", "Light", "Strip", "Outdoor"};

  LunaSmartHome() {
    activeMode = false;
    for (int i = 0; i < 4; i++) {
      relayState[i] = false;
      pendingCmd[i] = 0;
    }
    requestSync = false;
    currentStatus = SH_STATUS_DISCONNECTED;
    statusMsg = "STANDBY (OFF)";
    lastStatusPoll = 0;
    networkTaskHandle = NULL;
    stateMutex = NULL;
  }

  void begin() {
    stateMutex = xSemaphoreCreateMutex();
    // Launch FreeRTOS background task pinned to Core 0
    xTaskCreatePinnedToCore(
      taskTrampoline,
      "SmartHomeNet",
      4096,
      this,
      1,
      &networkTaskHandle,
      0 // Core 0 (leaving Core 1 dedicated to display rendering & touch)
    );
  }

  void setActive(bool active) {
    activeMode = active;
    if (active) {
      requestSync = true;
    }
  }

  bool isActive() const {
    return activeMode;
  }

  void update() {
    // Optional main loop hooks
  }

  // Optimistic toggle for instant UI responsiveness
  void toggleRelay(int idx) {
    if (idx < 0 || idx >= 4) return;
    relayState[idx] = !relayState[idx];
    pendingCmd[idx] = relayState[idx] ? 1 : 2;
  }

  void setRelay(int idx, bool on) {
    if (idx < 0 || idx >= 4) return;
    relayState[idx] = on;
    pendingCmd[idx] = on ? 1 : 2;
  }

  void turnAllOff() {
    for (int i = 0; i < 4; i++) {
      relayState[i] = false;
      pendingCmd[i] = 2; // Queue OFF command
    }
  }

  void requestSyncNow() {
    requestSync = true;
  }

  bool getRelayState(int idx) const {
    if (idx < 0 || idx >= 4) return false;
    return relayState[idx];
  }

  bool isWifiConnected() const {
    if (!activeMode) return false;
    return WiFi.status() == WL_CONNECTED;
  }

  bool isReady() const {
    return currentStatus == SH_STATUS_READY;
  }

  SmartHomeStatus getStatus() const {
    return currentStatus;
  }

  String getStatusMessage() const {
    return statusMsg;
  }

  // ---------------------------------------------------------------------------
  // Premium Smartwatch UI Drawing on ST7789 240x280 Canvas
  // ---------------------------------------------------------------------------
  void drawScreen(GFXcanvas16& display) {
    // Clear area below status bar (status bar is Y=0..22)
    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, 0x0000);

    // ── 1. Subtitle & Connectivity Pill (Y=26..44) ───────────────────────────
    int pillX = 12;
    int pillY = 26;
    int pillW = 216;
    int pillH = 18;
    display.fillRoundRect(pillX, pillY, pillW, pillH, 4, 0x0842);
    display.drawRoundRect(pillX, pillY, pillW, pillH, 4, 0x2124);

    // Connectivity Status Dot
    uint16_t dotCol = 0x7BEF; // Default gray
    if (currentStatus == SH_STATUS_READY) {
      dotCol = 0x07E0; // Emerald Green
    } else if (currentStatus == SH_STATUS_CONNECTED) {
      dotCol = 0x07FF; // Cyan
    } else if (currentStatus == SH_STATUS_CONNECTING) {
      dotCol = 0xFD20; // Amber
    } else if (currentStatus == SH_STATUS_ERROR) {
      dotCol = 0xF800; // Red
    }
    display.fillCircle(pillX + 9, pillY + 9, 3, dotCol);

    // Banner Text
    display.setTextSize(1);
    display.setTextColor(0xFFFF);
    display.setCursor(pillX + 18, pillY + 5);
    if (currentStatus == SH_STATUS_READY) {
      display.print("ESP32_Relay_AP  •  READY");
    } else if (currentStatus == SH_STATUS_CONNECTED) {
      display.print("CONNECTED  •  SYNCING...");
    } else if (currentStatus == SH_STATUS_CONNECTING) {
      display.print("CONNECTING  ESP32_Relay_AP");
    } else if (currentStatus == SH_STATUS_ERROR) {
      display.print("RECEIVER OFFLINE (8080)");
    } else {
      display.print("SEARCHING ESP32_Relay_AP");
    }

    // ── 2. 2x2 Relay Cards (Y=48..234) ───────────────────────────────────────
    // Grid Coordinates:
    // Left: X=10, W=106; Right: X=124, W=106
    // Top: Y=48, H=90; Bottom: Y=144, H=90
    const int cardW = 106;
    const int cardH = 90;
    const int xs[4] = {10, 124, 10, 124};
    const int ys[4] = {48, 48, 144, 144};

    for (int i = 0; i < 4; i++) {
      int cx = xs[i];
      int cy = ys[i];
      bool on = relayState[i];

      // Card Background & Glowing Border
      if (on) {
        display.fillRoundRect(cx, cy, cardW, cardH, 8, 0x0185); // Deep cyan-emerald tint
        display.drawRoundRect(cx, cy, cardW, cardH, 8, 0x07E0); // Bright emerald glow
        display.drawRoundRect(cx + 1, cy + 1, cardW - 2, cardH - 2, 7, 0x03E0);
      } else {
        display.fillRoundRect(cx, cy, cardW, cardH, 8, 0x0842); // Obsidian graphite
        display.drawRoundRect(cx, cy, cardW, cardH, 8, 0x2124); // Subtle dark border
      }

      int iconCenterX = cx + 53;
      int iconCenterY = cy + 24;
      uint16_t iconColor = on ? 0x07E0 : 0x6B4D;

      // Draw Custom Vector Icons
      if (i == 0) {
        // --- FAN ICON ---
        display.fillCircle(iconCenterX, iconCenterY, 3, iconColor);
        // 4 Fan Blades
        display.fillTriangle(iconCenterX, iconCenterY, iconCenterX - 4, iconCenterY - 11, iconCenterX + 4, iconCenterY - 11, iconColor);
        display.fillTriangle(iconCenterX, iconCenterY, iconCenterX - 4, iconCenterY + 11, iconCenterX + 4, iconCenterY + 11, iconColor);
        display.fillTriangle(iconCenterX, iconCenterY, iconCenterX - 11, iconCenterY - 4, iconCenterX - 11, iconCenterY + 4, iconColor);
        display.fillTriangle(iconCenterX, iconCenterY, iconCenterX + 11, iconCenterY - 4, iconCenterX + 11, iconCenterY + 4, iconColor);
      } else if (i == 1) {
        // --- LIGHT BULB ICON ---
        uint16_t bulbCol = on ? 0xFFE0 : 0x6B4D;
        display.fillCircle(iconCenterX, iconCenterY - 3, 6, bulbCol);
        display.fillRect(iconCenterX - 3, iconCenterY + 2, 6, 4, bulbCol);
        display.drawFastHLine(iconCenterX - 2, iconCenterY + 7, 4, bulbCol);
        if (on) {
          // Glow Rays
          display.drawLine(iconCenterX - 10, iconCenterY - 3, iconCenterX - 13, iconCenterY - 3, 0xFFE0);
          display.drawLine(iconCenterX + 10, iconCenterY - 3, iconCenterX + 13, iconCenterY - 3, 0xFFE0);
          display.drawLine(iconCenterX - 7, iconCenterY - 10, iconCenterX - 10, iconCenterY - 13, 0xFFE0);
          display.drawLine(iconCenterX + 7, iconCenterY - 10, iconCenterX + 10, iconCenterY - 13, 0xFFE0);
        }
      } else if (i == 2) {
        // --- STRIP / NEON LED ICON ---
        uint16_t stripCol = on ? 0x07FF : 0x6B4D;
        display.drawRoundRect(iconCenterX - 14, iconCenterY - 4, 28, 8, 3, stripCol);
        display.fillCircle(iconCenterX - 9, iconCenterY, 2, on ? 0xF81F : stripCol);
        display.fillCircle(iconCenterX - 3, iconCenterY, 2, on ? 0x07FF : stripCol);
        display.fillCircle(iconCenterX + 3, iconCenterY, 2, on ? 0xF81F : stripCol);
        display.fillCircle(iconCenterX + 9, iconCenterY, 2, on ? 0x07FF : stripCol);
      } else if (i == 3) {
        // --- OUTDOOR ICON ---
        uint16_t houseCol = on ? 0xFD20 : 0x6B4D;
        display.fillTriangle(iconCenterX, iconCenterY - 10, iconCenterX - 11, iconCenterY - 1, iconCenterX + 11, iconCenterY - 1, houseCol);
        display.drawRect(iconCenterX - 8, iconCenterY - 1, 16, 11, houseCol);
        display.fillRect(iconCenterX - 3, iconCenterY + 3, 6, 7, on ? 0xFD20 : houseCol);
      }

      // Device Name Label
      display.setTextSize(1);
      display.setTextColor(on ? 0xFFFF : 0xB596);
      int nameLen = strlen(relayNames[i]);
      int nameX = cx + (cardW - (nameLen * 6)) / 2;
      display.setCursor(nameX, cy + 46);
      display.print(relayNames[i]);

      // Status Pill (Bottom of card)
      int pillWidth = 58;
      int pillHeight = 18;
      int px = cx + (cardW - pillWidth) / 2;
      int py = cy + 62;

      if (on) {
        display.fillRoundRect(px, py, pillWidth, pillHeight, 5, 0x07E0); // Emerald Green
        display.setTextColor(0x0000); // Black bold text
        display.setCursor(px + 23, py + 5);
        display.print("ON");
        display.fillCircle(px + 12, py + 9, 3, 0x0000); // State dot
      } else {
        display.fillRoundRect(px, py, pillWidth, pillHeight, 5, 0x10A2); // Charcoal
        display.drawRoundRect(px, py, pillWidth, pillHeight, 5, 0x2945);
        display.setTextColor(0x7BEF); // Muted silver
        display.setCursor(px + 20, py + 5);
        display.print("OFF");
      }
    }

    // ── 3. Master ALL OFF Action Bar (Y=240..272) ────────────────────────────
    bool anyOn = relayState[0] || relayState[1] || relayState[2] || relayState[3];
    int barX = 10;
    int barY = 240;
    int barW = 220;
    int barH = 32;

    if (anyOn) {
      display.fillRoundRect(barX, barY, barW, barH, 8, 0x2000); // Dark crimson
      display.drawRoundRect(barX, barY, barW, barH, 8, 0xF800); // Glowing red outline
      display.setTextColor(0xFFFF);
    } else {
      display.fillRoundRect(barX, barY, barW, barH, 8, 0x0842); // Graphite
      display.drawRoundRect(barX, barY, barW, barH, 8, 0x2124);
      display.setTextColor(0x7BEF);
    }

    // Power Icon
    int powerX = barX + 22;
    int powerY = barY + 16;
    uint16_t powerCol = anyOn ? 0xF800 : 0x7BEF;
    display.drawCircle(powerX, powerY, 6, powerCol);
    display.drawFastVLine(powerX, powerY - 7, 6, anyOn ? 0xFFFF : 0x7BEF);

    // Master Button Label
    display.setTextSize(1);
    const char* barLabel = "ALL RELAYS OFF";
    int labelX = barX + (barW - (strlen(barLabel) * 6)) / 2 + 8;
    display.setCursor(labelX, barY + 12);
    display.print(barLabel);
  }
};

#endif // SMART_HOME_H
