#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "config.h"
#include "image_logo.h"

// 14 Full-Color Video AI animation names
inline const char* getSpriteAiAnimationName(int idx) {
  switch (idx) {
    case 0: return "Luna Idle";
    case 1: return "Angry Face";
    case 2: return "Hungry Menu";
    case 3: return "Getting Hungry";
    case 4: return "Eat Fish";
    case 5: return "Drink Milk";
    case 6: return "Eat Salad";
    case 7: return "Getting Sick";
    case 8: return "Luna Sick";
    case 9: return "Recovered";
    case 10: return "Going to Sleep";
    case 11: return "Sleeping";
    case 12: return "Waking Up";
    case 13: return "Luna Thinking";
    default: return "Luna Anime";
  }
}

// Backward compatibility alias
inline const char* getSpriteAi13AnimName(int idx) {
  return getSpriteAiAnimationName(idx);
}

inline SoundEffect getAnimationSound(int idx) {
  switch (idx) {
    case 0:  return SOUND_ANIM_IDLE;
    case 1:  return SOUND_ANIM_ANGRY;
    case 2:  return SOUND_ANIM_HUNGRY_MENU;
    case 3:  return SOUND_ANIM_GETTING_HUNGRY;
    case 4:  return SOUND_ANIM_EAT_FISH;
    case 5:  return SOUND_ANIM_DRINK_MILK;
    case 6:  return SOUND_ANIM_EAT_SALAD;
    case 7:  return SOUND_ANIM_GETTING_SICK;
    case 8:  return SOUND_ANIM_SICK;
    case 9:  return SOUND_ANIM_RECOVERED;
    case 10: return SOUND_ANIM_SLEEP;
    case 11: return SOUND_ANIM_SLEEPING;
    case 12: return SOUND_ANIM_WAKEUP;
    case 13: return SOUND_ANIM_THINKING;
    default: return SOUND_ANIM_IDLE;
  }
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

public:
  uint16_t* getTopBuffer() { return topBuf; }
  uint16_t* getBtmBuffer() { return btmBuf; }

  uint16_t getRawPixel(int16_t x, int16_t y) const {
    if (x < 0 || x >= 240 || y < 0 || y >= 240) return 0;
    if (y < 120) {
      return topBuf ? topBuf[x + y * 240] : 0;
    } else {
      return btmBuf ? btmBuf[x + (y - 120) * 240] : 0;
    }
  }
};

#include "robot_eye_animation.h"

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

  // Incoming Call Ringing Overlay state
  bool callRingingActive;
  String callerName;
  bool callMuted;
  unsigned long callRingStartTime;

  // WhatsApp Quick Reply Sheet state
  bool quickReplyActive;
  int quickReplySelectedIdx;

  // Transient Silent Mode Overlay state
  unsigned long silentOverlayStartTime;
  bool showSilentOverlay;
  bool silentOverlayState;

  // Video AI Robot Eye Animation Controller
  RobotEyeAnimation robotEyeAnim;

public:
  String headerText;
  bool timeSynced = false;
  bool hungryState = false;
  bool isFeeding = false;
  unsigned long feedingStartTime = 0;
  String thoughtText = "";
  void setHungry(bool h) { hungryState = h; }
  bool isHungry() const { return hungryState; }
  void startFeeding() { isFeeding = true; feedingStartTime = millis(); }
  void stopFeeding() { isFeeding = false; }
  bool isFeedingActive() const { return isFeeding; }
  unsigned long getFeedingStartTime() const { return feedingStartTime; }
  void setThoughtText(const String& txt) { thoughtText = txt; }
  String getThoughtText() const { return thoughtText; }
  RobotEyeAnimation& getRobotEyeAnim() { return robotEyeAnim; }

  // Call Ringing Controls
  void setCallRinging(bool ringing, String caller = "Incoming Call") {
    callRingingActive = ringing;
    if (ringing) {
      callerName = caller;
      callMuted = false;
      callRingStartTime = millis();
    }
  }
  void muteIncomingCall() { callMuted = true; }
  void dismissIncomingCall() { callRingingActive = false; callMuted = false; callerName = ""; }
  bool isCallRingingActive() const { return callRingingActive; }
  bool isCallMuted() const { return callMuted; }
  String getCallerName() const { return callerName; }

  // WhatsApp Quick Reply Controls
  void openQuickReply() { quickReplyActive = true; quickReplySelectedIdx = 0; }
  void closeQuickReply() { quickReplyActive = false; }
  bool isQuickReplyActive() const { return quickReplyActive; }
  int getQuickReplyCount() const { return 5; }
  const char* getQuickReplyPreset(int idx) const {
    static const char* presets[5] = {
      "OK",
      "I'll call you later",
      "In a meeting",
      "Can't talk right now",
      "On my way!"
    };
    if (idx >= 0 && idx < 5) return presets[idx];
    return "OK";
  }
  void cycleQuickReplyPreset() { quickReplySelectedIdx = (quickReplySelectedIdx + 1) % 5; }
  int getQuickReplySelectedIdx() const { return quickReplySelectedIdx; }

  // Silent Mode Overlay
  void triggerSilentOverlay(bool isSilent) {
    silentOverlayStartTime = millis();
    showSilentOverlay = true;
    silentOverlayState = isSilent;
  }

  // Word-wrapped text helper
  void drawWordWrappedText(const String& text, int x, int y, int maxWidth, int maxLines, int lineHeight, uint16_t color, uint8_t textSize) {
    display.setTextSize(textSize);
    display.setTextColor(color);
    int charWidth = 6 * textSize;
    int maxCharsPerLine = maxWidth / charWidth;
    if (maxCharsPerLine <= 0) return;

    int line = 0;
    int startIdx = 0;
    int len = text.length();

    while (startIdx < len && line < maxLines) {
      while (startIdx < len && text.charAt(startIdx) == ' ') startIdx++;
      if (startIdx >= len) break;

      int remaining = len - startIdx;
      if (remaining <= maxCharsPerLine) {
        display.setCursor(x, y + line * lineHeight);
        display.print(text.substring(startIdx));
        break;
      }

      int breakIdx = startIdx + maxCharsPerLine;
      int spaceIdx = -1;
      for (int i = breakIdx; i > startIdx; i--) {
        if (text.charAt(i) == ' ' || text.charAt(i) == '\n') {
          spaceIdx = i;
          break;
        }
      }

      if (spaceIdx > startIdx) {
        display.setCursor(x, y + line * lineHeight);
        display.print(text.substring(startIdx, spaceIdx));
        startIdx = spaceIdx + 1;
      } else {
        display.setCursor(x, y + line * lineHeight);
        display.print(text.substring(startIdx, breakIdx));
        startIdx = breakIdx;
      }
      line++;
    }
  }

  LunaFace(Adafruit_ST7789& tftDisp, LunaCanvas16& disp) 
    : tft(tftDisp), display(disp), currentExpr(EXPR_ROBOT_EYE), targetExpr(EXPR_ROBOT_EYE), defaultExpr(EXPR_ROBOT_EYE), stateLabel("Luna Idle"), frameDelayMs(105), expressionChanged(true) {
    currentFrame = 0;
    currentGifIndex = 0;
    lastFrameTime = 0;
    gifFinished = false;

    // Video AI animation controller init
    robotEyeAnim.reset();
    robotEyeAnim.play();

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

    callRingingActive = false;
    callerName = "";
    callMuted = false;
    callRingStartTime = 0;

    quickReplyActive = false;
    quickReplySelectedIdx = 0;

    showSilentOverlay = false;
    silentOverlayStartTime = 0;
    silentOverlayState = false;

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
    stateLabel = String(getSpriteAiAnimationName(robotEyeAnim.getAnimationIndex()));
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
      if (gifIdx >= 0 && gifIdx < TOTAL_ANIMATIONS) {
        setGifIndex(gifIdx);
        expr = (Expression)gifIdx;
      }
    } else if ((int)expr >= 0 && (int)expr < TOTAL_ANIMATIONS) {
      setGifIndex((int)expr);
    }
    if (currentExpr == expr && expr != EXPR_ALL_GIF) return;
    targetExpr = expr;
    currentExpr = expr;
    currentFrame = 0;
    lastFrameTime = millis();
    gifFinished = false;
    expressionChanged = true;

    robotEyeAnim.reset();
    robotEyeAnim.play();
    
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
    bool isWA = title.startsWith("WA:") || title.indexOf("WhatsApp") >= 0;
    popupDuration = isWA ? 8000 : 5000;
    
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

  String getPopupTitle() const {
    return popupTitle;
  }

  String getPopupBody() const {
    return popupBody;
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
    if (idx >= 0 && idx < TOTAL_ANIMATIONS) {
      currentGifIndex = idx;
      robotEyeAnim.setAnimationIndex(idx);
      robotEyeAnim.reset();
      robotEyeAnim.play();
      expressionChanged = true;
      updateLabelFromState();
    }
  }

  int getGifIndex() const {
    return robotEyeAnim.getAnimationIndex();
  }

  int getGifCount() const {
    return TOTAL_ANIMATIONS;
  }

  bool isGifFinished() {
    return robotEyeAnim.isCycleCompleted() || gifFinished;
  }

  void clearGifFinished() {
    robotEyeAnim.clearCycleCompleted();
    gifFinished = false;
  }

  bool update() {
    unsigned long now = millis();
    bool changed = false;

    if (expressionChanged) {
      expressionChanged = false;
      changed = true;
    }

    if (robotEyeAnim.update()) {
      changed = true;
    }

    // Scroll text logic
    if (currentExpr == EXPR_TEXT) {
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
      case SCREEN_POMODORO:      nm = "FOCUS";   break;
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
    bool isWA = popupTitle.startsWith("WA:") || popupTitle.indexOf("WhatsApp") >= 0;
    String displayTitle = popupTitle;
    if (displayTitle.startsWith("WA:")) {
      displayTitle = displayTitle.substring(3);
      displayTitle.trim();
    }

    const uint16_t accentCol = isWA ? 0x07E0 : ((robotVariant == "mr_luna") ? 0x07FF : 0xF8B8);
    const uint16_t cardBg    = 0x0841; // Dark slate
    const uint16_t textCol   = 0xFFFF; // White
    const uint16_t subCol    = 0x8410; // Silver
    const uint16_t borderCol = accentCol;

    // Outer glow borders
    display.drawRoundRect(2, 2, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 4, 12, borderCol);
    display.drawRoundRect(3, 3, SCREEN_WIDTH - 6, SCREEN_HEIGHT - 6, 11, 0x4A49);
    display.fillRoundRect(5, 5, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10, 10, cardBg);

    // Header strip
    display.fillRoundRect(5, 5, SCREEN_WIDTH - 10, 32, 10, isWA ? 0x0280 : 0x18C3);
    
    // Bell / Chat icon
    int bx = 20, by = 20;
    display.fillCircle(bx, by - 3, 5, accentCol);
    display.fillRect(bx - 6, by + 2, 13, 3, accentCol);
    display.fillCircle(bx, by + 7, 2, accentCol);

    // App badge
    display.fillRoundRect(36, 11, isWA ? 96 : 84, 18, 5, isWA ? 0x0BE4 : 0x2945);
    display.setTextSize(1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(42, 16);
    display.print(isWA ? "WHATSAPP" : "ALERT");

    // Time badge
    display.setTextColor(subCol);
    display.setCursor(SCREEN_WIDTH - 38, 16);
    display.print("NOW");

    display.drawFastHLine(8, 38, SCREEN_WIDTH - 16, 0x2124);

    // Sender / Title
    display.setTextSize(2);
    display.setTextColor(accentCol);
    display.setCursor(14, 46);
    String title = displayTitle;
    if (title.length() > 14) title = title.substring(0, 12) + "..";
    display.print(title);

    display.drawFastHLine(8, 68, SCREEN_WIDTH - 16, 0x2124);

    // Message body (word wrapped size 2)
    drawWordWrappedText(popupBody, 14, 76, SCREEN_WIDTH - 28, 4, 20, textCol, 2);

    // Progress bar (counts down over popup duration)
    unsigned long elapsed = millis() - popupStartTime;
    int barW = SCREEN_WIDTH - 28;
    int barFill = barW - (int)((float)elapsed / (float)popupDuration * barW);
    if (barFill < 0) barFill = 0;
    display.drawRoundRect(14, SCREEN_HEIGHT - 38, barW, 6, 2, 0x2124);
    display.fillRoundRect(15, SCREEN_HEIGHT - 37, barFill, 4, 1, accentCol);

    // Action buttons hints
    display.setTextSize(1);
    if (isWA) {
      display.setTextColor(0x07E0);
      const char* h = "BTN1: QUICK REPLY // BTN2: DISMISS";
      int hw = strlen(h) * 6;
      display.setCursor((SCREEN_WIDTH - hw) / 2, SCREEN_HEIGHT - 22);
      display.print(h);
    } else {
      display.setTextColor(subCol);
      const char* h = "CLICK BUTTON TO DISMISS";
      int hw = strlen(h) * 6;
      display.setCursor((SCREEN_WIDTH - hw) / 2, SCREEN_HEIGHT - 22);
      display.print(h);
    }
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
      // Rotating radar sweep line empty state
      int cx = SCREEN_WIDTH / 2;
      int cy = 96;
      int rad = 32;

      display.drawCircle(cx, cy, rad, themeBorder);
      display.drawCircle(cx, cy, rad - 10, themeBorder);
      display.drawCircle(cx, cy, 5, themeBorder);
      display.drawFastHLine(cx - rad - 4, cy, (rad + 4) * 2, themeBorder);
      display.drawFastVLine(cx, cy - rad - 4, (rad + 4) * 2, themeBorder);

      float sweepAngle = (millis() % 2400) * (2.0f * M_PI / 2400.0f);
      int sx = cx + (int)(cos(sweepAngle) * (rad - 2));
      int sy = cy + (int)(sin(sweepAngle) * (rad - 2));
      display.drawLine(cx, cy, sx, sy, themeAccent);

      display.setTextSize(2);
      display.setTextColor(themeText);
      const char* h1 = "STREAM CLEAR";
      int w1 = strlen(h1) * 12;
      display.setCursor((SCREEN_WIDTH - w1) / 2, 142);
      display.print(h1);

      display.setTextSize(1);
      display.setTextColor(themeSubText);
      const char* sub = "All telemetry notifications processed";
      int sW = strlen(sub) * 6;
      display.setCursor((SCREEN_WIDTH - sW) / 2, 166);
      display.print(sub);

      const char* nav = "BTN1: DISMISS // BTN2: NEXT";
      int nw = strlen(nav) * 6;
      display.setCursor((SCREEN_WIDTH - nw) / 2, SCREEN_HEIGHT - 18);
      display.print(nav);
      return;
    }
    
    if (notificationSelected) {
      NotificationItem& notif = notificationHistory[currentNotifViewIdx];
      bool isWA = notif.title.startsWith("WA:") || notif.title.indexOf("WhatsApp") >= 0;
      String cleanTitle = notif.title;
      if (cleanTitle.startsWith("WA:")) cleanTitle = cleanTitle.substring(3);
      cleanTitle.trim();

      uint16_t badgeCol = isWA ? 0x07E0 : themeAccent;
      display.fillRoundRect(14, 30, isWA ? 96 : 80, 18, 5, isWA ? 0x0280 : 0x18C3);
      display.drawRoundRect(14, 30, isWA ? 96 : 80, 18, 5, badgeCol);
      display.setTextSize(1);
      display.setTextColor(badgeCol);
      display.setCursor(20, 35);
      display.print(isWA ? "WHATSAPP" : "MESSAGE");

      display.setTextColor(themeSubText);
      display.setCursor(SCREEN_WIDTH - 48, 35);
      display.print(notif.timeStr);

      // Sender Title (size 2)
      display.setTextSize(2);
      display.setTextColor(themeText);
      display.setCursor(14, 54);
      String shortTitle = cleanTitle;
      if (shortTitle.length() > 14) shortTitle = shortTitle.substring(0, 12) + "..";
      display.print(shortTitle);

      display.drawFastHLine(14, 76, SCREEN_WIDTH - 28, themeAccent);

      // Body Card
      display.fillRoundRect(10, 82, SCREEN_WIDTH - 20, 114, 6, themeCardBg);
      display.drawRoundRect(10, 82, SCREEN_WIDTH - 20, 114, 6, themeBorder);
      display.fillRect(10, 88, 3, 102, isWA ? 0x07E0 : themeAccent);

      // Body text in size 2 with word wrapping
      drawWordWrappedText(notif.body, 18, 90, SCREEN_WIDTH - 36, 5, 20, themeText, 2);

      // Bottom control hints
      display.setTextSize(1);
      if (isWA) {
        display.setTextColor(0x07E0);
        const char* h = "BTN1: > QUICK REPLY // BTN2: BACK";
        int hw = strlen(h) * 6;
        display.setCursor((SCREEN_WIDTH - hw) / 2, SCREEN_HEIGHT - 20);
        display.print(h);
      } else {
        display.setTextColor(themeAccent);
        char h[40];
        snprintf(h, sizeof(h), "[%d/%d] BTN1: BACK // BTN2: NEXT", currentNotifViewIdx + 1, notificationCount);
        int hw = strlen(h) * 6;
        display.setCursor((SCREEN_WIDTH - hw) / 2, SCREEN_HEIGHT - 20);
        display.print(h);
      }
    } else {
      display.setTextColor(themeText);
      display.setTextSize(2);
      display.setCursor(14, 28);
      display.print("NOTIFICATIONS");
      
      display.drawFastHLine(12, 48, SCREEN_WIDTH - 24, themeBorder);
      
      for (int i = 0; i < notificationCount && i < 4; i++) {
        int y = 54 + i * 40;
        NotificationItem& notif = notificationHistory[i];
        bool isSel = (notificationsActive && i == currentNotifViewIdx);
        bool isWA = notif.title.startsWith("WA:") || notif.title.indexOf("WhatsApp") >= 0;
        
        if (isSel) {
          display.fillRoundRect(8, y, SCREEN_WIDTH - 16, 36, 4, themeBorder);
          display.drawRoundRect(8, y, SCREEN_WIDTH - 16, 36, 4, isWA ? 0x07E0 : themeAccent);
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

  void drawRobotFaceScreen() {
    robotEyeAnim.drawDirect(tft);
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

  // ── Pomodoro Focus Timer Screen (240x240) ──────────────────────────────────
  void drawPomodoroScreen(int remainingSec, int totalSec, int pomoState, int pomoMode, int completedSessions) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    // Clear display area below status bar
    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    // ── Header context ──────────────────────────────────────────────────────
    const char* modeLabel = "F O C U S";
    if (pomoMode == 1) modeLabel = "S H O R T  B R E A K";
    else if (pomoMode == 2) modeLabel = "L O N G  B R E A K";

    display.setTextSize(1);
    display.setTextColor(themeAccent);
    int mlW = strlen(modeLabel) * 6;
    display.setCursor((SCREEN_WIDTH - mlW) / 2, 26);
    display.print(modeLabel);

    // ── Mode selector micro capsules (25M / 5M / 15M) ───────────────────────
    const char* mNames[] = { "25M", "5M", "15M" };
    int mX = 36;
    for (int m = 0; m < 3; m++) {
      bool isSel = (pomoMode == m);
      uint16_t cBorder = isSel ? themeAccent : themeBorder;
      uint16_t cText   = isSel ? themeText   : themeSubText;
      display.drawRoundRect(mX, 38, 48, 15, 3, cBorder);
      if (isSel) {
        display.fillRect(mX + 1, 39, 46, 13, 0x0842);
      }
      display.setTextColor(cText);
      display.setTextSize(1);
      int w = strlen(mNames[m]) * 6;
      display.setCursor(mX + (48 - w) / 2, 42);
      display.print(mNames[m]);
      mX += 58;
    }

    // ── Dominant Hero Countdown Digits ──────────────────────────────────────
    int mins = remainingSec / 60;
    int secs = remainingSec % 60;
    char timeBuf[8];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", mins, secs);

    display.setTextSize(4);
    uint16_t timeColor = (pomoState == 1) ? themeAccent : ((pomoState == 3) ? 0x07E0 : themeText);
    display.setTextColor(timeColor);
    int timeW = 5 * 24;
    display.setCursor((SCREEN_WIDTH - timeW) / 2, 58);
    display.print(timeBuf);

    // ── Geometric Concentric Calibrated Reticle Arc ─────────────────────────
    int cx = SCREEN_WIDTH / 2;
    int cy = 118;
    int r  = 24;

    // Static reticle track
    display.drawCircle(cx, cy, r, themeBorder);
    display.drawCircle(cx, cy, 7, themeBorder);

    // Dynamic progress ticks
    float progressPct = 0.0f;
    if (totalSec > 0) {
      progressPct = (float)(totalSec - remainingSec) / (float)totalSec;
    }
    progressPct = constrain(progressPct, 0.0f, 1.0f);

    int arcDots = (int)(progressPct * 32.0f);
    for (int s = 0; s < arcDots; s++) {
      float angle = s * (2.0f * M_PI / 32.0f) - (M_PI / 2.0f);
      int px = cx + (int)(cos(angle) * r);
      int py = cy + (int)(sin(angle) * r);
      display.fillCircle(px, py, 2, themeAccent);
    }

    // State readout inside the reticle
    const char* stateLabel = "READY";
    if (pomoState == 1)      stateLabel = "ACTIVE";
    else if (pomoState == 2) stateLabel = "PAUSED";
    else if (pomoState == 3) stateLabel = "DONE";

    display.setTextSize(1);
    display.setTextColor(themeText);
    int slW = strlen(stateLabel) * 6;
    display.setCursor(cx - slW / 2, cy - 3);
    display.print(stateLabel);

    // ── Precision Divider ───────────────────────────────────────────────────
    display.drawFastHLine(30, 150, SCREEN_WIDTH - 60, themeBorder);

    // ── Tactile Action Trigger Pill Button ─────────────────────────────────
    int btnW = 144;
    int btnH = 28;
    int btnX = (SCREEN_WIDTH - btnW) / 2;
    int btnY = 158;

    uint16_t btnAccent = (pomoState == 1) ? 0xFDE0 : ((pomoState == 3) ? 0x07E0 : themeAccent);
    display.drawRoundRect(btnX, btnY, btnW, btnH, 6, btnAccent);
    display.fillRoundRect(btnX + 2, btnY + 2, btnW - 4, btnH - 4, 4, (pomoState == 1) ? 0x0842 : 0x0000);

    display.setTextSize(2);
    display.setTextColor(themeText);

    if (pomoState == 0) { // Ready to Start
      int tx = btnX + 32, ty = btnY + 14;
      display.fillTriangle(tx, ty - 5, tx, ty + 5, tx + 8, ty, themeAccent);
      display.setCursor(btnX + 48, btnY + 7);
      display.print("START");
    } else if (pomoState == 1) { // Active -> Show Pause
      int px = btnX + 32, py = btnY + 8;
      display.fillRect(px, py, 3, 11, 0xFDE0);
      display.fillRect(px + 6, py, 3, 11, 0xFDE0);
      display.setCursor(btnX + 48, btnY + 7);
      display.print("PAUSE");
    } else if (pomoState == 2) { // Paused -> Show Resume
      int tx = btnX + 26, ty = btnY + 14;
      display.fillTriangle(tx, ty - 5, tx, ty + 5, tx + 8, ty, themeAccent);
      display.setCursor(btnX + 42, btnY + 7);
      display.print("RESUME");
    } else { // Completed -> Show Reset
      int rx = btnX + 30, ry = btnY + 14;
      display.drawCircle(rx, ry, 5, 0x07E0);
      display.fillRect(rx - 1, ry - 6, 3, 3, 0x0000);
      display.fillTriangle(rx, ry - 6, rx + 4, ry - 4, rx, ry - 2, 0x07E0);
      display.setCursor(btnX + 44, btnY + 7);
      display.print("RESET");
    }

    // ── Secondary Telemetry & Interaction Hints ─────────────────────────────
    char sessBuf[32];
    snprintf(sessBuf, sizeof(sessBuf), "CYCLES COMPLETED: %02d", completedSessions);
    display.setTextSize(1);
    display.setTextColor(themeSubText);
    int sessW = strlen(sessBuf) * 6;
    display.setCursor((SCREEN_WIDTH - sessW) / 2, 196);
    display.print(sessBuf);

    const char* tip = "BTN1: START/PAUSE // BTN2: NEXT";
    int tipW = strlen(tip) * 6;
    display.setCursor((SCREEN_WIDTH - tipW) / 2, 214);
    display.print(tip);
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
        case SCREEN_POMODORO: {
          extern int pomoRemainingSec, pomoTotalSec, pomoState, pomoMode, pomoCompletedSessions;
          drawPomodoroScreen(pomoRemainingSec, pomoTotalSec, pomoState, pomoMode, pomoCompletedSessions);
          break;
        }
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

    // ── Silent Mode Transient Overlay ──────────────────────────────────────
    if (showSilentOverlay) {
      if (millis() - silentOverlayStartTime > 2000) {
        showSilentOverlay = false;
      } else {
        int bx = 50;
        int by = 6;
        int bw = 140;
        int bh = 28;
        
        display.fillRoundRect(bx, by, bw, bh, 8, 0x0000);
        display.drawRoundRect(bx, by, bw, bh, 8, silentOverlayState ? 0xF800 : 0x07E0);
        
        display.setTextSize(2);
        if (silentOverlayState) {
          display.setTextColor(0xF800);
          display.setCursor(bx + 12, by + 6);
          display.print("SILENT ON");
          
          int sx = bx + 115, sy = by + 14;
          display.fillRect(sx - 5, sy - 3, 4, 6, 0xF800);
          display.fillTriangle(sx - 1, sy - 6, sx - 1, sy + 6, sx + 3, sy, 0xF800);
          display.drawLine(sx + 6, sy - 3, sx + 10, sy + 1, 0xF800);
          display.drawLine(sx + 10, sy - 3, sx + 6, sy + 1, 0xF800);
        } else {
          display.setTextColor(0x07E0);
          display.setCursor(bx + 12, by + 6);
          display.print("SOUND ON");
          
          int sx = bx + 112, sy = by + 14;
          display.fillRect(sx - 5, sy - 3, 4, 6, 0x07E0);
          display.fillTriangle(sx - 1, sy - 6, sx - 1, sy + 6, sx + 3, sy, 0x07E0);
          display.drawPixel(sx + 6, sy - 2, 0x07E0);
          display.drawPixel(sx + 7, sy - 1, 0x07E0);
          display.drawPixel(sx + 7, sy + 1, 0x07E0);
          display.drawPixel(sx + 6, sy + 2, 0x07E0);
        }
      }
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

    // ── Incoming Call Ringing Overlay ──
    if (callRingingActive) {
      int ox = 8;
      int oy = 10;
      int ow = SCREEN_WIDTH - 16;
      int oh = SCREEN_HEIGHT - 20;

      bool pulse = ((millis() / 400) % 2 == 0);
      uint16_t borderCol = pulse ? 0x07E0 : 0x03E0;
      if (callMuted) borderCol = 0xFD20;

      display.fillRoundRect(ox, oy, ow, oh, 12, 0x0841);
      display.drawRoundRect(ox, oy, ow, oh, 12, borderCol);
      display.drawRoundRect(ox + 1, oy + 1, ow - 2, oh - 2, 11, borderCol);

      // Top Status Pill
      uint16_t pillBg = callMuted ? 0x8400 : (pulse ? 0x0520 : 0x03A0);
      display.fillRoundRect(ox + (ow - 140) / 2, oy + 8, 140, 20, 6, pillBg);
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(ox + (ow - 140) / 2 + 14, oy + 14);
      display.print(callMuted ? "CALL MUTED [SILENT]" : "INCOMING CALL...");

      // Animated Phone Icon
      int iconCenterX = ox + ow / 2;
      int iconCenterY = oy + 54;
      int iconR = 20;
      display.fillCircle(iconCenterX, iconCenterY, iconR, callMuted ? 0x4A69 : (pulse ? 0x07E0 : 0x05E0));
      display.drawCircle(iconCenterX, iconCenterY, iconR, TFT_WHITE);
      display.fillRoundRect(iconCenterX - 5, iconCenterY - 10, 10, 20, 3, TFT_WHITE);
      display.fillRect(iconCenterX - 3, iconCenterY - 5, 6, 10, callMuted ? 0x4A69 : (pulse ? 0x07E0 : 0x05E0));

      // Caller Name
      display.setTextSize(2);
      display.setTextColor(TFT_WHITE);
      String dName = callerName;
      if (dName.length() == 0) dName = "Unknown Caller";
      if (dName.length() > 14) dName = dName.substring(0, 12) + "..";
      int nameX = ox + (ow - (dName.length() * 12)) / 2;
      display.setCursor(max(ox + 8, nameX), oy + 84);
      display.print(dName);

      // Subtitle
      display.setTextSize(1);
      display.setTextColor(0x9CD3);
      const char* subTxt = "Phone / WhatsApp Call";
      int subX = ox + (ow - strlen(subTxt) * 6) / 2;
      display.setCursor(subX, oy + 106);
      display.print(subTxt);

      // Action Buttons (Optimized for 2 physical buttons BTN1 & BTN2)
      int btnW = 98;
      int btnH = 46;
      int btnY = oy + oh - 54;

      // Left Button: BTN1: MUTE
      int muteX = ox + 8;
      uint16_t muteBg = callMuted ? 0x3186 : 0x7BC0;
      display.fillRoundRect(muteX, btnY, btnW, btnH, 8, muteBg);
      display.drawRoundRect(muteX, btnY, btnW, btnH, 8, callMuted ? 0x6B4D : 0xFD20);
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(muteX + 16, btnY + 8);
      display.print("BTN1:");
      display.setTextSize(2);
      display.setCursor(muteX + 16, btnY + 22);
      display.print(callMuted ? "MUTED" : "MUTE");

      // Right Button: BTN2: CUT
      int cutX = ox + ow - 8 - btnW;
      display.fillRoundRect(cutX, btnY, btnW, btnH, 8, 0xC800);
      display.drawRoundRect(cutX, btnY, btnW, btnH, 8, 0xF980);
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(cutX + 16, btnY + 8);
      display.print("BTN2:");
      display.setTextSize(2);
      display.setCursor(cutX + 16, btnY + 22);
      display.print("CUT");
    }

    // ── WhatsApp Quick Reply Sheet Overlay ──
    if (quickReplyActive) {
      int ox = 8;
      int oy = 8;
      int ow = SCREEN_WIDTH - 16;
      int oh = SCREEN_HEIGHT - 16;

      display.fillRoundRect(ox, oy, ow, oh, 10, 0x0841);
      display.drawRoundRect(ox, oy, ow, oh, 10, 0x07E0);
      display.drawRoundRect(ox + 1, oy + 1, ow - 2, oh - 2, 9, 0x07E0);

      // Header Badge
      display.fillRoundRect(ox + 8, oy + 6, ow - 16, 20, 5, 0x0BE4);
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(ox + 14, oy + 12);
      display.print("WHATSAPP QUICK REPLY");

      // Subtitle / Button hint
      display.setTextSize(1);
      display.setTextColor(0x9CD3);
      display.setCursor(ox + 10, oy + 30);
      display.print("BTN1: Next   BTN2: Send");

      // 5 Preset Buttons
      int startY = oy + 44;
      int itemH = 26;
      int itemGap = 4;
      for (int i = 0; i < 5; i++) {
        int itemY = startY + i * (itemH + itemGap);
        bool isSel = (i == quickReplySelectedIdx);
        display.fillRoundRect(ox + 8, itemY, ow - 16, itemH, 5, isSel ? 0x0BE4 : 0x18C3);
        display.drawRoundRect(ox + 8, itemY, ow - 16, itemH, 5, isSel ? TFT_WHITE : 0x3A68);

        display.setTextSize(1);
        display.setTextColor(isSel ? TFT_WHITE : 0xCE79);
        display.setCursor(ox + 14, itemY + 8);
        display.print(getQuickReplyPreset(i));

        display.setTextColor(isSel ? TFT_YELLOW : 0x07E0);
        display.setCursor(ox + ow - 22, itemY + 8);
        display.print(isSel ? ">" : " ");
      }

      // Bottom Cancel hint
      display.setTextSize(1);
      display.setTextColor(0x7BEF);
      const char* cTxt = "HOLD BTN: CANCEL";
      int cW = strlen(cTxt) * 6;
      display.setCursor(ox + (ow - cW) / 2, oy + oh - 14);
      display.print(cTxt);
    }
    
    // Draw directly at (0, 0) for 1.3" display (no Y offset!)
    if (currentScreen != SCREEN_FACE) {
      display.flush(tft);
    }
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
