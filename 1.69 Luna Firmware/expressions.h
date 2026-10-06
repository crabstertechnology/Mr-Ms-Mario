#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "config.h"
#include "image_logo.h"
#include "qr_card.h"
#include "wallpaper_image.h"
#include "clock_wallpaper.h"
#include "clock_font.h"
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
extern int rtcHour;
extern int rtcMinute;
extern String rtcDay;
extern String rtcDate;
extern bool is12HourFormat;

// Smooth inertial scroll state (defined in main sketch)
extern float settingsScrollPx;
extern float gamesScrollPx;
extern float notifScrollPx;
extern int notifFilterTab;

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
    bool unread;
    String appType;
  };
  NotificationItem notificationHistory[10];
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

  // Active incoming call ringing overlay state
  bool callRingingActive;
  String callerName;
  bool callMuted;
  unsigned long callRingStartTime;

  // WhatsApp Quick Reply sheet state
  bool quickReplyActive;

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
  void requestRedraw() { robotEyeAnim.requestRedraw(); }
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

    callRingingActive = false;
    callerName = "";
    callMuted = false;
    callRingStartTime = 0;
    quickReplyActive = false;

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
  void addNotification(String title, String body, String timeStr, bool unread = true, String appType = "") {
    if (appType.length() == 0) {
      String low = title;
      low.toLowerCase();
      if (low.indexOf("wa:") >= 0 || low.indexOf("whatsapp") >= 0) appType = "whatsapp";
      else if (low.indexOf("insta") >= 0) appType = "instagram";
      else if (low.indexOf("gmail") >= 0 || low.indexOf("mail") >= 0) appType = "gmail";
      else if (low.indexOf("cal") >= 0) appType = "calendar";
      else if (low.indexOf("sys") >= 0) appType = "system";
      else appType = "system";
    }
    // Shift elements
    for (int i = 9; i > 0; i--) {
      notificationHistory[i] = notificationHistory[i - 1];
    }
    notificationHistory[0].title = title;
    notificationHistory[0].body = body;
    notificationHistory[0].timeStr = timeStr;
    notificationHistory[0].active = true;
    notificationHistory[0].unread = unread;
    notificationHistory[0].appType = appType;
    
    if (notificationCount < 10) {
      notificationCount++;
    }
    currentNotifViewIdx = 0; // Reset index to show the latest
  }

  void seedDefaultNotifications() {
    clearNotifications();
  }

  void clearNotifications() {
    notificationCount = 0;
    currentNotifViewIdx = 0;
    for (int i = 0; i < 10; i++) {
      notificationHistory[i].active = false;
      notificationHistory[i].unread = false;
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
  bool isNotificationUnread(int idx) const {
    if (idx >= 0 && idx < 10) return notificationHistory[idx].unread;
    return false;
  }
  void markNotificationRead(int idx) {
    if (idx >= 0 && idx < 10) notificationHistory[idx].unread = false;
  }

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

  // ------------------ Incoming Call State ------------------
  void setCallRinging(bool ringing, String caller = "Incoming Call") {
    callRingingActive = ringing;
    if (ringing) {
      callerName = caller;
      callMuted = false;
      callRingStartTime = millis();
    }
  }

  void muteIncomingCall() {
    callMuted = true;
  }

  void dismissIncomingCall() {
    callRingingActive = false;
    callMuted = false;
    callerName = "";
  }

  bool isCallRingingActive() const {
    return callRingingActive;
  }

  bool isCallMuted() const {
    return callMuted;
  }

  String getCallerName() const {
    return callerName;
  }

  // ------------------ WhatsApp Quick Reply State ------------------
  void openQuickReply() {
    quickReplyActive = true;
  }

  void closeQuickReply() {
    quickReplyActive = false;
  }

  bool isQuickReplyActive() const {
    return quickReplyActive;
  }

  int getQuickReplyCount() const {
    return 5;
  }

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

  String getNotificationTitle(int idx) const {
    if (idx >= 0 && idx < notificationCount) {
      return notificationHistory[idx].title;
    }
    return "";
  }

  String getNotificationBody(int idx) const {
    if (idx >= 0 && idx < notificationCount) {
      return notificationHistory[idx].body;
    }
    return "";
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
    popupDuration = isWA ? 8000 : 5000; // 8 seconds for WhatsApp so user can tap reply!
    
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
      case SCREEN_POMODORO:      nm = "F O C U S";    break;
      case SCREEN_MENU:          nm = "M E N U";      break;
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
    bool isWA = popupTitle.startsWith("WA:") || popupTitle.indexOf("WhatsApp") >= 0;
    String displayTitle = popupTitle;
    if (displayTitle.startsWith("WA:")) {
      displayTitle = displayTitle.substring(3);
      displayTitle.trim();
    }

    const uint16_t accentCol = isWA ? 0x07E0 : ((robotVariant == "mr_luna") ? 0x07FF : 0xFD99);
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
    display.fillRoundRect(5, 5, SCREEN_WIDTH - 10, 36, 11, isWA ? 0x0280 : 0x18C3);
    // Notification bell icon (3 circles + base)
    int bx = 22, by = 23;
    display.fillCircle(bx, by - 3, 5, accentCol);
    display.fillRect(bx - 6, by + 2, 13, 4, accentCol);
    display.fillCircle(bx, by + 8, 2, accentCol);
    display.fillRect(bx - 6, by + 2, 13, 2, isWA ? 0x0280 : 0x18C3);

    // App / sender badge
    display.fillRoundRect(38, 14, isWA ? 96 : 90, 17, 8, isWA ? 0x0BE4 : 0x2945);
    display.setTextSize(1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(44, 19);
    display.print(isWA ? "WHATSAPP" : "NEW MESSAGE");

    // Time badge (right side)
    display.setTextColor(subCol);
    display.setCursor(SCREEN_WIDTH - 42, 19);
    display.print("NOW");

    display.drawFastHLine(8, 41, SCREEN_WIDTH - 16, 0x2124);

    // ── Sender / App Title ────────────────────────────────────────────────────
    display.setTextSize(2);
    display.setTextColor(accentCol);
    display.setCursor(14, 50);
    String title = displayTitle;
    if (title.length() > 15) title = title.substring(0, 13) + "..";
    display.print(title);

    display.drawFastHLine(8, 72, SCREEN_WIDTH - 16, 0x2124);

    // ── Message body — size 2 with smart word wrapping ─────────────────────
    drawWordWrappedText(popupBody, 14, 80, SCREEN_WIDTH - 28, 5, 22, textCol, 2);

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

  void drawMiniHeart(int x, int y) {
    display.fillCircle(x + 2, y + 2, 2, 0xF800);
    display.fillCircle(x + 5, y + 2, 2, 0xF800);
    display.fillTriangle(x, y + 3, x + 7, y + 3, x + 3, y + 7, 0xF800);
  }

  void drawNotificationAppIcon(int x, int y, const String& appType) {
    int w = 30;
    int h = 30;
    int r = 7;
    if (appType == "whatsapp") {
      display.fillRoundRect(x, y, w, h, r, 0x2589); // WhatsApp vibrant green
      // White speech bubble & handset
      display.fillCircle(x + 15, y + 15, 9, TFT_WHITE);
      display.fillTriangle(x + 8, y + 19, x + 4, y + 24, x + 14, y + 21, TFT_WHITE);
      display.fillCircle(x + 15, y + 15, 7, 0x2589);
      display.drawLine(x + 11, y + 16, x + 13, y + 13, TFT_WHITE);
      display.drawLine(x + 13, y + 13, x + 17, y + 13, TFT_WHITE);
      display.drawLine(x + 17, y + 13, x + 19, y + 16, TFT_WHITE);
      display.drawPixel(x + 12, y + 17, TFT_WHITE);
      display.drawPixel(x + 18, y + 17, TFT_WHITE);
    } else if (appType == "instagram") {
      // Instagram gradient: top magenta/purple to bottom amber
      for (int i = 0; i < h; i++) {
        uint8_t redVal = 180 + (i * 70) / h;
        uint8_t grnVal = 30 + (i * 90) / h;
        uint8_t bluVal = 190 - (i * 150) / h;
        uint16_t c565 = ((redVal >> 3) << 11) | ((grnVal >> 2) << 5) | (bluVal >> 3);
        display.drawFastHLine(x + 2, y + i, w - 4, c565);
      }
      display.drawRoundRect(x, y, w, h, r, 0xD814);
      // Camera logo
      display.drawRoundRect(x + 6, y + 6, 18, 18, 5, TFT_WHITE);
      display.drawCircle(x + 15, y + 15, 5, TFT_WHITE);
      display.drawPixel(x + 20, y + 10, TFT_WHITE);
      display.drawPixel(x + 21, y + 10, TFT_WHITE);
    } else if (appType == "gmail") {
      display.fillRoundRect(x, y, w, h, r, TFT_WHITE);
      display.drawRoundRect(x, y, w, h, r, 0xCE79);
      // Gmail multi-color M
      display.fillRect(x + 6, y + 8, 3, 14, 0xEA24); // Red left
      display.fillRect(x + 21, y + 8, 3, 14, 0x3CA6); // Green right
      display.drawLine(x + 6, y + 8, x + 15, y + 16, 0xEA24);
      display.drawLine(x + 7, y + 8, x + 15, y + 15, 0xEA24);
      display.drawLine(x + 23, y + 8, x + 15, y + 16, 0x4B3F);
      display.drawLine(x + 22, y + 8, x + 15, y + 15, 0x4B3F);
      display.fillRect(x + 6, y + 20, 18, 2, 0x4A69);
    } else if (appType == "calendar") {
      display.fillRoundRect(x, y, w, h, r, TFT_WHITE);
      display.drawRoundRect(x, y, w, h, r, 0xCE79);
      display.fillRoundRect(x, y, w, 9, 3, 0x2A9F); // Blue top
      display.fillRect(x + 7, y + 1, 2, 4, TFT_WHITE); // Rings
      display.fillRect(x + 21, y + 1, 2, 4, TFT_WHITE);
      display.setTextSize(1);
      display.setTextColor(0x2A9F);
      display.setCursor(x + 10, y + 14);
      display.print("31");
    } else { // System / Settings
      display.fillRoundRect(x, y, w, h, r, 0x422B);
      display.fillCircle(x + 15, y + 15, 7, TFT_WHITE);
      display.fillCircle(x + 15, y + 15, 3, 0x422B);
      display.fillRect(x + 13, y + 5, 4, 3, TFT_WHITE);
      display.fillRect(x + 13, y + 22, 4, 3, TFT_WHITE);
      display.fillRect(x + 5, y + 13, 3, 4, TFT_WHITE);
      display.fillRect(x + 22, y + 13, 3, 4, TFT_WHITE);
      display.drawPixel(x + 8, y + 8, TFT_WHITE);
      display.drawPixel(x + 22, y + 8, TFT_WHITE);
      display.drawPixel(x + 8, y + 22, TFT_WHITE);
      display.drawPixel(x + 22, y + 22, TFT_WHITE);
    }
  }

  void drawNotificationTopStatusBar(int hour, int minute) {
    // 1. Time (White, bold, left)
    char timeBuf[12];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
    display.setTextSize(1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(14, 6);
    display.print(timeBuf);
    display.setCursor(15, 6);
    display.print(timeBuf); // bold

    // 2. Centered Pill: [ 🔔 NOTIFICATIONS ]
    display.fillRoundRect(70, 3, 90, 16, 8, 0x08A6);
    display.drawRoundRect(70, 3, 90, 16, 8, 0x243F);
    
    // Bell icon:
    display.fillCircle(78, 9, 3, 0x3CD9);
    display.fillRect(75, 9, 7, 4, 0x3CD9);
    display.drawFastHLine(74, 13, 9, 0x3CD9);
    display.drawPixel(78, 14, 0x3CD9);

    // Text: NOTIFICATIONS
    display.setTextSize(1);
    display.setTextColor(0x5CD9);
    display.setCursor(87, 7);
    display.print("NOTIFICATIONS");

    // 3. Bluetooth Icon
    display.drawLine(170, 6, 170, 16, TFT_WHITE);
    display.drawLine(170, 6, 174, 9, TFT_WHITE);
    display.drawLine(174, 9, 168, 13, TFT_WHITE);
    display.drawLine(168, 9, 174, 13, TFT_WHITE);
    display.drawLine(174, 13, 170, 16, TFT_WHITE);

    // 4. Battery Icon + percentage
    int batPct = getBatteryPercentage(batteryVolts);
    if (batPct <= 0 || batPct > 100) batPct = 85;
    display.drawRoundRect(182, 6, 18, 10, 2, TFT_WHITE);
    int barW = map(batPct, 0, 100, 0, 14);
    if (barW < 2) barW = 2;
    uint16_t batCol = (batPct > 20) ? 0x2E68 : 0xF900;
    display.fillRect(184, 8, barW, 6, batCol);
    display.fillRect(200, 9, 2, 4, TFT_WHITE);

    char batStr[8];
    snprintf(batStr, sizeof(batStr), "%d%%", batPct);
    display.setTextColor(TFT_WHITE);
    display.setCursor(205, 7);
    display.print(batStr);
  }

  void drawNotificationFilterTabs() {
    int y = 25;
    int h = 26;

    // Tab 0: "All"
    if (notifFilterTab == 0) {
      display.fillRoundRect(10, y, 52, h, 13, 0x243F); // Active royal blue
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(27, y + 9);
      display.print("All");
      display.setCursor(28, y + 9);
      display.print("All");
    } else {
      display.fillRoundRect(10, y, 52, h, 13, 0x10A2);
      display.drawRoundRect(10, y, 52, h, 13, 0x2124);
      display.setTextSize(1);
      display.setTextColor(0x9CD3);
      display.setCursor(27, y + 9);
      display.print("All");
    }

    // Tab 1: "Unread"
    if (notifFilterTab == 1) {
      display.fillRoundRect(66, y, 68, h, 13, 0x243F);
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(82, y + 9);
      display.print("Unread");
      display.setCursor(83, y + 9);
      display.print("Unread");
    } else {
      display.fillRoundRect(66, y, 68, h, 13, 0x10A2);
      display.drawRoundRect(66, y, 68, h, 13, 0x2124);
      display.setTextSize(1);
      display.setTextColor(0x9CD3);
      display.setCursor(82, y + 9);
      display.print("Unread");
    }

    // Tab 2: "Apps"
    if (notifFilterTab == 2) {
      display.fillRoundRect(138, y, 56, h, 13, 0x243F);
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(152, y + 9);
      display.print("Apps");
      display.setCursor(153, y + 9);
      display.print("Apps");
    } else {
      display.fillRoundRect(138, y, 56, h, 13, 0x10A2);
      display.drawRoundRect(138, y, 56, h, 13, 0x2124);
      display.setTextSize(1);
      display.setTextColor(0x9CD3);
      display.setCursor(152, y + 9);
      display.print("Apps");
    }

    // Settings Circle Button (X=198, Y=25, D=26)
    int cx = 211;
    int cy = y + 13;
    display.fillCircle(cx, cy, 13, 0x18C3);
    display.drawCircle(cx, cy, 13, 0x32A7);
    // Larger 14px Gear
    display.drawCircle(cx, cy, 5, 0xCE79);
    display.drawCircle(cx, cy, 2, 0x18C3);
    display.fillRect(cx - 2, cy - 8, 4, 3, 0xCE79);
    display.fillRect(cx - 2, cy + 5, 4, 3, 0xCE79);
    display.fillRect(cx - 8, cy - 2, 3, 4, 0xCE79);
    display.fillRect(cx + 5, cy - 2, 3, 4, 0xCE79);
    display.drawPixel(cx - 5, cy - 5, 0xCE79);
    display.drawPixel(cx + 4, cy - 5, 0xCE79);
    display.drawPixel(cx - 5, cy + 4, 0xCE79);
    display.drawPixel(cx + 4, cy + 4, 0xCE79);
  }

  void drawClearAllButton() {
    int bx = 16;
    int by = 244;
    int bw = 208;
    int bh = 30;
    int br = 15;

    display.fillRoundRect(bx, by, bw, bh, br, 0x0927);
    display.drawRoundRect(bx, by, bw, bh, br, 0x19CB);

    int tx = bx + 64;
    int ty = by + 9;
    display.drawFastHLine(tx - 2, ty, 12, TFT_WHITE);
    display.drawFastHLine(tx + 2, ty - 2, 4, TFT_WHITE);
    display.drawRect(tx, ty + 2, 8, 9, TFT_WHITE);
    display.drawFastVLine(tx + 2, ty + 4, 5, TFT_WHITE);
    display.drawFastVLine(tx + 5, ty + 4, 5, TFT_WHITE);

    display.setTextSize(1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(tx + 18, by + 11);
    display.print("Clear All");
    display.setCursor(tx + 19, by + 11);
    display.print("Clear All");
  }

  void drawNotificationDetail() {
    NotificationItem& notif = notificationHistory[currentNotifViewIdx];

    display.fillRect(0, 0, SCREEN_WIDTH, 26, 0x0842);
    display.setTextSize(1);
    display.setTextColor(0x2CD9);
    display.setCursor(12, 8);
    display.print("< Back");

    display.setTextColor(0x9CD3);
    display.setCursor(SCREEN_WIDTH - 64, 8);
    display.print(notif.timeStr);

    int y = 32;
    drawNotificationAppIcon(16, y, notif.appType);
    display.setTextSize(2);
    display.setTextColor(TFT_WHITE);
    display.setCursor(54, y + 6);
    display.print(notif.title);

    display.drawFastHLine(14, y + 36, SCREEN_WIDTH - 28, 0x2124);

    display.fillRoundRect(12, y + 44, SCREEN_WIDTH - 24, 144, 8, 0x10A2);
    display.drawRoundRect(12, y + 44, SCREEN_WIDTH - 24, 144, 8, 0x19CB);
    drawWordWrappedText(notif.body, 20, y + 56, SCREEN_WIDTH - 40, 6, 22, TFT_WHITE, 2);

    // Full-width Close button (no quick reply)
    display.fillRoundRect(20, 240, SCREEN_WIDTH - 40, 30, 8, 0x18C3);
    display.drawRoundRect(20, 240, SCREEN_WIDTH - 40, 30, 8, 0x2945);
    display.setTextSize(2);
    display.setTextColor(TFT_WHITE);
    const char* clTxt = "CLOSE";
    int clW = strlen(clTxt) * 12;
    display.setCursor((SCREEN_WIDTH - clW) / 2, 248);
    display.print(clTxt);
  }

  void drawNotificationPanel(int hour = 10, int minute = 28) {
    display.fillScreen(0x0842); // Deep dark background

    drawNotificationTopStatusBar(hour, minute);
    drawNotificationFilterTabs();

    if (notificationCount == 0) {
      int cx = SCREEN_WIDTH / 2;
      int cy = 135;

      display.drawCircle(cx, cy - 6, 36, 0x18C3);
      display.drawCircle(cx, cy - 6, 35, 0x10A2);
      display.fillCircle(cx, cy - 6, 30, 0x10A2);
      display.drawCircle(cx, cy - 6, 30, 0x243F);

      display.fillCircle(cx, cy - 12, 10, 0x3CD9);
      display.fillRect(cx - 10, cy - 12, 20, 14, 0x3CD9);
      display.drawFastHLine(cx - 14, cy + 2, 28, 0x3CD9);
      display.drawFastHLine(cx - 12, cy + 3, 24, 0x3CD9);
      display.fillCircle(cx, cy + 6, 3, 0x3CD9);

      // BIG "No Notifications" TEXT (Size 2)
      display.setTextSize(2);
      display.setTextColor(TFT_WHITE);
      const char* t1 = "No Notifications";
      int t1W = strlen(t1) * 12;
      display.setCursor((SCREEN_WIDTH - t1W) / 2, cy + 42);
      display.print(t1);
      return;
    }

    if (notificationSelected) {
      drawNotificationDetail();
      return;
    }

    int visibleIndices[10];
    int visibleCount = 0;
    for (int i = 0; i < notificationCount && i < 10; i++) {
      if (notifFilterTab == 1 && !notificationHistory[i].unread) continue;
      visibleIndices[visibleCount++] = i;
    }

    // Larger 52px card with 6px gap
    int cardH = 52;
    int cardGap = 6;
    int cardStep = cardH + cardGap; // 58px
    float maxScroll = max(0.0f, (float)(visibleCount * cardStep - 182));
    notifScrollPx = constrain(notifScrollPx, 0.0f, maxScroll);

    int startY = 56;

    for (int v = 0; v < visibleCount; v++) {
      int idx = visibleIndices[v];
      NotificationItem& notif = notificationHistory[idx];
      int cy = startY + v * cardStep - (int)notifScrollPx;

      if (cy + cardH < 50 || cy > 240) continue;

      // Card Background (H=52)
      display.fillRoundRect(10, cy, 218, cardH, 10, 0x10A2);
      display.drawRoundRect(10, cy, 218, cardH, 10, 0x1928);

      // Cyan Unread Dot (larger, glowing)
      if (notif.unread) {
        display.fillCircle(5, cy + 26, 3, 0x07FF);
        display.drawCircle(5, cy + 26, 4, 0x03B9);
      }

      // App Icon (30x30)
      drawNotificationAppIcon(15, cy + 11, notif.appType);

      // App Title (Row 1, bold white, size 1 double-drawn)
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(52, cy + 9);
      display.print(notif.title);
      display.setCursor(53, cy + 9);
      display.print(notif.title);

      // Timestamp (grey, right aligned)
      display.setTextColor(0x9CD3);
      int timeX = 208 - (notif.timeStr.length() * 6);
      display.setCursor(timeX, cy + 9);
      display.print(notif.timeStr);

      // Chevron >
      display.setTextColor(0x8410);
      display.setCursor(216, cy + 22);
      display.print(">");

      // Preview snippet (Row 2, brighter & larger text)
      display.setTextColor(0xF7BE);
      display.setCursor(52, cy + 26);
      String bodySnippet = notif.body;
      bool hasHeart = (bodySnippet.indexOf("<3") >= 0 || bodySnippet.indexOf("today?") >= 0);
      if (bodySnippet.length() > 24) {
        bodySnippet = bodySnippet.substring(0, 22) + "..";
      }
      display.print(bodySnippet);
      display.setCursor(53, cy + 26);
      display.print(bodySnippet);

      if (hasHeart && notif.appType == "whatsapp") {
        int heartX = 52 + (bodySnippet.length() * 6) + 4;
        if (heartX < 205) {
          drawMiniHeart(heartX, cy + 26);
        }
      }
    }

    // Scrollbar
    if (visibleCount * cardStep > 182) {
      int trackY = 56;
      int trackH = 180;
      display.drawFastVLine(234, trackY, trackH, 0x10A2);
      int thumbH = max(20, (trackH * 182) / (visibleCount * cardStep));
      int thumbY = trackY + (int)((notifScrollPx / maxScroll) * (trackH - thumbH));
      display.fillRoundRect(233, thumbY, 3, thumbH, 1, 0x632C);
    }

    // Pinned Bottom Button: [ 🗑️ Clear All ]
    drawClearAllButton();
  }

  String formatWatchDate(const String& dStr, const String& dtStr) {
    String shortDay = dStr.substring(0, 3);
    if (shortDay.length() > 0) {
      shortDay.setCharAt(0, toupper(shortDay.charAt(0)));
      for (int i = 1; i < (int)shortDay.length(); i++) shortDay.setCharAt(i, tolower(shortDay.charAt(i)));
    }
    String dayNum = "";
    String monName = "Oct";
    int space = dtStr.indexOf(' ');
    if (space > 0) {
      String p1 = dtStr.substring(0, space);
      String p2 = dtStr.substring(space + 1);
      p1.trim(); p2.trim();
      if (isDigit(p1.charAt(0))) {
        dayNum = String(p1.toInt());
        monName = p2;
      } else {
        monName = p1;
        dayNum = String(p2.toInt());
      }
    } else {
      dayNum = dtStr;
    }
    if (monName.length() >= 3) {
      monName = monName.substring(0, 3);
      monName.setCharAt(0, toupper(monName.charAt(0)));
      monName.setCharAt(1, tolower(monName.charAt(1)));
      monName.setCharAt(2, tolower(monName.charAt(2)));
    }
    if (shortDay.length() == 0) shortDay = "Tue";
    if (dayNum.length() == 0) dayNum = "1";
    return shortDay + ", " + monName + " " + dayNum;
  }

  // ── Unified Top Header (Consistent across Clock, Menu, Pomodoro, Calendar) ───
  void drawUnifiedTopHeader(int hour, int minute, const String& dayStr = "", const String& dateStr = "") {
    // 1. Top Left: Time (Bold Size 2, white)
    char timeBuf[12];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
    display.setTextSize(2);
    display.setTextColor(TFT_WHITE);
    display.setCursor(22, 8);
    display.print(timeBuf);

    // 2. Date Underneath: "Tue, Oct 6" (Size 1, 0x8CD6)
    display.setTextSize(1);
    display.setTextColor(0x8CD6);
    display.setCursor(22, 26);
    String dFormatted = formatWatchDate(dayStr, dateStr);
    display.print(dFormatted);

    // 3. Top Right: Bluetooth Icon (Thick solid double stroke, white)
    display.drawLine(166, 8, 166, 18, TFT_WHITE);
    display.drawLine(167, 8, 167, 18, TFT_WHITE);
    display.drawLine(166, 8, 171, 11, TFT_WHITE);
    display.drawLine(171, 11, 164, 15, TFT_WHITE);
    display.drawLine(164, 11, 171, 15, TFT_WHITE);
    display.drawLine(171, 15, 166, 18, TFT_WHITE);

    // 4. Battery Icon (2px thick solid border, terminal cap, green fill)
    int batPct = getBatteryPercentage(batteryVolts);
    if (batPct <= 0 || batPct > 100) batPct = 85;

    // Thick outer border (2px)
    display.drawRoundRect(178, 8, 18, 10, 2, TFT_WHITE);
    display.drawRoundRect(179, 9, 16, 8, 1, TFT_WHITE);
    // Terminal cap (solid 2x4)
    display.fillRect(196, 11, 2, 4, TFT_WHITE);

    // Battery bar fill
    int barW = map(batPct, 0, 100, 0, 12);
    if (barW < 2) barW = 2;
    uint16_t batCol = (batPct > 20) ? 0x2E68 : 0xF900;
    display.fillRect(181, 10, barW, 6, batCol);

    // 5. Battery Percentage text
    char batStr[8];
    snprintf(batStr, sizeof(batStr), "%d%%", batPct);
    display.setTextColor(TFT_WHITE);
    display.setCursor(201, 9);
    display.print(batStr);
  }

  // ── Top Header with Center Capsule Pill (Mockup design) ──────────────────
  void drawTopHeaderWithPill(int hour, int minute, const char* pillTitle, uint16_t pillColor = 0x03FF, const String& dayStr = "", const String& dateStr = "") {
    // 1. Top Left: Time (Bold Size 2, white)
    char timeBuf[12];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
    display.setTextSize(2);
    display.setTextColor(TFT_WHITE);
    display.setCursor(12, 5);
    display.print(timeBuf);

    // Date Underneath: "Tue, Oct 1" (Size 1, 0x8CD6)
    display.setTextSize(1);
    display.setTextColor(0x8CD6);
    display.setCursor(12, 23);
    String dFormatted = formatWatchDate(dayStr, dateStr);
    display.print(dFormatted);

    // 2. Center Pill: [ <icon> TITLE ] with glow border (matching uploaded mockup)
    if (pillTitle != NULL && strlen(pillTitle) > 0) {
      int pillW = strlen(pillTitle) * 6 + 32;
      int pillX = (SCREEN_WIDTH - pillW) / 2;
      display.fillRoundRect(pillX, 5, pillW, 18, 9, 0x0842);
      display.drawRoundRect(pillX, 5, pillW, 18, 9, pillColor);
      display.drawRoundRect(pillX + 1, 6, pillW - 2, 16, 8, pillColor); // 2px glow border
      
      // Mini icon inside pill
      int icX = pillX + 7, icY = 14;
      if (strcmp(pillTitle, "SETTINGS") == 0) {
        display.fillCircle(icX + 3, icY, 4, TFT_WHITE);
        display.fillCircle(icX + 3, icY, 2, 0x0842);
      } else if (strcmp(pillTitle, "FOCUS") == 0) {
        display.drawCircle(icX + 3, icY, 4, TFT_WHITE);
        display.drawLine(icX + 3, icY, icX + 3, icY - 2, TFT_WHITE);
      } else if (strcmp(pillTitle, "CALENDAR") == 0) {
        display.fillRect(icX, icY - 4, 7, 7, TFT_WHITE);
        display.fillRect(icX, icY - 4, 7, 2, 0xF968);
      }

      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(pillX + 20, 10);
      display.print(pillTitle);
    }

    // 3. Top Right: Bluetooth Icon (Crisp double stroke, white)
    display.drawLine(166, 8, 166, 18, TFT_WHITE);
    display.drawLine(167, 8, 167, 18, TFT_WHITE);
    display.drawLine(166, 8, 171, 11, TFT_WHITE);
    display.drawLine(171, 11, 164, 15, TFT_WHITE);
    display.drawLine(164, 11, 171, 15, TFT_WHITE);
    display.drawLine(171, 15, 166, 18, TFT_WHITE);

    // 4. Battery Icon (2px thick solid border, terminal cap, green fill)
    int batPct = getBatteryPercentage(batteryVolts);
    if (batPct <= 0 || batPct > 100) batPct = 85;

    display.drawRoundRect(178, 8, 18, 10, 2, TFT_WHITE);
    display.drawRoundRect(179, 9, 16, 8, 1, TFT_WHITE);
    display.fillRect(196, 11, 2, 4, TFT_WHITE);

    int barW = map(batPct, 0, 100, 0, 12);
    if (barW < 2) barW = 2;
    uint16_t batCol = (batPct > 20) ? 0x2E68 : 0xF900;
    display.fillRect(181, 10, barW, 6, batCol);

    // 5. Battery Percentage text
    char batStr[8];
    snprintf(batStr, sizeof(batStr), "%d%%", batPct);
    display.setTextColor(TFT_WHITE);
    display.setCursor(201, 9);
    display.print(batStr);
  }

  void drawTopCapsuleStatusBar(int hour, int minute, const char* title, int iconType) {
    drawTopHeaderWithPill(hour, minute, title, 0x03FF, "", "");
  }

  void drawCalendarScreen(int hour, int minute, String dateStr, String dayStr) {
    display.fillScreen(0x0000);

    // 1. Unified Top Header (No pill tag, clean time/date + BLE/battery)
    drawUnifiedTopHeader(hour, minute, dayStr, dateStr);

    // 2. Month Selector Header at Y = 38 (zero collision with header above!)
    display.setTextSize(1);
    display.setTextColor(0x04FF);
    display.setCursor(24, 39);
    display.print("<");
    display.setCursor(210, 39);
    display.print(">");

    display.setTextSize(2);
    display.setTextColor(TFT_WHITE);
    display.setCursor(56, 36);
    display.print("October 2026");

    // Divider line at Y = 56
    display.drawFastHLine(14, 56, SCREEN_WIDTH - 28, 0x1928);

    // 3. 7-Day Column Headers: S M T W T F S at Y = 62
    const char* dNames[7] = { "S", "M", "T", "W", "T", "F", "S" };
    int colCenters[7] = { 22, 55, 88, 120, 152, 185, 218 };
    display.setTextSize(1);
    display.setTextColor(0x8CD6);
    for (int i = 0; i < 7; i++) {
      int dw = strlen(dNames[i]) * 6;
      display.setCursor(colCenters[i] - dw / 2, 62);
      display.print(dNames[i]);
    }

    // 4. Monthly Calendar Grid (October 2026: 1st is Thursday = col 4)
    int startCol = 4; // Thursday
    int dayNum = 1;

    // Parse today's actual day number (Default to 6 for Oct 6, 2026)
    int currentDay = 6;
    String srcDate = (dateStr.length() > 0) ? dateStr : rtcDate;
    srcDate.trim();
    if (srcDate.length() > 0) {
      int sp = srcDate.indexOf(' ');
      if (sp > 0) {
        String p1 = srcDate.substring(0, sp); p1.trim();
        if (isDigit(p1.charAt(0))) currentDay = p1.toInt();
        else {
          String p2 = srcDate.substring(sp + 1); p2.trim();
          if (isDigit(p2.charAt(0))) currentDay = p2.toInt();
        }
      } else if (isDigit(srcDate.charAt(0))) {
        currentDay = srcDate.toInt();
      }
    }
    if (currentDay <= 0 || currentDay > 31) currentDay = 6;

    for (int r = 0; r < 5; r++) {
      int ry = 78 + r * 26;
      for (int c = 0; c < 7; c++) {
        if (r == 0 && c < startCol) continue;
        if (dayNum > 31) break;

        int cx = colCenters[c];
        if (dayNum == currentDay) { // Highlight ONLY the real current date!
          display.fillRoundRect(cx - 12, ry - 3, 24, 22, 6, 0x04FF);
          display.drawRoundRect(cx - 12, ry - 3, 24, 22, 6, 0x3DDF);
          display.setTextSize(1);
          display.setTextColor(TFT_WHITE);
          char numBuf[4];
          snprintf(numBuf, sizeof(numBuf), "%d", dayNum);
          int nw = strlen(numBuf) * 6;
          display.setCursor(cx - nw / 2, ry + 4);
          display.print(numBuf);
        } else {
          display.setTextSize(1);
          display.setTextColor((c == 0 || c == 6) ? 0x8CD6 : TFT_WHITE);
          char numBuf[4];
          snprintf(numBuf, sizeof(numBuf), "%d", dayNum);
          int nw = strlen(numBuf) * 6;
          display.setCursor(cx - nw / 2, ry + 4);
          display.print(numBuf);
        }
        dayNum++;
      }
    }

    // 5. Clean Footer Card at Y = 214 (No fake cards list, no '+' icon!)
    display.fillRoundRect(12, 214, 216, 44, 10, 0x0842);
    display.drawRoundRect(12, 214, 216, 44, 10, 0x1928);
    display.fillCircle(24, 228, 3, 0x04FF);
    display.setTextSize(1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(34, 225);
    display.print("No events scheduled today");
    display.setTextColor(0x8CD6);
    display.setCursor(34, 240);
    display.print("Agenda: 0 tasks pending");
  }

  void drawCalendarEvents() {
    drawCalendarScreen(rtcHour, rtcMinute, "", "");
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
    drawCalendarScreen(rtcHour, rtcMinute, dateStr, dayStr);
  }

  void drawSettingsMenuLandscape(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    display.fillScreen(0x0000); // AMOLED dark black background

    // 4 Modern Settings Cards (Display, Sounds & Vibration, Time & Date, Save & Exit)
    const int CONTENT_TOP    = 46;
    const int ITEM_H         = 44;
    const int ROW_H          = 50; // 44px + 6px gap
    const int TOTAL_ITEMS    = 4;
    float     scrollPx       = settingsScrollPx;

    const char* titles[TOTAL_ITEMS] = {
      "Display",
      "Sounds & Vibration",
      "Time & Date",
      "Save & Exit"
    };

    String subTitles[TOTAL_ITEMS];
    subTitles[0] = (brightness == 1) ? "Brightness: 33%" : ((brightness == 2) ? "Brightness: 66%" : "Brightness: 100%");
    subTitles[1] = silentMode ? "Volume: Muted" : "Volume: Active";
    subTitles[2] = is12HourFormat ? "12H Format (AM/PM)" : "24H Format (Military)";
    subTitles[3] = "Save & return"; // Short, clean subtitle fitting screen perfectly

    const uint16_t badgeColors[TOTAL_ITEMS] = {
      0x03FF, // Electric Blue for Display
      0x815E, // Purple for Sounds
      0x04F5, // Cyan for Time
      0xF9A4  // Coral / Red-Orange for Save & Exit
    };

    for (int i = 0; i < TOTAL_ITEMS; i++) {
      int yPos = CONTENT_TOP + i * ROW_H - (int)scrollPx;
      if (yPos + ITEM_H < CONTENT_TOP) continue;
      if (yPos > SCREEN_HEIGHT) break;

      bool isCurrent = (option == i);

      // Card Background (Dark AMOLED)
      display.fillRoundRect(12, yPos, 216, ITEM_H, 12, 0x0842);
      if (isCurrent) {
        // Glowing electric blue border on active item
        display.drawRoundRect(11, yPos - 1, 218, ITEM_H + 2, 13, 0x0215);
        display.drawRoundRect(12, yPos, 216, ITEM_H, 12, 0x03FF);
        display.drawRoundRect(13, yPos + 1, 214, ITEM_H - 2, 11, 0x03FF);
      } else {
        display.drawRoundRect(12, yPos, 216, ITEM_H, 12, 0x1928);
      }

      // Left Icon Badge (30x30 rounded circle)
      int bx = 18;
      int by = yPos + 7;
      display.fillRoundRect(bx, by, 30, 30, 15, badgeColors[i]);

      // Badge Mini Icons
      switch (i) {
        case 0: // Display / Sun
          display.fillCircle(bx + 15, by + 15, 5, TFT_WHITE);
          display.drawFastVLine(bx + 15, by + 5, 3, TFT_WHITE);
          display.drawFastVLine(bx + 15, by + 22, 3, TFT_WHITE);
          display.drawFastHLine(bx + 5, by + 15, 3, TFT_WHITE);
          display.drawFastHLine(bx + 22, by + 15, 3, TFT_WHITE);
          break;

        case 1: // Sound / Speaker
          display.fillTriangle(bx + 11, by + 15, bx + 18, by + 9, bx + 18, by + 21, TFT_WHITE);
          display.fillRect(bx + 7, by + 12, 5, 6, TFT_WHITE);
          if (!silentMode) {
            display.drawCircle(bx + 21, by + 15, 4, TFT_WHITE);
          } else {
            display.drawLine(bx + 19, by + 11, bx + 23, by + 19, TFT_WHITE);
          }
          break;

        case 2: // Time / Clock
          display.drawCircle(bx + 15, by + 15, 9, TFT_WHITE);
          display.drawCircle(bx + 15, by + 15, 8, TFT_WHITE);
          display.drawLine(bx + 15, by + 15, bx + 15, by + 9, TFT_WHITE);
          display.drawLine(bx + 15, by + 15, bx + 19, by + 15, TFT_WHITE);
          break;

        case 3: // Save & Exit Checkmark
          display.drawLine(bx + 8, by + 15, bx + 13, by + 20, TFT_WHITE);
          display.drawLine(bx + 9, by + 15, bx + 14, by + 20, TFT_WHITE);
          display.drawLine(bx + 13, by + 20, bx + 22, by + 10, TFT_WHITE);
          display.drawLine(bx + 14, by + 20, bx + 23, by + 10, TFT_WHITE);
          break;
      }

      // Title
      display.setTextSize(2);
      display.setTextColor(TFT_WHITE);
      display.setCursor(55, yPos + 7);
      if (i == 1) {
        display.setTextSize(1);
        display.setCursor(55, yPos + 8);
        display.print("Sounds & Vibration");
      } else {
        display.print(titles[i]);
      }

      // Subtitle
      display.setTextSize(1);
      display.setTextColor(0x8CD6);
      display.setCursor(55, yPos + 26);
      display.print(subTitles[i]);

      // Right Chevron '>'
      int chX = 216;
      int chY = yPos + 22;
      display.drawLine(chX - 4, chY - 5, chX, chY, 0x8CD6);
      display.drawLine(chX, chY, chX - 4, chY + 5, 0x8CD6);
    }

    // Lock Top Header Bar: Clean black barrier so scrolled content never clips/bleeds into status bar
    display.fillRect(0, 0, SCREEN_WIDTH, 42, 0x0000);
    drawUnifiedTopHeader(rtcHour, rtcMinute, rtcDay, rtcDate);
  }

  void drawPomodoroScreen(int remainingSec, int totalSec, int pomoState, int pomoMode, int completedSessions, int hour, int minute, const String& dayStr = "", const String& dateStr = "") {
    display.fillScreen(0x0000);

    // 1. Unified Top Header (No pill tag, clean time/date + BLE/battery)
    drawUnifiedTopHeader(hour, minute, dayStr, dateStr);

    // 2. Central Circular Progress Ring (Clean, spacious, aligned)
    int cx = 120;
    int cy = 108;

    // Outer and Inner tracks
    for (int r = 56; r <= 64; r++) {
      display.drawCircle(cx, cy, r, 0x0A28);
    }
    display.drawCircle(cx, cy, 65, 0x1B3F);
    display.drawCircle(cx, cy, 55, 0x1B3F);

    // Swept progress arc (seamless radial lines)
    float progress = (totalSec > 0) ? (float)(totalSec - remainingSec) / (float)totalSec : 0.0f;
    progress = constrain(progress, 0.0f, 1.0f);
    float targetAngle = -M_PI_2 + progress * (2.0f * M_PI);
    uint16_t arcCol = (pomoState == 1) ? 0x04FF : 0x2A94;
    for (float a = -M_PI_2; a <= targetAngle; a += 0.015f) {
      float ca = cos(a);
      float sa = sin(a);
      int x1 = cx + (int)(ca * 57.0f);
      int y1 = cy + (int)(sa * 57.0f);
      int x2 = cx + (int)(ca * 63.0f);
      int y2 = cy + (int)(sa * 63.0f);
      display.drawLine(x1, y1, x2, y2, arcCol);
    }

    // ── All text fits cleanly inside inner diameter (110px) with ZERO overlap ──
    // Tomato / Focus mini badge at Y = 66
    display.fillCircle(120, 68, 4, 0xF960);
    display.drawFastHLine(119, 63, 3, 0x07E0);

    // Mode title (Size 1) at Y = 78
    display.setTextSize(1);
    display.setTextColor(0x8CD6);
    const char* modeTitle = (pomoMode == 0) ? "FOCUS" : ((pomoMode == 1) ? "SHORT BREAK" : "LONG BREAK");
    int mw = strlen(modeTitle) * 6;
    display.setCursor(cx - mw / 2, 78);
    display.print(modeTitle);

    // Huge Digital Countdown Timer (Size 3, 90px wide, centered at 75) at Y = 94
    int mins = remainingSec / 60;
    int secs = remainingSec % 60;
    char tBuf[8];
    snprintf(tBuf, sizeof(tBuf), "%02d:%02d", mins, secs);
    display.setTextSize(3);
    display.setTextColor(TFT_WHITE);
    display.setCursor(75, 94);
    display.print(tBuf);

    // State Label (Size 1) at Y = 124
    display.setTextSize(1);
    display.setTextColor(pomoState == 1 ? 0x07E0 : (pomoState == 2 ? 0xFDE0 : 0x8CD6));
    const char* stLabel = (pomoState == 1) ? "RUNNING" : ((pomoState == 2) ? "PAUSED" : "READY");
    int sw = strlen(stLabel) * 6;
    display.setCursor(cx - sw / 2, 124);
    display.print(stLabel);

    // Session Indicator at Y = 138
    display.setTextColor(0x5AEB);
    char sessBuf[16];
    snprintf(sessBuf, sizeof(sessBuf), "Session %d/4", (completedSessions % 4) + 1);
    int sbw = strlen(sessBuf) * 6;
    display.setCursor(cx - sbw / 2, 138);
    display.print(sessBuf);

    // 3. Mode Buttons (Short Break & Long Break) at Y = 186
    // Left Pill: Short Break 5 min
    int lbx = 18, lby = 186, lbw = 98, lbh = 30;
    if (pomoMode == 1) {
      display.fillRoundRect(lbx, lby, lbw, lbh, 15, 0x04FF);
      display.drawRoundRect(lbx, lby, lbw, lbh, 15, TFT_WHITE);
      display.setTextColor(0x0000);
    } else {
      display.fillRoundRect(lbx, lby, lbw, lbh, 15, 0x0842);
      display.drawRoundRect(lbx, lby, lbw, lbh, 15, 0x2A94);
      display.setTextColor(TFT_WHITE);
    }
    display.setTextSize(1);
    display.setCursor(lbx + 12, lby + 6);
    display.print("Short Break");
    display.setCursor(lbx + 28, lby + 16);
    display.print("5 min");

    // Right Pill: Long Break 15 min
    int rbx = 124, rby = 186, rbw = 98, rbh = 30;
    if (pomoMode == 2) {
      display.fillRoundRect(rbx, rby, rbw, rbh, 15, 0x04FF);
      display.drawRoundRect(rbx, rby, rbw, rbh, 15, TFT_WHITE);
      display.setTextColor(0x0000);
    } else {
      display.fillRoundRect(rbx, rby, rbw, rbh, 15, 0x0842);
      display.drawRoundRect(rbx, rby, rbw, rbh, 15, 0x2A94);
      display.setTextColor(TFT_WHITE);
    }
    display.setTextSize(1);
    display.setCursor(rbx + 14, rby + 6);
    display.print("Long Break");
    display.setCursor(rbx + 28, rby + 16);
    display.print("15 min");

    // 4. Transport Controls at Y = 226: [ Reset ]  [ ▶ / ❚❚ ]  [ Skip ]
    // Left Reset Button
    display.fillCircle(56, 246, 17, 0x0842);
    display.drawCircle(56, 246, 17, 0x2A94);
    display.drawFastVLine(51, 241, 10, TFT_WHITE);
    display.drawFastVLine(52, 241, 10, TFT_WHITE);
    display.fillTriangle(61, 241, 61, 251, 52, 246, TFT_WHITE);
    display.setTextSize(1);
    display.setTextColor(0x8CD6);
    display.setCursor(41, 268);
    display.print("Reset");

    // Center Play / Pause Button
    display.fillCircle(120, 246, 22, 0x04FF);
    display.drawCircle(120, 246, 22, TFT_WHITE);
    display.drawCircle(120, 246, 21, TFT_WHITE);
    if (pomoState == 1) { // Running -> Pause icon
      display.fillRect(115, 238, 4, 16, TFT_WHITE);
      display.fillRect(121, 238, 4, 16, TFT_WHITE);
    } else { // Stopped/Paused -> Play triangle
      display.fillTriangle(115, 237, 115, 255, 129, 246, TFT_WHITE);
    }
    display.setTextSize(1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(pomoState == 1 ? 105 : 107, 269);
    display.print(pomoState == 1 ? "Pause" : "Start");

    // Right Skip Button
    display.fillCircle(184, 246, 17, 0x0842);
    display.drawCircle(184, 246, 17, 0x2A94);
    display.fillTriangle(179, 241, 179, 251, 188, 246, TFT_WHITE);
    display.drawFastVLine(188, 241, 10, TFT_WHITE);
    display.drawFastVLine(189, 241, 10, TFT_WHITE);
    display.setTextSize(1);
    display.setTextColor(0x8CD6);
    display.setCursor(172, 268);
    display.print("Skip");
    // (All bottom stats cards and visible '<' button REMOVED!)
  }

  void drawPomodoroScreen(int remainingSec, int totalSec, int pomoState, int pomoMode, int completedSessions) {
    extern int rtcHour, rtcMinute;
    extern String rtcDay, rtcDate;
    drawPomodoroScreen(remainingSec, totalSec, pomoState, pomoMode, completedSessions, rtcHour, rtcMinute, rtcDay, rtcDate);
  }

  void drawAppLauncherMenu(int hour, int minute, String dayStr, String dateStr) {
    display.fillScreen(0x0000);

    // 1. Subtle mountain wallpaper silhouette at bottom
    display.fillTriangle(0, 250, 80, 215, 170, 265, 0x0826);
    display.fillTriangle(80, 265, 160, 225, 240, 255, 0x0948);
    display.fillTriangle(0, 265, 120, 238, 240, 280, 0x11AB);
    display.fillRect(0, 272, 240, 8, 0x0926);

    // 2. Unified Top Header (Consistent across all screens)
    drawUnifiedTopHeader(hour, minute, dayStr, dateStr);

    // 3. 6 App Launcher Tiles (2 Rows x 3 Columns) — Vertically centered on screen!
    // Row 0: Y = 58, H = 58, r = 16
    // Col 0: Notifications (X = 14, W = 62)
    display.fillRoundRect(14, 58, 62, 58, 16, 0x1A5D);
    display.drawRoundRect(14, 58, 62, 58, 16, 0x4DDF);
    display.drawRoundRect(15, 59, 60, 56, 15, 0x4DDF);
    display.drawRoundRect(16, 60, 58, 54, 14, 0x4DDF);
    int bcx = 45, bcy = 84;
    display.fillCircle(bcx, bcy - 5, 7, TFT_WHITE);
    display.fillRect(bcx - 9, bcy - 5, 19, 10, TFT_WHITE);
    display.fillRoundRect(bcx - 11, bcy + 4, 23, 4, 2, TFT_WHITE);
    display.fillCircle(bcx, bcy + 8, 3, TFT_WHITE);
    display.fillCircle(65, 63, 9, 0xF800);
    display.drawCircle(65, 63, 9, TFT_WHITE);
    display.drawCircle(65, 63, 8, TFT_WHITE);
    display.setTextSize(1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(63, 60);
    display.print("3");
    display.setCursor(10, 120);
    display.print("Notifications");

    // Col 1: Calendar (X = 89, W = 62)
    display.fillRoundRect(89, 58, 62, 58, 16, 0x0A28);
    display.drawRoundRect(89, 58, 62, 58, 16, 0x2DBF);
    display.drawRoundRect(90, 59, 60, 56, 15, 0x2DBF);
    display.drawRoundRect(91, 60, 58, 54, 14, 0x2DBF);
    int ccx = 120, ccy = 84;
    display.fillRoundRect(ccx - 14, ccy - 12, 28, 25, 4, TFT_WHITE);
    display.fillRoundRect(ccx - 14, ccy - 12, 28, 9, 4, 0xF968);
    display.fillRect(ccx - 14, ccy - 7, 28, 4, 0xF968);
    display.fillRect(ccx - 8, ccy - 15, 3, 5, 0x2124);
    display.fillRect(ccx + 5, ccy - 15, 3, 5, 0x2124);
    display.setTextSize(1);
    display.setTextColor(0x0000);
    display.setCursor(ccx - 6, ccy - 1);
    display.print("31");
    display.setTextColor(TFT_WHITE);
    display.setCursor(98, 120);
    display.print("Calendar");

    // Col 2: Focus (X = 164, W = 62)
    display.fillRoundRect(164, 58, 62, 58, 16, 0x3866);
    display.drawRoundRect(164, 58, 62, 58, 16, 0xFA60);
    display.drawRoundRect(165, 59, 60, 56, 15, 0xFA60);
    display.drawRoundRect(166, 60, 58, 54, 14, 0xFA60);
    int fcx = 195, fcy = 84;
    display.fillCircle(fcx - 1, fcy + 1, 12, 0xF9A4);
    display.fillRoundRect(fcx - 3, fcy - 13, 6, 5, 2, 0x2E68);
    display.drawPixel(fcx - 6, fcy - 11, 0x2E68);
    display.drawPixel(fcx + 4, fcy - 11, 0x2E68);
    display.fillCircle(fcx + 7, fcy + 6, 7, 0x0BE4);
    display.drawCircle(fcx + 7, fcy + 6, 7, TFT_WHITE);
    display.drawCircle(fcx + 7, fcy + 6, 6, TFT_WHITE);
    display.drawLine(fcx + 7, fcy + 6, fcx + 7, fcy + 2, 0x0000);
    display.drawLine(fcx + 7, fcy + 6, fcx + 9, fcy + 6, 0x0000);
    display.setTextColor(TFT_WHITE);
    display.setCursor(179, 120);
    display.print("Focus");
    display.setTextColor(0x7CDF);
    display.setCursor(171, 130);
    display.print("Pomodoro");

    // Row 1: Y = 152, H = 58, r = 16
    // Col 0: Games (X = 14, W = 62)
    display.fillRoundRect(14, 152, 62, 58, 16, 0x288C);
    display.drawRoundRect(14, 152, 62, 58, 16, 0x89DF);
    display.drawRoundRect(15, 153, 60, 56, 15, 0x89DF);
    display.drawRoundRect(16, 154, 58, 54, 14, 0x89DF);
    int gcx = 45, gcy = 178;
    display.fillRoundRect(gcx - 16, gcy - 9, 32, 19, 7, 0xDF7E);
    display.drawRoundRect(gcx - 16, gcy - 9, 32, 19, 7, 0x0000);
    display.fillRect(gcx - 12, gcy - 3, 8, 4, 0x288C);
    display.fillRect(gcx - 10, gcy - 5, 4, 8, 0x288C);
    display.fillCircle(gcx + 6, gcy - 3, 2, 0xF800);
    display.fillCircle(gcx + 11, gcy, 2, 0x07E0);
    display.fillCircle(gcx + 6, gcy + 3, 2, 0x001F);
    display.fillCircle(gcx + 1, gcy, 2, 0xFFE0);
    display.setTextColor(TFT_WHITE);
    display.setCursor(30, 214);
    display.print("Games");

    // Col 1: QR (X = 89, W = 62)
    display.fillRoundRect(89, 152, 62, 58, 16, 0x0287);
    display.drawRoundRect(89, 152, 62, 58, 16, 0x0E1E);
    display.drawRoundRect(90, 153, 60, 56, 15, 0x0E1E);
    display.drawRoundRect(91, 154, 58, 54, 14, 0x0E1E);
    int qcx = 120, qcy = 178;
    display.drawRect(qcx - 13, qcy - 13, 9, 9, 0x07F3);
    display.drawRect(qcx - 12, qcy - 12, 7, 7, 0x07F3);
    display.fillRect(qcx - 11, qcy - 11, 5, 5, TFT_WHITE);
    display.drawRect(qcx + 4, qcy - 13, 9, 9, 0x07F3);
    display.drawRect(qcx + 5, qcy - 12, 7, 7, 0x07F3);
    display.fillRect(qcx + 6, qcy - 11, 5, 5, TFT_WHITE);
    display.drawRect(qcx - 13, qcy + 4, 9, 9, 0x07F3);
    display.drawRect(qcx - 12, qcy + 5, 7, 7, 0x07F3);
    display.fillRect(qcx - 11, qcy + 6, 5, 5, TFT_WHITE);
    display.fillRect(qcx - 1, qcy - 1, 4, 4, 0x07F3);
    display.fillRect(qcx + 5, qcy + 5, 5, 5, TFT_WHITE);
    display.setTextColor(TFT_WHITE);
    display.setCursor(114, 214);
    display.print("QR");
    display.setTextColor(0x7CDF);
    display.setCursor(99, 224);
    display.print("My Card");

    // Col 2: Settings (X = 164, W = 62)
    display.fillRoundRect(164, 152, 62, 58, 16, 0x11AB);
    display.drawRoundRect(164, 152, 62, 58, 16, 0x44DF);
    display.drawRoundRect(165, 153, 60, 56, 15, 0x44DF);
    display.drawRoundRect(166, 154, 58, 54, 14, 0x44DF);
    int scx = 195, scy = 178;
    display.fillCircle(scx, scy, 11, 0xDF7E);
    display.fillRect(scx - 4, scy - 14, 8, 28, 0xDF7E);
    display.fillRect(scx - 14, scy - 4, 28, 8, 0xDF7E);
    display.fillCircle(scx, scy, 5, 0x11AB);
    display.fillCircle(scx, scy, 2, 0xDF7E);
    display.setTextColor(TFT_WHITE);
    display.setCursor(171, 214);
    display.print("Settings");
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
    robotEyeAnim.draw(display);
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
      display.drawRGBBitmap(0, 0, liveMapBuffer, SCREEN_WIDTH, SCREEN_HEIGHT);
      return;
    }

    display.fillScreen(0x0000); // Pure AMOLED black background
    display.setTextWrap(false);

    // ── 1. Clean Top Status Header (No oval NAVIGATION pill) ───────────────
    // Left: Current Watch Time (Size 2, White) + Date (Size 1)
    char timeBuf[12];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
    display.setTextSize(2);
    display.setTextColor(TFT_WHITE);
    display.setCursor(20, 8);
    display.print(timeBuf);

    display.setTextSize(1);
    display.setTextColor(0x8CD6);
    display.setCursor(20, 26);
    display.print(formatWatchDate(rtcDay, rtcDate));

    // Right: Bluetooth & Battery Status
    display.drawLine(166, 8, 166, 18, TFT_WHITE);
    display.drawLine(167, 8, 167, 18, TFT_WHITE);
    display.drawLine(166, 8, 171, 11, TFT_WHITE);
    display.drawLine(171, 11, 164, 15, TFT_WHITE);
    display.drawLine(164, 11, 171, 15, TFT_WHITE);
    display.drawLine(171, 15, 166, 18, TFT_WHITE);

    int batPct = getBatteryPercentage(batteryVolts);
    if (batPct <= 0 || batPct > 100) batPct = 85;
    display.drawRoundRect(178, 8, 18, 10, 2, TFT_WHITE);
    display.drawRoundRect(179, 9, 16, 8, 1, TFT_WHITE);
    display.fillRect(196, 11, 2, 4, TFT_WHITE);
    int barW = map(batPct, 0, 100, 0, 12);
    if (barW < 2) barW = 2;
    uint16_t batCol = (batPct > 20) ? 0x2E68 : 0xF900;
    display.fillRect(181, 10, barW, 6, batCol);
    display.setCursor(200, 9);
    display.setTextColor(TFT_WHITE);
    display.print(String(batPct) + "%");

    // ── 2. Upper Turn & Distance Instruction Header ────────────────────────
    String distStr = (mapDistance.length() == 0 || mapDistance == "--") ? "200 m" : mapDistance;
    String roadStr = (mapRoad.length() == 0) ? "Anna Salai" : mapRoad;

    String dirUpper = mapDirection;
    dirUpper.toUpperCase();
    dirUpper.trim();

    // Determine Direction Type & Instruction Text
    enum NavDirType {
      NAV_STRAIGHT,
      NAV_RIGHT,
      NAV_LEFT,
      NAV_UTURN,
      NAV_ROUNDABOUT,
      NAV_SLIGHT_RIGHT,
      NAV_SLIGHT_LEFT,
      NAV_SHARP_RIGHT,
      NAV_SHARP_LEFT,
      NAV_ARRIVE
    };

    NavDirType navType = NAV_RIGHT; // Default to Turn Right
    String turnText = "Turn Right";

    if (dirUpper.indexOf("UTURN") >= 0 || dirUpper.indexOf("U-TURN") >= 0 || dirUpper.indexOf("REVERSE") >= 0) {
      navType = NAV_UTURN;
      turnText = "Make U-Turn";
    } else if (dirUpper.indexOf("ROUNDABOUT") >= 0 || dirUpper.indexOf("ROTARY") >= 0 || dirUpper.indexOf("CIRCLE") >= 0 || dirUpper.indexOf("ROUND") >= 0) {
      navType = NAV_ROUNDABOUT;
      turnText = "Roundabout";
    } else if (dirUpper.indexOf("SHARP_RIGHT") >= 0 || dirUpper.indexOf("SHARP RIGHT") >= 0) {
      navType = NAV_SHARP_RIGHT;
      turnText = "Sharp Right";
    } else if (dirUpper.indexOf("SHARP_LEFT") >= 0 || dirUpper.indexOf("SHARP LEFT") >= 0) {
      navType = NAV_SHARP_LEFT;
      turnText = "Sharp Left";
    } else if (dirUpper.indexOf("SLIGHT_RIGHT") >= 0 || dirUpper.indexOf("SLIGHT RIGHT") >= 0 || dirUpper.indexOf("KEEP_RIGHT") >= 0 || dirUpper.indexOf("KEEP RIGHT") >= 0 || dirUpper.indexOf("FORK_RIGHT") >= 0) {
      navType = NAV_SLIGHT_RIGHT;
      turnText = "Keep Right";
    } else if (dirUpper.indexOf("SLIGHT_LEFT") >= 0 || dirUpper.indexOf("SLIGHT LEFT") >= 0 || dirUpper.indexOf("KEEP_LEFT") >= 0 || dirUpper.indexOf("KEEP LEFT") >= 0 || dirUpper.indexOf("FORK_LEFT") >= 0) {
      navType = NAV_SLIGHT_LEFT;
      turnText = "Keep Left";
    } else if (dirUpper.indexOf("LEFT") >= 0) {
      navType = NAV_LEFT;
      turnText = "Turn Left";
    } else if (dirUpper.indexOf("RIGHT") >= 0) {
      navType = NAV_RIGHT;
      turnText = "Turn Right";
    } else if (dirUpper.indexOf("ARRIVE") >= 0 || dirUpper.indexOf("DEST") >= 0) {
      navType = NAV_ARRIVE;
      turnText = "Destination Ahead";
    } else if (dirUpper.indexOf("STRAIGHT") >= 0 || dirUpper.indexOf("CONTINUE") >= 0 || dirUpper.indexOf("AHEAD") >= 0) {
      navType = NAV_STRAIGHT;
      turnText = "Continue Straight";
    }

    // Distance in Size 3 Bold White at Y: 36
    display.setTextSize(3);
    display.setTextColor(TFT_WHITE);
    int distW = distStr.length() * 18;
    int distX = (SCREEN_WIDTH - distW) / 2;
    display.setCursor(distX, 36);
    display.print(distStr);

    // Turn Text in Size 2 Bold White at Y: 66
    display.setTextSize(2);
    display.setTextColor(TFT_WHITE);
    int turnW = turnText.length() * 12;
    int turnX = (SCREEN_WIDTH - turnW) / 2;
    if (turnX < 8) turnX = 8;
    display.setCursor(turnX, 66);
    display.print(turnText);

    // Road Name in Size 1 Soft Cyan/Grey at Y: 86
    display.setTextSize(1);
    display.setTextColor(0x8CD6);
    int roadW = roadStr.length() * 6;
    int roadX = (SCREEN_WIDTH - roadW) / 2;
    if (roadX < 8) roadX = 8;
    display.setCursor(roadX, 86);
    display.print(roadStr);

    // ── 3. Mathematically Precise & Symmetrical Direction Arrow ────────────
    const uint16_t ARROW_COLOR = 0x07FF; // Electric cyan
    const uint16_t ARROW_GLOW  = 0x0215; // Darker cyan outline

    if (navType == NAV_RIGHT || navType == NAV_SHARP_RIGHT) {
      // ── RIGHT TURN ARROW (Symmetrical, clean 90° bend) ──
      // Vertical stem (Y: 185 down to Y: 137, X: 94..110, width 16)
      display.fillRect(94, 145, 16, 40, ARROW_COLOR);
      display.drawRect(93, 144, 18, 42, ARROW_GLOW);
      // Corner block (X: 94..110, Y: 137..153)
      display.fillRect(94, 137, 16, 16, ARROW_COLOR);
      // Horizontal arm (X: 110..138, Y: 137..153, height 16)
      display.fillRect(110, 137, 28, 16, ARROW_COLOR);
      display.drawRect(109, 136, 30, 18, ARROW_GLOW);
      // Sharp symmetrical arrowhead pointing right (Tip: 172, 145)
      display.fillTriangle(172, 145, 138, 123, 138, 167, ARROW_COLOR);
      display.drawTriangle(174, 145, 137, 122, 137, 168, TFT_WHITE);

    } else if (navType == NAV_LEFT || navType == NAV_SHARP_LEFT) {
      // ── LEFT TURN ARROW (Symmetrical mirror of right turn) ──
      // Vertical stem (Y: 185 down to Y: 137, X: 130..146, width 16)
      display.fillRect(130, 145, 16, 40, ARROW_COLOR);
      display.drawRect(129, 144, 18, 42, ARROW_GLOW);
      // Corner block
      display.fillRect(130, 137, 16, 16, ARROW_COLOR);
      // Horizontal arm (X: 102..130, Y: 137..153, height 16)
      display.fillRect(102, 137, 28, 16, ARROW_COLOR);
      display.drawRect(101, 136, 30, 18, ARROW_GLOW);
      // Sharp symmetrical arrowhead pointing left (Tip: 68, 145)
      display.fillTriangle(68, 145, 102, 123, 102, 167, ARROW_COLOR);
      display.drawTriangle(66, 145, 103, 122, 103, 168, TFT_WHITE);

    } else if (navType == NAV_UTURN) {
      // ── U-TURN 180° LOOP ARROW (Symmetrical arch and downward head) ──
      // Right stem going up (X: 132..148, Y: 135..185)
      display.fillRect(132, 135, 16, 50, ARROW_COLOR);
      display.drawRect(131, 134, 18, 52, ARROW_GLOW);
      // Top semicircular arch (Outer R=28, Inner R=12, Center=(120, 135))
      display.fillCircle(120, 135, 28, ARROW_COLOR);
      display.fillCircle(120, 135, 12, 0x0000);
      display.fillRect(90, 135, 60, 28, 0x0000); // clear lower half of circle
      // Left stem going down (X: 92..108, Y: 135..158)
      display.fillRect(92, 135, 16, 24, ARROW_COLOR);
      display.drawRect(91, 134, 18, 26, ARROW_GLOW);
      // Downward arrowhead (Tip: 100, 190, Base: Y=158, X: 78..122)
      display.fillTriangle(100, 190, 78, 158, 122, 158, ARROW_COLOR);
      display.drawTriangle(100, 192, 76, 157, 124, 157, TFT_WHITE);

    } else if (navType == NAV_ROUNDABOUT) {
      // ── ROUNDABOUT / ROTARY ARROW ──
      int rcX = 120;
      int rcY = 146;
      // Circular ring track (Outer R=28, Inner R=14, uniform 14px width)
      display.fillCircle(rcX, rcY, 28, ARROW_COLOR);
      display.fillCircle(rcX, rcY, 14, 0x0000);
      display.drawCircle(rcX, rcY, 28, TFT_WHITE);
      display.drawCircle(rcX, rcY, 14, 0x03FF);
      // Entry stem from bottom (X: 113..127, Y: 174..188)
      display.fillRect(113, 174, 14, 14, ARROW_COLOR);
      // Exit diagonal branch towards upper right
      display.fillRect(136, 118, 16, 16, ARROW_COLOR);
      display.fillTriangle(172, 108, 146, 106, 160, 132, ARROW_COLOR);
      display.drawTriangle(174, 107, 145, 105, 161, 133, TFT_WHITE);

    } else if (navType == NAV_SLIGHT_RIGHT) {
      // ── SLIGHT RIGHT / KEEP RIGHT ARROW ──
      display.fillRect(112, 160, 16, 26, ARROW_COLOR);
      for (int i = 0; i < 16; i++) {
        display.drawLine(112 + i, 162, 146 + i, 128, ARROW_COLOR);
      }
      display.fillTriangle(170, 110, 144, 110, 160, 136, ARROW_COLOR);
      display.drawTriangle(172, 109, 143, 109, 161, 137, TFT_WHITE);

    } else if (navType == NAV_SLIGHT_LEFT) {
      // ── SLIGHT LEFT / KEEP LEFT ARROW ──
      display.fillRect(112, 160, 16, 26, ARROW_COLOR);
      for (int i = 0; i < 16; i++) {
        display.drawLine(112 + i, 162, 78 + i, 128, ARROW_COLOR);
      }
      display.fillTriangle(70, 110, 80, 136, 96, 110, ARROW_COLOR);
      display.drawTriangle(68, 109, 79, 137, 97, 109, TFT_WHITE);

    } else if (navType == NAV_ARRIVE) {
      // ── ARRIVE / DESTINATION MARKER ──
      display.drawCircle(120, 146, 20, 0x0215);
      display.drawCircle(120, 146, 12, 0x03FF);
      display.fillCircle(120, 146, 5, ARROW_COLOR);
      display.drawFastVLine(118, 108, 38, TFT_WHITE);
      display.fillRect(119, 108, 22, 14, ARROW_COLOR);
      display.drawRect(119, 108, 22, 14, TFT_WHITE);

    } else {
      // ── STRAIGHT / CONTINUE ARROW ──
      display.fillRect(111, 135, 18, 50, ARROW_COLOR);
      display.drawRect(110, 134, 20, 52, ARROW_GLOW);
      display.fillTriangle(120, 105, 96, 138, 144, 138, ARROW_COLOR);
      display.drawTriangle(120, 103, 94, 139, 146, 139, TFT_WHITE);
    }

    // ── 4. Bottom 3-Column Telemetry Card (Y: 210..274, Height = 64) ───────
    // Values are rendered BIG (Size 2 bold) and spaced with zero clipping
    int cX = 6;
    int cY = 210;
    int cW = 228;
    int cH = 64;
    display.fillRoundRect(cX, cY, cW, cH, 12, 0x0842); // Dark card fill
    display.drawRoundRect(cX, cY, cW, cH, 12, 0x1928); // Subtle border

    // Vertical column divider lines (Col 1: 6..80, Col 2: 80..160, Col 3: 160..234)
    display.drawFastVLine(80,  cY + 8, cH - 16, 0x1928);
    display.drawFastVLine(160, cY + 8, cH - 16, 0x1928);

    // Sanitize ETA (keep clean "HH:MM")
    String etaVal = mapEta;
    etaVal.trim();
    if (etaVal.length() == 0 || etaVal == "--") {
      etaVal = "10:42";
    } else {
      if (etaVal.startsWith("ETA ") || etaVal.startsWith("eta ")) {
        etaVal = etaVal.substring(4);
        etaVal.trim();
      }
      int sp = etaVal.indexOf(' ');
      if (sp > 0 && (etaVal.indexOf("AM") > 0 || etaVal.indexOf("PM") > 0 || etaVal.indexOf("am") > 0 || etaVal.indexOf("pm") > 0)) {
        etaVal = etaVal.substring(0, sp);
      }
      int c1 = etaVal.indexOf(':');
      if (c1 > 0) {
        int c2 = etaVal.indexOf(':', c1 + 1);
        if (c2 > 0) {
          etaVal = etaVal.substring(0, c2);
        }
      }
    }

    // Sanitize Remaining Distance
    String remVal = mapTotalDist;
    remVal.trim();
    if (remVal.length() == 0 || remVal == "--") {
      remVal = "1.8 km";
    } else {
      if (remVal.startsWith("DIST ") || remVal.startsWith("dist ")) {
        remVal = remVal.substring(5);
        remVal.trim();
      }
    }

    // Sanitize Time Left
    String leftVal = mapTotalTime;
    leftVal.trim();
    if (leftVal.length() == 0 || leftVal == "--") {
      leftVal = "14 min";
    } else {
      if (leftVal.startsWith("TIME ") || leftVal.startsWith("time ")) {
        leftVal = leftVal.substring(5);
        leftVal.trim();
      }
      leftVal.replace("minutes", "min");
      leftVal.replace("hours", "hr");
      leftVal.replace("hour", "hr");
    }

    // Column Subtitles (Size 1, soft cyan)
    display.setTextSize(1);
    display.setTextColor(0x8CD6);
    display.setCursor(34,  cY + 12); display.print("ETA");
    display.setCursor(102, cY + 12); display.print("REMAIN");
    display.setCursor(185, cY + 12); display.print("TIME");

    // Helper for large, prominent values with optional unit
    auto drawValueWithUnit = [&](const String& valStr, int cx, int yBase) {
      String numPart = "";
      String unitPart = "";
      int spaceIdx = valStr.indexOf(' ');
      if (spaceIdx > 0) {
        numPart = valStr.substring(0, spaceIdx);
        unitPart = valStr.substring(spaceIdx + 1);
        unitPart.trim();
      } else {
        int idx = 0;
        while (idx < (int)valStr.length() && (isDigit(valStr.charAt(idx)) || valStr.charAt(idx) == '.' || valStr.charAt(idx) == ':')) {
          idx++;
        }
        if (idx > 0 && idx < (int)valStr.length()) {
          numPart = valStr.substring(0, idx);
          unitPart = valStr.substring(idx);
        } else {
          numPart = valStr;
          unitPart = "";
        }
      }

      int numW  = numPart.length() * 12; // Size 2 = 12px per char
      int unitW = unitPart.length() * 6; // Size 1 = 6px per char
      int gap   = (unitPart.length() > 0) ? 3 : 0;
      int totalW = numW + gap + unitW;

      if (totalW <= 72) {
        // Fits comfortably in Size 2 Bold White!
        int startX = cx - (totalW / 2);
        display.setTextSize(2);
        display.setTextColor(TFT_WHITE);
        display.setCursor(startX, yBase);
        display.print(numPart);

        if (unitPart.length() > 0) {
          display.setTextSize(1);
          display.setTextColor(0x8CD6);
          display.setCursor(startX + numW + gap, yBase + 7);
          display.print(unitPart);
        }
      } else {
        // Double-stroke bold Size 1 fallback for long strings
        display.setTextSize(1);
        display.setTextColor(TFT_WHITE);
        int w = valStr.length() * 6;
        int startX = cx - (w / 2);
        display.setCursor(startX, yBase + 5);     display.print(valStr);
        display.setCursor(startX + 1, yBase + 5); display.print(valStr);
      }
    };

    // Render all 3 values in large, bold, visible text (YBase = cY + 28)
    drawValueWithUnit(etaVal,  43,  cY + 28);
    drawValueWithUnit(remVal,  120, cY + 28);
    drawValueWithUnit(leftVal, 197, cY + 28);
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

  // Draw a 4-bit alpha blended digit glyph
  void drawAlphaGlyph(int x, int y, uint8_t digit, uint16_t color, bool drawShadow = true) {
    if (digit > 9) return;
    const DigitGlyph& g = CLOCK_DIGITS[digit];
    uint8_t w = g.w;
    uint8_t h = g.h;
    const uint8_t* p = g.data;
    uint16_t* buf = display.getBuffer();

    // 1. Draw soft drop shadow first (offset +1, +2 in black)
    if (drawShadow) {
      const uint8_t* sp = p;
      int sy = y + 2;
      int sx = x + 1;
      for (int r = 0; r < h; r++) {
        int py = sy + r;
        if (py >= 0 && py < SCREEN_HEIGHT) {
          for (int c = 0; c < w; c += 2) {
            uint8_t b = pgm_read_byte(sp++);
            uint8_t a1 = (b >> 4) & 0x0F;
            uint8_t a2 = b & 0x0F;
            int px1 = sx + c;
            if (px1 >= 0 && px1 < SCREEN_WIDTH && a1 > 2) {
              int idx = py * SCREEN_WIDTH + px1;
              buf[idx] = blendRGB565(buf[idx], 0x0000, a1 >> 1); // 50% shadow
            }
            int px2 = px1 + 1;
            if (px2 < sx + w && px2 >= 0 && px2 < SCREEN_WIDTH && a2 > 2) {
              int idx = py * SCREEN_WIDTH + px2;
              buf[idx] = blendRGB565(buf[idx], 0x0000, a2 >> 1);
            }
          }
        } else {
          sp += (w + 1) / 2;
        }
      }
    }

    // 2. Draw main anti-aliased glyph
    for (int r = 0; r < h; r++) {
      int py = y + r;
      if (py >= 0 && py < SCREEN_HEIGHT) {
        for (int c = 0; c < w; c += 2) {
          uint8_t b = pgm_read_byte(p++);
          uint8_t a1 = (b >> 4) & 0x0F;
          uint8_t a2 = b & 0x0F;
          int px1 = x + c;
          if (px1 >= 0 && px1 < SCREEN_WIDTH && a1 > 0) {
            int idx = py * SCREEN_WIDTH + px1;
            buf[idx] = blendRGB565(buf[idx], color, a1);
          }
          int px2 = px1 + 1;
          if (px2 < x + w && px2 >= 0 && px2 < SCREEN_WIDTH && a2 > 0) {
            int idx = py * SCREEN_WIDTH + px2;
            buf[idx] = blendRGB565(buf[idx], color, a2);
          }
        }
      } else {
        p += (w + 1) / 2;
      }
    }
  }

  // Draw two-digit number (e.g. "10", "08", "28")
  int drawTwoDigits(int startX, int startY, int val, uint16_t color) {
    int d1 = (val / 10) % 10;
    int d2 = val % 10;
    drawAlphaGlyph(startX, startY, d1, color, true);
    int w1 = CLOCK_DIGITS[d1].w;
    int d2_x = startX + w1 + 3; // 3px digit kerning
    drawAlphaGlyph(d2_x, startY, d2, color, true);
    int w2 = CLOCK_DIGITS[d2].w;
    return (d2_x + w2) - startX;
  }

  void drawHeartIcon(int cx, int cy, uint16_t col) {
    display.fillCircle(cx - 3, cy - 2, 4, col);
    display.fillCircle(cx + 3, cy - 2, 4, col);
    display.fillTriangle(cx - 7, cy - 1, cx + 7, cy - 1, cx, cy + 7, col);
  }

  void drawShoeIcon(int cx, int cy, uint16_t col) {
    display.fillRoundRect(cx - 7, cy + 1, 14, 5, 2, col);
    display.fillRoundRect(cx - 4, cy - 4, 8, 7, 2, col);
    display.fillTriangle(cx + 4, cy + 3, cx + 7, cy + 3, cx + 5, cy - 1, col);
    display.drawFastHLine(cx - 7, cy + 6, 14, 0xFFFF);
  }

  void drawFlameIcon(int cx, int cy, uint16_t col) {
    display.fillCircle(cx, cy + 2, 5, col);
    display.fillTriangle(cx - 5, cy + 2, cx + 5, cy + 2, cx, cy - 6, col);
    display.fillTriangle(cx - 3, cy + 1, cx + 1, cy + 1, cx - 1, cy - 4, 0xFFE0);
  }

  void drawPinIcon(int cx, int cy, uint16_t col) {
    display.fillCircle(cx, cy - 2, 5, col);
    display.fillTriangle(cx - 4, cy - 1, cx + 4, cy - 1, cx, cy + 6, col);
    display.fillCircle(cx, cy - 2, 2, 0x0862);
  }

  void drawClockTopStatusBar(int hour, int minute, bool is12Hour) {
    char tBuf[8];
    int dispH = hour;
    if (is12Hour) {
      dispH = hour % 12;
      if (dispH == 0) dispH = 12;
    }
    snprintf(tBuf, sizeof(tBuf), "%02d:%02d", dispH, minute);
    display.setTextSize(1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(12, 7);
    display.print(tBuf);

    int pillX = 76, pillY = 4, pillW = 64, pillH = 16;
    display.fillRoundRect(pillX, pillY, pillW, pillH, 8, 0x1275);
    int icx = pillX + 9, icy = pillY + 8;
    display.drawCircle(icx, icy, 5, TFT_WHITE);
    display.drawLine(icx, icy, icx, icy - 3, TFT_WHITE);
    display.drawLine(icx, icy, icx + 2, icy, TFT_WHITE);
    display.setTextSize(1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(pillX + 18, pillY + 5);
    display.print("CLOCK");

    int bx = 152, by = 6;
    uint16_t bleColor = bleConnectedStatus ? 0x05BF : 0x4296;
    display.drawLine(bx + 2, by, bx + 2, by + 10, bleColor);
    display.drawLine(bx + 2, by + 1, bx + 5, by + 3, bleColor);
    display.drawLine(bx + 5, by + 3, bx, by + 7, bleColor);
    display.drawLine(bx, by + 3, bx + 5, by + 7, bleColor);
    display.drawLine(bx + 5, by + 7, bx + 2, by + 9, bleColor);

    int batX = 166, batY = 7;
    int batW = 18, batH = 9;
    display.drawRoundRect(batX, batY, batW, batH, 2, TFT_WHITE);
    display.fillRect(batX + batW, batY + 2, 2, 5, TFT_WHITE);
    
    int pct = getBatteryPercentage(batteryVolts);
    int fillW = constrain((pct * (batW - 4)) / 100, 1, batW - 4);
    uint16_t batColor = (pct <= 20) ? 0xF800 : ((pct <= 50) ? 0xFD20 : 0x2688);
    display.fillRect(batX + 2, batY + 2, fillW, batH - 4, batColor);

    display.setTextSize(1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(batX + batW + 5, batY + 1);
    display.printf("%d%%", pct);

    if (silentMode) {
      int mx = 222, my = 6;
      display.fillRect(mx, my + 3, 2, 4, TFT_WHITE);
      display.fillTriangle(mx + 2, my + 1, mx + 2, my + 9, mx + 5, my + 5, TFT_WHITE);
      display.drawLine(mx - 1, my + 1, mx + 7, my + 9, 0xF800);
    }
  }

  void drawBottomCards(int steps) {
    int cardY = 192;
    int cardW = 48;
    int cardH = 58;
    int gap   = 6;
    int startX = 15;

    int bpm = 72 + (millis() / 1500) % 3;
    int dispSteps = (steps > 0) ? steps : 3456;
    int kcal = (dispSteps * 4) / 100;
    if (kcal < 10) kcal = 320;
    float km = (dispSteps * 0.00075f);
    if (km < 0.1f) km = 2.4f;

    for (int i = 0; i < 4; i++) {
      int cx = startX + i * (cardW + gap);
      display.fillRoundRect(cx, cardY, cardW, cardH, 10, 0x08A4);
      // Double stroke 2px thick border (no pixel scattering)
      display.drawRoundRect(cx, cardY, cardW, cardH, 10, 0x3A9F);
      display.drawRoundRect(cx + 1, cardY + 1, cardW - 2, cardH - 2, 9, 0x3A9F);

      int icx = cx + cardW / 2;
      int icy = cardY + 12;

      char valBuf[16];
      const char* subTxt = "";

      if (i == 0) {
        drawHeartIcon(icx, icy, 0xF986);
        snprintf(valBuf, sizeof(valBuf), "%d", bpm);
        subTxt = "bpm";
      } else if (i == 1) {
        drawShoeIcon(icx, icy, 0x34BF);
        if (dispSteps >= 1000) {
          snprintf(valBuf, sizeof(valBuf), "%d,%03d", dispSteps / 1000, dispSteps % 1000);
        } else {
          snprintf(valBuf, sizeof(valBuf), "%d", dispSteps);
        }
        subTxt = "steps";
      } else if (i == 2) {
        drawFlameIcon(icx, icy, 0xFD00);
        snprintf(valBuf, sizeof(valBuf), "%d", kcal);
        subTxt = "kcal";
      } else {
        drawPinIcon(icx, icy, 0x268C);
        snprintf(valBuf, sizeof(valBuf), "%.1f", km);
        subTxt = "km";
      }

      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      int vW = strlen(valBuf) * 6;
      display.setCursor(cx + (cardW - vW) / 2, cardY + 27);
      display.print(valBuf);

      display.setTextColor(0x8C71);
      int sW = strlen(subTxt) * 6;
      display.setCursor(cx + (cardW - sW) / 2, cardY + 44);
      display.print(subTxt);
    }

    display.fillRoundRect(107, 264, 15, 3, 1, TFT_WHITE);
    display.fillRoundRect(126, 264, 8, 3, 1, 0x218A);
  }

  void drawClockScreen(int hour, int minute, int second, String day, String date, int style, bool is12Hour, int steps) {
    int dispHour = hour;
    if (is12Hour) {
      dispHour = hour % 12;
      if (dispHour == 0) dispHour = 12;
    }

    // ── SINGLE MODERN NIGHT MOUNTAIN LAKE WATCHFACE ─────────────────────
    // 1. Copy 240x280 wallpaper from PROGMEM directly to PSRAM canvas
    memcpy_P(display.getBuffer(), clock_night_bg, 240 * 280 * sizeof(uint16_t));

    // 2. Unified Top Header (Consistent across Clock, Menu, and Apps)
    drawUnifiedTopHeader(hour, minute, day, date);

    // 3. Two-line big stacked digits (White hours, Sky-Blue minutes) - nicely centered
    drawTwoDigits(32, 60, dispHour, TFT_WHITE);
    drawTwoDigits(32, 114, minute, 0x44DF);

    // 4. Clean date text (e.g. "Tue, Oct 1")
    String formattedDate = formatWatchDate(day, date);
    int dX = 34, dY = 175;
    display.setTextSize(2);
    display.setTextColor(0x0000);
    display.setCursor(dX + 1, dY + 1);
    display.print(formattedDate);
    display.setTextColor(TFT_WHITE);
    display.setCursor(dX, dY);
    display.print(formattedDate);
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
      if (currentScreen != SCREEN_FACE && currentScreen != SCREEN_CARD && currentScreen != SCREEN_MAPS && currentScreen != SCREEN_CLOCK && currentScreen != SCREEN_NOTIFICATIONS && currentScreen != SCREEN_CALENDAR && currentScreen != SCREEN_POMODORO && currentScreen != SCREEN_MENU) {
        drawStatusBar(hour, minute);
      }
      
      switch (currentScreen) {
        case SCREEN_MENU:
          drawAppLauncherMenu(hour, minute, day, date);
          break;
        case SCREEN_CLOCK:
          drawClockScreen(hour, minute, second, day, date, style, is12Hour, touchCount);
          break;
        case SCREEN_SETTINGS:
          drawSettingsMenuLandscape(menuOption, optionSelected, bleActive, gifSpeed, clockStyle, negativeDisplay, oledBrightness);
          break;
        case SCREEN_NOTIFICATIONS:
          drawNotificationPanel(hour, minute);
          break;
        case SCREEN_CALENDAR:
          drawCalendarScreen(hour, minute, date, day);
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
        case SCREEN_POMODORO:
          extern int pomoRemainingSec, pomoTotalSec, pomoState, pomoMode, pomoCompletedSessions;
          drawPomodoroScreen(pomoRemainingSec, pomoTotalSec, pomoState, pomoMode, pomoCompletedSessions, hour, minute, day, date);
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

    // ── Incoming Call Ringing Overlay ──
    if (callRingingActive) {
      int ox = 8;
      int oy = 22;
      int ow = SCREEN_WIDTH - 16;
      int oh = SCREEN_HEIGHT - 32;

      bool pulse = ((millis() / 400) % 2 == 0);
      uint16_t borderCol = pulse ? 0x07E0 : 0x03E0;
      if (callMuted) borderCol = 0xFD20;

      display.fillRoundRect(ox, oy, ow, oh, 14, 0x0841);
      display.drawRoundRect(ox, oy, ow, oh, 14, borderCol);
      display.drawRoundRect(ox + 1, oy + 1, ow - 2, oh - 2, 13, borderCol);

      // Top Status Pill
      uint16_t pillBg = callMuted ? 0x8400 : (pulse ? 0x0520 : 0x03A0);
      display.fillRoundRect(ox + (ow - 140) / 2, oy + 12, 140, 22, 6, pillBg);
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(ox + (ow - 140) / 2 + 16, oy + 19);
      display.print(callMuted ? "CALL MUTED [SILENT]" : "INCOMING CALL...");

      // Animated Phone Icon
      int iconCenterX = ox + ow / 2;
      int iconCenterY = oy + 68;
      int iconR = 24;
      display.fillCircle(iconCenterX, iconCenterY, iconR, callMuted ? 0x4A69 : (pulse ? 0x07E0 : 0x05E0));
      display.drawCircle(iconCenterX, iconCenterY, iconR, TFT_WHITE);
      display.fillRoundRect(iconCenterX - 6, iconCenterY - 12, 12, 24, 4, TFT_WHITE);
      display.fillRect(iconCenterX - 4, iconCenterY - 6, 8, 12, callMuted ? 0x4A69 : (pulse ? 0x07E0 : 0x05E0));

      // Caller Name
      display.setTextSize(2);
      display.setTextColor(TFT_WHITE);
      String dName = callerName;
      if (dName.length() == 0) dName = "Unknown Caller";
      if (dName.length() > 14) dName = dName.substring(0, 12) + "..";
      int nameX = ox + (ow - (dName.length() * 12)) / 2;
      display.setCursor(max(ox + 8, nameX), oy + 108);
      display.print(dName);

      // Subtitle
      display.setTextSize(1);
      display.setTextColor(0x9CD3);
      const char* subTxt = "ODialer / Phone";
      int subX = ox + (ow - strlen(subTxt) * 6) / 2;
      display.setCursor(subX, oy + 134);
      display.print(subTxt);

      // Action Buttons
      int btnW = 98;
      int btnH = 50;
      int btnY = oy + oh - 64;

      // Left Button: MUTE (Canvas: X: 16..114, Y: 206..256)
      int muteX = ox + 8;
      uint16_t muteBg = callMuted ? 0x3186 : 0x7BC0;
      display.fillRoundRect(muteX, btnY, btnW, btnH, 10, muteBg);
      display.drawRoundRect(muteX, btnY, btnW, btnH, 10, callMuted ? 0x6B4D : 0xFD20);
      display.setTextSize(2);
      display.setTextColor(TFT_WHITE);
      display.setCursor(muteX + (callMuted ? 18 : 24), btnY + 10);
      display.print(callMuted ? "MUTED" : "MUTE");
      display.setTextSize(1);
      display.setTextColor(callMuted ? 0xCE79 : 0xFFE0);
      display.setCursor(muteX + 16, btnY + 34);
      display.print(callMuted ? "SILENCED" : "SILENCE RING");

      // Right Button: CUT / REJECT (Canvas: X: 126..224, Y: 206..256)
      int cutX = ox + ow - 8 - btnW;
      display.fillRoundRect(cutX, btnY, btnW, btnH, 10, 0xC800);
      display.drawRoundRect(cutX, btnY, btnW, btnH, 10, 0xF980);
      display.setTextSize(2);
      display.setTextColor(TFT_WHITE);
      display.setCursor(cutX + 28, btnY + 10);
      display.print("CUT");
      display.setTextSize(1);
      display.setTextColor(0xFDF7);
      display.setCursor(cutX + 14, btnY + 34);
      display.print("DECLINE CALL");
    }

    // ── WhatsApp Quick Reply Sheet Overlay ──
    if (quickReplyActive) {
      int ox = 8;
      int oy = 14;
      int ow = SCREEN_WIDTH - 16;
      int oh = SCREEN_HEIGHT - 26;

      display.fillRoundRect(ox, oy, ow, oh, 12, 0x0841);
      display.drawRoundRect(ox, oy, ow, oh, 12, 0x07E0);
      display.drawRoundRect(ox + 1, oy + 1, ow - 2, oh - 2, 11, 0x07E0);

      // Header Badge
      display.fillRoundRect(ox + 10, oy + 8, ow - 20, 22, 5, 0x0BE4);
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(ox + 18, oy + 15);
      display.print("WHATSAPP QUICK REPLY");

      // Subtitle
      display.setTextSize(1);
      display.setTextColor(0x9CD3);
      display.setCursor(ox + 14, oy + 36);
      display.print("Tap preset to send:");

      // 5 Preset Buttons
      int startY = oy + 50;
      int itemH = 28;
      int itemGap = 5;
      for (int i = 0; i < 5; i++) {
        int itemY = startY + i * (itemH + itemGap);
        display.fillRoundRect(ox + 10, itemY, ow - 20, itemH, 6, 0x18C3);
        display.drawRoundRect(ox + 10, itemY, ow - 20, itemH, 6, 0x3A68);

        display.setTextSize(1);
        display.setTextColor(TFT_WHITE);
        display.setCursor(ox + 18, itemY + 10);
        display.print(getQuickReplyPreset(i));

        display.setTextColor(0x07E0);
        display.setCursor(ox + ow - 24, itemY + 10);
        display.print(">");
      }

      // Cancel Button at bottom
      int cancelY = startY + 5 * (itemH + itemGap) + 4;
      int cancelH = 26;
      display.fillRoundRect(ox + 10, cancelY, ow - 20, cancelH, 6, 0x3186);
      display.drawRoundRect(ox + 10, cancelY, ow - 20, cancelH, 6, 0x6B4D);
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      const char* cTxt = "[ CANCEL ]";
      int cW = strlen(cTxt) * 6;
      display.setCursor(ox + (ow - cW) / 2, cancelY + 9);
      display.print(cTxt);
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
