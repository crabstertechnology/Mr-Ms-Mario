#ifndef LUNA_TYPES_H
#define LUNA_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Fast RGB565 color pack macro: R (5-bit), G (6-bit), B (5-bit)
#define RGB565(r, g, b) (((uint16_t)((r) & 0xF8) << 8) | ((uint16_t)((g) & 0xFC) << 3) | ((uint16_t)(b) >> 3))

// Common 16-bit Color Constants
#define LUNA_COLOR_BLACK       0x0000
#define LUNA_COLOR_WHITE       0xFFFF
#define LUNA_COLOR_NAVY        0x000F
#define LUNA_COLOR_DARKBLUE    0x0845
#define LUNA_COLOR_CYAN        0x07FF
#define LUNA_COLOR_BLUE        0x001F
#define LUNA_COLOR_GREEN       0x07E0
#define LUNA_COLOR_RED         0xF800
#define LUNA_COLOR_YELLOW      0xFFE0
#define LUNA_COLOR_GRAY        0x8410
#define LUNA_COLOR_DARKGRAY    0x18E3

// Alignment
enum LunaTextAlign {
  LUNA_ALIGN_LEFT   = 0,
  LUNA_ALIGN_CENTER = 1,
  LUNA_ALIGN_RIGHT  = 2
};

// Node Type Enum
enum LunaNodeType {
  LUNA_NODE_CONTAINER = 0,
  LUNA_NODE_TEXT      = 1,
  LUNA_NODE_CARD      = 2,
  LUNA_NODE_BUTTON    = 3,
  LUNA_NODE_SPINNER   = 4,
  LUNA_NODE_IMAGE     = 5,
  LUNA_NODE_CUSTOM    = 99
};

// Button State
enum LunaButtonState {
  LUNA_BTN_STATE_NORMAL  = 0,
  LUNA_BTN_STATE_PRESSED = 1
};

// Action Type
enum LunaActionType {
  LUNA_ACTION_NONE     = 0,
  LUNA_ACTION_NAVIGATE = 1,
  LUNA_ACTION_SCROLL   = 2
};

// Layout specification
typedef struct {
  int16_t x;
  int16_t y;
  int16_t width;
  int16_t height;
} LunaRect;

// Visual Style
typedef struct {
  uint16_t backgroundColor;
  uint16_t borderColor;
  uint8_t borderWidth;
  uint8_t borderRadius;
  uint8_t opacity; // 0..255 (255 = 100% opaque)
  uint16_t accentColor;
  bool hasBg;
  bool hasBorder;
} LunaStyle;

// Typography specification
typedef struct {
  uint16_t color;
  uint8_t fontSize;
  uint8_t align; // LunaTextAlign
  const char* fontFamily;
} LunaTypography;

// Action specification
typedef struct {
  uint8_t type; // LunaActionType
  const char* targetScreenId;
  int16_t scrollAmount;
} LunaAction;

// Event mapping
typedef struct {
  const char* trigger; // e.g. "onClick"
  LunaAction action;
} LunaEvent;

// Animation metadata
typedef struct {
  const char* property; // e.g. "rotation"
  int16_t fromValue;
  int16_t toValue;
  uint16_t durationMs;
  bool loop;
} LunaAnimation;

// Font Glyph Definition
typedef struct {
  uint16_t character;   // Unicode code point or ASCII char code
  uint8_t width;        // bitmap width
  uint8_t height;       // bitmap height
  uint8_t xAdvance;     // horizontal advance to next character
  int8_t xOffset;       // offset from current cursor X
  int8_t yOffset;       // offset from current cursor Y (relative to baseline)
  const uint8_t* bitmap;// 1-bit per pixel bitmap data (packed MSB first)
} LunaGlyph;

// Font Face Definition
typedef struct {
  const char* family;   // e.g. "Outfit"
  uint8_t size;         // e.g. 16
  uint16_t weight;      // e.g. 700 (standard CSS weight 100..900)
  uint8_t baseline;     // e.g. 13
  uint8_t lineHeight;   // e.g. 18
  const LunaGlyph* glyphs;
  uint16_t glyphCount;
} LunaFont;

// Image Asset Definition
typedef struct {
  const char* id;
  uint16_t width;
  uint16_t height;
  const uint16_t* pixels; // 16-bit RGB565 bitmap array
  uint32_t byteSize;
} LunaImageAsset;

const LunaImageAsset* findAssetById(const char* id);

// Static Node Definition (from compiler)
typedef struct {
  const char* id;
  const char* type;
  uint8_t nodeType; // LunaNodeType
  LunaRect layout;
  LunaStyle style;
  LunaTypography typography;
  const char* text; // Primary text/label
  const char* subtitle; // Secondary text (for cards)
  uint16_t subtextColor;
  uint8_t spinnerSize;
  const char* assetId; // Image asset ID if nodeType == LUNA_NODE_IMAGE
  const LunaFont* font; // Custom proportional font if specified
  const LunaStyle* pressedStyle; // Optional pressed style
  const LunaEvent* events;
  uint8_t eventCount;
  const LunaAnimation* animations;
  uint8_t animationCount;
} LunaNodeDef;


// Static Screen Definition (from compiler)
typedef struct {
  const char* id;
  const char* name;
  uint16_t backgroundColor;
  const char* backgroundPattern;
  bool isScrollable;
  int16_t maxScrollY;
  const LunaNodeDef* nodes;
  uint8_t nodeCount;
  const char* swipeLeftTarget;
  const char* swipeRightTarget;
} LunaScreenDef;

// Static Project Definition (from compiler)
typedef struct {
  const char* version;
  const char* name;
  uint16_t deviceWidth;
  uint16_t deviceHeight;
  const char* initialScreenId;
  const LunaScreenDef* screens;
  uint8_t screenCount;
} LunaProjectDef;

#ifdef __cplusplus
}
#endif

#endif // LUNA_TYPES_H
