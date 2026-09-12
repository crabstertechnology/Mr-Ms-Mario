#ifndef LUNA_RENDERER_H
#define LUNA_RENDERER_H

#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "LunaTypes.h"
#include "LunaNode.h"
#include "LunaScreen.h"

class LunaRenderer {
public:
  LunaRenderer();

  // Initialization
  void begin(Arduino_Canvas* canvas, uint16_t width, uint16_t height);

  // Core Semantic Renderer API
  void clear(uint16_t color);
  void drawScreen(const LunaScreen& screen);
  void drawNode(const LunaNode& node);
  void present();

  // Low-level primitive drawing
  void drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, const LunaStyle& style);
  void drawCard(int16_t x, int16_t y, int16_t w, int16_t h, const LunaStyle& style,
                const char* title, const char* subtitle, const LunaTypography& typo, uint16_t subtextColor);
  void drawButton(int16_t x, int16_t y, int16_t w, int16_t h, const LunaStyle& style,
                  const char* label, const LunaTypography& typo, uint8_t state);
  void drawText(int16_t x, int16_t y, int16_t w, int16_t h, const char* text, const LunaTypography& typo);
  void drawTextWithFont(int16_t x, int16_t y, int16_t w, int16_t h, const char* text, const LunaFont* font, uint16_t color, uint8_t align);
  int16_t measureText(const char* text, const LunaFont* font);
  void drawGlyph(int16_t x, int16_t y, const LunaGlyph* glyph, uint16_t color);
  void drawImage(int16_t x, int16_t y, const LunaImageAsset* asset);
  void drawSpinner(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t size, uint16_t accentColor,
                   uint16_t bgColor, int16_t rotationDeg, const char* label, const LunaTypography& typo);


  // Telemetry & Diagnostics
  uint32_t getLastRenderDurationUs() const { return m_lastRenderDurationUs; }
  float getMeasuredFps() const { return m_measuredFps; }
  uint16_t getWidth() const { return m_width; }
  uint16_t getHeight() const { return m_height; }

private:
  Arduino_Canvas* m_canvas;
  uint16_t m_width;
  uint16_t m_height;
  uint32_t m_lastRenderDurationUs;
  uint32_t m_lastPresentMillis;
  float m_measuredFps;

  void drawStarfieldBackground(uint16_t bgColor);
};

#endif // LUNA_RENDERER_H
