#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "config.h"
#include "sprite_ai_data_1_3.h"  // 1.69" GIF animations downscaled to 120x120 RGB565
#include "image_logo.h"

// 12 Full-Color Sprite AI animation names
inline const char* getSpriteAi13AnimName(int idx) {
  static const char* const names[12] = {
    "IDLE", "HAPPY", "SAD", "ANGRY", "SURPRISED", "SLEEPING",
    "WINK", "EXCITED", "LOVE", "SCARED", "LAUGH", "PEACE"
  };
  if (idx < 0 || idx >= SPRITE_AI13_ANIMATION_COUNT) return "IDLE";
  return names[idx];
}
#include "qr_card.h"
#include "wallpaper_image.h"

extern LunaQR qrCard;

// Color compatibility macros for Adafruit GFX
#define TFT_BLACK       ST77XX_BLACK
#define TFT_WHITE       ST77XX_WHITE
#define TFT_RED         ST77XX_RED
#define TFT_GREEN       ST77XX_GREEN
#define TFT_BLUE        ST77XX_BLUE
#define TFT_CYAN        ST77XX_CYAN
#define TFT_MAGENTA     ST77XX_MAGENTA
#define TFT_YELLOW      ST77XX_YELLOW
#define TFT_ORANGE      ST77XX_ORANGE
#define TFT_LIGHTGREY   0xC618
#define TFT_DARKGREY    0x7BEF

#include "games.h"

// External references to settings/status variables defined in the main sketch
extern bool bleActive;
extern int gifSpeed;
extern int clockStyle;
extern int oledBrightness;
extern bool negativeDisplay;
extern String robotVariant;
extern volatile bool hardwareLoopbackActive;
extern int menuOption;
extern bool optionSelected;
extern bool settingsActive;
extern bool notificationsActive;
extern bool notificationSelected;
extern unsigned int touchCount;
extern SmartwatchScreen currentScreen;

extern bool gamesActive;
extern bool gamePlaying;
extern int gameSelected;
extern LunaGames games;
extern LunaAudio audio;
extern float batteryVolts;
extern bool silentMode;

inline int getBatteryPercentage(float volts) {
  int pct = 0;
  if (volts >= 4.15f) pct = 100;
  else if (volts >= 4.05f) pct = (int)(90 + (volts - 4.05f) * 100.0f);
  else if (volts >= 3.95f) pct = (int)(80 + (volts - 3.95f) * 100.0f);
  else if (volts >= 3.87f) pct = (int)(70 + (volts - 3.87f) * 125.0f);
  else if (volts >= 3.82f) pct = (int)(60 + (volts - 3.82f) * 200.0f);
  else if (volts >= 3.79f) pct = (int)(50 + (volts - 3.79f) * 333.0f);
  else if (volts >= 3.75f) pct = (int)(40 + (volts - 3.75f) * 250.0f);
  else if (volts >= 3.72f) pct = (int)(30 + (volts - 3.72f) * 333.0f);
  else if (volts >= 3.68f) pct = (int)(20 + (volts - 3.68f) * 250.0f);
  else if (volts >= 3.60f) pct = (int)(10 + (volts - 3.60f) * 125.0f);
  else if (volts >= 3.30f) pct = (int)((volts - 3.30f) * 33.3f);
  else pct = 0;
  if (pct > 100) pct = 100;
  if (pct < 0) pct = 0;
  return pct;
}

// Custom dual-chunk canvas that splits 240x240 into two 57.6KB buffers (fits ESP32-C3 heap limits)
class LunaCanvas16 : public GFXcanvas16 {
private:
  uint16_t* topBuf;
  uint16_t* btmBuf;

public:
  LunaCanvas16(uint16_t w, uint16_t h)
    : GFXcanvas16(w, h, false), topBuf(nullptr), btmBuf(nullptr) {}

  bool allocate() {
    if (!topBuf) {
      topBuf = (uint16_t*)malloc(240 * 120 * sizeof(uint16_t));
    }
    if (!btmBuf) {
      btmBuf = (uint16_t*)malloc(240 * 120 * sizeof(uint16_t));
    }
    buffer = topBuf; // Non-null pointer for Adafruit_GFX internal validity checks
    buffer_owned = false;
    if (topBuf && btmBuf) {
      fillScreen(0x0000);
      return true;
    }
    return false;
  }

  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    if (x < 0 || x >= 240 || y < 0 || y >= 240) return;
    if (y < 120) {
      if (topBuf) topBuf[x + y * 240] = color;
    } else {
      if (btmBuf) btmBuf[x + (y - 120) * 240] = color;
    }
  }

  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override {
    if (y < 0 || y >= 240 || w <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > 240) w = 240 - x;
    if (w <= 0) return;

    uint16_t* p = (y < 120) ? (topBuf + y * 240 + x) : (btmBuf + (y - 120) * 240 + x);
    if (p) {
      for (int16_t i = 0; i < w; i++) p[i] = color;
    }
  }

  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override {
    if (x < 0 || x >= 240 || h <= 0) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > 240) h = 240 - y;
    if (h <= 0) return;

    for (int16_t i = 0; i < h; i++) {
      int16_t cy = y + i;
      if (cy < 120) {
        if (topBuf) topBuf[x + cy * 240] = color;
      } else {
        if (btmBuf) btmBuf[x + (cy - 120) * 240] = color;
      }
    }
  }

  void fillScreen(uint16_t color) override {
    if (topBuf) {
      for (int i = 0; i < 240 * 120; i++) topBuf[i] = color;
    }
    if (btmBuf) {
      for (int i = 0; i < 240 * 120; i++) btmBuf[i] = color;
    }
  }

  void flush(Adafruit_ST7789& targetTft) {
    if (topBuf) {
      targetTft.drawRGBBitmap(0, 0, topBuf, 240, 120);
    }
    if (btmBuf) {
      targetTft.drawRGBBitmap(0, 120, btmBuf, 240, 120);
    }
  }

  // Blazingly fast 2x hardware scaling of 120x120 PROGMEM sprite to full 240x240 screen
  void drawRGBBitmap2x(const uint16_t* pgmData) {
    if (!topBuf || !btmBuf || !pgmData) return;

    // Top half: sy 0..59 -> lines 0..119 in topBuf
    for (int sy = 0; sy < 60; sy++) {
      const uint16_t* srcRow = pgmData + sy * 120;
      uint32_t* dst32_0 = (uint32_t*)(topBuf + (sy * 2) * 240);
      uint32_t* dst32_1 = (uint32_t*)(topBuf + (sy * 2 + 1) * 240);
      for (int sx = 0; sx < 120; sx++) {
        uint16_t color = pgm_read_word(&srcRow[sx]);
        uint32_t dColor = ((uint32_t)color << 16) | color;
        dst32_0[sx] = dColor;
        dst32_1[sx] = dColor;
      }
    }

    // Bottom half: sy 60..119 -> lines 120..239 in btmBuf
    for (int sy = 60; sy < 120; sy++) {
      const uint16_t* srcRow = pgmData + sy * 120;
      int by = sy - 60;
      uint32_t* dst32_0 = (uint32_t*)(btmBuf + (by * 2) * 240);
      uint32_t* dst32_1 = (uint32_t*)(btmBuf + (by * 2 + 1) * 240);
      for (int sx = 0; sx < 120; sx++) {
        uint16_t color = pgm_read_word(&srcRow[sx]);
        uint32_t dColor = ((uint32_t)color << 16) | color;
        dst32_0[sx] = dColor;
        dst32_1[sx] = dColor;
      }
    }
  }

protected:
  void drawFastRawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    drawFastHLine(x, y, w, color);
  }

  void drawFastRawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    drawFastVLine(x, y, h, color);
  }

  uint16_t getRawPixel(int16_t x, int16_t y) const {
    if (x < 0 || x >= 240 || y < 0 || y >= 240) return 0;
    if (y < 120) {
      return topBuf ? topBuf[x + y * 240] : 0;
    } else {
      return btmBuf ? btmBuf[x + (y - 120) * 240] : 0;
    }
  }
};

class LunaFace {
private:
  Adafruit_ST7789& tft;
  LunaCanvas16& display;
  Expression currentExpr;
  Expression targetExpr;
  Expression defaultExpr; // Custom default expression for Idle state

  // Animation frame control
  int currentFrame;
  int currentGifIndex;        // index into ALL_GIFS_TABLE for EXPR_ALL_GIF mode
  unsigned long lastFrameTime;
  bool gifFinished;
  bool expressionChanged;

  // Text state
  String notificationTitle;
  String notificationText;
  int scrollPos;
  unsigned long lastScrollTime;
  String stateLabel;
  int frameDelayMs;

  // Map navigation telemetry state
  String mapDirection;
  String mapDistance;
  String mapRoad;
  String mapTotalTime;
  String mapTotalDist;
  String mapEta;

  // Track BLE & WiFi status internally for top bar drawing
  bool bleConnectedStatus;
  bool wifiConnectedStatus;

  // Smartwatch state structures
  struct NotificationItem {
    String title;
    String body;
    String timeStr;
    bool active;
  };
  NotificationItem notificationHistory[5];
  int notificationCount;
  int currentNotifViewIdx;

public:
  static const int MAX_FACE_CAL_EVENTS = 10;
  struct CalendarEventItem {
    String id;
    String type;
    String dateStr;
    String timeStr;
    String title;
    bool active;
  };

private:
  CalendarEventItem calendarEvents[MAX_FACE_CAL_EVENTS];
  int calendarEventCount;
  int currentCalViewIdx;
  bool showCalendarGrid;

  // Temporary popup notification state
  bool popupActive;
  unsigned long popupStartTime;
  unsigned long popupDuration;
  String popupTitle;
  String popupBody;

  // Alarm / Meeting / Reminder Ringing Overlay state
  bool alarmRingingActive;
  String ringingType;
  String ringingTitle;
  String ringingTime;

public:
  String headerText;

  LunaFace(Adafruit_ST7789& tftDisp, LunaCanvas16& disp) 
    : tft(tftDisp), display(disp), currentExpr(EXPR_IDLE), targetExpr(EXPR_IDLE), defaultExpr(EXPR_IDLE), stateLabel("IDLE"), frameDelayMs(100), expressionChanged(true) {
    currentFrame = 0;
    currentGifIndex = 0;
    lastFrameTime = 0;
    gifFinished = false;

    // Sprite AI animation state init
    spriteAnimIndex = 0;
    spriteFrameIndex = 0;
    lastSpriteFrameTime = 0;

    notificationTitle = "";
    notificationText = "";
    scrollPos = SCREEN_WIDTH;
    lastScrollTime = 0;

    mapDirection = "STRAIGHT";
    mapDistance = "--";
    mapRoad = "";
    mapTotalTime = "";
    mapTotalDist = "";
    mapEta = "";

    bleConnectedStatus = false;
    wifiConnectedStatus = false;

    // Smartwatch state initialization
    notificationCount = 0;
    currentNotifViewIdx = 0;
    calendarEventCount = 0;
    currentCalViewIdx = 0;
    showCalendarGrid = false; // Default to rich event card view

    popupActive = false;
    popupStartTime = 0;
    popupDuration = 5000;
    popupTitle = "";
    popupBody = "";
    headerText = "";

    alarmRingingActive = false;
    ringingType = "ALARM";
    ringingTitle = "";
    ringingTime = "";

    for (int i = 0; i < 5; i++) {
      notificationHistory[i].active = false;
    }
    for (int i = 0; i < MAX_FACE_CAL_EVENTS; i++) {
      calendarEvents[i].active = false;
      calendarEvents[i].id = "";
      calendarEvents[i].title = "";
    }
  }

  void setConnectivityStatus(bool bleConnected, bool wifiConnected) {
    bleConnectedStatus = bleConnected;
    wifiConnectedStatus = wifiConnected;
  }

  void updateLabelFromState() {
    Expression exprToLabel = currentExpr;
    if (exprToLabel == EXPR_IDLE) {
      exprToLabel = defaultExpr;
    }
    
    if (exprToLabel == EXPR_ALL_GIF) {
      stateLabel = String(getSpriteAi13AnimName(spriteAnimIndex));
    } else if ((int)exprToLabel >= 0 && (int)exprToLabel < SPRITE_AI13_ANIMATION_COUNT) {
      stateLabel = String(getSpriteAi13AnimName((int)exprToLabel));
    } else {
      switch (exprToLabel) {
        case EXPR_CLOCK:     stateLabel = "CLOCK"; break;
        case EXPR_TEXT:      stateLabel = "TEXT"; break;
        case EXPR_MAP:       stateLabel = "MAPS"; break;
        default:             stateLabel = "IDLE"; break;
      }
    }
  }

  void setDefaultExpression(Expression expr) {
    defaultExpr = expr;
    updateLabelFromState();
  }

  void setFrameDelay(int ms) {
    frameDelayMs = ms;
  }

  void setExpression(Expression expr) {
    if ((int)expr >= 100) {
      int gifIdx = (int)expr - 100;
      if (gifIdx >= 0 && gifIdx < SPRITE_AI13_ANIMATION_COUNT) {
        setGifIndex(gifIdx);
        expr = (Expression)gifIdx;
      }
    } else if ((int)expr >= 0 && (int)expr < SPRITE_AI13_ANIMATION_COUNT) {
      setGifIndex((int)expr);
    }
    if (currentExpr == expr && expr != EXPR_ALL_GIF) return;
    targetExpr = expr;
    currentExpr = expr;
    currentFrame = 0;
    lastFrameTime = millis();
    gifFinished = false;
    expressionChanged = true;
    
    if (expr == EXPR_TEXT) {
      scrollPos = SCREEN_WIDTH;
      lastScrollTime = millis();
    }
    updateLabelFromState();
  }

  Expression getExpression() {
    return currentExpr;
  }

  // ------------------ Smartwatch Notification History ------------------
  void addNotification(String title, String body, String timeStr) {
    for (int i = 4; i > 0; i--) {
      notificationHistory[i] = notificationHistory[i - 1];
    }
    notificationHistory[0].title = title;
    notificationHistory[0].body = body;
    notificationHistory[0].timeStr = timeStr;
    notificationHistory[0].active = true;
    
    if (notificationCount < 5) {
      notificationCount++;
    }
    currentNotifViewIdx = 0;
  }

  void clearNotifications() {
    notificationCount = 0;
    currentNotifViewIdx = 0;
    for (int i = 0; i < 5; i++) {
      notificationHistory[i].active = false;
    }
  }

  void cycleNotificationView() {
    if (notificationCount > 0) {
      currentNotifViewIdx = (currentNotifViewIdx + 1) % notificationCount;
    }
  }

  int getNotificationCount() const { return notificationCount; }
  int getCurrentNotifViewIdx() const { return currentNotifViewIdx; }
  void setCurrentNotifViewIdx(int idx) { currentNotifViewIdx = idx; }

  // ------------------ Smartwatch Calendar Events (Up to 20 slots) ------------------
  void clearCalendarEvents() {
    calendarEventCount = 0;
    currentCalViewIdx = 0;
    for (int i = 0; i < MAX_FACE_CAL_EVENTS; i++) {
      calendarEvents[i].active = false;
      calendarEvents[i].id = "";
      calendarEvents[i].title = "";
    }
  }

  void removeCalendarEvent(String id) {
    int found = -1;
    for (int i = 0; i < calendarEventCount; i++) {
      if (calendarEvents[i].id == id || calendarEvents[i].title == id) {
        found = i;
        break;
      }
    }
    if (found != -1) {
      for (int i = found; i < calendarEventCount - 1; i++) {
        calendarEvents[i] = calendarEvents[i + 1];
      }
      calendarEvents[calendarEventCount - 1].active = false;
      calendarEventCount--;
      if (currentCalViewIdx >= calendarEventCount && currentCalViewIdx > 0) {
        currentCalViewIdx = calendarEventCount - 1;
      }
    }
  }

  void addCalendarEvent(String id, String type, String dateStr, String timeStr, String title) {
    if (calendarEventCount < MAX_FACE_CAL_EVENTS) {
      calendarEventCount++;
    }
    for (int i = calendarEventCount - 1; i > 0; i--) {
      calendarEvents[i] = calendarEvents[i - 1];
    }
    calendarEvents[0].id = id;
    calendarEvents[0].type = type;
    calendarEvents[0].dateStr = dateStr;
    calendarEvents[0].timeStr = timeStr;
    calendarEvents[0].title = title;
    calendarEvents[0].active = true;
    currentCalViewIdx = 0;
  }

  void addCalendarEvent(String type, String timeStr, String title) {
    addCalendarEvent(String(millis()), type, "*", timeStr, title);
  }

  void cycleCalendarView() {
    if (calendarEventCount > 0) {
      currentCalViewIdx = (currentCalViewIdx + 1) % calendarEventCount;
    }
  }

  void toggleCalendarMode() {
    showCalendarGrid = !showCalendarGrid;
  }

  void setAlarmRinging(bool ringing, String type = "alarm", String title = "", String time = "") {
    alarmRingingActive = ringing;
    ringingType = type;
    ringingTitle = title;
    ringingTime = time;
  }

  bool isAlarmRingingActive() const {
    return alarmRingingActive;
  }

  // ------------------ Notifications Adaptors ------------------
  void setNotificationText(String text, int hour = 12, int minute = 0) {
    setDetailedNotification("Notification", text, hour, minute);
  }

  void setDetailedNotification(String title, String body, int hour = 12, int minute = 0) {
    popupTitle = title;
    popupBody = body;
    popupActive = true;
    popupStartTime = millis();
    popupDuration = 5000;
    
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
    addNotification(title, body, String(timeBuf));
  }

  void setPopupDismiss() {
    popupActive = false;
  }

  bool isPopupActive() const {
    return popupActive;
  }

  // ------------------ Map Navigation State ------------------
  void setMapTelemetry(String dir, String turnDist, String road, String tTime, String tDist, String eta) {
    mapDirection = dir;
    mapDistance = turnDist;
    mapRoad = road;
    mapTotalTime = tTime;
    mapTotalDist = tDist;
    mapEta = eta;
    setExpression(EXPR_MAP);
  }

  void setMapNavigation(String direction, String distance, String description) {
    setMapTelemetry(direction, distance, description, "", "", "");
  }

  void setStateLabel(String label) {
    stateLabel = label;
  }

  String getStateLabel() {
    return stateLabel;
  }

  String getMapDirection() {
    return mapDirection;
  }

  void setGifIndex(int idx) {
    if (idx >= 0 && idx < SPRITE_AI13_ANIMATION_COUNT) {
      currentGifIndex = idx;
      spriteAnimIndex = idx;
      spriteFrameIndex = 0;
      lastSpriteFrameTime = millis();
      currentFrame = 0;
      lastFrameTime = millis();
      gifFinished = false;
      expressionChanged = true;
      updateLabelFromState();
    }
  }

  int getGifIndex() const {
    return spriteAnimIndex;
  }

  int getGifCount() const {
    return SPRITE_AI13_ANIMATION_COUNT;
  }

  bool isGifFinished() {
    return gifFinished;
  }

  void clearGifFinished() {
    gifFinished = false;
  }

  bool update() {
    unsigned long now = millis();
    bool changed = false;

    if (expressionChanged) {
      expressionChanged = false;
      changed = true;
    }

    // Frame Animation logic
    if (currentExpr != EXPR_TEXT) {
      int maxFrames = SPRITE_AI13_FRAME_COUNT;
      int delayToUse = frameDelayMs;

      if (now - lastFrameTime >= (unsigned long)delayToUse) {
        lastFrameTime = now;
        currentFrame++;
        if (currentFrame >= maxFrames) {
          currentFrame = 0;
          gifFinished = true;
        }
        changed = true;
      }
    } else {
      // Text scrolling
      if (now - lastScrollTime >= 30) {
        lastScrollTime = now;
        scrollPos -= 2;
        int textWidth = notificationText.length() * 12;
        if (scrollPos < -textWidth) {
          scrollPos = SCREEN_WIDTH;
        }
        changed = true;
      }
    }
    return changed;
  }

  // ------------------ Theme System (Precision Instrument OS) ------------------
  struct ThemeColors {
    uint16_t bg;
    uint16_t text;
    uint16_t accent;
    uint16_t cardBg;
    uint16_t border;
    uint16_t subText;
  };

  ThemeColors getTheme() {
    ThemeColors t;
    if (!negativeDisplay) {
      // Monolith Deep Black (Default - Precision Instrument OS)
      t.bg      = 0x0000; // Deep pitch black
      t.text    = 0xFFFF; // Crisp Pure White
      t.accent  = (robotVariant == "mr_luna") ? 0x07FF : 0xF8B8; // Electric Cyan or Luna Pink
      t.cardBg  = 0x0842; // Dark graphite
      t.border  = 0x2124; // 1px titanium precision rule
      t.subText = 0x8410; // Muted technical silver
    } else {
      // Inverted High-Contrast
      t.bg      = 0xFFFF;
      t.text    = 0x0000;
      t.accent  = 0x001F;
      t.cardBg  = 0xEF5D;
      t.border  = 0xCE79;
      t.subText = 0x632C;
    }
    return t;
  }

  // ------------------ Smartwatch UI Drawing Methods ------------------

  void drawStatusBar(int hour, int minute) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    // Status rail height 22px
    display.fillRect(0, 0, SCREEN_WIDTH, 22, themeBg);
    display.drawFastHLine(0, 22, SCREEN_WIDTH, themeBorder);

    // ── Left zone: HH:MM ─────────────
    display.setTextSize(2);
    display.setTextColor(themeText);
    char tBuf[6];
    snprintf(tBuf, sizeof(tBuf), "%02d:%02d", hour, minute);
    display.setCursor(14, 4);
    display.print(tBuf);

    // ── Right zone: battery and connectivity ─────────
    int bx = SCREEN_WIDTH - 32;

    int batteryPct = getBatteryPercentage(batteryVolts);
    uint16_t batteryColor = 0x07E0; // Green
    if (batteryPct < 20) {
      batteryColor = 0xF800; // Red
    } else if (batteryPct < 50) {
      batteryColor = 0xFFE0; // Amber
    }

    // Battery icon
    display.drawRect(bx, 6, 18, 10, themeSubText);
    display.drawFastVLine(bx + 18, 8, 6, themeSubText);
    int bars = (batteryPct * 4) / 100;
    if (bars > 4) bars = 4;
    for (int b = 0; b < bars; b++) {
      display.fillRect(bx + 2 + b * 4, 8, 3, 6, batteryColor);
    }

    // Battery %
    display.setTextColor(themeSubText);
    display.setTextSize(1);
    String pctStr = String(batteryPct) + "%";
    int pctStrW = pctStr.length() * 6;
    display.setCursor(bx - 4 - pctStrW, 7);
    display.print(pctStr);

    // BLE glyph
    int bleX = bx - 14 - pctStrW;
    int bleY = 11;
    uint16_t bleColor = bleConnectedStatus ? themeAccent : themeBorder;
    if (bleConnectedStatus) {
      display.drawLine(bleX, bleY - 4, bleX, bleY + 4, bleColor);
      display.drawLine(bleX, bleY - 4, bleX + 3, bleY - 2, bleColor);
      display.drawLine(bleX + 3, bleY - 2, bleX - 2, bleY + 2, bleColor);
      display.drawLine(bleX - 2, bleY - 2, bleX + 3, bleY + 2, bleColor);
      display.drawLine(bleX + 3, bleY + 2, bleX, bleY + 4, bleColor);
    } else {
      display.drawCircle(bleX, bleY, 2, bleColor);
    }

    // Silent mode status indicator
    if (silentMode) {
      int silentX = bleX - 12;
      int silentY = bleY;
      uint16_t silentColor = 0xF800;
      display.fillRect(silentX, silentY - 2, 2, 4, silentColor);
      display.fillTriangle(silentX + 2, silentY - 4, silentX + 2, silentY + 4, silentX + 4, silentY, silentColor);
      display.drawLine(silentX + 6, silentY - 2, silentX + 8, silentY, silentColor);
    }

    // ── Centre zone: screen name pill ─────
    const char* nm = "LUNA";
    switch (currentScreen) {
      case SCREEN_CLOCK:         nm = "CLOCK";   break;
      case SCREEN_NOTIFICATIONS: nm = "NOTIFS";  break;
      case SCREEN_CALENDAR:      nm = "CAL";     break;
      case SCREEN_GAMES:         nm = "ARCADE";  break;
      case SCREEN_FACE:          nm = "LUNA";    break;
      case SCREEN_MAPS:          nm = "MAPS";    break;
      case SCREEN_CARD:          nm = "CARD";    break;
      case SCREEN_SETTINGS:      nm = "SETUP";   break;
      default:                   nm = "LUNA";    break;
    }
    int nmLen = strlen(nm) * 6;
    int nmX = (SCREEN_WIDTH - nmLen) / 2;
    if (nmX > 75 && nmX + nmLen < bx - 30) {
      display.fillCircle(nmX - 5, 10, 2, themeAccent);
      display.setTextColor(themeText);
      display.setTextSize(1);
      display.setCursor(nmX, 7);
      display.print(nm);
    }
  }

  void drawPopup() {
    ThemeColors theme = getTheme();
    uint16_t themeAccent = theme.accent;
    uint16_t themeText   = theme.text;
    uint16_t themeCardBg = theme.cardBg;
    uint16_t themeBorder = theme.border;

    display.drawRoundRect(4, 4, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 8, 12, themeAccent);
    display.fillRoundRect(6, 6, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 12, 10, themeCardBg);

    // Title Capsule
    display.fillRoundRect(16, 14, 110, 18, 5, themeAccent);
    display.setTextColor(TFT_WHITE);
    display.setTextSize(1);
    display.setCursor(22, 19);
    display.print("NEW ALERT");

    display.drawFastHLine(12, 38, SCREEN_WIDTH - 24, themeBorder);
    
    // Title
    display.setTextColor(themeText);
    display.setTextSize(2);
    display.setCursor(16, 46);
    String title = popupTitle;
    if (title.length() > 16) title = title.substring(0, 14) + "...";
    display.print(title);
    
    // Body text
    display.setTextSize(2);
    int yStart = 72;
    int charsPerLine = (SCREEN_WIDTH - 32) / 12;
    int line = 0;
    int maxLines = (SCREEN_HEIGHT - 110) / 20;
    for (unsigned int i = 0; i < popupBody.length() && line < maxLines; i += charsPerLine) {
      unsigned int endIdx = i + charsPerLine;
      if (endIdx > popupBody.length()) endIdx = popupBody.length();
      String lineStr = popupBody.substring(i, endIdx);
      display.setCursor(16, yStart + line * 20);
      display.print(lineStr);
      line++;
    }
    
    // Dismiss hint
    display.setTextColor(themeAccent);
    display.setTextSize(1);
    const char* dTxt = "[CLICK BUTTON TO DISMISS]";
    int dW = strlen(dTxt) * 6;
    display.setCursor((SCREEN_WIDTH - dW) / 2, SCREEN_HEIGHT - 20);
    display.print(dTxt);
  }

  void drawNotificationPanel() {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeCardBg  = theme.cardBg;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    if (notificationCount == 0) {
      int centerX = SCREEN_WIDTH / 2;
      display.fillCircle(centerX, 80, 28, themeCardBg);
      display.drawCircle(centerX, 80, 28, themeAccent);
      
      display.setTextColor(themeText);
      display.setTextSize(2);
      const char* h1 = "NO NOTIFS";
      int w1 = strlen(h1) * 12;
      display.setCursor((SCREEN_WIDTH - w1) / 2, 126);
      display.print(h1);
      
      display.setTextColor(themeSubText);
      display.setTextSize(1);
      const char* h2 = "HISTORY IS EMPTY";
      int w2 = strlen(h2) * 6;
      display.setCursor((SCREEN_WIDTH - w2) / 2, 150);
      display.print(h2);

      const char* nav = "BTN1: DISMISS // BTN2: NEXT";
      int nw = strlen(nav) * 6;
      display.setCursor((SCREEN_WIDTH - nw) / 2, SCREEN_HEIGHT - 18);
      display.print(nav);
      return;
    }
    
    if (notificationSelected) {
      NotificationItem& notif = notificationHistory[currentNotifViewIdx];
      
      display.drawRoundRect(6, 26, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 32, 10, themeAccent);
      display.fillRoundRect(8, 28, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 36, 8, themeCardBg);
      
      display.setTextColor(themeText);
      display.setTextSize(2);
      display.setCursor(14, 34);
      String title = notif.title;
      if (title.length() > 10) title = title.substring(0, 8) + "...";
      display.print(title);
      
      display.setTextColor(themeAccent);
      display.setTextSize(2);
      display.setCursor(SCREEN_WIDTH - 76, 34);
      display.print(notif.timeStr);
      
      display.drawFastHLine(12, 54, SCREEN_WIDTH - 24, themeBorder);
      
      display.setTextColor(themeText);
      display.setTextSize(2);
      int yStart = 64;
      int charsPerLine = (SCREEN_WIDTH - 28) / 12;
      int line = 0;
      int maxLines = (SCREEN_HEIGHT - 106) / 20;
      for (unsigned int i = 0; i < notif.body.length() && line < maxLines; i += charsPerLine) {
        unsigned int endIdx = i + charsPerLine;
        if (endIdx > notif.body.length()) endIdx = notif.body.length();
        display.setCursor(14, yStart + line * 20);
        display.print(notif.body.substring(i, endIdx));
        line++;
      }
      
      display.setTextColor(themeAccent);
      display.setTextSize(1);
      char footerBuf[32];
      snprintf(footerBuf, sizeof(footerBuf), "[%d/%d] BTN1: BACK // BTN2: NEXT", currentNotifViewIdx + 1, notificationCount);
      int footerW = strlen(footerBuf) * 6;
      display.setCursor((SCREEN_WIDTH - footerW) / 2, SCREEN_HEIGHT - 18);
      display.print(footerBuf);
    } else {
      display.setTextColor(themeText);
      display.setTextSize(2);
      display.setCursor(14, 28);
      display.print("NOTIFICATIONS");
      
      display.drawFastHLine(12, 48, SCREEN_WIDTH - 24, themeBorder);
      
      for (int i = 0; i < notificationCount && i < 4; i++) {
        int y = 54 + i * 40;
        NotificationItem& notif = notificationHistory[i];
        
        if (notificationsActive && i == currentNotifViewIdx) {
          display.fillRoundRect(8, y, SCREEN_WIDTH - 16, 36, 4, themeBorder);
          display.drawRoundRect(8, y, SCREEN_WIDTH - 16, 36, 4, themeAccent);
        } else {
          display.drawRoundRect(8, y, SCREEN_WIDTH - 16, 36, 4, themeBorder);
        }
        
        display.setCursor(14, y + 4);
        display.setTextSize(2);
        display.setTextColor(themeText);
        String shortTitle = notif.title;
        if (shortTitle.length() > 11) shortTitle = shortTitle.substring(0, 9) + "..";
        display.print(shortTitle);
        
        display.setCursor(SCREEN_WIDTH - 52, y + 4);
        display.setTextSize(1);
        display.setTextColor(themeAccent);
        display.print(notif.timeStr);
        
        display.setCursor(14, y + 22);
        display.setTextSize(1);
        display.setTextColor(themeSubText);
        String snippet = notif.body;
        if (snippet.length() > 28) snippet = snippet.substring(0, 26) + "...";
        display.print(snippet);
      }
      
      display.setTextSize(1);
      display.setTextColor(themeAccent);
      const char* tip = "BTN1: OPEN // BTN2: SCREEN";
      int tipW = strlen(tip) * 6;
      display.setCursor((SCREEN_WIDTH - tipW) / 2, SCREEN_HEIGHT - 16);
      display.print(tip);
    }
  }

  void drawCalendarEvents() {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeCardBg  = theme.cardBg;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    if (calendarEventCount == 0) {
      int centerX = SCREEN_WIDTH / 2;
      display.drawRect(centerX - 12, 60, 24, 24, themeSubText);
      display.drawFastHLine(centerX - 12, 68, 24, themeSubText);
      display.fillRect(centerX - 8, 56, 2, 6, themeSubText);
      display.fillRect(centerX + 6, 56, 2, 6, themeSubText);
      
      display.setTextColor(themeText);
      display.setTextSize(2);
      const char* e1 = "NO EVENTS";
      int ew1 = strlen(e1) * 12;
      display.setCursor((SCREEN_WIDTH - ew1) / 2, 100);
      display.print(e1);

      display.setTextColor(themeSubText);
      display.setTextSize(1);
      const char* e2 = "SYNC EVENTS IN PHONE APP";
      int ew2 = strlen(e2) * 6;
      display.setCursor((SCREEN_WIDTH - ew2) / 2, 130);
      display.print(e2);

      const char* nav = "BTN1: GRID // BTN2: NEXT";
      int nw = strlen(nav) * 6;
      display.setCursor((SCREEN_WIDTH - nw) / 2, SCREEN_HEIGHT - 18);
      display.print(nav);
      return;
    }
    
    CalendarEventItem& ev = calendarEvents[currentCalViewIdx];

    // Card Container
    display.drawRoundRect(6, 26, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 32, 10, themeBorder);
    display.fillRoundRect(8, 28, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 36, 8, themeCardBg);

    // Event Category Pill
    String tUpper = ev.type;
    tUpper.toUpperCase();
    uint16_t pillColor = themeAccent;
    if (tUpper.indexOf("ALARM") >= 0) pillColor = 0xF800; // Red
    else if (tUpper.indexOf("REMIND") >= 0) pillColor = 0xFD20; // Amber
    else if (tUpper.indexOf("BIRTH") >= 0 || tUpper.indexOf("BDAY") >= 0) pillColor = 0xF81F; // Magenta
    else if (tUpper.indexOf("MEET") >= 0) pillColor = 0x051F; // Blue

    display.fillRoundRect(14, 34, 76, 20, 4, pillColor);
    display.setTextSize(1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(20, 40);
    display.print(tUpper.substring(0, 8));

    // Time readout
    display.setTextColor(themeText);
    display.setTextSize(3);
    display.setCursor(SCREEN_WIDTH - 96, 32);
    display.print(ev.timeStr);

    display.drawFastHLine(14, 60, SCREEN_WIDTH - 28, themeBorder);

    // Title
    display.setTextColor(themeText);
    display.setTextSize(2);
    int yT = 68;
    int charsPerLine = (SCREEN_WIDTH - 32) / 12;
    int line = 0;
    for (unsigned int i = 0; i < ev.title.length() && line < 3; i += charsPerLine) {
      unsigned int endIdx = i + charsPerLine;
      if (endIdx > ev.title.length()) endIdx = ev.title.length();
      display.setCursor(16, yT + line * 20);
      display.print(ev.title.substring(i, endIdx));
      line++;
    }

    display.drawFastHLine(14, 134, SCREEN_WIDTH - 28, themeBorder);

    // Metadata
    display.setTextSize(1);
    display.setTextColor(themeSubText);
    display.setCursor(16, 142);
    if (ev.dateStr.length() > 0 && ev.dateStr != "*") {
      display.printf("DATE   // %s", ev.dateStr.c_str());
    } else {
      display.print("DATE   // DAILY RECURRING");
    }

    display.setCursor(16, 158);
    display.print("STATUS // ACTIVE ON LUNA");

    // Pagination
    char pageBuf[32];
    snprintf(pageBuf, sizeof(pageBuf), "INDEX %02d/%02d", currentCalViewIdx + 1, calendarEventCount);
    display.setTextColor(themeAccent);
    display.setCursor(16, 178);
    display.print(pageBuf);

    // Button navigation hints
    display.setTextColor(themeSubText);
    const char* navHint = "BTN1: NEXT // BTN2: SCREEN";
    int nhw = strlen(navHint) * 6;
    display.setCursor((SCREEN_WIDTH - nhw) / 2, SCREEN_HEIGHT - 18);
    display.print(navHint);
  }

  void drawCalendarGrid(String dateStr, String dayStr) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    // Month & Year header
    display.setTextSize(2);
    display.setTextColor(themeText);
    display.setCursor(16, 28);
    display.print("CALENDAR");

    display.setTextColor(themeSubText);
    display.setCursor(136, 28);
    display.print("2026");

    display.drawFastHLine(14, 48, SCREEN_WIDTH - 28, themeBorder);

    // Giant Day Hero
    display.setTextSize(4);
    display.setTextColor(themeText);
    display.setCursor(18, 56);
    display.print(dateStr.substring(0, 2));

    display.setTextSize(2);
    display.setTextColor(themeAccent);
    display.setCursor(80, 58);
    String dayUpper = dayStr;
    dayUpper.toUpperCase();
    display.print(dayUpper);

    display.drawRoundRect(80, 80, 48, 16, 4, themeAccent);
    display.setTextSize(1);
    display.setTextColor(themeText);
    display.setCursor(86, 84);
    display.print("TODAY");

    // Divider
    display.drawFastHLine(14, 106, SCREEN_WIDTH - 28, themeBorder);

    // Scheduled Events count preview
    display.setTextSize(1);
    display.setTextColor(themeSubText);
    display.setCursor(16, 116);
    display.printf("SCHEDULED EVENTS: %d", calendarEventCount);

    if (calendarEventCount > 0) {
      for (int i = 0; i < calendarEventCount && i < 2; i++) {
        int y = 132 + i * 32;
        display.fillRoundRect(14, y, SCREEN_WIDTH - 28, 28, 4, themeBorder);
        display.setCursor(20, y + 6);
        display.setTextSize(1);
        display.setTextColor(themeText);
        display.printf("[%s] %s", calendarEvents[i].timeStr.c_str(), calendarEvents[i].title.c_str());
      }
    } else {
      display.setCursor(16, 140);
      display.print("No events scheduled for today.");
    }

    display.setTextSize(1);
    display.setTextColor(themeAccent);
    const char* nav = "BTN1: EVENTS // BTN2: NEXT";
    int nw = strlen(nav) * 6;
    display.setCursor((SCREEN_WIDTH - nw) / 2, SCREEN_HEIGHT - 18);
    display.print(nav);
  }

  // ---- Sprite AI animation state (for primary expression face) ----
  int spriteAnimIndex;      // which of the 12 sprite_ai animations is active (0-11)
  int spriteFrameIndex;     // current frame within that animation (0-3)
  unsigned long lastSpriteFrameTime;

  // Map Expression -> sprite_ai animation index (all 12 animations supported)
  int exprToSpriteAnim(Expression expr) {
    Expression e = expr;
    if (e == EXPR_IDLE) e = defaultExpr;
    int idx = (int)e;
    if (idx >= 0 && idx < SPRITE_AI13_ANIMATION_COUNT) {
      return idx;
    }
    return spriteAnimIndex;
  }

  void drawRobotFaceScreen() {
    Expression exprToDraw = currentExpr;
    if (exprToDraw == EXPR_IDLE) {
      exprToDraw = defaultExpr;
    }

    int animIdx = exprToSpriteAnim(exprToDraw);
    if (animIdx != spriteAnimIndex) {
      spriteAnimIndex = animIdx;
      spriteFrameIndex = 0;
      lastSpriteFrameTime = millis();
    }

    // Advance sprite frame timer
    unsigned long now = millis();
    if (now - lastSpriteFrameTime >= (unsigned long)frameDelayMs) {
      lastSpriteFrameTime = now;
      spriteFrameIndex = (spriteFrameIndex + 1) % SPRITE_AI13_FRAME_COUNT;
    }

    const uint16_t* frameData = getSpriteAi13Frame(spriteAnimIndex, spriteFrameIndex);
    if (frameData != nullptr) {
      display.drawRGBBitmap2x(frameData);
    } else {
      display.fillScreen(TFT_BLACK);
    }

    // Silent mode indicator (top-right)
    if (silentMode) {
      uint16_t silentColor = TFT_WHITE;
      int silentX = SCREEN_WIDTH - 20;
      int silentY = 10;
      display.fillRect(silentX, silentY - 2, 2, 4, silentColor);
      display.fillTriangle(silentX + 2, silentY - 4, silentX + 2, silentY + 4, silentX + 4, silentY, silentColor);
      display.drawLine(silentX + 6, silentY - 2, silentX + 8, silentY, silentColor);
      display.drawLine(silentX + 8, silentY - 2, silentX + 6, silentY, silentColor);
    }
  }

  void drawMapScreenLandscape(int hour, int minute, bool is12Hour) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeCardBg  = theme.cardBg;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    // Full screen edge-to-edge 240x240 (No status bar)
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, themeBg);
    display.setTextWrap(false);

    int cardX = 6;
    int cardW = SCREEN_WIDTH - 12; // 228

    // Upper Card: Maneuver & Distance (Y: 4..124, H = 120)
    display.fillRoundRect(cardX, 4, cardW, 120, 10, themeCardBg);
    display.drawRoundRect(cardX, 4, cardW, 120, 10, themeBorder);

    int cx = SCREEN_WIDTH / 2;
    int cy = 34;

    String dirUpper = mapDirection;
    dirUpper.toUpperCase();

    if (dirUpper.indexOf("LEFT") >= 0) {
      display.fillRect(cx + 8, cy - 6, 12, 30, themeAccent);
      display.fillRect(cx - 18, cy - 6, 28, 12, themeAccent);
      display.fillTriangle(cx - 30, cy, cx - 14, cy - 14, cx - 14, cy + 14, themeAccent);
    } else if (dirUpper.indexOf("RIGHT") >= 0) {
      display.fillRect(cx - 20, cy - 6, 12, 30, themeAccent);
      display.fillRect(cx - 10, cy - 6, 28, 12, themeAccent);
      display.fillTriangle(cx + 30, cy, cx + 14, cy - 14, cx + 14, cy + 14, themeAccent);
    } else if (dirUpper.indexOf("UTURN") >= 0 || dirUpper.indexOf("U-TURN") >= 0) {
      display.fillCircle(cx, cy - 8, 18, themeAccent);
      display.fillCircle(cx, cy - 8, 10, themeCardBg);
      display.fillRect(cx - 22, cy - 8, 44, 20, themeCardBg);
      display.fillRect(cx - 18, cy - 8, 8, 24, themeAccent);
      display.fillRect(cx + 10, cy - 8, 8, 18, themeAccent);
      display.fillTriangle(cx + 14, cy + 20, cx + 4, cy + 8, cx + 24, cy + 8, themeAccent);
    } else if (dirUpper.indexOf("ROUNDABOUT") >= 0 || dirUpper.indexOf("ROUND") >= 0) {
      display.fillCircle(cx, cy, 18, themeAccent);
      display.fillCircle(cx, cy, 11, themeCardBg);
      display.fillRect(cx - 4, cy + 10, 8, 14, themeAccent);
      display.fillTriangle(cx + 16, cy - 20, cx + 4, cy - 14, cx + 16, cy - 8, themeAccent);
    } else {
      display.fillRect(cx - 6, cy - 6, 12, 32, themeAccent);
      display.fillTriangle(cx, cy - 24, cx - 18, cy - 4, cx + 18, cy - 4, themeAccent);
    }

    // Next Turn Distance
    display.setTextSize(3);
    display.setTextColor(themeText, themeCardBg);
    String distStr = (mapDistance == "" || mapDistance == "--") ? "---" : mapDistance;
    int distW = distStr.length() * 18;
    int distX = (SCREEN_WIDTH - distW) / 2;
    display.setCursor(distX, 64);
    display.print(distStr);

    // Turn Instruction / Road
    String dirLabel = mapRoad;
    if (dirLabel.length() == 0) {
      if (dirUpper.indexOf("LEFT") >= 0) dirLabel = "Turn Left";
      else if (dirUpper.indexOf("RIGHT") >= 0) dirLabel = "Turn Right";
      else if (dirUpper.indexOf("UTURN") >= 0 || dirUpper.indexOf("U-TURN") >= 0) dirLabel = "Make U-Turn";
      else if (dirUpper.indexOf("ROUNDABOUT") >= 0 || dirUpper.indexOf("ROUND") >= 0) dirLabel = "Roundabout";
      else dirLabel = "Continue Straight";
    }
    display.setTextSize(2);
    display.setTextColor(themeAccent, themeCardBg);
    int rw = dirLabel.length() * 12;
    int rx = (SCREEN_WIDTH - rw) / 2;
    if (rx < cardX + 6) rx = cardX + 6;
    display.setCursor(rx, 94);
    display.print(dirLabel);

    // Lower Card: Route Details (Y: 128..236, H = 108)
    display.fillRoundRect(cardX, 128, cardW, 108, 10, themeCardBg);
    display.drawRoundRect(cardX, 128, cardW, 108, 10, themeBorder);

    display.setTextSize(1);
    display.setTextColor(themeSubText, themeCardBg);
    display.setCursor(cardX + 12, 138);
    display.print("TRIP TELEMETRY");

    display.drawFastHLine(cardX + 10, 150, cardW - 20, themeBorder);

    display.setTextSize(2);
    display.setTextColor(themeText, themeCardBg);
    display.setCursor(cardX + 12, 160);
    display.print(mapTotalTime.length() > 0 ? mapTotalTime : "IN TRANSIT");

    display.setCursor(cardX + 12, 184);
    display.setTextSize(1);
    display.setTextColor(themeSubText, themeCardBg);
    display.printf("DIST: %s", mapTotalDist.length() > 0 ? mapTotalDist.c_str() : "--");

    display.setCursor(cardX + 12, 198);
    display.printf("ETA : %s", mapEta.length() > 0 ? mapEta.c_str() : "--");

    // Right-aligned clock
    char cBuf[8];
    snprintf(cBuf, sizeof(cBuf), "%02d:%02d", hour, minute);
    display.setTextSize(2);
    display.setTextColor(themeAccent, themeCardBg);
    display.setCursor(SCREEN_WIDTH - 76, 170);
    display.print(cBuf);
  }

  void drawClockScreen(int hour, int minute, int second, String day, String date, int style, bool is12Hour, int steps) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    int dispHour = hour;
    if (is12Hour) {
      dispHour = hour % 12;
      if (dispHour == 0) dispHour = 12;
    }

    if ((style % 2) == 0) {
      // ── STYLE 0: MONOLITH MINIMAL PRECISION WATCHFACE (240x240) ──────────
      char timeStr[6];
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispHour, minute);

      // Giant time at Y=36
      display.setTextSize(5);
      display.setTextColor(themeText);
      display.setCursor(20, 36);
      display.print(timeStr);

      // Seconds / AM-PM
      display.setTextSize(2);
      display.setTextColor(themeAccent);
      display.setCursor(182, 38);
      if (is12Hour) {
        display.print((hour >= 12) ? "PM" : "AM");
      } else {
        display.print(":");
        if (second < 10) display.print("0");
        display.print(second);
      }

      display.drawFastHLine(16, 84, SCREEN_WIDTH - 32, themeBorder);

      // Date Block
      display.fillRect(16, 92, 2, 42, themeAccent);

      display.setTextSize(3);
      display.setTextColor(themeText);
      display.setCursor(26, 92);
      String dayUpper = day;
      dayUpper.toUpperCase();
      display.print(dayUpper);

      display.setTextSize(2);
      display.setTextColor(themeSubText);
      display.setCursor(26, 118);
      String dateUpper = date;
      dateUpper.toUpperCase();
      display.print(dateUpper);

      // Right-aligned secondary telemetry tags
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.setCursor(140, 94);  display.print("SYS // NOMINAL");
      display.setCursor(140, 108); display.print("RTC // SYNCED");
      display.setCursor(140, 122); display.print("PWR // OPTIMAL");

      display.drawFastHLine(16, 142, SCREEN_WIDTH - 32, themeBorder);

      // Lower Telemetry Rails
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.setCursor(16, 150);
      display.print("ACTIVITY");

      display.setTextSize(2);
      display.setTextColor(themeText);
      display.setCursor(16, 162);
      display.print(steps);
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.print(" STPS");

      display.drawFastHLine(16, 180, 90, themeBorder);
      int stepW = (steps > 0) ? min(90, (steps * 90) / 10000) : 10;
      display.drawFastHLine(16, 180, stepW, themeAccent);

      // Power rail
      int batteryPct = getBatteryPercentage(batteryVolts);
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.setCursor(130, 150);
      display.print("TELEMETRY");

      display.setTextSize(2);
      display.setTextColor(themeText);
      display.setCursor(130, 162);
      display.print(batteryPct);
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.print("% PWR");

      display.drawFastHLine(130, 180, 90, themeBorder);
      int battW = min(90, (batteryPct * 90) / 100);
      uint16_t battCol = (batteryPct < 25) ? 0xF800 : ((batteryPct < 55) ? 0xFFE0 : 0x07E0);
      display.drawFastHLine(130, 180, battW, battCol);

      // Button navigation hints
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      const char* hint = "BTN1: STYLE // BTN2: SCREEN";
      int hw = strlen(hint) * 6;
      display.setCursor((SCREEN_WIDTH - hw) / 2, SCREEN_HEIGHT - 18);
      display.print(hint);

    } else {
      // ── STYLE 1: AEROSPACE CHRONO TELEMETRY (240x240) ──────────────
      char hStr[4], mStr[4];
      snprintf(hStr, sizeof(hStr), "%02d", dispHour);
      snprintf(mStr, sizeof(mStr), "%02d", minute);

      display.setTextSize(5);
      display.setTextColor(themeText);
      display.setCursor(20, 32);
      display.print(hStr);

      display.setTextColor(themeAccent);
      display.setCursor(20, 80);
      display.print(mStr);

      display.drawFastVLine(96, 30, 96, themeBorder);

      display.setTextSize(2);
      display.setTextColor(themeText);
      display.setCursor(108, 36);
      display.print(day);

      display.setTextSize(2);
      display.setTextColor(themeSubText);
      display.setCursor(108, 58);
      display.print(date);

      display.setTextSize(1);
      display.setTextColor(themeAccent);
      display.setCursor(108, 84);
      display.printf("SEC: %02d", second);

      display.setTextColor(themeSubText);
      display.setCursor(108, 100);
      display.printf("STP: %d", steps);

      display.drawFastHLine(16, 136, SCREEN_WIDTH - 32, themeBorder);

      int secW = (second * (SCREEN_WIDTH - 32)) / 60;
      display.drawFastHLine(16, 154, SCREEN_WIDTH - 32, themeBorder);
      display.drawFastHLine(16, 154, secW, themeAccent);

      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.setCursor(16, 168);
      display.print("AEROSPACE CHRONO // T-INDEX");

      const char* hint = "BTN1: STYLE // BTN2: SCREEN";
      int hw = strlen(hint) * 6;
      display.setCursor((SCREEN_WIDTH - hw) / 2, SCREEN_HEIGHT - 18);
      display.print(hint);
    }
  }

  void drawSettingsMenuLandscape(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    display.setTextSize(2);
    display.setTextColor(themeText);
    display.setCursor(16, 26);
    display.print("SETTINGS");

    display.drawFastHLine(14, 44, SCREEN_WIDTH - 28, themeBorder);

    const int CONTENT_TOP = 48;
    const int ITEM_H      = 22;
    const int TOTAL_ITEMS = 8;

    const char* titles[] = {
      "Brightness", "Sound FX", "Clock Face", "Speed",
      "Theme", "Bluetooth", "Save", "Exit"
    };

    for (int i = 0; i < TOTAL_ITEMS; i++) {
      int yPos = CONTENT_TOP + i * ITEM_H;
      if (yPos >= SCREEN_HEIGHT - 24) break;

      bool isCurrent = (option == i) && settingsActive;

      if (isCurrent) {
        display.fillRect(10, yPos + 2, 3, 16, themeAccent);
      }

      display.setTextSize(1);
      display.setTextColor(isCurrent ? themeAccent : themeText);
      display.setCursor(18, yPos + 6);
      display.print(titles[i]);

      int cX = 140;
      int cY = yPos + 4;

      display.setTextColor(themeSubText);
      switch (i) {
        case 0:
          display.setCursor(cX, cY + 2);
          display.print(brightness == 1 ? "33%" : (brightness == 2 ? "66%" : "100%"));
          break;
        case 1:
          display.setCursor(cX, cY + 2);
          display.print(!silentMode ? "ACTIVE" : "MUTED");
          break;
        case 2:
          display.setCursor(cX, cY + 2);
          display.print((clockStyle % 2 == 0) ? "MONOLITH" : "CHRONO");
          break;
        case 3:
          display.setCursor(cX, cY + 2);
          display.printf("%dms", speed);
          break;
        case 4:
          display.setCursor(cX, cY + 2);
          display.print(invertOn ? "LIGHT" : "DARK");
          break;
        case 5:
          display.setCursor(cX, cY + 2);
          display.print(bleOn ? "ON" : "OFF");
          break;
        case 6:
          display.setCursor(cX, cY + 2);
          display.print("PRESS");
          break;
        case 7:
          display.setCursor(cX, cY + 2);
          display.print("PRESS");
          break;
      }
    }

    display.setTextSize(1);
    display.setTextColor(themeAccent);
    const char* nav = "BTN1: MOVE // BTN2: SELECT";
    int nw = strlen(nav) * 6;
    display.setCursor((SCREEN_WIDTH - nw) / 2, SCREEN_HEIGHT - 16);
    display.print(nav);
  }

  // ------------------ Primary Smartwatch Draw Adapter ------------------
  void draw(int hour, int minute, int second, String day, String date, int style = 0, bool is12Hour = false) {
    display.fillScreen(TFT_BLACK);
    
    if (popupActive && (millis() - popupStartTime > popupDuration)) {
      popupActive = false;
    }

    if (popupActive) {
      drawPopup();
    } else {
      if (currentScreen != SCREEN_FACE && currentScreen != SCREEN_MAPS && currentScreen != SCREEN_CARD) {
        drawStatusBar(hour, minute);
      }
      
      switch (currentScreen) {
        case SCREEN_CLOCK:
          drawClockScreen(hour, minute, second, day, date, style, is12Hour, touchCount);
          break;
        case SCREEN_SETTINGS:
          drawSettingsMenuLandscape(menuOption, optionSelected, bleActive, gifSpeed, clockStyle, negativeDisplay, oledBrightness);
          break;
        case SCREEN_NOTIFICATIONS:
          drawNotificationPanel();
          break;
        case SCREEN_CALENDAR:
          if (showCalendarGrid) {
            drawCalendarGrid(date, day);
          } else {
            drawCalendarEvents();
          }
          break;
        case SCREEN_GAMES:
          if (!gamesActive) {
            ThemeColors theme = getTheme();
            uint16_t themeAccent  = theme.accent;
            uint16_t themeBg      = theme.bg;
            uint16_t themeText    = theme.text;
            uint16_t themeCardBg  = theme.cardBg;
            uint16_t themeBorder  = theme.border;
            uint16_t themeSubText = theme.subText;

            display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

            display.fillRoundRect(8, 28, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 36, 10, themeCardBg);
            display.drawRoundRect(8, 28, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 36, 10, themeBorder);

            display.setTextSize(2);
            display.setTextColor(themeAccent);
            display.setCursor(20, 42);
            display.print("LUNA ARCADE");

            display.setTextSize(1);
            display.setTextColor(themeSubText);
            display.setCursor(20, 64);
            display.print("7 RETRO ENGINE TITLES");

            display.drawFastHLine(18, 78, SCREEN_WIDTH - 36, themeBorder);

            // Center gamepad illustration
            int icx = SCREEN_WIDTH / 2;
            int icy = 114;
            display.fillRoundRect(icx - 36, icy - 16, 72, 32, 6, themeBg);
            display.drawRoundRect(icx - 36, icy - 16, 72, 32, 6, themeBorder);
            display.fillRect(icx - 24, icy - 8, 6, 16, themeAccent);
            display.fillRect(icx - 29, icy - 3, 16, 6, themeAccent);
            display.fillCircle(icx + 18, icy - 3, 3, 0xF800);
            display.fillCircle(icx + 10, icy + 4, 3, themeAccent);
            display.fillCircle(icx + 26, icy + 4, 3, 0x07E0);

            // Instructions
            display.setTextSize(1);
            display.setTextColor(themeText);
            display.setCursor(24, 150);
            display.print("BTN1 : START / JUMP / MOVE");

            display.setTextColor(themeSubText);
            display.setCursor(24, 168);
            display.print("BTN2 : MENU / NEXT / EXIT");

            const char* tip = "BTN1: LAUNCH // BTN2: SCREEN";
            int tw = strlen(tip) * 6;
            display.setCursor((SCREEN_WIDTH - tw) / 2, SCREEN_HEIGHT - 18);
            display.print(tip);
          } else {
            if (!gamePlaying) {
              games.drawMenu(display);
            } else {
              if (gameSelected == 1) {
                games.updateAndDrawRacer(display, audio);
              } else if (gameSelected == 2) {
                games.updateAndDrawSpace(display, audio);
              } else if (gameSelected == 3) {
                games.updateAndDrawFlappy(display, audio);
              } else if (gameSelected == 4) {
                games.updateAndDrawCatcher(display, audio);
              } else if (gameSelected == 5) {
                games.updateAndDrawJump(display, audio);
              } else if (gameSelected == 6) {
                games.updateAndDrawStacker(display, audio);
              } else if (gameSelected == 7) {
                games.updateAndDrawMemory(display, audio);
              }
            }
          }
          break;
        case SCREEN_FACE:
          drawRobotFaceScreen();
          break;
        case SCREEN_MAPS:
          drawMapScreenLandscape(hour, minute, is12Hour);
          break;
        case SCREEN_CARD:
          qrCard.drawQRScreen(display);
          break;
        default:
          drawRobotFaceScreen();
          break;
      }
    }

    if (!popupActive && currentScreen == SCREEN_FACE && headerText.length() > 0) {
      uint16_t headerBg = negativeDisplay ? TFT_WHITE : TFT_BLUE;
      uint16_t headerFg = negativeDisplay ? TFT_BLUE : TFT_WHITE;
      display.fillRect(0, 0, SCREEN_WIDTH, 22, headerBg);
      display.setTextColor(headerFg);
      display.setTextSize(2);
      
      int textW = headerText.length() * 12;
      int startX = (SCREEN_WIDTH - textW) / 2;
      if (startX < 0) startX = 0;
      
      display.setCursor(startX, 3);
      display.print(headerText);
      display.drawFastHLine(0, 22, SCREEN_WIDTH, headerFg);
    }

    // ── Alarm / Meeting / Reminder Ringing Overlay ──
    if (alarmRingingActive) {
      int ox = 8;
      int oy = 12;
      int ow = SCREEN_WIDTH - 16;
      int oh = SCREEN_HEIGHT - 24;
      uint16_t alertColor = 0xF800; // Red for Alarm/Meeting
      if (ringingType.indexOf("remind") >= 0) alertColor = 0xFD20; // Amber/Orange for Reminder
      else if (ringingType.indexOf("bday") >= 0 || ringingType.indexOf("birth") >= 0) alertColor = 0xF81F; // Magenta for Birthday

      // Pulsing border effect
      bool pulse = ((millis() / 350) % 2 == 0);
      display.fillRoundRect(ox, oy, ow, oh, 12, 0x0841); // Dark slate card
      display.drawRoundRect(ox, oy, ow, oh, 12, pulse ? alertColor : TFT_WHITE);
      display.drawRoundRect(ox + 1, oy + 1, ow - 2, oh - 2, 11, pulse ? alertColor : 0x4208);

      // Event Type Badge
      display.fillRoundRect(ox + 12, oy + 12, 106, 20, 5, alertColor);
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(ox + 16, oy + 18);
      String alertLabel = ringingType;
      alertLabel.toUpperCase();
      if (alertLabel.length() == 0) alertLabel = "ALARM";
      display.print(alertLabel + " RINGING!");

      // Event Time
      display.setTextSize(3);
      display.setTextColor(TFT_WHITE);
      display.setCursor(ox + 12, oy + 38);
      display.print(ringingTime.length() > 0 ? ringingTime : "NOW");

      // Event Title
      display.setTextSize(2);
      display.setTextColor(alertColor);
      int ty = oy + 70;
      int cpl = (ow - 24) / 12;
      int tLines = 0;
      for (unsigned int i = 0; i < ringingTitle.length() && tLines < 3; i += cpl) {
        unsigned int endI = i + cpl;
        if (endI > ringingTitle.length()) endI = ringingTitle.length();
        display.setCursor(ox + 12, ty + tLines * 18);
        display.print(ringingTitle.substring(i, endI));
        tLines++;
      }

      // Button dismissal prompt
      int dbW = ow - 24;
      int dbH = 32;
      int dbX = ox + 12;
      int dbY = oy + oh - 40;
      display.fillRoundRect(dbX, dbY, dbW, dbH, 8, pulse ? alertColor : 0x2124);
      display.setTextColor(TFT_WHITE);
      display.setTextSize(1);
      const char* dTxt = "CLICK BUTTON TO DISMISS";
      int dtw = strlen(dTxt) * 6;
      display.setCursor(dbX + (dbW - dtw) / 2, dbY + 12);
      display.print(dTxt);
    }
    
    // Draw directly at (0, 0) for 1.3" display (no Y offset!)
    display.flush(tft);
  }

  // Legacy compatibility
  void drawSettingsMenu(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    display.fillScreen(TFT_WHITE);
    drawStatusBar(12, 0);
    drawSettingsMenuLandscape(option, selected, bleOn, speed, clockStyle, invertOn, brightness);
    display.flush(tft);
  }
};

#endif // EXPRESSIONS_H
