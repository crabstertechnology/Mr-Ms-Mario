#pragma once
#include <Arduino.h>
#include "Arduino_GFX_Library.h"

// Text Datum constants matching TFT_eSPI
#define TL_DATUM 0
#define TC_DATUM 1
#define TR_DATUM 2
#define ML_DATUM 3
#define MC_DATUM 4
#define MR_DATUM 5
#define BL_DATUM 6
#define BC_DATUM 7
#define BR_DATUM 8

class LunaGFXWrapper {
public:
  Arduino_GFX *gfx = nullptr;
  uint8_t textDatum = TL_DATUM;
  uint16_t currentTextColor = 0xFFFF;
  uint16_t currentBgColor = 0x0000;
  bool hasBg = false;

  void attach(Arduino_GFX *g) { gfx = g; }

  void fillScreen(uint16_t color) {
    if (gfx) gfx->fillScreen(color);
  }

  void flush() {
    if (gfx) gfx->flush();
  }

  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (gfx) gfx->fillRoundRect(x, y, w, h, r, color);
  }

  void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (gfx) gfx->drawRoundRect(x, y, w, h, r, color);
  }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (gfx) gfx->fillRect(x, y, w, h, color);
  }

  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (gfx) gfx->drawRect(x, y, w, h, color);
  }

  void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (gfx) gfx->fillCircle(x, y, r, color);
  }

  void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (gfx) gfx->drawCircle(x, y, r, color);
  }

  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (gfx) gfx->drawFastVLine(x, y, h, color);
  }

  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (gfx) gfx->drawFastHLine(x, y, w, color);
  }

  void drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (gfx) gfx->drawPixel(x, y, color);
  }

  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (gfx) gfx->drawLine(x0, y0, x1, y1, color);
  }

  void setTextDatum(uint8_t d) {
    textDatum = d;
  }

  void setTextColor(uint16_t c) {
    currentTextColor = c;
    hasBg = false;
    if (gfx) gfx->setTextColor(c);
  }

  void setTextColor(uint16_t c, uint16_t bg) {
    currentTextColor = c;
    currentBgColor = bg;
    hasBg = true;
    if (gfx) gfx->setTextColor(c, bg);
  }

  uint8_t getScale(uint8_t font) {
    if (font <= 1) return 1;
    if (font == 2) return 2;
    if (font == 4) return 3;
    if (font >= 6) return 4;
    return 1;
  }

  int16_t textWidth(const char* str, uint8_t font = 1) {
    if (!str) return 0;
    uint8_t s = getScale(font);
    return strlen(str) * 6 * s;
  }

  void drawString(const char* str, int16_t x, int16_t y, uint8_t font = 1) {
    if (!gfx || !str) return;
    uint8_t s = getScale(font);
    gfx->setTextSize(s);
    if (hasBg) {
      gfx->setTextColor(currentTextColor, currentBgColor);
    } else {
      gfx->setTextColor(currentTextColor);
    }

    int16_t tw = strlen(str) * 6 * s;
    int16_t th = 8 * s;
    int16_t cx = x;
    int16_t cy = y;

    switch (textDatum) {
      case TL_DATUM: break;
      case TC_DATUM: cx = x - tw / 2; break;
      case TR_DATUM: cx = x - tw; break;
      case ML_DATUM: cy = y - th / 2; break;
      case MC_DATUM: cx = x - tw / 2; cy = y - th / 2; break;
      case MR_DATUM: cx = x - tw; cy = y - th / 2; break;
      case BL_DATUM: cy = y - th; break;
      case BC_DATUM: cx = x - tw / 2; cy = y - th; break;
      case BR_DATUM: cx = x - tw; cy = y - th; break;
      default: break;
    }
    gfx->setCursor(cx, cy);
    gfx->print(str);
  }
};

// Aliased so Luna UI Studio exported code compiles transparently
typedef LunaGFXWrapper TFT_eSPI;
