// ============================================================================
// GENERATED FILE — DO NOT EDIT MANUALLY.
// SOURCE: Luna UI Studio
// REGENERATE FROM STUDIO.
// ============================================================================
#pragma once
#include "luna_gfx_compat.h"
#include <math.h>

extern TFT_eSPI tft;

// ============================================================================
// ── KINESIS / MONOLITH EMBEDDED DRAWING PRIMITIVES ──
// ============================================================================

// ── 1. Monolith Time Hero ──
inline void drawMonolithTime(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* timeStr, const char* secStr, const char* dateStr, uint8_t batteryPct, const char* statusText, uint16_t textCol, uint16_t accentCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.fillCircle(x + 14, y + 14, 3, accentCol);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(statusText, x + 22, y + 10, 1);
  char bBuf[16];
  snprintf(bBuf, sizeof(bBuf), "%d%%", batteryPct);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(0x07E0, bg);
  tft.drawString(bBuf, x + w - 12, y + 10, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(timeStr, x + 12, y + 26, 6);
  int16_t tw = tft.textWidth(timeStr, 6);
  tft.setTextColor(accentCol, bg);
  tft.drawString(secStr, x + 14 + tw, y + 34, 2);
  tft.drawFastHLine(x + 12, y + h - 22, w - 24, border);
  tft.setTextColor(0x8410, bg);
  tft.drawString(dateStr, x + 12, y + h - 16, 1);
  tft.drawFastHLine(x + w - 28, y + h - 13, 16, accentCol);
}

// ── 2. Glance Bar ──
inline void drawGlanceBar(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, const char* detail, uint16_t accentCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.fillRoundRect(x + 6, y + 6, 26, h - 12, 4, 0x18C3);
  tft.drawPixel(x + 19, y + 12, accentCol);
  tft.drawLine(x + 19, y + 12, x + 15, y + 19, accentCol);
  tft.drawLine(x + 15, y + 19, x + 22, y + 19, accentCol);
  tft.drawLine(x + 22, y + 19, x + 17, y + 28, accentCol);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(label, x + 38, y + 8, 1);
  tft.setTextColor(textCol, bg);
  tft.drawString(detail, x + 38, y + 22, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(0x8410, bg);
  tft.drawString(">", x + w - 10, y + 14, 2);
}

// ── 3. Tactile Action Button ──
inline void drawTactileButton(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(label, x + w / 2, y + h / 2, 2);
}

// ── 4. Notification Monolith ──
inline void drawNotificationBlock(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* sender, const char* timeStr, const char* preview, uint8_t unread, uint16_t accentCol, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  if (unread) tft.fillRoundRect(x, y, 4, h, 2, accentCol);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(sender, x + 12, y + 8, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString(timeStr, x + w - 10, y + 8, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(preview, x + 12, y + 26, 1);
  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString("DISMISS >", x + w - 10, y + h - 6, 1);
}

// ── 5. Agenda Glance Item ──
inline void drawAgendaBlock(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* countdown, const char* timeRange, const char* title, const char* location, uint16_t accentCol, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.fillRoundRect(x + 10, y + 8, 54, 16, 4, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(accentCol, 0x18C3);
  tft.drawString(countdown, x + 37, y + 16, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString(timeRange, x + w - 10, y + 10, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 10, y + 32, 2);
  tft.setTextColor(subCol, bg);
  tft.drawString(location, x + 10, y + 56, 1);
}

// ── 6. Game Hero Launcher ──
inline void drawGameLauncher(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* title, const char* genre, const char* highScore, uint16_t accentCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.fillRoundRect(x + 12, y + 12, 36, 36, 8, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(accentCol, 0x18C3);
  tft.drawString("G", x + 30, y + 30, 4);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString("ACTIVE", x + w - 12, y + 14, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 12, y + 58, 2);
  tft.setTextColor(0x8410, bg);
  tft.drawString(genre, x + 12, y + 82, 1);
  tft.setTextColor(accentCol, bg);
  tft.drawString(highScore, x + 12, y + 100, 2);
  tft.fillRoundRect(x + 12, y + h - 42, w - 24, 30, 6, accentCol);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(0x0000, accentCol);
  tft.drawString("PLAY NOW >", x + w / 2, y + h - 27, 2);
}

// ── 7. Focus / Pomodoro Chamber ──
inline void drawFocusChamber(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* timeRemaining, const char* modeLabel, const char* sessionTag, uint8_t progress, uint16_t accentCol, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(modeLabel, x + 12, y + 10, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString(sessionTag, x + w - 12, y + 10, 1);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(timeRemaining, x + w / 2, y + 56, 6);
  tft.drawRoundRect(x + 12, y + 92, w - 24, 8, 3, border);
  tft.fillRoundRect(x + 13, y + 93, ((w - 26) * progress) / 100, 6, 2, accentCol);
  tft.fillRoundRect(x + 12, y + h - 38, w - 24, 26, 6, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textCol, 0x18C3);
  tft.drawString("PAUSE SESSION", x + w / 2, y + h - 25, 1);
}

// ── 8. Digital ID & QR Card ──
inline void drawQRUtilityCard(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* title, const char* subtitle, const char* idTag, uint16_t accentCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 12, y + 10, 1);
  tft.setTextColor(0x8410, bg);
  tft.drawString(subtitle, x + 12, y + 24, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(idTag, x + w - 12, y + 10, 1);
  int qX = x + w / 2 - 32;
  int qY = y + 42;
  tft.fillRect(qX - 4, qY - 4, 72, 72, 0xFFFF);
  const uint8_t qrMatrix[8] = { 0xE7, 0x95, 0x99, 0xE7, 0x2A, 0xEB, 0x8C, 0xE7 };
  for (int rIdx = 0; rIdx < 8; rIdx++) {
    for (int cIdx = 0; cIdx < 8; cIdx++) {
      if ((qrMatrix[rIdx] >> (7 - cIdx)) & 1) {
        tft.fillRect(qX + cIdx * 8, qY + rIdx * 8, 8, 8, 0x0000);
      }
    }
  }
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(0x8410, bg);
  tft.drawString("SCAN TO PAIR LUNA", x + w / 2, y + h - 8, 1);
}

// ── 9. IMU 6-Axis Spirit Level ──
inline void drawIMULevelCard(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* title, const char* pitch, const char* roll, const char* status, uint16_t accentCol, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 12, y + 10, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(status, x + w - 12, y + 10, 1);
  int cX = x + w / 2;
  int cY = y + 62;
  tft.drawCircle(cX, cY, 24, border);
  tft.drawCircle(cX, cY, 12, border);
  tft.fillCircle(cX + 3, cY - 2, 5, accentCol);
  tft.drawFastHLine(x + 12, y + h - 26, w - 24, border);
  tft.setTextDatum(BL_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString("PITCH:", x + 16, y + h - 8, 1);
  tft.setTextColor(textCol, bg);
  tft.drawString(pitch, x + 60, y + h - 8, 1);
  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(roll, x + w - 16, y + h - 8, 1);
  tft.setTextColor(subCol, bg);
  tft.drawString("ROLL:", x + w - 60, y + h - 8, 1);
}

// ── 10. Tactile Setting Row ──
inline void drawSettingRow(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, const char* valueStr, const char* sublabel, uint8_t checked, uint16_t accentCol, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(label, x + 12, y + 8, 1);
  if (sublabel && strlen(sublabel) > 0) {
    tft.setTextColor(subCol, bg);
    tft.drawString(sublabel, x + 12, y + 26, 1);
  }
  if (valueStr && strlen(valueStr) > 0) {
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(accentCol, bg);
    tft.drawString(valueStr, x + w - 44, y + 16, 1);
  }
  int sX = x + w - 38;
  int sY = y + h / 2 - 8;
  tft.fillRoundRect(sX, sY, 28, 16, 8, checked ? accentCol : 0x2945);
  tft.fillCircle(checked ? sX + 20 : sX + 8, sY + 8, 6, 0xFFFF);
}

// ── 11. Device Specifications Readout ──
inline void drawDeviceSpecCard(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* devName, const char* soc, const char* mem, const char* power, const char* buses, uint16_t accentCol, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(devName, x + 12, y + 10, 2);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString("VERIFIED", x + w - 12, y + 12, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString("SoC", x + 12, y + 36, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(soc, x + w - 12, y + 36, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString("Memory", x + 12, y + 54, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(mem, x + w - 12, y + 54, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString("Power", x + 12, y + 72, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(power, x + w - 12, y + 72, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString("Buses", x + 12, y + 90, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(buses, x + w - 12, y + 90, 1);
  tft.drawFastHLine(x + 12, y + h - 22, w - 24, border);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString("LUNA OS v2.5 • ESP32-S3", x + w / 2, y + h - 6, 1);
}

// ============================================================================
// ── CONTROLLED LUNA COMPONENT PRIMITIVES EMBEDDED DRAWING ROUTINES ──
// ============================================================================

inline void drawLunaButton(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(label, x + w / 2, y + h / 2, 2);
}

inline void drawLunaIconButton(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* iconStr, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(iconStr, x + w / 2, y + h / 2, 2);
}

inline void drawLunaSurface(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* title, const char* sub, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 12, y + 10, 2);
  tft.setTextColor(subCol, bg);
  tft.drawString(sub, x + 12, y + 32, 1);
}

inline void drawLunaListItem(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* title, const char* sub, uint8_t unread, uint16_t textCol, uint16_t subCol, uint16_t accentCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  if (unread) tft.fillRoundRect(x, y, 4, h, 2, accentCol);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 12, y + 8, 2);
  tft.setTextColor(subCol, bg);
  tft.drawString(sub, x + 12, y + 26, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString(">", x + w - 10, y + 14, 2);
}

inline void drawLunaToggle(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, uint8_t checked, uint16_t activeCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(label, x + 12, y + h / 2, 2);
  int sX = x + w - 44;
  int sY = y + h / 2 - 9;
  tft.fillRoundRect(sX, sY, 32, 18, 9, checked ? activeCol : 0x2167);
  tft.fillCircle(checked ? sX + 23 : sX + 9, sY + 9, 7, 0xFFFF);
}

inline void drawLunaSlider(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, int16_t val, uint16_t accentCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(0x8410, bg);
  tft.drawString(label, x + 12, y + 8, 1);
  char buf[16];
  snprintf(buf, sizeof(buf), "%d%%", val);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(buf, x + w - 12, y + 8, 1);
  int tX = x + 12;
  int tY = y + 30;
  int tW = w - 24;
  tft.fillRoundRect(tX, tY, tW, 8, 4, 0x18C3);
  int fillW = constrain((val * tW) / 100, 0, tW);
  tft.fillRoundRect(tX, tY, fillW, 8, 4, accentCol);
  tft.fillCircle(tX + fillW, tY + 4, 8, 0xFFFF);
  tft.drawCircle(tX + fillW, tY + 4, 8, accentCol);
}

inline void drawLunaProgress(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, int16_t val, uint16_t fillCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  int tX = x + 12;
  int tY = y + h / 2 - 4;
  int tW = w - 24;
  tft.fillRoundRect(tX, tY, tW, 8, 4, 0x18C3);
  int fillW = constrain((val * tW) / 100, 0, tW);
  tft.fillRoundRect(tX, tY, fillW, 8, 4, fillCol);
}

inline void drawLunaIndicator(int16_t x, int16_t y, int16_t w, int16_t h, const char* label, uint16_t col) {
  tft.fillRoundRect(x, y, w, h, h / 2, 0x10A4);
  tft.drawRoundRect(x, y, w, h, h / 2, col);
  tft.fillCircle(x + 10, y + h / 2, 3, col);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(col, 0x10A4);
  tft.drawString(label, x + 18, y + h / 2, 1);
}

inline void drawLunaHeader(int16_t x, int16_t y, int16_t w, int16_t h, const char* title, uint16_t bg, uint16_t border, uint16_t textCol) {
  tft.fillRect(x, y, w, h, bg);
  tft.drawFastHLine(x, y + h - 1, w, border);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 14, y + h / 2, 2);
}

inline void drawLunaNavigation(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t activeIdx, uint8_t total, uint16_t activeCol, uint16_t inactiveCol) {
  int totalW = total * 12;
  int startX = x + (w - totalW) / 2;
  int cY = y + h / 2;
  for (uint8_t i = 0; i < total; i++) {
    if (i == activeIdx) {
      tft.fillRoundRect(startX + i * 12, cY - 2, 10, 4, 2, activeCol);
    } else {
      tft.fillCircle(startX + i * 12 + 2, cY, 2, inactiveCol);
    }
  }
}

inline void drawLunaTimer(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* timeStr, const char* label, int16_t progress, uint16_t accentCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(label, x + 14, y + 10, 1);
  int cX = x + w / 2;
  int cY = y + 78;
  tft.drawCircle(cX, cY, 44, 0x18C3);
  tft.drawCircle(cX, cY, 45, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(timeStr, cX, cY, 4);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(0x15D0, bg);
  tft.drawString("ACTIVE", cX, y + h - 10, 1);
}

inline void drawLunaNumber(int16_t x, int16_t y, int16_t w, int16_t h, const char* valStr, const char* unitStr, const char* labelStr, uint16_t col, uint16_t subCol) {
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(subCol, 0x0841);
  tft.drawString(labelStr, x, y, 1);
  tft.setTextColor(col, 0x0841);
  tft.drawString(valStr, x, y + 14, 4);
  int tw = tft.textWidth(valStr, 4);
  tft.setTextColor(subCol, 0x0841);
  tft.drawString(unitStr, x + tw + 4, y + 20, 1);
}

inline void drawLunaStatus(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, uint16_t col) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.fillCircle(x + 8, y + h / 2, 2, col);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(col, bg);
  tft.drawString(label, x + 15, y + h / 2, 1);
}

// ── Legacy Elements ──
// ── Glass Card ──
inline void drawGlassCard(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* title, const char* sub, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 10, y + 10, 2);
  tft.setTextColor(0x7BEF, bg);
  tft.drawString(sub, x + 10, y + 32, 1);
}

// ── Stat Card ──
inline void drawStatCard(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, const char* value, const char* unit, uint16_t accent, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(accent, bg);
  tft.drawString(label, x + 8, y + 8, 1);
  tft.setTextColor(textCol, bg);
  tft.drawString(value, x + 8, y + 20, 4);
  tft.setTextColor(accent, bg);
  tft.drawString(unit, x + 8 + tft.textWidth(value, 4), y + 26, 1);
}

// ── Transaction Card ──
inline void drawTransactionCard(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, uint16_t iconBg, const char* title, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.fillRoundRect(x + 8, y + 8, 32, h - 16, 6, iconBg);
  tft.setTextColor(0xFFFF, iconBg);
  tft.drawString("$", x + 18, y + 16, 2);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 48, y + 18, 2);
  tft.setTextColor(0x7BEF, bg);
  tft.drawString(">", x + w - 16, y + 16, 2);
}

// ── Retro Arcade Box ──
inline void drawRetroArcadeBox(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t bg, uint16_t border, const char* title, const char* score, uint16_t textCol) {
  tft.fillRect(x, y, w, h, bg);
  tft.drawRect(x, y, w, h, border);
  tft.drawRect(x + 3, y + 3, w - 6, h - 6, border);
  tft.setTextColor(border, bg);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(title, x + 8, y + 8, 1);
  tft.setTextColor(textCol, bg);
  tft.drawString(score, x + 8, y + 22, 2);
}

// ── Neon Glow Button ──
inline void drawNeonGlowButton(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.drawRoundRect(x + 1, y + 1, w - 2, h - 2, r, border);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, x + w / 2, y + h / 2, 2);
}

// ── Retro Button ──
inline void drawRetroButton(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t bg, uint16_t border, const char* label, uint16_t textCol) {
  tft.fillRect(x, y, w, h, bg);
  tft.drawRect(x, y, w, h, border);
  tft.fillRect(x + w, y + 4, 4, h - 4, border);
  tft.fillRect(x + 4, y + h, w - 4, 4, border);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, x + w / 2, y + h / 2, 1);
}

// ── Download Button ──
inline void drawDownloadButton(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, const char* label, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("v", x + 18, y + h / 2, 2);
  tft.drawString(label, x + w / 2 + 6, y + h / 2, 2);
}

// ── Neon Yellow Button (Damithkumara) ──
inline void drawNeonYellowButton(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t bg, uint16_t border, const char* label, uint16_t textCol) {
  tft.fillRect(x, y, w, h, bg);
  tft.drawRect(x, y, w, h, border);
  tft.drawRect(x + 1, y + 1, w - 2, h - 2, border);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, x + w / 2, y + h / 2, 2);
}

// ── Happy Coding Button (UIverse Neumorphic) ──
inline void drawHappyCodingButton(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t bg, uint16_t border, const char* label, uint16_t textCol) {
  // Beveled neumorphic rounded button with 3D inset bottom lip (#D6D6E7 -> 0xD6B9)
  tft.fillRoundRect(x, y, w, h, 8, bg);
  tft.drawRoundRect(x, y, w, h, 8, border);
  tft.drawRoundRect(x + 1, y + 1, w - 2, h - 2, 7, border);
  tft.fillRoundRect(x + 2, y + h - 5, w - 4, 3, 2, 0xD6B9);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, x + w / 2, y + (h - 3) / 2, 2);
}

// ── Toggle Switch ──
inline void drawToggleSwitch(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t on, const char* label, uint16_t accent, uint16_t bg) {
  uint16_t trackBg = on ? accent : 0x4208;
  tft.fillRoundRect(x, y + (h - 14) / 2, 36, 14, 7, trackBg);
  int16_t knobX = on ? x + 36 - 14 : x + 2;
  tft.fillCircle(knobX + 6, y + h / 2, 6, 0xFFFF);
  tft.setTextColor(on ? accent : 0x7BEF, bg);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(label, x + 42, y + h / 2, 2);
}

// ── Spinner Loader ──
inline void drawSpinnerLoader(int16_t x, int16_t y, int16_t w, int16_t h, const char* label, uint16_t accent, uint16_t bg) {
  int16_t cx = x + w / 2, cy = y + h / 2 - 8, r = 14;
  tft.drawCircle(cx, cy, r, 0x4208);
  tft.drawCircle(cx, cy, r, accent);
  tft.fillCircle(cx, cy - r, 4, accent);
  tft.setTextColor(accent, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, cx, y + h - 10, 1);
}

// ── Dot Loader ──
inline void drawDotLoader(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t accent) {
  int16_t cx = x + w / 2, cy = y + h / 2;
  for (int i = 0; i < 3; i++) tft.fillCircle(cx - 16 + i * 16, cy, 5, accent);
}

// ── Pulse Loader ──
inline void drawPulseLoader(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t accent) {
  int16_t cx = x + w / 2, cy = y + h / 2;
  tft.drawCircle(cx, cy, 18, accent);
  tft.drawCircle(cx, cy, 12, accent);
  tft.fillCircle(cx, cy, 6, accent);
}

// ── Neon Checkbox ──
inline void drawNeonCheckbox(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t checked, const char* label, uint16_t accent, uint16_t bg, uint16_t textCol) {
  uint16_t boxBg = checked ? 0x07FF : bg;
  tft.fillRoundRect(x, y + (h - 16) / 2, 16, 16, 3, boxBg);
  tft.drawRoundRect(x, y + (h - 16) / 2, 16, 16, 3, checked ? accent : 0x4208);
  if (checked) { tft.setTextColor(bg, boxBg); tft.setTextDatum(MC_DATUM); tft.drawString("v", x + 8, y + h / 2, 1); }
  tft.setTextColor(textCol, 0x0000);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(label, x + 22, y + h / 2, 2);
}

// ── Progress Bar ──
inline void drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t val, const char* label, uint16_t fill, uint16_t bg, uint16_t border) {
  tft.setTextColor(fill, 0x0000);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(label, x, y, 1);
  tft.fillRoundRect(x, y + 14, w, h - 14, 4, bg);
  tft.drawRoundRect(x, y + 14, w, h - 14, 4, border);
  int16_t fw = (int32_t)(w - 4) * val / 100;
  if (fw > 0) tft.fillRoundRect(x + 2, y + 16, fw, h - 18, 3, fill);
}

// ── Digital Clock ──
inline void drawDigitalClock(int16_t x, int16_t y, const char* timeStr, const char* dateStr, uint16_t col, uint16_t dateCol) {
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(col);
  tft.drawString(timeStr, x + 100, y + 26, 6);
  tft.setTextColor(dateCol);
  tft.drawString(dateStr, x + 100, y + 58, 2);
}

// ── Custom Label ──
inline void drawCustomLabel(int16_t x, int16_t y, int16_t w, int16_t h, const char* text, uint8_t fontSize, uint16_t col, uint16_t bg) {
  tft.fillRoundRect(x, y, w, h, 6, bg);
  tft.setTextColor(col, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(text, x + w / 2, y + h / 2, fontSize > 14 ? 2 : 1);
}

// ── Alert Badge ──
inline void drawAlertBadge(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, const char* title, const char* msg, uint16_t bg, uint16_t accent, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, accent);
  tft.fillCircle(x + 16, y + h / 2, 4, accent);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 28, y + 8, 2);
  tft.setTextColor(accent, bg);
  tft.drawString(msg, x + 28, y + 26, 1);
}

// ── Text Input ──
inline void drawTextInput(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, const char* placeholder, uint16_t bg, uint16_t border, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(placeholder, x + 10, y + h / 2, 1);
}

// ── Generic Element fallback ──
inline void drawGenericElement(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, const char* label, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, textCol);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, x + w / 2, y + h / 2, 1);
}

// ── Pattern Elements (User Requested) ──
inline void drawPatternStars(int16_t x, int16_t y, int16_t w, int16_t h) {
  tft.fillRect(x, y, w, h, 0x0842);
  const uint16_t sX[] = { 15, 42, 68, 92, 115, 140, 185, 210, 230, 30, 75, 120, 160, 200, 50, 85, 130, 175, 220, 10 };
  const uint16_t sY[] = { 20, 15, 45, 30, 55, 25, 40, 18, 50, 80, 95, 70, 85, 90, 130, 145, 120, 135, 150, 175 };
  for (int i = 0; i < 20; i++) {
    int px = x + (sX[i] % (w > 0 ? w : 240));
    int py = y + (sY[i] % (h > 0 ? h : 280));
    tft.drawPixel(px, py, 0xFFFF);
  }
}

inline void drawPatternCyberGrid(int16_t x, int16_t y, int16_t w, int16_t h, int16_t sz, uint16_t lineCol, uint16_t bgCol) {
  tft.fillRect(x, y, w, h, bgCol);
  if (sz < 8) sz = 16;
  for (int gx = x; gx < x + w; gx += sz) tft.drawFastVLine(gx, y, h, lineCol);
  for (int gy = y; gy < y + h; gy += sz) tft.drawFastHLine(x, gy, w, lineCol);
}

inline void drawPatternDotMatrix(int16_t x, int16_t y, int16_t w, int16_t h, int16_t sp, uint16_t dotCol, uint16_t bgCol) {
  tft.fillRect(x, y, w, h, bgCol);
  if (sp < 6) sp = 12;
  for (int gx = x + sp/2; gx < x + w; gx += sp) {
    for (int gy = y + sp/2; gy < y + h; gy += sp) {
      tft.drawPixel(gx, gy, dotCol);
    }
  }
}

inline void drawPatternScanlines(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t scanCol, uint16_t bgCol) {
  tft.fillRect(x, y, w, h, bgCol);
  for (int gy = y; gy < y + h; gy += 4) tft.drawFastHLine(x, gy, w, scanCol);
}

inline void drawPatternCarbonFiber(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t bgCol, uint16_t accCol) {
  tft.fillRect(x, y, w, h, bgCol);
  for (int gy = y; gy < y + h; gy += 8) {
    for (int gx = x + (gy % 16 == 0 ? 0 : 4); gx < x + w; gx += 8) {
      tft.fillRect(gx, gy, 3, 3, accCol);
    }
  }
}

inline void drawPatternHexagon(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t strokeCol, uint16_t bgCol) {
  tft.fillRect(x, y, w, h, bgCol);
  for (int gy = y; gy < y + h; gy += 24) tft.drawFastHLine(x, gy, w, strokeCol);
  for (int gx = x; gx < x + w; gx += 24) tft.drawFastVLine(gx, y, h, strokeCol);
}
