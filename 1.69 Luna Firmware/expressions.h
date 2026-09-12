#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "config.h"
#include "image_logo.h"
#include "qr_card.h"
#include "wallpaper_image.h"
#include "imu.h"
#include "robot_eye_animation.h"
#include "image_transfer.h"

extern LunaQR qrCard;
extern LunaIMU imu;
extern bool calibrateRequest;
extern LunaImageTransfer imgTransfer;

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
extern bool isAsleep;
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

// Smooth inertial scroll state (defined in main sketch)
extern float settingsScrollPx;
extern float gamesScrollPx;

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


class LunaFace {
private:
  Adafruit_ST7789& tft;
  GFXcanvas16& display;
  Expression currentExpr;
  Expression targetExpr;
  Expression defaultExpr; // Custom default expression for Idle state

  // Animation frame control
  int currentFrame;
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

  // Map navigation state
  String mapDirection;
  String mapDistance;
  String mapRoad;
  String mapTotalTime;
  String mapTotalDist;
  String mapEta;
  String mapDescription;
  uint16_t* liveMapBuffer;
  bool hasLiveMap;
  unsigned long lastLiveMapMs;

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

  struct CalendarEventItem {
    String id;
    String type;
    String dateStr;
    String timeStr;
    String title;
    bool active;
  };
  static const int MAX_FACE_CAL_EVENTS = 20;
  CalendarEventItem calendarEvents[MAX_FACE_CAL_EVENTS];
  int calendarEventCount;
  int currentCalViewIdx;
  bool showCalendarGrid;

  // Active alarm/meeting/reminder ringing overlay state
  bool alarmRingingActive;
  String ringingType;
  String ringingTitle;
  String ringingTime;

  // Temporary popup notification state
  bool popupActive;
  unsigned long popupStartTime;
  unsigned long popupDuration;
  String popupTitle;
  String popupBody;

  // Silent Mode Transient Overlay state
  unsigned long silentOverlayStartTime;
  bool showSilentOverlay;
  bool silentOverlayState;

  // Sprite AI Robot Eye Animation Controller
  RobotEyeAnimation robotEyeAnim;

public:
  String headerText;
  bool timeSynced = false; // true after first TIME: sync from companion app
  RobotEyeAnimation& getRobotEyeAnim() { return robotEyeAnim; }
  LunaFace(Adafruit_ST7789& tftDisp, GFXcanvas16& disp) 
    : tft(tftDisp), display(disp), currentExpr(EXPR_ROBOT_EYE), targetExpr(EXPR_ROBOT_EYE), defaultExpr(EXPR_ROBOT_EYE), stateLabel("ROBOT_EYE"), frameDelayMs(100), expressionChanged(true) {
    currentFrame = 0;
    lastFrameTime = 0;
    gifFinished = false;

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
    mapDescription = "";
    liveMapBuffer = nullptr;
    hasLiveMap = false;
    lastLiveMapMs = 0;

    bleConnectedStatus = false;
    wifiConnectedStatus = false;

    // Smartwatch state initialization
    notificationCount = 0;
    currentNotifViewIdx = 0;
    calendarEventCount = 0;
    currentCalViewIdx = 0;
    showCalendarGrid = true;

    popupActive = false;
    popupStartTime = 0;
    popupDuration = 5000;
    popupTitle = "";
    popupBody = "";
    headerText = "";

    silentOverlayStartTime = 0;
    showSilentOverlay = false;
    silentOverlayState = false;

    alarmRingingActive = false;
    ringingType = "";
    ringingTitle = "";
    ringingTime = "";

    for (int i = 0; i < 5; i++) {
      notificationHistory[i].active = false;
    }
    for (int i = 0; i < MAX_FACE_CAL_EVENTS; i++) {
      calendarEvents[i].active = false;
      calendarEvents[i].id = "";
      calendarEvents[i].dateStr = "";
      calendarEvents[i].timeStr = "";
      calendarEvents[i].title = "";
    }
  }

  uint16_t getExpressionColor(Expression expr) {
    return TFT_CYAN;
  }

  void setConnectivityStatus(bool bleConnected, bool wifiConnected) {
    bleConnectedStatus = bleConnected;
    wifiConnectedStatus = wifiConnected;
  }

  void updateLabelFromState() {
    stateLabel = "ROBOT_EYE";
  }

  void setDefaultExpression(Expression expr) {
    defaultExpr = expr;
    updateLabelFromState();
  }

  void setFrameDelay(int ms) {
    frameDelayMs = ms;
  }

  void setExpression(Expression expr) {
    if (currentExpr == expr) return;
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
    // Shift elements
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
    currentNotifViewIdx = 0; // Reset index to show the latest
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

  // ------------------ Smartwatch Calendar Events ------------------
  void clearCalendarEvents() {
    calendarEventCount = 0;
    currentCalViewIdx = 0;
    for (int i = 0; i < MAX_FACE_CAL_EVENTS; i++) {
      calendarEvents[i].active = false;
      calendarEvents[i].id = "";
      calendarEvents[i].type = "";
      calendarEvents[i].dateStr = "";
      calendarEvents[i].timeStr = "";
      calendarEvents[i].title = "";
    }
  }

  void removeCalendarEvent(String id) {
    int foundIdx = -1;
    for (int i = 0; i < calendarEventCount; i++) {
      if (calendarEvents[i].id == id || (calendarEvents[i].id.length() == 0 && calendarEvents[i].title == id)) {
        foundIdx = i;
        break;
      }
    }
    if (foundIdx != -1) {
      for (int i = foundIdx; i < calendarEventCount - 1; i++) {
        calendarEvents[i] = calendarEvents[i + 1];
      }
      calendarEvents[calendarEventCount - 1].active = false;
      calendarEvents[calendarEventCount - 1].id = "";
      calendarEvents[calendarEventCount - 1].title = "";
      calendarEventCount--;
      if (currentCalViewIdx >= calendarEventCount && calendarEventCount > 0) {
        currentCalViewIdx = calendarEventCount - 1;
      }
    }
  }

  void addCalendarEvent(String id, String type, String dateStr, String timeStr, String title) {
    // Check if event already exists with this ID — update in place!
    for (int i = 0; i < calendarEventCount; i++) {
      if (calendarEvents[i].id.length() > 0 && calendarEvents[i].id == id) {
        calendarEvents[i].type = type;
        calendarEvents[i].dateStr = dateStr;
        calendarEvents[i].timeStr = timeStr;
        calendarEvents[i].title = title;
        calendarEvents[i].active = true;
        return;
      }
    }
    for (int i = MAX_FACE_CAL_EVENTS - 1; i > 0; i--) {
      calendarEvents[i] = calendarEvents[i - 1];
    }
    calendarEvents[0].id = id;
    calendarEvents[0].type = type;
    calendarEvents[0].dateStr = dateStr;
    calendarEvents[0].timeStr = timeStr;
    calendarEvents[0].title = title;
    calendarEvents[0].active = true;
    
    if (calendarEventCount < MAX_FACE_CAL_EVENTS) {
      calendarEventCount++;
    }
    currentCalViewIdx = 0;
  }

  void addCalendarEvent(String type, String timeStr, String title) {
    addCalendarEvent(String(millis()), type, "*", timeStr, title);
  }

  void cycleCalendarView() {
    if (!showCalendarGrid && calendarEventCount > 0) {
      currentCalViewIdx = (currentCalViewIdx + 1) % calendarEventCount;
    }
  }

  void toggleCalendarMode() {
    showCalendarGrid = !showCalendarGrid;
  }

  void setAlarmRinging(bool ringing, String type = "", String title = "", String time = "") {
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
    popupDuration = 5000; // 5 seconds
    
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

  void triggerSilentOverlay(bool isSilent) {
    silentOverlayStartTime = millis();
    showSilentOverlay = true;
    silentOverlayState = isSilent;
  }

  // ------------------ Map Navigation State ------------------
  void initLiveMapBuffer() {
    if (liveMapBuffer == nullptr) {
      if (psramFound()) {
        liveMapBuffer = (uint16_t*)ps_malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));
      }
      if (liveMapBuffer == nullptr) {
        liveMapBuffer = (uint16_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));
      }
      if (liveMapBuffer != nullptr) {
        memset(liveMapBuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));
      }
    }
  }

  void updateLiveMapLine(int y, const uint16_t* linePixels, int count) {
    initLiveMapBuffer();
    if (liveMapBuffer != nullptr && y >= 0 && y < SCREEN_HEIGHT) {
      int maxPx = (count > SCREEN_WIDTH) ? SCREEN_WIDTH : count;
      memcpy(&liveMapBuffer[y * SCREEN_WIDTH], linePixels, maxPx * sizeof(uint16_t));
      hasLiveMap = true;
      lastLiveMapMs = millis();
    }
  }

  void updateLiveMapChunk(int offsetPx, const uint16_t* pixels, int count) {
    initLiveMapBuffer();
    if (liveMapBuffer != nullptr) {
      int totalPx = SCREEN_WIDTH * SCREEN_HEIGHT;
      if (offsetPx >= 0 && offsetPx < totalPx) {
        int maxPx = (offsetPx + count > totalPx) ? (totalPx - offsetPx) : count;
        memcpy(&liveMapBuffer[offsetPx], pixels, maxPx * sizeof(uint16_t));
        hasLiveMap = true;
        lastLiveMapMs = millis();
      }
    }
  }

  void clearLiveMap() {
    hasLiveMap = false;
  }

  void setMapTelemetry(String direction, String turnDist, String road, String totalTime, String totalDist, String eta) {
    mapDirection = direction;
    mapDistance = turnDist;
    mapRoad = road;
    mapTotalTime = totalTime;
    mapTotalDist = totalDist;
    mapEta = eta;
    mapDescription = (totalTime.length() > 0 && totalDist.length() > 0) ? (totalTime + " · " + totalDist) : totalTime;
    setExpression(EXPR_MAP);
  }

  void setMapNavigation(String direction, String distance, String description) {
    mapDirection = direction;
    mapDistance = distance;
    mapDescription = description;
    setExpression(EXPR_MAP);
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
    (void)idx;
    currentFrame = 0;
    lastFrameTime = millis();
    gifFinished = false;
    expressionChanged = true;
    updateLabelFromState();
  }

  int getGifIndex() {
    return 0;
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
      if (now - lastScrollTime > 15) {
        lastScrollTime = now;
        scrollPos -= 2;
        int textLength = notificationText.length() * 18;
        if (scrollPos < -textLength) {
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
    // Monolith Deep Black (Precision Instrument OS - Always Dark Mode)
    t.bg      = 0x0000; // Deep pitch black
    t.text    = 0xFFFF; // Crisp Pure White
    t.accent  = (robotVariant == "mr_luna") ? 0x07FF : 0xF8B8; // Electric Precision Cyan or Luna Pink
    t.cardBg  = 0x0842; // Dark graphite
    t.border  = 0x2945; // Subtle dark border
    t.subText = 0x9CD3; // Muted technical silver
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

    // Status rail with 1px precision baseline rule
    // Height 24px and shifted down & inward to clear curved corner bezels
    display.fillRect(0, 0, SCREEN_WIDTH, 24, themeBg);
    display.drawFastHLine(0, 24, SCREEN_WIDTH, themeBorder);

    // ── Left zone: HH:MM ─────────────
    display.setTextSize(2);
    display.setTextColor(themeText);
    char tBuf[6];
    snprintf(tBuf, sizeof(tBuf), "%02d:%02d", hour, minute);
    // Inset from X=10 to X=18, Y from 4 to 5 to prevent cropping from curved glass and top bezel
    display.setCursor(18, 5);
    display.print(tBuf);

    // ── Right zone: battery and connectivity ─────────
    // Inset bx from SCREEN_WIDTH - 28 (212) to SCREEN_WIDTH - 38 (202) so terminal cap and battery clear right curve
    int bx = SCREEN_WIDTH - 38;

    // Calculate battery percentage
    int batteryPct = getBatteryPercentage(batteryVolts);
    uint16_t batteryColor = 0x07E0; // Instrument Green
    if (batteryPct < 20) {
      batteryColor = 0xF800; // Critical Red
    } else if (batteryPct < 50) {
      batteryColor = 0xFFE0; // Warning Amber
    }

    // Precision 4-segment battery rail (thin vector ticks)
    display.drawRect(bx, 7, 20, 10, themeSubText);
    display.drawFastVLine(bx + 20, 9, 6, themeSubText); // Terminal cap
    int bars = (batteryPct * 4) / 100;
    if (bars > 4) bars = 4;
    for (int b = 0; b < bars; b++) {
      display.fillRect(bx + 2 + b * 4, 9, 3, 6, batteryColor);
    }

    // Battery percentage readout
    display.setTextColor(themeSubText);
    display.setTextSize(1);
    String pctStr = String(batteryPct) + "%";
    int pctStrW = pctStr.length() * 6;
    display.setCursor(bx - 6 - pctStrW, 8);
    display.print(pctStr);

    // BLE vector glyph (precision diamond antenna)
    int bleX = bx - 16 - pctStrW;
    int bleY = 12;
    uint16_t bleColor = bleConnectedStatus ? themeAccent : themeBorder;
    if (bleConnectedStatus) {
      display.drawLine(bleX, bleY - 5, bleX, bleY + 5, bleColor);
      display.drawLine(bleX, bleY - 5, bleX + 3, bleY - 2, bleColor);
      display.drawLine(bleX + 3, bleY - 2, bleX - 2, bleY + 2, bleColor);
      display.drawLine(bleX - 2, bleY - 2, bleX + 3, bleY + 2, bleColor);
      display.drawLine(bleX + 3, bleY + 2, bleX, bleY + 5, bleColor);
    } else {
      display.drawCircle(bleX, bleY, 2, bleColor);
    }

    // WiFi / Signal indicator (3 micro stepped ticks)
    int wifiX = bx - 28 - pctStrW;
    int wifiY = 9;
    uint16_t wifiColor = wifiConnectedStatus ? 0x07E0 : themeBorder;
    display.fillRect(wifiX,     wifiY + 4, 2, 2, wifiColor);
    display.fillRect(wifiX + 3, wifiY + 2, 2, 4, wifiColor);
    display.fillRect(wifiX + 6, wifiY,     2, 6, wifiColor);

    // Silent mode status indicator
    if (silentMode) {
      int silentX = wifiX - 12;
      int silentY = wifiY + 3;
      uint16_t silentColor = 0xF800;
      display.fillRect(silentX, silentY - 2, 2, 4, silentColor);
      display.fillTriangle(silentX + 2, silentY - 4, silentX + 2, silentY + 4, silentX + 4, silentY, silentColor);
      display.drawLine(silentX + 6, silentY - 2, silentX + 8, silentY, silentColor);
    }

    // ── Centre zone: screen context (clean spaced typography + micro cyan pip) ─────
    const char* nm = "LUNA";
    switch (currentScreen) {
      case SCREEN_CLOCK:         nm = "C L O C K";    break;
      case SCREEN_NOTIFICATIONS: nm = "N O T I F S";  break;
      case SCREEN_CALENDAR:      nm = "C A L";        break;
      case SCREEN_GAMES:         nm = "A R C A D E";  break;
      case SCREEN_FACE:          nm = "L U N A";      break;
      case SCREEN_MAPS:          nm = "M A P S";      break;
      case SCREEN_CARD:          nm = "C A R D";      break;
      case SCREEN_SETTINGS:      nm = "S E T U P";    break;
      case SCREEN_LEVEL:         nm = "L E V E L";    break;
      case SCREEN_POMODORO:      nm = "F O C U S";    break;
      default:                   nm = "L U N A";      break;
    }
    int nmLen = strlen(nm) * 6;
    int nmX = (SCREEN_WIDTH - nmLen) / 2;
    if (nmX > 75 && nmX + nmLen < wifiX - 6) {
      // Precision micro dot
      display.fillCircle(nmX - 6, 11, 2, themeAccent);
      display.setTextColor(themeText);
      display.setTextSize(1);
      display.setCursor(nmX, 8);
      display.print(nm);
    }
  }

  // Helper for rendering word-wrapped text cleanly without breaking words in half
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
      // Skip leading spaces on new line
      while (startIdx < len && text.charAt(startIdx) == ' ') startIdx++;
      if (startIdx >= len) break;

      int remaining = len - startIdx;
      if (remaining <= maxCharsPerLine) {
        display.setCursor(x, y + line * lineHeight);
        display.print(text.substring(startIdx));
        break;
      }

      // Look for a break point within maxCharsPerLine
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
        // No space found — forced break
        display.setCursor(x, y + line * lineHeight);
        display.print(text.substring(startIdx, breakIdx));
        startIdx = breakIdx;
      }
      line++;
    }
  }

  void drawPopup() {
    const uint16_t accentCol = (robotVariant == "mr_luna") ? 0x07FF : 0xFD99; // Cyan or soft pink
    const uint16_t cardBg    = 0x0841; // Dark graphite
    const uint16_t textCol   = 0xFFFF; // White
    const uint16_t subCol    = 0x8410; // Silver
    const uint16_t borderCol = accentCol;

    // ── Outer glow borders (double ring) ──────────────────────────────────────
    display.drawRoundRect(2,  2,  SCREEN_WIDTH - 4,  SCREEN_HEIGHT - 4,  14, borderCol);
    display.drawRoundRect(3,  3,  SCREEN_WIDTH - 6,  SCREEN_HEIGHT - 6,  13, 0x4A49);
    // ── Card fill ─────────────────────────────────────────────────────────────
    display.fillRoundRect(5,  5,  SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10, 11, cardBg);

    // ── Header strip ──────────────────────────────────────────────────────────
    display.fillRoundRect(5, 5, SCREEN_WIDTH - 10, 36, 11, 0x18C3); // Darker header area
    // Notification bell icon (3 circles + base)
    int bx = 22, by = 23;
    display.fillCircle(bx, by - 3, 5, accentCol);
    display.fillRect(bx - 6, by + 2, 13, 4, accentCol);
    display.fillCircle(bx, by + 8, 2, accentCol);
    display.fillRect(bx - 6, by + 2, 13, 2, 0x18C3); // Cut top of base
    // App / sender badge
    display.fillRoundRect(38, 14, 90, 17, 8, 0x2945);
    display.setTextSize(1);
    display.setTextColor(accentCol);
    display.setCursor(44, 19);
    display.print("NEW MESSAGE");
    // Time badge (right side)
    display.setTextColor(subCol);
    display.setCursor(SCREEN_WIDTH - 42, 19);
    display.print("NOW");

    display.drawFastHLine(8, 41, SCREEN_WIDTH - 16, 0x2124);

    // ── Sender / App Title ────────────────────────────────────────────────────
    display.setTextSize(2);
    display.setTextColor(accentCol);
    display.setCursor(14, 50);
    String title = popupTitle;
    if (title.length() > 15) title = title.substring(0, 13) + "..";
    display.print(title);

    display.drawFastHLine(8, 72, SCREEN_WIDTH - 16, 0x2124);

    // ── Message body — size 2 with smart word wrapping ─────────────────────
    drawWordWrappedText(popupBody, 14, 82, SCREEN_WIDTH - 28, 6, 22, textCol, 2);

    // ── Dismiss progress bar (counts down over popup duration) ────────────────
    unsigned long elapsed  = millis() - popupStartTime;
    int barW   = SCREEN_WIDTH - 28;
    int barFill = barW - (int)((float)elapsed / (float)popupDuration * barW);
    if (barFill < 0) barFill = 0;
    display.drawRoundRect(14, SCREEN_HEIGHT - 22, barW, 8, 3, 0x2124);
    display.fillRoundRect(15, SCREEN_HEIGHT - 21, barFill, 6, 2, accentCol);

    // ── Tap-to-dismiss hint ───────────────────────────────────────────────────
    display.setTextSize(1);
    display.setTextColor(subCol);
    display.setCursor((SCREEN_WIDTH - 84) / 2, SCREEN_HEIGHT - 11);
    display.print("TAP TO DISMISS");
  }

  void drawNotificationPanel() {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    // Clear display below the status bar
    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    // ── Header ──────────────────────────────────────────────────────────────
    display.setTextSize(2);
    display.setTextColor(themeText);
    display.setCursor(20, 28);
    display.print("NOTIFICATIONS");

    display.drawFastHLine(20, 48, SCREEN_WIDTH - 40, themeBorder);

    if (notificationCount == 0) {
      // Futuristic radar sweep idle state
      int cx = SCREEN_WIDTH / 2;
      int cy = 116;
      int rad = 36;

      display.drawCircle(cx, cy, rad, themeBorder);
      display.drawCircle(cx, cy, rad - 12, themeBorder);
      display.drawCircle(cx, cy, 6, themeBorder);
      display.drawFastHLine(cx - rad - 4, cy, (rad + 4) * 2, themeBorder);
      display.drawFastVLine(cx, cy - rad - 4, (rad + 4) * 2, themeBorder);

      // Rotating radar sweep line
      float sweepAngle = (millis() % 2400) * (2.0f * M_PI / 2400.0f);
      int sx = cx + (int)(cos(sweepAngle) * (rad - 2));
      int sy = cy + (int)(sin(sweepAngle) * (rad - 2));
      display.drawLine(cx, cy, sx, sy, themeAccent);

      display.setTextSize(2);
      display.setTextColor(themeText);
      int tW = 12 * 12;
      display.setCursor((SCREEN_WIDTH - tW) / 2, 172);
      display.print("STREAM CLEAR");

      display.setTextSize(1);
      display.setTextColor(themeSubText);
      const char* sub = "All telemetry notifications processed";
      int sW = strlen(sub) * 6;
      display.setCursor((SCREEN_WIDTH - sW) / 2, 196);
      display.print(sub);

      display.setTextColor(themeBorder);
      display.setCursor(50, 256);
      display.print("LUNA TIMELINE ACTIVE");
      return;
    }

    // ── Detail View vs Timeline Stream ──────────────────────────────────────
    if (notificationSelected) {
      NotificationItem& notif = notificationHistory[currentNotifViewIdx];

      // ── Sender badge + timestamp row ──────────────────────────────────────
      display.fillRoundRect(20, 52, 84, 18, 5, 0x18C3);
      display.drawRoundRect(20, 52, 84, 18, 5, themeAccent);
      display.setTextSize(1);
      display.setTextColor(themeAccent);
      display.setCursor(26, 57);
      display.print("MESSAGE");

      display.setTextColor(themeSubText);
      display.setCursor(SCREEN_WIDTH - 48, 57);
      display.print(notif.timeStr);

      // ── Sender / Title (size 2 = 12px tall, bold) ────────────────────────
      display.setTextSize(2);
      display.setTextColor(themeText);
      display.setCursor(20, 76);
      String shortTitle = notif.title;
      if (shortTitle.length() > 15) shortTitle = shortTitle.substring(0, 13) + "..";
      display.print(shortTitle);

      display.drawFastHLine(14, 100, SCREEN_WIDTH - 28, themeAccent);

      // ── Body Card — enlarged to fit size-2 text ──────────────────────────
      display.fillRoundRect(14, 106, SCREEN_WIDTH - 28, 134, 6, theme.cardBg);
      display.drawRoundRect(14, 106, SCREEN_WIDTH - 28, 134, 6, themeBorder);
      // Accent left bar
      display.fillRect(14, 114, 3, 118, themeAccent);

      // ── Full body text — SIZE 2 (12px) with word wrapping for comfortable reading ───
      drawWordWrappedText(notif.body, 22, 116, SCREEN_WIDTH - 44, 5, 23, themeText, 2);

      // ── Bottom control row ───────────────────────────────────────────────
      char footBuf[24];
      snprintf(footBuf, sizeof(footBuf), "%d / %d", currentNotifViewIdx + 1, notificationCount);
      display.setTextSize(1);
      display.setTextColor(themeAccent);
      display.setCursor(20, 248);
      display.print(footBuf);

      // Dismiss pill button
      display.fillRoundRect(142, 242, 80, 18, 5, theme.cardBg);
      display.drawRoundRect(142, 242, 80, 18, 5, themeAccent);
      display.setCursor(152, 247);
      display.setTextColor(themeAccent);
      display.print("DISMISS >");

      display.setTextColor(themeBorder);
      display.setCursor(20, 266);
      display.print("TAP: BACK  HOLD: CLEAR ALL");

    } else {
      // ── Clean Notification Timeline Stream ────────────────────────────────
      int maxDisplay = min(notificationCount, 3);
      for (int i = 0; i < maxDisplay; i++) {
        int y = 56 + i * 62;
        NotificationItem& notif = notificationHistory[i];
        bool isSel = (notificationsActive && i == currentNotifViewIdx);

        // Card Container
        uint16_t cBg = isSel ? (negativeDisplay ? 0xE73C : 0x10A2) : theme.cardBg;
        display.fillRoundRect(16, y, SCREEN_WIDTH - 32, 54, 6, cBg);
        display.drawRoundRect(16, y, SCREEN_WIDTH - 32, 54, 6, isSel ? themeAccent : themeBorder);
        if (isSel) {
          display.fillRect(16, y + 8, 3, 38, themeAccent);
        }

        // App/Type Badge
        display.setTextSize(1);
        display.setTextColor(isSel ? themeAccent : themeSubText);
        display.setCursor(26, y + 6);
        display.print("[MSG]");

        // Timestamp (right aligned)
        display.setTextColor(themeSubText);
        display.setCursor(160, y + 6);
        display.print(notif.timeStr);

        // Title
        display.setTextSize(2);
        display.setTextColor(isSel ? themeText : 0xCE79);
        display.setCursor(26, y + 18);
        String tStr = notif.title;
        if (tStr.length() > 14) tStr = tStr.substring(0, 13) + "..";
        display.print(tStr);

        // Preview snippet
        display.setTextSize(1);
        display.setTextColor(themeSubText);
        display.setCursor(26, y + 38);
        String bStr = notif.body;
        if (bStr.length() > 28) bStr = bStr.substring(0, 26) + "...";
        display.print(bStr);
      }

      // Bottom tip
      display.setTextSize(1);
      display.setTextColor(themeBorder);
      display.setCursor(20, 256);
      display.print("TAP: OPEN // SWIPE: NAVIGATE");
    }
  }

  void drawCalendarEvents() {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    // Clear display below the status bar
    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    // Section marker
    display.setTextSize(1);
    display.setTextColor(themeAccent);
    display.setCursor(20, 28);
    display.print("AGENDA // EVENT DETAILS");

    display.drawFastHLine(20, 42, SCREEN_WIDTH - 40, themeBorder);

    if (calendarEventCount == 0) {
      // Empty state card
      display.fillRoundRect(16, 56, SCREEN_WIDTH - 32, 136, 8, theme.cardBg);
      display.drawRoundRect(16, 56, SCREEN_WIDTH - 32, 136, 8, themeBorder);
      display.fillRect(16, 68, 3, 112, themeAccent);

      // Category / Status pill
      display.drawRoundRect(28, 68, 76, 16, 4, themeAccent);
      display.setTextSize(1);
      display.setTextColor(themeAccent);
      display.setCursor(34, 72);
      display.print("ALL CLEAR");

      // Balanced Title (Size 2 = 12x16 font, crisp and perfectly sized)
      display.setTextSize(2);
      display.setTextColor(themeText);
      display.setCursor(28, 96);
      display.print("NO EVENTS");

      // Clean message — comfortably inside box margins
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.setCursor(28, 126);
      display.print("No meetings scheduled today.");
      display.setCursor(28, 142);
      display.print("Your schedule is free.");

      // Sync status tag
      display.setTextColor(themeAccent);
      display.setCursor(28, 166);
      display.print("CALENDAR SYNC // ACTIVE");

      display.setTextColor(themeBorder);
      display.setCursor(34, 256);
      display.print("TAP: RETURN // SWIPE: CALENDAR");
      return;
    }

    CalendarEventItem& ev = calendarEvents[currentCalViewIdx];

    // Category / Status micro pill
    display.drawRoundRect(20, 52, 74, 16, 4, themeAccent);
    display.setTextSize(1);
    display.setTextColor(themeAccent);
    display.setCursor(24, 56);
    if (ev.type.indexOf("bday") >= 0 || ev.type.indexOf("birthday") >= 0) {
      display.print("ANNIVERSARY");
    } else if (ev.type.indexOf("meet") >= 0) {
      display.print("MEETING");
    } else if (ev.type.indexOf("alarm") >= 0) {
      display.print("ALARM");
    } else if (ev.type.indexOf("remind") >= 0) {
      display.print("REMINDER");
    } else {
      display.print("EVENT");
    }

    // Time readout
    display.setTextColor(themeSubText);
    display.setCursor(102, 56);
    display.print(ev.timeStr);

    // Large Bold Event Title
    display.setTextSize(3);
    display.setTextColor(themeText);
    display.setCursor(20, 80);

    // Multi-line word wrap for large title
    int yT = 80;
    int charsPerLine = (SCREEN_WIDTH - 40) / 18;
    int line = 0;
    for (unsigned int i = 0; i < ev.title.length() && line < 3; i += charsPerLine) {
      unsigned int endIdx = i + charsPerLine;
      if (endIdx > ev.title.length()) endIdx = ev.title.length();
      display.setCursor(20, yT + line * 26);
      display.print(ev.title.substring(i, endIdx));
      line++;
    }

    // Precision divider
    display.drawFastHLine(20, 170, SCREEN_WIDTH - 40, themeBorder);

    // Event metadata tags
    display.setTextSize(1);
    display.setTextColor(themeSubText);
    display.setCursor(20, 184);
    if (ev.dateStr.length() > 0 && ev.dateStr != "*") {
      display.printf("DATE     // %s", ev.dateStr.c_str());
    } else {
      display.print("SCHEDULE // DAILY RECURRING");
    }
    display.setCursor(20, 200);
    String typeUpper = ev.type;
    typeUpper.toUpperCase();
    display.printf("TYPE     // %s", typeUpper.c_str());
    display.setCursor(20, 216);
    display.print("STATUS   // ACTIVE IN HARDWARE");

    // Pagination
    char pageBuf[16];
    snprintf(pageBuf, sizeof(pageBuf), "INDEX %02d / %02d", currentCalViewIdx + 1, calendarEventCount);
    display.setTextColor(themeAccent);
    display.setCursor(20, 238);
    display.print(pageBuf);

    display.setTextColor(themeBorder);
    display.setCursor(44, 256);
    display.print("TAP TO RETURN // SWIPE NEXT");
  }

  void parseDateInfo(String dateStr, String dayStr, int& dayOut, int& monthOut, int& yearOut, int& startWeekdayOut, int& daysInMonthOut) {
    dayOut = 10;
    monthOut = 9;
    yearOut = 2026;
    
    dateStr.trim();
    int spaceIdx = dateStr.indexOf(' ');
    if (spaceIdx > 0) {
      dayOut = dateStr.substring(0, spaceIdx).toInt();
      if (dayOut <= 0) dayOut = 10;
    }
    
    String monthStr = "";
    if (spaceIdx > 0) {
      int nextSpaceIdx = dateStr.indexOf(' ', spaceIdx + 1);
      if (nextSpaceIdx > spaceIdx) {
        monthStr = dateStr.substring(spaceIdx + 1, nextSpaceIdx);
        yearOut = dateStr.substring(nextSpaceIdx + 1).toInt();
        if (yearOut < 2000) yearOut = 2026;
      } else {
        monthStr = dateStr.substring(spaceIdx + 1);
      }
    }
    
    monthStr.trim();
    monthStr.toUpperCase();
    if (monthStr.startsWith("JAN")) monthOut = 1;
    else if (monthStr.startsWith("FEB")) monthOut = 2;
    else if (monthStr.startsWith("MAR")) monthOut = 3;
    else if (monthStr.startsWith("APR")) monthOut = 4;
    else if (monthStr.startsWith("MAY")) monthOut = 5;
    else if (monthStr.startsWith("JUN")) monthOut = 6;
    else if (monthStr.startsWith("JUL")) monthOut = 7;
    else if (monthStr.startsWith("AUG")) monthOut = 8;
    else if (monthStr.startsWith("SEP")) monthOut = 9;
    else if (monthStr.startsWith("OCT")) monthOut = 10;
    else if (monthStr.startsWith("NOV")) monthOut = 11;
    else if (monthStr.startsWith("DEC")) monthOut = 12;
    
    int daysPerMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (monthOut >= 1 && monthOut <= 12) {
      daysInMonthOut = daysPerMonth[monthOut];
    } else {
      daysInMonthOut = 30;
    }
    
    startWeekdayOut = 2;
  }

  void drawCalendarGrid(String dateStr, String dayStr) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    int curDay, curMonth, curYear, startWeekday, daysInMonth;
    parseDateInfo(dateStr, dayStr, curDay, curMonth, curYear, startWeekday, daysInMonth);
    
    // Clear display below status bar
    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    // ── Editorial Month Header ──────────────────────────────────────────────
    const char* monthNames[] = { "", "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER" };
    const char* mName = (curMonth >= 1 && curMonth <= 12) ? monthNames[curMonth] : "SEPTEMBER";

    display.setTextSize(2);
    display.setTextColor(themeText);
    display.setCursor(20, 28);
    display.print(mName);
    display.setTextColor(themeSubText);
    display.setCursor(144, 28);
    display.print(curYear);

    display.drawFastHLine(20, 48, SCREEN_WIDTH - 40, themeBorder);

    // ── Giant Date Hero Block ───────────────────────────────────────────────
    char dayNumStr[4];
    snprintf(dayNumStr, sizeof(dayNumStr), "%02d", curDay);

    display.setTextSize(5);
    display.setTextColor(themeText);
    display.setCursor(20, 54);
    display.print(dayNumStr);

    // Asymmetric day name + TODAY badge
    display.setTextSize(2);
    display.setTextColor(themeAccent);
    display.setCursor(92, 56);
    String dUpper = dayStr;
    dUpper.toUpperCase();
    display.print(dUpper);

    // TODAY badge
    display.drawRoundRect(92, 80, 50, 16, 4, themeAccent);
    display.fillRoundRect(93, 81, 48, 14, 3, theme.cardBg);
    display.setTextSize(1);
    display.setTextColor(themeText);
    display.setCursor(99, 84);
    display.print("TODAY");

    // ── Horizontal Day Strip Ribbon ─────────────────────────────────────────
    display.drawFastHLine(20, 108, SCREEN_WIDTH - 40, themeBorder);

    // Show 5 days centered on today
    int startD = curDay - 2;
    int endD   = curDay + 2;
    int rx = 22;
    for (int d = startD; d <= endD; d++) {
      int showD = d;
      if (showD < 1) showD += daysInMonth;
      if (showD > daysInMonth) showD -= daysInMonth;

      bool isToday = (d == curDay);
      if (isToday) {
        display.drawRoundRect(rx, 114, 34, 30, 6, themeAccent);
        display.fillRoundRect(rx + 1, 115, 32, 28, 5, theme.cardBg);
        display.setTextColor(themeText);
      } else {
        display.setTextColor(themeSubText);
      }

      display.setTextSize(2);
      display.setCursor(showD < 10 ? rx + 11 : rx + 5, 120);
      display.print(showD);

      rx += 40;
    }

    display.drawFastHLine(20, 150, SCREEN_WIDTH - 40, themeBorder);

    // ── Daily Agenda / Schedule Section ─────────────────────────────────────
    display.setTextSize(1);
    display.setTextColor(themeText);
    display.setCursor(20, 156);
    display.print("TODAY'S SCHEDULE");

    if (calendarEventCount > 0) {
      // Show count tag
      display.setTextColor(themeAccent);
      display.setCursor(164, 156);
      char cntBuf[16];
      snprintf(cntBuf, sizeof(cntBuf), "%d EVENT%s", calendarEventCount, calendarEventCount > 1 ? "S" : "");
      display.print(cntBuf);

      // Render scrollable/paginated event list
      int startIdx = currentCalViewIdx % calendarEventCount;
      int maxShow = min(calendarEventCount, 2);
      for (int i = 0; i < maxShow; i++) {
        int eIdx = (startIdx + i) % calendarEventCount;
        int ey = 170 + i * 40;
        CalendarEventItem& ev = calendarEvents[eIdx];

        // Card container
        display.fillRoundRect(18, ey, SCREEN_WIDTH - 36, 36, 5, theme.cardBg);
        display.drawRoundRect(18, ey, SCREEN_WIDTH - 36, 36, 5, themeBorder);
        display.fillRect(18, ey, 3, 36, themeAccent);

        // Time pill
        display.setTextSize(1);
        display.setTextColor(themeAccent);
        display.setCursor(26, ey + 6);
        display.print(ev.timeStr.length() > 0 ? ev.timeStr : "09:30");

        // Event type badge
        display.setTextColor(themeSubText);
        display.setCursor(80, ey + 6);
        String typeTag = ev.type.length() > 0 ? ("// " + ev.type) : "// MEETING";
        typeTag.toUpperCase();
        display.print(typeTag);

        // Title
        display.setTextSize(2);
        display.setTextColor(themeText);
        display.setCursor(26, ey + 18);
        String evTitle = ev.title;
        if (evTitle.length() > 14) evTitle = evTitle.substring(0, 13) + "..";
        display.print(evTitle);
      }

      display.setTextSize(1);
      display.setTextColor(themeBorder);
      display.setCursor(34, 256);
      display.print("TAP: NEXT EVENT // SWIPE: CALENDAR");

    } else {
      // ── Zero Blank Space: Rich Daily Overview Telemetry ───────────────────
      display.setTextColor(0x07E0);
      display.setCursor(154, 156);
      display.print("[ ALL CLEAR ]");

      // Card container filling Y in [170, 248]
      display.fillRoundRect(18, 170, SCREEN_WIDTH - 36, 78, 6, theme.cardBg);
      display.drawRoundRect(18, 170, SCREEN_WIDTH - 36, 78, 6, themeBorder);
      display.fillRect(18, 178, 3, 62, themeAccent);

      // Status indicator
      display.fillCircle(28, 184, 3, 0x07E0);
      display.setTextSize(1);
      display.setTextColor(themeText);
      display.setCursor(36, 180);
      display.print("NO MEETINGS SCHEDULED TODAY");

      display.setTextSize(2);
      display.setTextColor(themeAccent);
      display.setCursor(28, 196);
      display.print("FOCUS WINDOW");



      // Productivity / Day progress bar
      display.drawFastHLine(28, 220, SCREEN_WIDTH - 56, themeBorder);
      int dayProgressW = constrain(((curDay % 10) + 1) * 18, 20, SCREEN_WIDTH - 56);
      display.drawFastHLine(28, 220, dayProgressW, themeAccent);

      // Clean status line — NO OVERFLOWING TEXT
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.setCursor(28, 228);
      display.print("STATUS // ALL CLEAR");

      display.setTextColor(themeBorder);
      display.setCursor(30, 256);
      display.print("CALENDAR SYNCED // READY");
    }
  }

  void drawSettingsMenuLandscape(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    // Clear display below the status bar
    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    // ── Header ──────────────────────────────────────────────────────────────
    display.setTextSize(2);
    display.setTextColor(themeText);
    display.setCursor(20, 28);
    display.print("SETTINGS");

    display.drawFastHLine(20, 48, SCREEN_WIDTH - 40, themeBorder);

    // ── Continuous Vertical Scroll Surface ──────────────────────────────────
    const int CONTENT_TOP    = 52;
    const int CONTENT_BOTTOM = 258;
    const int ITEM_H         = 50;
    const int TOTAL_ITEMS    = 7;
    float     scrollPx       = settingsScrollPx;

    const char* titles[] = {
      "Brightness",
      "Sound FX",
      "Clock Face",
      "Speed",
      "Bluetooth",
      "Save",
      "Exit"
    };

    const char* categories[] = {
      "// DISPLAY",
      "",
      "// WATCHFACE",
      "",
      "// WIRELESS",
      "// SYSTEM",
      ""
    };

    for (int i = 0; i < TOTAL_ITEMS; i++) {
      int yPos = CONTENT_TOP + i * ITEM_H - (int)scrollPx;

      // Skip items outside the viewport
      if (yPos + ITEM_H <= CONTENT_TOP) continue;
      if (yPos >= CONTENT_BOTTOM)       break;

      bool isCurrent = (option == i) && settingsActive;

      // Category Header (if present)
      if (strlen(categories[i]) > 0 && yPos >= CONTENT_TOP) {
        display.setTextSize(1);
        display.setTextColor(themeAccent);
        display.setCursor(20, yPos);
        display.print(categories[i]);
      }

      int rowY = (strlen(categories[i]) > 0) ? yPos + 12 : yPos + 4;

      // Selected item precision left indicator tick
      if (isCurrent) {
        display.fillRect(10, rowY, 3, 24, themeAccent);
      }

      // Title (Strictly left column: X in [20, 140])
      display.setTextSize(2);
      display.setTextColor(isCurrent ? themeText : themeSubText);
      display.setCursor(20, rowY + 2);
      display.print(titles[i]);

      // Right-side value / widget (Strictly right column: X in [154, 220])
      int cX = 154;
      int cW = 66;
      int cH = 22;
      int cY = rowY + 1;

      switch (i) {
        case 0: { // Brightness (stepped bars + %)
          display.drawRoundRect(cX, cY, cW, cH, 4, themeBorder);
          for (int b = 0; b < 3; b++) {
            uint16_t col = (brightness > b) ? themeAccent : themeBorder;
            display.fillRect(cX + 6 + b * 6, cY + cH - 5 - (b + 1) * 4, 4, (b + 1) * 4, col);
          }
          display.setTextSize(1);
          display.setTextColor(themeText);
          display.setCursor(cX + 30, cY + 7);
          display.print(brightness == 1 ? "33%" : (brightness == 2 ? "66%" : "100%"));
        } break;

        case 1: { // Sound FX
          if (!silentMode) {
            display.fillRoundRect(cX, cY, cW, cH, 4, 0x07E0);
            display.setTextSize(1);
            display.setTextColor(0x0000);
            display.setCursor(cX + 16, cY + 7);
            display.print("ACTIVE");
          } else {
            display.fillRoundRect(cX, cY, cW, cH, 4, theme.cardBg);
            display.drawRoundRect(cX, cY, cW, cH, 4, themeBorder);
            display.setTextSize(1);
            display.setTextColor(themeSubText);
            display.setCursor(cX + 18, cY + 7);
            display.print("MUTED");
          }
        } break;

        case 2: { // Clock Style
          display.drawRoundRect(cX, cY, cW, cH, 4, themeAccent);
          display.setTextSize(1);
          display.setTextColor(themeAccent);
          if ((clockStyle % 2) == 0) {
            display.setCursor(cX + 11, cY + 7);
            display.print("LUNA OS");
          } else {
            display.setCursor(cX + 14, cY + 7);
            display.print("CHRONO");
          }
        } break;

        case 3: { // Speed
          display.drawRoundRect(cX, cY, cW, cH, 4, themeBorder);
          display.setTextSize(1);
          display.setTextColor(themeText);
          display.setCursor(cX + 16, cY + 7);
          display.print(speed);
          display.print("ms");
        } break;

        case 4: { // BLE
          display.drawRoundRect(cX, cY, cW, cH, 4, themeBorder);
          display.setTextSize(1);
          display.setTextColor(bleOn ? 0x07E0 : themeSubText);
          display.setCursor(cX + 24, cY + 7);
          display.print(bleOn ? "ON" : "OFF");
        } break;

        case 5: { // Save
          display.fillRoundRect(cX, cY, cW, cH, 4, themeAccent);
          display.setTextSize(1);
          display.setTextColor(TFT_WHITE);
          display.setCursor(cX + 20, cY + 7);
          display.print("SAVE");
        } break;

        case 6: { // Exit
          display.drawRoundRect(cX, cY, cW, cH, 4, 0xF800);
          display.setTextSize(1);
          display.setTextColor(0xF800);
          display.setCursor(cX + 20, cY + 7);
          display.print("EXIT");
        } break;
      }

      // Subtle 1px hairline row separator
      display.drawFastHLine(20, yPos + ITEM_H - 2, SCREEN_WIDTH - 40, themeBorder);
    }

    // ── Right-Edge Continuous Scroll Rail ───────────────────────────────────
    if (settingsActive) {
      const int trackTop = CONTENT_TOP;
      const int trackH   = CONTENT_BOTTOM - CONTENT_TOP;
      const int maxScroll = TOTAL_ITEMS * ITEM_H - trackH;
      int thumbH = max(16, trackH * trackH / (TOTAL_ITEMS * ITEM_H));
      int thumbY = trackTop;
      if (maxScroll > 0) {
        thumbY = trackTop + (int)((float)(trackH - thumbH) * constrain(scrollPx / (float)maxScroll, 0.0f, 1.0f));
      }
      display.drawFastVLine(SCREEN_WIDTH - 4, trackTop, trackH, 0x10A2);
      display.fillRect(SCREEN_WIDTH - 5, thumbY, 3, thumbH, themeAccent);
    }
  }

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
    display.setCursor((SCREEN_WIDTH - mlW) / 2, 30);
    display.print(modeLabel);

    // ── Mode selector micro capsules (25M / 5M / 15M) ───────────────────────
    const char* mNames[] = { "25M", "5M", "15M" };
    int mX = 36;
    for (int m = 0; m < 3; m++) {
      bool isSel = (pomoMode == m);
      uint16_t cBorder = isSel ? themeAccent : themeBorder;
      uint16_t cText   = isSel ? themeText   : themeSubText;
      display.drawRoundRect(mX, 42, 48, 16, 4, cBorder);
      if (isSel) {
        display.fillRect(mX + 1, 43, 46, 14, 0x0842);
      }
      display.setTextColor(cText);
      display.setTextSize(1);
      int w = strlen(mNames[m]) * 6;
      display.setCursor(mX + (48 - w) / 2, 46);
      display.print(mNames[m]);
      mX += 58;
    }

    // ── Dominant Hero Countdown Digits ──────────────────────────────────────
    int mins = remainingSec / 60;
    int secs = remainingSec % 60;
    char timeBuf[8];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", mins, secs);

    display.setTextSize(5);
    uint16_t timeColor = (pomoState == 1) ? themeAccent : ((pomoState == 3) ? 0x07E0 : themeText);
    display.setTextColor(timeColor);
    int timeW = 5 * 30;
    display.setCursor((SCREEN_WIDTH - timeW) / 2, 70);
    display.print(timeBuf);

    // ── Geometric Concentric Calibrated Reticle Arc ─────────────────────────
    int cx = SCREEN_WIDTH / 2;
    int cy = 142;
    int r  = 30;

    // Static reticle track
    display.drawCircle(cx, cy, r, themeBorder);
    display.drawCircle(cx, cy, 8, themeBorder);

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
    display.drawFastHLine(30, 184, SCREEN_WIDTH - 60, themeBorder);

    // ── Floating Tactile Action Trigger ─────────────────────────────────────
    int btnW = 144;
    int btnH = 34;
    int btnX = (SCREEN_WIDTH - btnW) / 2;
    int btnY = 196;

    uint16_t btnAccent = (pomoState == 1) ? 0xFDE0 : ((pomoState == 3) ? 0x07E0 : themeAccent);
    display.drawRoundRect(btnX, btnY, btnW, btnH, 8, btnAccent);
    display.fillRoundRect(btnX + 2, btnY + 2, btnW - 4, btnH - 4, 6, (pomoState == 1) ? 0x0842 : 0x0000);

    display.setTextSize(2);
    display.setTextColor(themeText);

    if (pomoState == 0) { // Ready to Start
      int tx = btnX + 32, ty = btnY + 17;
      display.fillTriangle(tx, ty - 6, tx, ty + 6, tx + 9, ty, themeAccent);
      display.setCursor(btnX + 48, btnY + 9);
      display.print("START");
    } else if (pomoState == 1) { // Active -> Show Pause
      int px = btnX + 32, py = btnY + 11;
      display.fillRect(px, py, 3, 12, 0xFDE0);
      display.fillRect(px + 6, py, 3, 12, 0xFDE0);
      display.setCursor(btnX + 48, btnY + 9);
      display.print("PAUSE");
    } else if (pomoState == 2) { // Paused -> Show Resume
      int tx = btnX + 26, ty = btnY + 17;
      display.fillTriangle(tx, ty - 6, tx, ty + 6, tx + 9, ty, themeAccent);
      display.setCursor(btnX + 42, btnY + 9);
      display.print("RESUME");
    } else { // Completed -> Show Reset
      int rx = btnX + 30, ry = btnY + 17;
      display.drawCircle(rx, ry, 5, 0x07E0);
      display.fillRect(rx - 1, ry - 7, 3, 3, 0x0000);
      display.fillTriangle(rx, ry - 7, rx + 4, ry - 5, rx, ry - 3, 0x07E0);
      display.setCursor(btnX + 44, btnY + 9);
      display.print("RESET");
    }

    // ── Secondary Telemetry & Interaction Hints ─────────────────────────────
    char sessBuf[32];
    snprintf(sessBuf, sizeof(sessBuf), "CYCLE COMPLETED: %02d", completedSessions);
    display.setTextSize(1);
    display.setTextColor(themeSubText);
    int sessW = strlen(sessBuf) * 6;
    display.setCursor((SCREEN_WIDTH - sessW) / 2, 240);
    display.print(sessBuf);

    const char* tip = "TAP TRIGGER // HOLD TO RESET";
    int tipW = strlen(tip) * 6;
    display.setCursor((SCREEN_WIDTH - tipW) / 2, 256);
    display.print(tip);
  }



  void draw7SegmentDigit(int x, int y, char ch, int w, int h, int t, uint16_t color) {
    uint8_t mask = 0;
    if (ch >= '0' && ch <= '9') {
      static const uint8_t digitMasks[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
      mask = digitMasks[ch - '0'];
    } else {
      switch (ch) {
        case 'A': case 'a': mask = 0x77; break;
        case 'B': case 'b': mask = 0x7C; break;
        case 'C': case 'c': mask = 0x39; break;
        case 'D': case 'd': mask = 0x5E; break;
        case 'E': case 'e': mask = 0x79; break;
        case 'F': case 'f': mask = 0x71; break;
        case 'H': case 'h': mask = 0x76; break;
        case 'L': case 'l': mask = 0x38; break;
        case 'O': case 'o': mask = 0x3F; break;
        case 'P': case 'p': mask = 0x73; break;
        case 'R': case 'r': mask = 0x50; break;
        case 'S': case 's': mask = 0x6D; break;
        case 'T': case 't': mask = 0x07; break;
        case 'U': case 'u': mask = 0x3E; break;
        case '-':           mask = 0x40; break;
        default: mask = 0; break;
      }
    }

    int halfH = h / 2;

    if (ch == 'T' || ch == 't') {
      display.fillRect(x, y, w, t, color);
      display.fillRect(x + w/2 - t/2, y, t, h, color);
      return;
    }
    if (ch == 'M' || ch == 'm') {
      display.fillRect(x, y, t, h, color);
      display.fillRect(x + w - t, y, t, h, color);
      display.fillRect(x + t, y, t, t, color);
      display.fillRect(x + w - 2*t, y, t, t, color);
      display.fillRect(x + w/2 - t/2, y + t, t, halfH - t, color);
      return;
    }
    if (ch == 'W' || ch == 'w') {
      display.fillRect(x, y, t, h, color);
      display.fillRect(x + w - t, y, t, h, color);
      display.fillRect(x + w/2 - t/2, y + halfH, t, halfH - t, color);
      display.fillRect(x + t, y + h - t, w - 2*t, t, color);
      return;
    }

    if (mask & 0x01) display.fillRect(x + t, y, w - 2*t, t, color);
    if (mask & 0x02) display.fillRect(x + w - t, y + t, t, halfH - t, color);
    if (mask & 0x04) display.fillRect(x + w - t, y + halfH, t, h - halfH - t, color);
    if (mask & 0x08) display.fillRect(x + t, y + h - t, w - 2*t, t, color);
    if (mask & 0x10) display.fillRect(x, y + halfH, t, h - halfH - t, color);
    if (mask & 0x20) display.fillRect(x, y + t, t, halfH - t, color);
    if (mask & 0x40) display.fillRect(x + t, y + halfH - t/2, w - 2*t, t, color);
  }

  void drawLCDString(int x, int y, String str, int digitW, int digitH, int t, int spacing, uint16_t color) {
    int curX = x;
    for (unsigned int i = 0; i < str.length(); i++) {
      char ch = str[i];
      if (ch == ' ') {
        curX += digitW + spacing;
      } else if (ch == ':') {
        display.fillRect(curX + digitW/2 - t/2, y + digitH/4 - t/2, t, t, color);
        display.fillRect(curX + digitW/2 - t/2, y + (3*digitH)/4 - t/2, t, t, color);
        curX += digitW/2 + spacing;
      } else {
        draw7SegmentDigit(curX, y, ch, digitW, digitH, t, color);
        curX += digitW + spacing;
      }
    }
  }

  void drawBitmapScaled(int x, int y, const unsigned char* bitmap, int w, int h, int targetW, int targetH, uint16_t color) {
    int lastSy = -1;
    int rowOffset = 0;
    int bytesPerRow = (w + 7) / 8;
    for (int ty = 0; ty < targetH; ty++) {
      int sy = (ty * h) / targetH;
      if (sy != lastSy) {
        lastSy = sy;
        rowOffset = sy * bytesPerRow;
      }
      for (int tx = 0; tx < targetW; tx++) {
        int sx = (tx * w) / targetW;
        uint8_t byteVal = pgm_read_byte(&bitmap[rowOffset + (sx / 8)]);
        if (byteVal & (128 >> (sx & 7))) {
          display.drawPixel(x + tx, y + ty, color);
        }
      }
    }
  }

  void drawRobotFaceScreen() {
    display.fillScreen(TFT_BLACK);
    
    const uint16_t* frameData = robotEyeAnim.getCurrentFrameData();
    if (frameData != nullptr) {
      display.drawRGBBitmap(robotEyeAnim.getXOffset(), robotEyeAnim.getYOffset(), frameData, robotEyeAnim.getWidth(), robotEyeAnim.getHeight());
    }
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
    if (hasLiveMap && liveMapBuffer != nullptr && (millis() - lastLiveMapMs < 60000)) {
      // Draw live Google Maps screen bitmap frame edge-to-edge!
      display.drawRGBBitmap(0, 0, liveMapBuffer, SCREEN_WIDTH, SCREEN_HEIGHT);
      return;
    }

    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeCardBg  = theme.cardBg;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    // Full screen 240x280 edge-to-edge background (no status bar, no header row)
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, themeBg);
    display.setTextWrap(false);

    int cardX = 8;
    int cardW = SCREEN_WIDTH - 16; // 224

    // ── 1. Upper Maneuver & Distance Card (Y: 6..138, Height = 132) ──
    int card1Y = 6;
    int card1H = 132;
    display.fillRoundRect(cardX, card1Y, cardW, card1H, 12, themeCardBg);
    display.drawRoundRect(cardX, card1Y, cardW, card1H, 12, themeBorder);

    // Centered Maneuver Arrow
    int cx = SCREEN_WIDTH / 2;
    int cy = 38;

    String dirUpper = mapDirection;
    dirUpper.toUpperCase();

    if (dirUpper.indexOf("LEFT") >= 0) {
      // Clean Left Turn Arrow
      display.fillRect(cx + 8, cy - 6, 12, 34, themeAccent);
      display.fillRect(cx - 22, cy - 6, 32, 12, themeAccent);
      display.fillTriangle(cx - 36, cy,
                           cx - 18, cy - 16,
                           cx - 18, cy + 16, themeAccent);

    } else if (dirUpper.indexOf("RIGHT") >= 0) {
      // Clean Right Turn Arrow
      display.fillRect(cx - 20, cy - 6, 12, 34, themeAccent);
      display.fillRect(cx - 10, cy - 6, 32, 12, themeAccent);
      display.fillTriangle(cx + 36, cy,
                           cx + 18, cy - 16,
                           cx + 18, cy + 16, themeAccent);

    } else if (dirUpper.indexOf("UTURN") >= 0 || dirUpper.indexOf("U-TURN") >= 0) {
      // Clean U-Turn Arrow
      display.fillCircle(cx, cy - 8, 20, themeAccent);
      display.fillCircle(cx, cy - 8, 10, themeCardBg);
      display.fillRect(cx - 24, cy - 8, 48, 24, themeCardBg);
      display.fillRect(cx - 20, cy - 8, 10, 28, themeAccent);
      display.fillRect(cx + 10, cy - 8, 10, 20, themeAccent);
      display.fillTriangle(cx + 15, cy + 22,
                           cx + 4, cy + 9,
                           cx + 26, cy + 9, themeAccent);

    } else if (dirUpper.indexOf("ROUNDABOUT") >= 0 || dirUpper.indexOf("ROUND") >= 0) {
      // Roundabout circle with exit
      display.fillCircle(cx, cy, 20, themeAccent);
      display.fillCircle(cx, cy, 12, themeCardBg);
      display.fillRect(cx - 4, cy + 12, 8, 16, themeAccent);
      display.fillTriangle(cx + 18, cy - 22,
                           cx + 5, cy - 16,
                           cx + 18, cy - 9, themeAccent);

    } else {
      // STRAIGHT arrow pointing up
      display.fillRect(cx - 6, cy - 8, 12, 36, themeAccent);
      display.fillTriangle(cx, cy - 28,
                           cx - 20, cy - 6,
                           cx + 20, cy - 6, themeAccent);
    }

    // Next Turn Distance — Large bold Size 3 at Y: 74
    display.setTextSize(3);
    display.setTextColor(themeText, themeCardBg);
    String distStr = (mapDistance == "" || mapDistance == "--") ? "---" : mapDistance;
    int distW = distStr.length() * 18;
    int distX = (SCREEN_WIDTH - distW) / 2;
    if (distX < cardX + 6) distX = cardX + 6;
    display.setCursor(distX, 74);
    display.print(distStr);

    // Turn Direction / Road Instruction at Y: 106
    String dirLabel = mapRoad;
    if (dirLabel.length() == 0) {
      if (dirUpper.indexOf("LEFT") >= 0) dirLabel = "Turn Left";
      else if (dirUpper.indexOf("RIGHT") >= 0) dirLabel = "Turn Right";
      else if (dirUpper.indexOf("UTURN") >= 0 || dirUpper.indexOf("U-TURN") >= 0) dirLabel = "Make U-Turn";
      else if (dirUpper.indexOf("ROUNDABOUT") >= 0 || dirUpper.indexOf("ROUND") >= 0) dirLabel = "Roundabout";
      else dirLabel = "Continue Straight";
    }

    if (dirLabel.length() <= 16) {
      display.setTextSize(2);
      display.setTextColor(themeAccent, themeCardBg);
      int lblW = dirLabel.length() * 12;
      int lblX = (SCREEN_WIDTH - lblW) / 2;
      if (lblX < cardX + 6) lblX = cardX + 6;
      display.setCursor(lblX, 106);
      display.print(dirLabel);
    } else {
      display.setTextSize(1);
      display.setTextColor(themeAccent, themeCardBg);
      int lblW = dirLabel.length() * 6;
      int lblX = (SCREEN_WIDTH - lblW) / 2;
      if (lblX < cardX + 6) lblX = cardX + 6;
      display.setCursor(lblX, 110);
      display.print(dirLabel);
    }

    // ── 2. Route Telemetry Card (Y: 144..240, Height = 96) ───────────
    int card2Y = 144;
    int card2H = 96;
    display.fillRoundRect(cardX, card2Y, cardW, card2H, 12, themeCardBg);
    display.drawRoundRect(cardX, card2Y, cardW, card2H, 12, themeBorder);

    // Subtitle badge "REMAINING TRIP" at Y: 154
    display.setTextSize(1);
    display.setTextColor(themeSubText, themeCardBg);
    const char* remTxt = "REMAINING TRIP";
    int remW = strlen(remTxt) * 6;
    display.setCursor((SCREEN_WIDTH - remW) / 2, 154);
    display.print(remTxt);

    // Route Summary (Total Minutes & Total Kilometers) at Y: 170
    String summaryStr = "";
    if (mapTotalTime.length() > 0 && mapTotalDist.length() > 0) {
      summaryStr = mapTotalTime + " - " + mapTotalDist;
    } else if (mapTotalTime.length() > 0) {
      summaryStr = mapTotalTime;
    } else if (mapTotalDist.length() > 0) {
      summaryStr = mapTotalDist;
    } else if (mapDescription.length() > 0) {
      summaryStr = mapDescription;
    } else {
      summaryStr = "GPS SYNC";
    }

    if (summaryStr.length() <= 16) {
      display.setTextSize(2);
      display.setTextColor(themeText, themeCardBg);
      int sW = summaryStr.length() * 12;
      int sX = (SCREEN_WIDTH - sW) / 2;
      if (sX < cardX + 6) sX = cardX + 6;
      display.setCursor(sX, 170);
      display.print(summaryStr);
    } else {
      display.setTextSize(1);
      display.setTextColor(themeText, themeCardBg);
      int sW = summaryStr.length() * 6;
      int sX = (SCREEN_WIDTH - sW) / 2;
      if (sX < cardX + 6) sX = cardX + 6;
      display.setCursor(sX, 174);
      display.print(summaryStr);
    }

    // Divider Line at Y: 196
    display.drawFastHLine(cardX + 16, 196, cardW - 32, themeBorder);

    // Destination Arrival / ETA at Y: 206
    String etaStr = (mapEta.length() > 0) ? ("ETA " + mapEta) : "ON ROUTE";
    display.setTextSize(2);
    display.setTextColor(themeAccent, themeCardBg);
    int eW = etaStr.length() * 12;
    int eX = (SCREEN_WIDTH - eW) / 2;
    if (eX < cardX + 6) eX = cardX + 6;
    display.setCursor(eX, 206);
    display.print(etaStr);

    // ── 3. Bottom Live & Watch Time Bar (Y: 246..274, Height = 28) ───
    int footY = 246;
    int footH = 28;
    display.fillRoundRect(cardX, footY, cardW, footH, 8, themeCardBg);
    display.drawRoundRect(cardX, footY, cardW, footH, 8, themeBorder);

    // Left: Live Navigation Indicator Dot + Label
    display.fillCircle(cardX + 12, footY + 14, 3, 0x07E0); // Bright green dot
    display.setTextSize(1);
    display.setTextColor(themeSubText, themeCardBg);
    display.setCursor(cardX + 20, footY + 10);
    display.print("MAPS LIVE");

    // Right: Current Watch Time
    char timeBuf[12];
    if (is12Hour) {
      int h12 = hour % 12;
      if (h12 == 0) h12 = 12;
      snprintf(timeBuf, sizeof(timeBuf), "%d:%02d %s", h12, minute, (hour >= 12) ? "PM" : "AM");
    } else {
      snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
    }
    int tW = strlen(timeBuf) * 6;
    display.setTextSize(1);
    display.setTextColor(themeText, themeCardBg);
    display.setCursor(cardX + cardW - 10 - tW, footY + 10);
    display.print(timeBuf);
  }

  // Optimized bold outlined text — 3 shadow passes + 1 main = 4 total (was 6)
  void drawBoldOutlinedText(const char* text, int x, int y, uint16_t textColor, uint16_t shadowColor = 0x0000) {
    // Draw shadow at bottom-right, bottom-left, top-right (3 passes covers all visible edges)
    display.setTextColor(shadowColor);
    display.setCursor(x - 1, y + 1); display.print(text); // bottom-left shadow
    display.setCursor(x + 1, y + 1); display.print(text); // bottom-right shadow
    display.setCursor(x,     y - 1); display.print(text); // top shadow
    // Main text on top
    display.setTextColor(textColor);
    display.setCursor(x, y); display.print(text);
  }

  void drawBoldOutlinedText(const String& text, int x, int y, uint16_t textColor, uint16_t shadowColor = 0x0000) {
    display.setTextColor(shadowColor);
    display.setCursor(x - 1, y + 1); display.print(text);
    display.setCursor(x + 1, y + 1); display.print(text);
    display.setCursor(x,     y - 1); display.print(text);
    display.setTextColor(textColor);
    display.setCursor(x, y); display.print(text);
  }

  void drawClockScreen(int hour, int minute, int second, String day, String date, int style, bool is12Hour, int steps) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    // Clear display below status bar
    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    int dispHour = hour;
    if (is12Hour) {
      dispHour = hour % 12;
      if (dispHour == 0) dispHour = 12;
    }

    if ((style % 2) == 0) {
      // ── STYLE 0: MONOLITH MINIMAL PRECISION WATCHFACE (LUNA OS) ──────────
      // Giant dominant floating digital time
      char timeStr[6];
      if (!timeSynced) {
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispHour, minute);
      } else {
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispHour, minute);
      }

      display.setTextSize(5);
      display.setTextColor(themeText);
      display.setCursor(20, 58);
      display.print(timeStr);

      // Floating live seconds or AM/PM
      display.setTextSize(2);
      display.setTextColor(themeAccent);
      display.setCursor(182, 60);
      if (is12Hour) {
        display.print((hour >= 12) ? "PM" : "AM");
      } else {
        display.print(":");
        if (second < 10) display.print("0");
        display.print(second);
      }

      // Thin precision horizontal divider
      display.drawFastHLine(20, 114, SCREEN_WIDTH - 40, themeBorder);

      // Asymmetric date anchor (vertical cyan indicator rule)
      display.fillRect(20, 126, 2, 46, themeAccent);

      // Weekday in bold uppercase
      display.setTextSize(3);
      display.setTextColor(themeText);
      display.setCursor(30, 128);
      String dayUpper = day;
      dayUpper.toUpperCase();
      display.print(dayUpper);

      // Date string
      display.setTextSize(2);
      display.setTextColor(themeSubText);
      display.setCursor(30, 156);
      String dateUpper = date;
      dateUpper.toUpperCase();
      display.print(dateUpper);

      // Right-aligned secondary telemetry tags
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.setCursor(144, 130); display.print("SYS // NOMINAL");
      display.setCursor(144, 145); display.print("RTC // SYNCED");
      display.setCursor(144, 160); display.print("PWR // OPTIMAL");

      // Thin precision divider
      display.drawFastHLine(20, 188, SCREEN_WIDTH - 40, themeBorder);

      // ── Lower Telemetry Rails ──
      // Steps telemetry rail
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.setCursor(20, 198);
      display.print("ACTIVITY");

      display.setTextSize(2);
      display.setTextColor(themeText);
      display.setCursor(20, 210);
      display.print(steps);
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.print(" STEPS");

      display.drawFastHLine(20, 230, 90, themeBorder);
      int stepW = (steps > 0) ? min(90, (steps * 90) / 10000) : 10;
      display.drawFastHLine(20, 230, stepW, themeAccent);

      // Power telemetry rail
      int batteryPct = getBatteryPercentage(batteryVolts);
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.setCursor(130, 198);
      display.print("TELEMETRY");

      display.setTextSize(2);
      display.setTextColor(themeText);
      display.setCursor(130, 210);
      display.print(batteryPct);
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.print("% PWR");

      display.drawFastHLine(130, 230, 90, themeBorder);
      int battW = min(90, (batteryPct * 90) / 100);
      uint16_t battCol = (batteryPct < 25) ? 0xF800 : ((batteryPct < 55) ? 0xFFE0 : 0x07E0);
      display.drawFastHLine(130, 230, battW, battCol);

      // Sub-footer signature
      display.setTextSize(1);
      display.setTextColor(themeBorder);
      display.setCursor(54, 256);
      display.print("LUNA OS // INSTRUMENT");

    } else {
      // ── STYLE 1: AEROSPACE CHRONO TELEMETRY ──────────────────────────────
      char hStr[4], mStr[4];
      snprintf(hStr, sizeof(hStr), "%02d", dispHour);
      snprintf(mStr, sizeof(mStr), "%02d", minute);

      display.setTextSize(6);
      display.setTextColor(themeText);
      display.setCursor(24, 46);
      display.print(hStr);

      display.setTextColor(themeAccent);
      display.setCursor(24, 102);
      display.print(mStr);

      // Precision right column
      display.drawFastVLine(114, 46, 110, themeBorder);

      display.setTextSize(2);
      display.setTextColor(themeText);
      display.setCursor(126, 52);
      display.print(day);

      display.setTextSize(2);
      display.setTextColor(themeSubText);
      display.setCursor(126, 76);
      display.print(date);

      display.setTextSize(1);
      display.setTextColor(themeAccent);
      display.setCursor(126, 106);
      display.print("SEC: ");
      display.print(second);

      display.setTextColor(themeSubText);
      display.setCursor(126, 124);
      display.print("STEPS: ");
      display.print(steps);

      display.drawFastHLine(20, 180, SCREEN_WIDTH - 40, themeBorder);

      // Live 60-second linear gauge
      int secW = (second * (SCREEN_WIDTH - 40)) / 60;
      display.drawFastHLine(20, 200, SCREEN_WIDTH - 40, themeBorder);
      display.drawFastHLine(20, 200, secW, themeAccent);

      display.setTextSize(1);
      display.setTextColor(themeSubText);
      display.setCursor(20, 214);
      display.print("AEROSPACE CHRONO // T-INDEX");
    }
  }

  void drawTextScreen() {
    uint16_t LUNA_CYAN   = 0x07FF;
    uint16_t LUNA_PINK   = 0xF8B8;
    uint16_t LUNA_DARK   = 0x0842;
    uint16_t LUNA_GLASS  = 0x18E3;

    display.setTextWrap(false);
    
    display.drawRoundRect(4, 4, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 8, 12, LUNA_CYAN);
    display.drawRoundRect(5, 5, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10, 11, LUNA_PINK);
    display.fillRoundRect(8, 8, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 16, 9, LUNA_DARK);
    
    // Envelope Folder Mascot Icon
    int bx = 22, by = 20;
    display.fillRoundRect(bx - 8, by - 6, 16, 12, 3, LUNA_PINK);
    display.fillTriangle(bx - 8, by - 6, bx + 8, by - 6, bx, by, LUNA_DARK);

    display.setTextSize(2);
    display.setTextColor(LUNA_CYAN);
    display.setCursor(44, 12);
    
    String displayTitle = notificationTitle;
    if (displayTitle.length() == 0) {
      displayTitle = "Alert";
    }
    if (displayTitle.length() > 14) {
      displayTitle = displayTitle.substring(0, 11) + "...";
    }
    display.print(displayTitle);
    
    display.drawFastHLine(12, 36, SCREEN_WIDTH - 24, LUNA_GLASS);

    display.setTextColor(TFT_WHITE);
    display.setTextSize(3);
    
    int textLength = notificationText.length() * 18;
    if (textLength <= SCREEN_WIDTH - 24) {
      int startX = (SCREEN_WIDTH - textLength) / 2;
      display.setCursor(startX, SCREEN_HEIGHT / 2 - 12);
      display.print(notificationText);
    } else {
      display.setCursor(scrollPos, SCREEN_HEIGHT / 2 - 12);
      display.print(notificationText);
    }

    display.setTextSize(2);
    display.setTextColor(LUNA_PINK);
    int footerW = 10 * 12;
    display.setCursor((SCREEN_WIDTH - footerW) / 2, SCREEN_HEIGHT - 28);
    display.print("Luna Alert");
  }

  int getMixedSizeTextWidth(String text) {
    int w = 0;
    for (unsigned int i = 0; i < text.length(); i++) {
      char c = text.charAt(i);
      if (c == ' ') {
        w += 6;
      } else if ((c >= '0' && c <= '9') || c == '.' || c == ':') {
        w += 12;
      } else {
        w += 6;
      }
    }
    return w;
  }

  void drawMixedSizeText(String text, int startX, int y2, int y1) {
    int currentX = startX;
    for (unsigned int i = 0; i < text.length(); i++) {
      char c = text.charAt(i);
      if (c == ' ') {
        currentX += 6;
      } else if ((c >= '0' && c <= '9') || c == '.' || c == ':') {
        display.setTextSize(2);
        display.setCursor(currentX, y2);
        display.print(c);
        currentX += 12;
      } else {
        display.setTextSize(1);
        display.setCursor(currentX, y1);
        display.print(c);
        currentX += 6;
      }
    }
    display.setTextSize(1);
  }

  void drawLevelScreen() {
    ThemeColors theme = getTheme();
    uint16_t themeAccent  = theme.accent;
    uint16_t themeBg      = theme.bg;
    uint16_t themeText    = theme.text;
    uint16_t themeBorder  = theme.border;
    uint16_t themeSubText = theme.subText;

    // Clear display below the status bar
    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    // Header context
    display.setTextSize(1);
    display.setTextColor(themeAccent);
    display.setCursor(20, 28);
    display.print("AEROSPACE ATTITUDE // QMI8658");

    // Read IMU data
    float ax = 0.0f, ay = 0.0f, az = 1.0f;
    float gx = 0.0f, gy = 0.0f, gz = 0.0f;
    bool hasData = false;

    static float offsetX = 0.0f;
    static float offsetY = 0.0f;
    static float offsetZ = 0.0f;

    if (imu.isInitialized()) {
      hasData = imu.readMotion(ax, ay, az, gx, gy, gz);
    }

    if (!hasData) {
      // Simulation mode
      float t = millis() / 1000.0f;
      ax = sin(t * 1.5f) * 0.4f;
      ay = cos(t * 1.2f) * 0.3f;
      az = sqrt(max(0.0f, 1.0f - ax*ax - ay*ay));
      gx = cos(t * 2.0f) * 80.0f;
      gy = sin(t * 2.5f) * 60.0f;
      gz = sin(t * 1.0f) * 40.0f;
    }

    // Apply calibration request
    if (calibrateRequest) {
      offsetX = ax;
      offsetY = ay;
      offsetZ = az - 1.0f;
      calibrateRequest = false;
      audio.playSound(SOUND_POWERUP);
    }

    ax -= offsetX;
    ay -= offsetY;
    az -= offsetZ;

    // Compute Pitch & Roll in degrees
    float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 57.29578f;
    float roll = atan2(ay, az) * 57.29578f;

    // Center coordinates for large central visualization
    int cx = 120;
    int cy = 104;
    int maxRadius = 44;

    // Precision aerospace reticle rings
    display.drawCircle(cx, cy, maxRadius, themeBorder);
    display.drawCircle(cx, cy, 24, themeBorder);
    display.drawCircle(cx, cy, 8, themeBorder);

    // Calibrated crosshairs with precision ticks
    display.drawFastHLine(cx - maxRadius - 6, cy, (maxRadius + 6) * 2, themeBorder);
    display.drawFastVLine(cx, cy - maxRadius - 6, (maxRadius + 6) * 2, themeBorder);
    // 45-degree tick marks
    display.drawLine(cx - 16, cy - 16, cx - 22, cy - 22, themeBorder);
    display.drawLine(cx + 16, cy - 16, cx + 22, cy - 22, themeBorder);
    display.drawLine(cx - 16, cy + 16, cx - 22, cy + 22, themeBorder);
    display.drawLine(cx + 16, cy + 16, cx + 22, cy + 22, themeBorder);

    // Target positions
    float targetBx = (float)cx - (ay * 38.0f);
    float targetBy = (float)cy - (ax * 38.0f);

    float dist = sqrt((targetBx - cx) * (targetBx - cx) + (targetBy - cy) * (targetBy - cy));
    if (dist > (float)(maxRadius - 6)) {
      float ang = atan2(targetBy - cy, targetBx - cx);
      targetBx = cx + cos(ang) * (float)(maxRadius - 6);
      targetBy = cy + sin(ang) * (float)(maxRadius - 6);
    }

    // Physical gliding interpolation (low-pass filter for smooth flight dynamics)
    static float smoothBx    = 120.0f;
    static float smoothBy    = 104.0f;
    static float smoothPitch = 0.0f;
    static float smoothRoll  = 0.0f;

    smoothBx    += (targetBx - smoothBx)    * 0.28f;
    smoothBy    += (targetBy - smoothBy)    * 0.28f;
    smoothPitch += (pitch - smoothPitch)    * 0.25f;
    smoothRoll  += (roll - smoothRoll)      * 0.25f;

    bool isLevel = (fabsf(smoothPitch) < 1.0f && fabsf(smoothRoll) < 1.0f);

    // Draw gliding reticle indicator
    if (isLevel) {
      display.fillCircle((int)smoothBx, (int)smoothBy, 5, 0x07E0);
      display.drawCircle((int)smoothBx, (int)smoothBy, 8, 0x07E0);
      display.setTextSize(1);
      display.setTextColor(0x07E0);
      const char* lvlTxt = "LEVELED";
      display.setCursor(cx - (strlen(lvlTxt) * 6) / 2, 154);
      display.print(lvlTxt);
    } else {
      display.fillCircle((int)smoothBx, (int)smoothBy, 4, themeAccent);
      display.drawCircle((int)smoothBx, (int)smoothBy, 7, themeText);
      display.setTextSize(1);
      display.setTextColor(themeAccent);
      char tiltBuf[16];
      snprintf(tiltBuf, sizeof(tiltBuf), "TILT: %.1f deg", sqrt(ax * ax + ay * ay) * 57.3f);
      display.setCursor(cx - (strlen(tiltBuf) * 6) / 2, 154);
      display.print(tiltBuf);
    }

    // ── Asymmetric Unboxed Pitch & Roll Typography ──────────────────────────
    display.drawFastHLine(20, 168, SCREEN_WIDTH - 40, themeBorder);

    // Left column: Pitch
    display.setTextSize(1);
    display.setTextColor(themeSubText);
    display.setCursor(20, 174);
    display.print("PITCH");

    display.setTextSize(3);
    display.setTextColor(themeText);
    char pitchStr[12];
    snprintf(pitchStr, sizeof(pitchStr), "%+.1f", smoothPitch);
    display.setCursor(20, 188);
    display.print(pitchStr);
    display.setTextSize(2);
    display.print((char)247);

    // Right column: Roll
    display.setTextSize(1);
    display.setTextColor(themeSubText);
    display.setCursor(130, 174);
    display.print("ROLL");

    display.setTextSize(3);
    display.setTextColor(themeText);
    char rollStr[12];
    snprintf(rollStr, sizeof(rollStr), "%+.1f", smoothRoll);
    display.setCursor(130, 188);
    display.print(rollStr);
    display.setTextSize(2);
    display.print((char)247);

    // ── Zero-Center Balance / Gyro Attitude Deflection Bar ──────────────────
    int barY = 222;
    int barW = SCREEN_WIDTH - 40;
    display.drawFastHLine(20, barY, barW, themeBorder);
    display.drawFastVLine(120, barY - 4, 9, themeSubText); // Zero mark

    // Deflection marker
    int rollMarkerX = 120 + (int)(smoothRoll * 1.2f);
    rollMarkerX = constrain(rollMarkerX, 22, SCREEN_WIDTH - 22);
    display.fillTriangle(rollMarkerX, barY - 3, rollMarkerX - 3, barY - 8, rollMarkerX + 3, barY - 8, themeAccent);

    // Telemetry readout
    display.setTextSize(1);
    display.setTextColor(themeSubText);
    display.setCursor(20, 236);
    char gyroBuf[36];
    snprintf(gyroBuf, sizeof(gyroBuf), "RATE // X:%d Y:%d Z:%d", (int)gx, (int)gy, (int)gz);
    display.print(gyroBuf);

    // Bottom Calibration Trigger
    display.setTextColor(themeBorder);
    display.setCursor(44, 256);
    display.print("TAP SCREEN TO ZERO CAL");
  }

  // ------------------ Primary Smartwatch Draw Adapter ------------------
  void draw(int hour, int minute, int second, String day, String date, int style = 0, bool is12Hour = false) {
    if (isAsleep) {
      uint16_t LCD_BG_COLOR = 0xCE75; // Light greenish-grey LCD background
      uint16_t LCD_FG_COLOR = 0x0000; // Black segments / lines

      display.fillScreen(LCD_BG_COLOR);

      // Draw premium Casio outer frame (double lines for robustness)
      display.drawRoundRect(6, 10, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 20, 16, LCD_FG_COLOR);
      display.drawRoundRect(7, 11, SCREEN_WIDTH - 14, SCREEN_HEIGHT - 22, 15, LCD_FG_COLOR);

      // ── Grid Lines ──────────────────────────────────────────────────────────
      // Top horizontal line
      display.drawFastHLine(8, 85, SCREEN_WIDTH - 16, LCD_FG_COLOR);
      // Top vertical divider
      display.drawFastVLine(120, 12, 73, LCD_FG_COLOR);
      // Bottom horizontal line
      display.drawFastHLine(8, 195, SCREEN_WIDTH - 16, LCD_FG_COLOR);

      // ── TOP SECTION ─────────────────────────────────────────────────────────
      // Left side: Date label & value
      int dayNum = 1;
      int monthNum = 1;
      // parse date string (usually "06 Jul" or similar)
      String dateStr = date;
      dateStr.trim();
      if (dateStr.length() > 0) {
        if (isDigit(dateStr[0])) {
          int spaceIdx = dateStr.indexOf(' ');
          int dashIdx = dateStr.indexOf('-');
          int sepIdx = (spaceIdx > 0) ? spaceIdx : dashIdx;
          if (sepIdx > 0) {
            dayNum = dateStr.substring(0, sepIdx).toInt();
            String monStr = dateStr.substring(sepIdx + 1);
            monStr.trim();
            monStr.toUpperCase();
            if (monStr.startsWith("JAN")) monthNum = 1;
            else if (monStr.startsWith("FEB")) monthNum = 2;
            else if (monStr.startsWith("MAR")) monthNum = 3;
            else if (monStr.startsWith("APR")) monthNum = 4;
            else if (monStr.startsWith("MAY")) monthNum = 5;
            else if (monStr.startsWith("JUN")) monthNum = 6;
            else if (monStr.startsWith("JUL")) monthNum = 7;
            else if (monStr.startsWith("AUG")) monthNum = 8;
            else if (monStr.startsWith("SEP")) monthNum = 9;
            else if (monStr.startsWith("OCT")) monthNum = 10;
            else if (monStr.startsWith("NOV")) monthNum = 11;
            else if (monStr.startsWith("DEC")) monthNum = 12;
            else if (isDigit(monStr[0])) monthNum = monStr.toInt();
          } else {
            dayNum = dateStr.toInt();
          }
        } else {
          int spaceIdx = dateStr.indexOf(' ');
          if (spaceIdx > 0) {
            String monStr = dateStr.substring(0, spaceIdx);
            monStr.trim();
            monStr.toUpperCase();
            if (monStr.startsWith("JAN")) monthNum = 1;
            else if (monStr.startsWith("FEB")) monthNum = 2;
            else if (monStr.startsWith("MAR")) monthNum = 3;
            else if (monStr.startsWith("APR")) monthNum = 4;
            else if (monStr.startsWith("MAY")) monthNum = 5;
            else if (monStr.startsWith("JUN")) monthNum = 6;
            else if (monStr.startsWith("JUL")) monthNum = 7;
            else if (monStr.startsWith("AUG")) monthNum = 8;
            else if (monStr.startsWith("SEP")) monthNum = 9;
            else if (monStr.startsWith("OCT")) monthNum = 10;
            else if (monStr.startsWith("NOV")) monthNum = 11;
            else if (monStr.startsWith("DEC")) monthNum = 12;
            
            dayNum = dateStr.substring(spaceIdx + 1).toInt();
          }
        }
      }
      char dateBuf[6];
      snprintf(dateBuf, sizeof(dateBuf), "%02d-%02d", monthNum, dayNum);

      display.setTextSize(1);
      display.setTextColor(LCD_FG_COLOR);
      display.setCursor(52, 20);
      display.print("Date");
      drawLCDString(30, 34, dateBuf, 11, 20, 2, 2, LCD_FG_COLOR);

      // Right side: Week label & value
      display.setCursor(168, 20);
      display.print("Week");

      String wkStr = day;
      wkStr.toUpperCase();
      if (wkStr.startsWith("MON")) wkStr = "MO";
      else if (wkStr.startsWith("TUE")) wkStr = "TU";
      else if (wkStr.startsWith("WED")) wkStr = "WE";
      else if (wkStr.startsWith("THU")) wkStr = "TH";
      else if (wkStr.startsWith("FRI")) wkStr = "FR";
      else if (wkStr.startsWith("SAT")) wkStr = "SA";
      else if (wkStr.startsWith("SUN")) wkStr = "SU";
      else wkStr = "TU";
      drawLCDString(167, 34, wkStr, 11, 20, 2, 2, LCD_FG_COLOR);

      // Battery Icon on Left Half
      int bx = 96, by = 20;
      display.drawRect(bx, by, 16, 8, LCD_FG_COLOR);
      display.fillRect(bx + 16, by + 2, 2, 4, LCD_FG_COLOR);
      
      int pct = getBatteryPercentage(batteryVolts);

      
      // Draw Casio segmented battery levels
      if (pct >= 20) display.fillRect(bx + 2, by + 2, 3, 4, LCD_FG_COLOR);
      if (pct >= 50) display.fillRect(bx + 6, by + 2, 3, 4, LCD_FG_COLOR);
      if (pct >= 80) display.fillRect(bx + 10, by + 2, 3, 4, LCD_FG_COLOR);

      // ── MIDDLE SECTION ──────────────────────────────────────────────────────
      // Astronaut at X=65, Y=140
      int ax = 65, ay = 140;
      display.drawCircle(ax, ay - 14, 12, LCD_FG_COLOR);
      display.fillRoundRect(ax - 8, ay - 19, 16, 9, 3, LCD_FG_COLOR);
      display.drawPixel(ax - 5, ay - 17, LCD_BG_COLOR);
      display.drawPixel(ax - 4, ay - 17, LCD_BG_COLOR);
      display.drawFastHLine(ax - 8, ay - 2, 16, LCD_FG_COLOR);
      display.drawRoundRect(ax - 10, ay - 2, 20, 22, 5, LCD_FG_COLOR);
      display.drawRect(ax - 4, ay + 3, 8, 6, LCD_FG_COLOR);
      display.drawFastVLine(ax, ay + 3, 6, LCD_FG_COLOR);
      display.drawPixel(ax - 2, ay + 12, LCD_FG_COLOR);
      display.drawPixel(ax + 2, ay + 12, LCD_FG_COLOR);
      
      // Arm waving micro-animation
      if (second % 2 == 0) {
        display.drawLine(ax - 10, ay + 4, ax - 18, ay - 2, LCD_FG_COLOR);
        display.drawCircle(ax - 18, ay - 3, 2, LCD_FG_COLOR);
      } else {
        display.drawLine(ax - 10, ay + 4, ax - 18, ay + 6, LCD_FG_COLOR);
        display.drawCircle(ax - 18, ay + 7, 2, LCD_FG_COLOR);
      }
      
      display.drawLine(ax + 10, ay + 4, ax + 16, ay + 12, LCD_FG_COLOR);
      display.drawCircle(ax + 16, ay + 13, 2, LCD_FG_COLOR);
      display.drawRoundRect(ax - 8, ay + 20, 6, 8, 2, LCD_FG_COLOR);
      display.drawRoundRect(ax + 2, ay + 20, 6, 8, 2, LCD_FG_COLOR);

      // Saturn Planet at X=165, Y=135
      int cx = 165, cy = 135;
      display.drawCircle(cx, cy, 14, LCD_FG_COLOR);
      for (int i = -24; i <= 24; i++) {
        int rx = cx + i;
        int ry = cy - i / 4;
        bool insidePlanet = (i*i + (ry-cy)*(ry-cy) < 14*14);
        if (!insidePlanet || i < 0) {
          display.drawPixel(rx, ry, LCD_FG_COLOR);
          display.drawPixel(rx, ry + 1, LCD_FG_COLOR);
        }
      }
      display.drawFastHLine(cx - 10, cy - 3, 20, LCD_FG_COLOR);
      display.drawFastHLine(cx - 8, cy + 4, 16, LCD_FG_COLOR);

      // Bluetooth & Alarm Icons
      int iconX = 24, iconY = 120;
      if (bleConnectedStatus) {
        display.drawLine(iconX, iconY, iconX, iconY + 8, LCD_FG_COLOR);
        display.drawLine(iconX, iconY, iconX + 3, iconY + 2, LCD_FG_COLOR);
        display.drawLine(iconX + 3, iconY + 2, iconX - 2, iconY + 6, LCD_FG_COLOR);
        display.drawLine(iconX - 2, iconY + 2, iconX + 3, iconY + 6, LCD_FG_COLOR);
        display.drawLine(iconX + 3, iconY + 6, iconX, iconY + 8, LCD_FG_COLOR);
      }
      
      int alarmX = 208, alarmY = 150;
      display.drawCircle(alarmX, alarmY, 5, LCD_FG_COLOR);
      display.drawLine(alarmX - 5, alarmY - 5, alarmX - 3, alarmY - 3, LCD_FG_COLOR);
      display.drawLine(alarmX + 5, alarmY - 5, alarmX + 3, alarmY - 3, LCD_FG_COLOR);
      display.drawLine(alarmX - 3, alarmY + 5, alarmX - 5, alarmY + 7, LCD_FG_COLOR);
      display.drawLine(alarmX + 3, alarmY + 5, alarmX + 5, alarmY + 7, LCD_FG_COLOR);

      // Sparkle Casio Stars (with alternating blink animation)
      if (second % 2 == 0) {
        display.drawFastHLine(110 - 2, 105, 5, LCD_FG_COLOR);
        display.drawFastVLine(110, 105 - 2, 5, LCD_FG_COLOR);
      }
      display.drawFastHLine(124 - 1, 160, 3, LCD_FG_COLOR);
      display.drawFastVLine(124, 160 - 1, 3, LCD_FG_COLOR);
      if (second % 2 != 0) {
        display.drawFastHLine(210 - 2, 115, 5, LCD_FG_COLOR);
        display.drawFastVLine(210, 115 - 2, 5, LCD_FG_COLOR);
      }

      // ── BOTTOM SECTION ──────────────────────────────────────────────────────
      char timeBuf[6];
      char secBuf[3];

      if (!timeSynced) {
        // Not yet synced from app — show dashes
        snprintf(timeBuf, sizeof(timeBuf), "--:--");
        snprintf(secBuf, sizeof(secBuf), "--");
      } else {
        int dispHour = hour;
        if (is12Hour) {
          dispHour = hour % 12;
          if (dispHour == 0) dispHour = 12;
        }
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", dispHour, minute);
        snprintf(secBuf, sizeof(secBuf), "%02d", second);
      }

      int startTimeX = 39;
      int startSecX = 170;

      // Draw Time
      int curX = startTimeX;
      int digitW = 24, digitH = 48, t = 4, spacing = 3;
      for (int i = 0; i < 5; i++) {
        char ch = timeBuf[i];
        if (ch == ':') {
          if (timeSynced && second % 2 == 0) {
            display.fillRect(curX + 5, 210 + 12, t, t, LCD_FG_COLOR);
            display.fillRect(curX + 5, 210 + 32, t, t, LCD_FG_COLOR);
          } else if (!timeSynced) {
            // Static colon when unsynced
            display.fillRect(curX + 5, 210 + 12, t, t, LCD_FG_COLOR);
            display.fillRect(curX + 5, 210 + 32, t, t, LCD_FG_COLOR);
          }
          curX += 14;
        } else {
          draw7SegmentDigit(curX, 210, ch, digitW, digitH, t, LCD_FG_COLOR);
          curX += digitW + spacing;
        }
      }

      // Draw Seconds
      drawLCDString(startSecX, 226, secBuf, 14, 28, 2, 2, LCD_FG_COLOR);

      tft.drawRGBBitmap(0, 20, display.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
      return;
    }

    ThemeColors theme = getTheme();
    display.fillScreen(theme.bg);
    
    if (popupActive && (millis() - popupStartTime > popupDuration)) {
      popupActive = false;
    }

    if (popupActive) {
      drawPopup();
    } else {
      if (currentScreen != SCREEN_FACE && currentScreen != SCREEN_CARD && currentScreen != SCREEN_MAPS) {
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
            uint16_t arcBg      = TFT_WHITE;
            uint16_t arcCardBg  = 0xF7BE;
            uint16_t arcBorder  = 0xCE79;
            uint16_t arcText    = 0x1082;
            uint16_t arcAccent  = 0x001F;
            uint16_t arcSubText = 0x632C;

            // Clear display below the status bar
            display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, arcBg);

            // Sleek card container
            display.fillRoundRect(12, 30, SCREEN_WIDTH - 24, SCREEN_HEIGHT - 40, 10, arcCardBg);
            display.drawRoundRect(12, 30, SCREEN_WIDTH - 24, SCREEN_HEIGHT - 40, 10, arcBorder);

            // Arcade Category Pill Tag
            display.drawRoundRect(24, 42, 58, 16, 4, arcAccent);
            display.setTextSize(1);
            display.setTextColor(arcAccent);
            display.setCursor(29, 46);
            display.print("ARCADE");

            // Large Title
            display.setTextSize(2);
            display.setTextColor(arcText);
            display.setCursor(24, 64);
            display.print("LUNA GAMES");

            display.setTextSize(1);
            display.setTextColor(arcSubText);
            display.setCursor(24, 84);
            display.print("7 RETRO ENGINE TITLES");

            // Divider
            display.drawFastHLine(24, 98, SCREEN_WIDTH - 48, arcBorder);

            // Center Vector Gamepad Illustration (Y in [112, 172])
            int icx = SCREEN_WIDTH / 2;
            int icy = 142;
            display.fillRoundRect(icx - 44, icy - 20, 88, 40, 8, arcBg);
            display.drawRoundRect(icx - 44, icy - 20, 88, 40, 8, arcBorder);
            // D-pad left
            display.fillRect(icx - 30, icy - 10, 6, 20, arcAccent);
            display.fillRect(icx - 37, icy - 3, 20, 6, arcAccent);
            // Action buttons right
            display.fillCircle(icx + 24, icy - 4, 4, 0xF800);
            display.fillCircle(icx + 14, icy + 4, 4, arcAccent);
            display.fillCircle(icx + 34, icy + 4, 4, 0x07E0);
            // Center screen / slot
            display.drawRoundRect(icx - 6, icy - 8, 12, 16, 2, arcBorder);

            // Relocated Action Pill Button (Moved down to comfortable thumb position)
            int btnW = 160;
            int btnH = 36;
            int btnX = (SCREEN_WIDTH - btnW) / 2;
            int btnY = 194;
            display.fillRoundRect(btnX, btnY, btnW, btnH, 8, arcAccent);
            display.setTextColor(TFT_WHITE);
            display.setTextSize(2);
            const char* stTxt = "PLAY GAMES >";
            int stW = strlen(stTxt) * 12;
            display.setCursor(btnX + (btnW - stW) / 2, btnY + 10);
            display.print(stTxt);

            display.setTextColor(arcSubText);
            display.setTextSize(1);
            const char* hint = "TAP TO LAUNCH ARCADE";
            int hW = strlen(hint) * 6;
            display.setCursor((SCREEN_WIDTH - hW) / 2, 244);
            display.print(hint);
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
        case SCREEN_LEVEL:
          drawLevelScreen();
          break;
        case SCREEN_POMODORO:
          extern int pomoRemainingSec, pomoTotalSec, pomoState, pomoMode, pomoCompletedSessions;
          drawPomodoroScreen(pomoRemainingSec, pomoTotalSec, pomoState, pomoMode, pomoCompletedSessions);
          break;
      }

    }

    if (!popupActive && currentScreen == SCREEN_FACE && headerText.length() > 0) {
      uint16_t headerBg = negativeDisplay ? TFT_WHITE : TFT_BLUE; // matches screen bgColor
      uint16_t headerFg = negativeDisplay ? TFT_BLUE : TFT_WHITE; // matches screen drawing color
      display.fillRect(0, 0, SCREEN_WIDTH, 24, headerBg);
      display.setTextColor(headerFg);
      display.setTextSize(2);
      
      // Center the header text
      int textW = headerText.length() * 12;
      int startX = (SCREEN_WIDTH - textW) / 2;
      if (startX < 0) startX = 0;
      
      display.setCursor(startX, 4);
      display.print(headerText);
      display.drawFastHLine(0, 24, SCREEN_WIDTH, headerFg);
    }

    // ── Silent Mode Transient Overlay ──────────────────────────────────────
    if (showSilentOverlay) {
      if (millis() - silentOverlayStartTime > 2000) {
        showSilentOverlay = false;
      } else {
        // Draw elegant pill/capsule on top-center of the screen
        int bx = 50;
        int by = 6;
        int bw = 140;
        int bh = 28;
        
        // Draw shadow/background and colored outline
        display.fillRoundRect(bx, by, bw, bh, 8, 0x0000); // Black bg
        display.drawRoundRect(bx, by, bw, bh, 8, silentOverlayState ? 0xF800 : 0x07E0); // Red/Green outline
        
        display.setTextSize(2);
        if (silentOverlayState) {
          display.setTextColor(0xF800); // Red
          display.setCursor(bx + 12, by + 6);
          display.print("SILENT ON");
          
          // Draw small speaker cone + X
          int sx = bx + 115, sy = by + 14;
          display.fillRect(sx - 5, sy - 3, 4, 6, 0xF800);
          display.fillTriangle(sx - 1, sy - 6, sx - 1, sy + 6, sx + 3, sy, 0xF800);
          display.drawLine(sx + 6, sy - 3, sx + 10, sy + 1, 0xF800);
          display.drawLine(sx + 10, sy - 3, sx + 6, sy + 1, 0xF800);
        } else {
          display.setTextColor(0x07E0); // Green
          display.setCursor(bx + 12, by + 6);
          display.print("SOUND ON");
          
          // Draw speaker cone + sound waves
          int sx = bx + 112, sy = by + 14;
          display.fillRect(sx - 5, sy - 3, 4, 6, 0x07E0);
          display.fillTriangle(sx - 1, sy - 6, sx - 1, sy + 6, sx + 3, sy, 0x07E0);
          display.drawPixel(sx + 6, sy - 2, 0x07E0);
          display.drawPixel(sx + 7, sy - 1, 0x07E0);
          display.drawPixel(sx + 7, sy, 0x07E0);
          display.drawPixel(sx + 7, sy + 1, 0x07E0);
          display.drawPixel(sx + 6, sy + 2, 0x07E0);
        }
      }
    }

    // ── Alarm / Meeting / Reminder Ringing Overlay ──
    if (alarmRingingActive) {
      int ox = 10;
      int oy = 26;
      int ow = SCREEN_WIDTH - 20;
      int oh = SCREEN_HEIGHT - 38;
      uint16_t alertColor = 0xF800; // Red for Alarm/Meeting
      if (ringingType.indexOf("remind") >= 0) alertColor = 0xFD20; // Amber/Orange for Reminder
      else if (ringingType.indexOf("bday") >= 0) alertColor = 0xF81F; // Magenta for Birthday

      // Pulsing border effect
      bool pulse = ((millis() / 350) % 2 == 0);
      display.fillRoundRect(ox, oy, ow, oh, 12, 0x0841); // Very dark slate card
      display.drawRoundRect(ox, oy, ow, oh, 12, pulse ? alertColor : TFT_WHITE);
      display.drawRoundRect(ox + 1, oy + 1, ow - 2, oh - 2, 11, pulse ? alertColor : 0x4208);

      // Event Type Badge
      display.fillRoundRect(ox + 14, oy + 14, 110, 22, 5, alertColor);
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(ox + 20, oy + 21);
      String alertLabel = ringingType;
      alertLabel.toUpperCase();
      if (alertLabel.length() == 0) alertLabel = "ALARM";
      display.print(alertLabel + " RINGING!");

      // Event Time
      display.setTextSize(3);
      display.setTextColor(TFT_WHITE);
      display.setCursor(ox + 14, oy + 46);
      display.print(ringingTime.length() > 0 ? ringingTime : "NOW");

      // Event Title
      display.setTextSize(2);
      display.setTextColor(alertColor);
      int ty = oy + 80;
      int cpl = (ow - 28) / 12;
      int tLines = 0;
      for (unsigned int i = 0; i < ringingTitle.length() && tLines < 3; i += cpl) {
        unsigned int endI = i + cpl;
        if (endI > ringingTitle.length()) endI = ringingTitle.length();
        display.setCursor(ox + 14, ty + tLines * 20);
        display.print(ringingTitle.substring(i, endI));
        tLines++;
      }

      // Tap to dismiss button
      int dbW = ow - 28;
      int dbH = 34;
      int dbX = ox + 14;
      int dbY = oy + oh - 44;
      display.fillRoundRect(dbX, dbY, dbW, dbH, 8, pulse ? alertColor : 0x2124);
      display.setTextColor(TFT_WHITE);
      display.setTextSize(1);
      const char* dTxt = "TAP ANYWHERE TO DISMISS";
      int dtw = strlen(dTxt) * 6;
      display.setCursor(dbX + (dbW - dtw) / 2, dbY + 13);
      display.print(dTxt);
    }
    
    tft.drawRGBBitmap(0, 20, display.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  // Legacy compatibility
  void drawSettingsMenu(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    display.fillScreen(TFT_WHITE);
    drawStatusBar(12, 0);
    drawSettingsMenuLandscape(option, selected, bleOn, speed, clockStyle, invertOn, brightness);
    tft.drawRGBBitmap(0, 20, display.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
  }
};

#endif // EXPRESSIONS_H
