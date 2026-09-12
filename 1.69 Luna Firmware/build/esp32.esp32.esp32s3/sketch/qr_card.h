#line 1 "W:\\Mr.mario\\1.69 Luna Firmware\\qr_card.h"
// =============================================================================
// qr_card.h — Luna Digital Business Card
// Uses the QRCode library by Richard Moore for reliable, scannable QR codes.
// Protocol: "QRCARD:<url>"  received via BLE text characteristic
// "QRCARD:CLEAR"            clears stored card from NVS
// Button 2 cycles to this screen; Button 1 single exits back to GIF face.
// Compatible with 1.3" ST7789 (240x240)
// =============================================================================
#ifndef QR_CARD_H
#define QR_CARD_H

#include <Arduino.h>
#include <Preferences.h>
// Use local copy of QRCode lib (Richard Moore) to avoid conflict with ESP32 built-in esp_qrcode
#include "qrcode_lib.h"
#include "config.h"

#define QR_NVS_NS  "luna_qr"   // NVS namespace
#define QR_NVS_URL "qr_url"    // NVS key for stored URL
#define QR_NVS_CFG "qr_set"    // NVS key for configured flag

// Max QRCode buffer for version 10 (57×57 modules = 408 bytes)
// Using 512 as a safe upper bound
#define QR_MAX_BUF 512

class LunaQR {
public:
  bool configured = false;
  char storedUrl[256] = {0};
  
  // QR generation cache to optimize framerate
  bool qrGenerated = false;
  QRCode qrcode;
  uint8_t qrcodeData[QR_MAX_BUF];

  // ── NVS: Load stored card URL on boot ──────────────────────────────────────
  void begin() {
    Preferences prefs;
    prefs.begin(QR_NVS_NS, true);
    configured = prefs.getBool(QR_NVS_CFG, false);
    String url = prefs.getString(QR_NVS_URL, "");
    prefs.end();

    if (configured && url.length() > 0) {
      strncpy(storedUrl, url.c_str(), 255);
      storedUrl[255] = '\0';
      qrGenerated = false;
      Serial.println("[QR] Loaded stored URL: " + url);
    } else {
      strncpy(storedUrl, "https://shop.crabstertech.in", 255);
      storedUrl[255] = '\0';
      configured = true;
      qrGenerated = false;
      Serial.println("[QR] Default URL active: https://shop.crabstertech.in");
    }
  }

  // ── NVS: Save a new card URL ────────────────────────────────────────────────
  bool saveUrl(const String& url) {
    if (url.length() == 0 || url.length() > 255) return false;
    Preferences prefs;
    prefs.begin(QR_NVS_NS, false);
    prefs.putString(QR_NVS_URL, url);
    prefs.putBool(QR_NVS_CFG, true);
    prefs.end();
    strncpy(storedUrl, url.c_str(), 255);
    storedUrl[255] = '\0';
    configured = true;
    qrGenerated = false; // Invalidate cache
    Serial.println("[QR] Saved URL: " + url);
    return true;
  }

  // ── NVS: Clear card from device memory ─────────────────────────────────────
  void clearCard() {
    Preferences prefs;
    prefs.begin(QR_NVS_NS, false);
    prefs.remove(QR_NVS_URL);
    prefs.putBool(QR_NVS_CFG, false);
    prefs.end();
    configured = false;
    qrGenerated = false; // Invalidate cache
    memset(storedUrl, 0, sizeof(storedUrl));
    Serial.println("[QR] Card cleared from NVS.");
  }

  // ── Render QR screen onto GFX canvas ───────────────────────────────────────
  // Called every frame while SCREEN_CARD is active.
  void drawQRScreen(GFXcanvas16& display) {
    display.fillScreen(0xFFFF); // white background

    if (!configured || storedUrl[0] == '\0') {
      _drawNoCardScreen(display);
      return;
    }

    if (!qrGenerated) {
      // ── Pick minimum QR version for the URL length (ECC_MEDIUM) ──────────────
      int urlLen = strlen(storedUrl);
      uint8_t ver;
      if      (urlLen <= 20)  ver = 2;
      else if (urlLen <= 32)  ver = 3;
      else if (urlLen <= 50)  ver = 4;
      else if (urlLen <= 64)  ver = 5;
      else if (urlLen <= 84)  ver = 6;
      else if (urlLen <= 93)  ver = 7;
      else if (urlLen <= 122) ver = 8;
      else if (urlLen <= 154) ver = 9;
      else                    ver = 10;

      int8_t result = qrcode_initText(&qrcode, qrcodeData, ver, ECC_MEDIUM, storedUrl);
      if (result != 0) {
        // Try one version higher with LOW ECC as fallback
        result = qrcode_initText(&qrcode, qrcodeData, ver + 1 > 10 ? 10 : ver + 1, ECC_LOW, storedUrl);
      }
      if (result != 0) {
        Serial.println("[QR] ERROR: Could not generate QR code for URL.");
        _drawErrorScreen(display);
        return;
      }
      qrGenerated = true;
      Serial.printf("[QR] Generated V%d QR (%dx%d) for: %s\n", ver, qrcode.size, qrcode.size, storedUrl);
    }

    // ── Calculate rendering dimensions ───────────────────────────────────────
    // Available vertical space: SCREEN_HEIGHT minus bottom label minus top margin
    int availH = SCREEN_HEIGHT - 28;
    int availW = SCREEN_WIDTH;

    // Use 2-module quiet zone to maximise pixel size on the screen
    int quietModules = 2;
    int totalModules = qrcode.size + quietModules * 2;

    // Choose module pixel size — biggest that fits
    int modPx = min(availW, availH) / totalModules;
    if (modPx < 1) modPx = 1;
    // Cap module pixel size to a higher value for 240x240 screens
    int maxModPx = (SCREEN_WIDTH >= 240) ? 8 : 4;
    if (modPx > maxModPx) modPx = maxModPx;

    int qrRenderedPx = totalModules * modPx;
    int offsetX = (SCREEN_WIDTH  - qrRenderedPx) / 2 + quietModules * modPx;
    int offsetY = (availH        - qrRenderedPx) / 2 + quietModules * modPx + 2;

    // White quiet-zone background rectangle
    int bgX = offsetX - quietModules * modPx;
    int bgY = offsetY - quietModules * modPx;
    display.fillRect(bgX, bgY, qrRenderedPx, qrRenderedPx, 0xFFFF);

    // ── Draw each module ──────────────────────────────────────────────────────
    for (int y = 0; y < qrcode.size; y++) {
      for (int x = 0; x < qrcode.size; x++) {
        uint16_t color = qrcode_getModule(&qrcode, x, y) ? 0x0000 : 0xFFFF;
        display.fillRect(offsetX + x * modPx, offsetY + y * modPx, modPx, modPx, color);
      }
    }

    // ── Bottom label ──────────────────────────────────────────────────────────
    int txtSize = (SCREEN_WIDTH >= 240) ? 2 : 1;
    display.setTextSize(txtSize);
    display.setTextColor(0x0000);
    const char* label = "SCAN ME";
    int labelW = strlen(label) * (6 * txtSize);
    display.setCursor((SCREEN_WIDTH - labelW) / 2, SCREEN_HEIGHT - (11 * txtSize));
    display.print(label);
  }

private:
  void _drawNoCardScreen(GFXcanvas16& display) {
    display.setTextWrap(true);
    int txtSize = (SCREEN_WIDTH >= 240) ? 2 : 1;
    int startY = (SCREEN_HEIGHT - (3 * txtSize * 12)) / 2;
    display.setTextSize(txtSize);
    display.setTextColor(0x0000);

    String line1 = "No Business Card";
    String line2 = "Open Luna app and";
    String line3 = "sync your QR card.";

    int charWidth = 6 * txtSize;
    display.setCursor((SCREEN_WIDTH - line1.length() * charWidth) / 2, startY);
    display.println(line1);
    display.setCursor((SCREEN_WIDTH - line2.length() * charWidth) / 2, startY + txtSize * 14);
    display.println(line2);
    display.setCursor((SCREEN_WIDTH - line3.length() * charWidth) / 2, startY + txtSize * 28);
    display.println(line3);
  }

  void _drawErrorScreen(GFXcanvas16& display) {
    int txtSize = (SCREEN_WIDTH >= 240) ? 2 : 1;
    int startY = (SCREEN_HEIGHT - (2 * txtSize * 12)) / 2;
    int charWidth = 6 * txtSize;
    display.setTextSize(txtSize);

    display.setTextColor(0xF800); // red
    String line1 = "QR Error";
    display.setCursor((SCREEN_WIDTH - line1.length() * charWidth) / 2, startY);
    display.println(line1);

    display.setTextColor(0x0000);
    String line2 = "URL too long?";
    display.setCursor((SCREEN_WIDTH - line2.length() * charWidth) / 2, startY + txtSize * 14);
    display.println(line2);
  }
};

extern LunaQR qrCard;

#endif // QR_CARD_H
