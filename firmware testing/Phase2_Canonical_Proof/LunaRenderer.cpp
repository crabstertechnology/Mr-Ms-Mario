#include "LunaRenderer.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

LunaRenderer::LunaRenderer()
  : m_canvas(nullptr)
  , m_width(240)
  , m_height(280)
  , m_lastRenderDurationUs(0)
  , m_lastPresentMillis(0)
  , m_measuredFps(0.0f)
{
}

void LunaRenderer::begin(Arduino_Canvas* canvas, uint16_t width, uint16_t height) {
  m_canvas = canvas;
  m_width = width;
  m_height = height;
  m_lastPresentMillis = millis();
}

void LunaRenderer::clear(uint16_t color) {
  if (m_canvas) {
    m_canvas->fillScreen(color);
  }
}

void LunaRenderer::drawStarfieldBackground(uint16_t bgColor) {
  if (!m_canvas) return;
  m_canvas->fillScreen(bgColor);

  // Deterministic background starfield lattice (reproducible without dynamic heap)
  static const uint16_t stars[][3] = {
    { 24,  30, 0x5AEB }, { 190,  45, 0xFFFF }, {  80,  70, 0x8410 },
    { 220, 110, 0x6B6D }, {  35, 140, 0x9CD3 }, { 160, 160, 0x4208 },
    {  95, 210, 0xFFFF }, { 210, 230, 0x5AEB }, {  50, 260, 0x8410 },
    { 140, 275, 0x6B6D }, { 175,  95, 0x52AA }, {  15, 185, 0xFFFF }
  };
  const size_t starCount = sizeof(stars) / sizeof(stars[0]);

  for (size_t i = 0; i < starCount; i++) {
    m_canvas->drawPixel(stars[i][0], stars[i][1], stars[i][2]);
  }
}

void LunaRenderer::drawScreen(const LunaScreen& screen) {
  if (!m_canvas) return;

  // 1. Draw Background
  const char* pattern = screen.getBackgroundPattern();
  if (pattern && strcmp(pattern, "stars") == 0) {
    drawStarfieldBackground(screen.getBackgroundColor());
  } else {
    clear(screen.getBackgroundColor());
  }

  // 2. Draw Child Nodes
  uint8_t count = screen.getNodeCount();
  for (uint8_t i = 0; i < count; i++) {
    const LunaNode* node = screen.getNode(i);
    if (node) {
      drawNode(*node);
    }
  }
}

void LunaRenderer::drawNode(const LunaNode& node) {
  if (!m_canvas) return;

  const LunaRect& r = node.getLayout();
  const LunaStyle& s = node.getStyle();
  const LunaTypography& t = node.getTypography();

  switch (node.getNodeType()) {
    case LUNA_NODE_CARD:
      drawCard(r.x, r.y, r.width, r.height, s, node.getText(), node.getSubtitle(), t, node.getSubtextColor());
      break;

    case LUNA_NODE_BUTTON:
      drawButton(r.x, r.y, r.width, r.height, s, node.getText(), t, node.getState());
      break;

    case LUNA_NODE_TEXT:
      if (node.getFont()) {
        drawTextWithFont(r.x, r.y, r.width, r.height, node.getText(), node.getFont(), t.color, t.align);
      } else {
        drawText(r.x, r.y, r.width, r.height, node.getText(), t);
      }
      break;

    case LUNA_NODE_IMAGE: {
      const LunaImageAsset* asset = findAssetById(node.getAssetId());
      if (asset) {
        drawImage(r.x, r.y, asset);
      }
      break;
    }

    case LUNA_NODE_SPINNER:
      drawSpinner(r.x, r.y, r.width, r.height, node.getSpinnerSize(),
                  s.accentColor ? s.accentColor : LUNA_COLOR_CYAN,
                  s.backgroundColor ? s.backgroundColor : LUNA_COLOR_DARKBLUE,
                  node.getAnimatedRotation(), node.getText(), t);
      break;

    case LUNA_NODE_CONTAINER:
    default:
      drawRectangle(r.x, r.y, r.width, r.height, s);
      break;
  }
}


void LunaRenderer::drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, const LunaStyle& style) {
  if (!m_canvas) return;

  if (style.hasBg) {
    if (style.borderRadius > 0) {
      m_canvas->fillRoundRect(x, y, w, h, style.borderRadius, style.backgroundColor);
    } else {
      m_canvas->fillRect(x, y, w, h, style.backgroundColor);
    }
  }

  if (style.hasBorder && style.borderWidth > 0) {
    for (uint8_t b = 0; b < style.borderWidth; b++) {
      if (style.borderRadius > b) {
        m_canvas->drawRoundRect(x + b, y + b, w - (b * 2), h - (b * 2), style.borderRadius - b, style.borderColor);
      } else {
        m_canvas->drawRect(x + b, y + b, w - (b * 2), h - (b * 2), style.borderColor);
      }
    }
  }
}

void LunaRenderer::drawCard(int16_t x, int16_t y, int16_t w, int16_t h, const LunaStyle& style,
                            const char* title, const char* subtitle, const LunaTypography& typo, uint16_t subtextColor) {
  if (!m_canvas) return;

  // Background & Border
  drawRectangle(x, y, w, h, style);

  // Title text (left aligned inside card)
  if (title && strlen(title) > 0) {
    m_canvas->setTextSize(1);
    m_canvas->setTextColor(typo.color);
    m_canvas->setCursor(x + 12, y + 14);
    m_canvas->print(title);
  }

  // Subtitle text
  if (subtitle && strlen(subtitle) > 0) {
    m_canvas->setTextSize(1);
    m_canvas->setTextColor(subtextColor ? subtextColor : LUNA_COLOR_GRAY);
    m_canvas->setCursor(x + 12, y + 36);
    m_canvas->print(subtitle);
  }
}

void LunaRenderer::drawButton(int16_t x, int16_t y, int16_t w, int16_t h, const LunaStyle& style,
                             const char* label, const LunaTypography& typo, uint8_t state) {
  if (!m_canvas) return;

  LunaStyle btnStyle = style;
  // If pressed, slightly shift visual feedback if no explicit pressedStyle
  if (state == LUNA_BTN_STATE_PRESSED) {
    btnStyle.backgroundColor = (btnStyle.backgroundColor == LUNA_COLOR_BLACK) ? LUNA_COLOR_DARKBLUE : (btnStyle.backgroundColor ^ 0x18C3);
  }

  drawRectangle(x, y, w, h, btnStyle);

  if (label && strlen(label) > 0) {
    uint8_t scale = 1;
    if (typo.fontSize >= 16) scale = 2;

    int16_t textLen = (int16_t)strlen(label);
    int16_t textW = textLen * 6 * scale;
    int16_t textH = 8 * scale;

    int16_t textX = x + (w - textW) / 2;
    int16_t textY = y + (h - textH) / 2;

    m_canvas->setTextSize(scale);
    m_canvas->setTextColor(typo.color);
    m_canvas->setCursor(textX, textY);
    m_canvas->print(label);
  }
}

void LunaRenderer::drawText(int16_t x, int16_t y, int16_t w, int16_t h, const char* text, const LunaTypography& typo) {
  if (!m_canvas || !text || strlen(text) == 0) return;

  uint8_t scale = 1;
  if (typo.fontSize >= 20) scale = 3;
  else if (typo.fontSize >= 14) scale = 2;

  int16_t textLen = (int16_t)strlen(text);
  int16_t textW = textLen * 6 * scale;
  int16_t textH = 8 * scale;

  int16_t textX = x;
  if (typo.align == LUNA_ALIGN_CENTER) {
    textX = x + (w - textW) / 2;
  } else if (typo.align == LUNA_ALIGN_RIGHT) {
    textX = x + w - textW - 4;
  } else {
    textX = x + 4;
  }

  int16_t textY = y + (h - textH) / 2;

  m_canvas->setTextSize(scale);
  m_canvas->setTextColor(typo.color);
  m_canvas->setCursor(textX, textY);
  m_canvas->print(text);
}

void LunaRenderer::drawSpinner(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t size, uint16_t accentColor,
                              uint16_t bgColor, int16_t rotationDeg, const char* label, const LunaTypography& typo) {
  if (!m_canvas) return;

  int16_t cx = x + (w / 2);
  int16_t cy = y + (size / 2) + 2;
  int16_t radius = (size / 2) - 3;

  // Background ring track
  m_canvas->drawCircle(cx, cy, radius, bgColor);

  // Rotating multi-segment arc dots
  for (int i = 0; i < 8; i++) {
    float angle = (float)(rotationDeg + (i * 45)) * (float)M_PI / 180.0f;
    int16_t px = cx + (int16_t)(cosf(angle) * (float)radius);
    int16_t py = cy + (int16_t)(sinf(angle) * (float)radius);

    // Graduated brightness: leading dot is full accent, trailing dots fade
    uint16_t dotColor = (i >= 5) ? accentColor : ((i >= 3) ? LUNA_COLOR_DARKBLUE : bgColor);
    m_canvas->fillCircle(px, py, (i >= 6) ? 2 : 1, dotColor);
  }

  // Optional spinner label below
  if (label && strlen(label) > 0) {
    int16_t textLen = (int16_t)strlen(label);
    int16_t textW = textLen * 6;
    int16_t textX = x + (w - textW) / 2;
    int16_t textY = cy + radius + 8;

    m_canvas->setTextSize(1);
    m_canvas->setTextColor(typo.color ? typo.color : accentColor);
    m_canvas->setCursor(textX, textY);
    m_canvas->print(label);
  }
}

void LunaRenderer::present() {
  if (!m_canvas) return;

  uint32_t startUs = micros();
  m_canvas->flush();
  m_lastRenderDurationUs = micros() - startUs;

  uint32_t now = millis();
  uint32_t frameDelta = now - m_lastPresentMillis;
  if (frameDelta > 0) {
    float instantFps = 1000.0f / (float)frameDelta;
    m_measuredFps = (m_measuredFps * 0.9f) + (instantFps * 0.1f); // smooth EMA filter
  }
  m_lastPresentMillis = now;
}

// Helper: look up glyph in LunaFont table
static const LunaGlyph* findGlyph(const LunaFont* font, char c) {
  if (!font || !font->glyphs) return nullptr;
  uint16_t code = (uint16_t)(uint8_t)c;
  for (uint16_t i = 0; i < font->glyphCount; i++) {
    if (font->glyphs[i].character == code) {
      return &font->glyphs[i];
    }
  }
  return nullptr;
}

int16_t LunaRenderer::measureText(const char* text, const LunaFont* font) {
  if (!text) return 0;
  if (!font) {
    return strlen(text) * 6; // fallback 5x7 ASCII width
  }

  int16_t totalWidth = 0;
  for (const char* p = text; *p; p++) {
    const LunaGlyph* g = findGlyph(font, *p);
    if (g) {
      totalWidth += g->xAdvance;
    } else {
      totalWidth += (font->size / 2);
    }
  }
  return totalWidth;
}

void LunaRenderer::drawGlyph(int16_t x, int16_t y, const LunaGlyph* glyph, uint16_t color) {
  if (!m_canvas || !glyph || !glyph->bitmap) return;

  uint8_t w = glyph->width;
  uint8_t h = glyph->height;
  uint8_t bytesPerRow = (w + 7) / 8;

  for (uint8_t r = 0; r < h; r++) {
    for (uint8_t c = 0; c < w; c++) {
      uint8_t byteVal = glyph->bitmap[(r * bytesPerRow) + (c / 8)];
      if (byteVal & (0x80 >> (c % 8))) {
        m_canvas->drawPixel(x + c, y + r, color);
      }
    }
  }
}

void LunaRenderer::drawTextWithFont(int16_t x, int16_t y, int16_t w, int16_t h,
                                    const char* text, const LunaFont* font, uint16_t color, uint8_t align) {
  if (!m_canvas || !text || strlen(text) == 0) return;
  if (!font) {
    LunaTypography t = { color, 14, align, "default" };
    drawText(x, y, w, h, text, t);
    return;
  }

  int16_t textWidth = measureText(text, font);
  int16_t cursorX = x;
  if (align == LUNA_ALIGN_CENTER) {
    cursorX = x + (w - textWidth) / 2;
  } else if (align == LUNA_ALIGN_RIGHT) {
    cursorX = x + w - textWidth;
  } else {
    cursorX = x + 2;
  }

  int16_t baselineY = y + (h - font->lineHeight) / 2 + font->baseline;

  for (const char* p = text; *p; p++) {
    char c = *p;
    const LunaGlyph* g = findGlyph(font, c);
    if (g) {
      drawGlyph(cursorX + g->xOffset, baselineY + g->yOffset, g, color);
      cursorX += g->xAdvance;
    } else {
      cursorX += (font->size / 2);
    }
  }
}

void LunaRenderer::drawImage(int16_t x, int16_t y, const LunaImageAsset* asset) {
  if (!m_canvas || !asset || !asset->pixels) return;
  m_canvas->draw16bitRGBBitmap(x, y, (uint16_t*)asset->pixels, asset->width, asset->height);
}

// Weak symbol fallback for findAssetById when no assets are compiled
extern "C" {
  __attribute__((weak)) const LunaImageAsset* findAssetById(const char* id) {
    (void)id;
    return nullptr;
  }
}

