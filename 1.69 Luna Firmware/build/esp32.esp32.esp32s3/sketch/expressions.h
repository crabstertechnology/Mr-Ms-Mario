#line 1 "W:\\Mr.mario\\1.69 Luna Firmware\\expressions.h"
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
    String type;
    String timeStr;
    String title;
    bool active;
  };
  CalendarEventItem calendarEvents[5];
  int calendarEventCount;
  int currentCalViewIdx;
  bool showCalendarGrid;

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

    for (int i = 0; i < 5; i++) {
      notificationHistory[i].active = false;
      calendarEvents[i].active = false;
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
  void addCalendarEvent(String type, String timeStr, String title) {
    for (int i = 4; i > 0; i--) {
      calendarEvents[i] = calendarEvents[i - 1];
    }
    calendarEvents[0].type = type;
    calendarEvents[0].timeStr = timeStr;
    calendarEvents[0].title = title;
    calendarEvents[0].active = true;
    
    if (calendarEventCount < 5) {
      calendarEventCount++;
    }
    currentCalViewIdx = 0;
  }

  void cycleCalendarView() {
    if (!showCalendarGrid && calendarEventCount > 0) {
      currentCalViewIdx = (currentCalViewIdx + 1) % calendarEventCount;
    }
  }

  void toggleCalendarMode() {
    showCalendarGrid = !showCalendarGrid;
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

  // ------------------ Theme System ------------------
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
      // Light Theme (Default)
      t.bg      = TFT_WHITE;
      t.text    = 0x2104; // Premium Charcoal Black
      t.accent  = (robotVariant == "mr_luna") ? 0x197A : 0xF8B8; // Royal Blue or Luna Pink
      t.cardBg  = 0xF7BE; // Soft Pastel Gray/White
      t.border  = 0xD69A; // Sleek Light Grey
      t.subText = 0x7BEF; // Muted Dark Grey
    } else {
      // Dark Theme (Inverted)
      t.bg      = TFT_BLACK;
      t.text    = TFT_WHITE;
      t.accent  = (robotVariant == "mr_luna") ? 0x07FF : 0xF8B8; // Cyan or Pink
      t.cardBg  = 0x0842;
      t.border  = 0x2104;
      t.subText = 0x7BCF;
    }
    return t;
  }

  // ------------------ Smartwatch UI Drawing Methods ------------------
  
  void drawStatusBar(int hour, int minute) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent = theme.accent;
    uint16_t themeBg     = theme.bg;
    uint16_t themeText   = theme.text;
    uint16_t themeCardBg = theme.cardBg;
    uint16_t themeBorder = theme.border;
    uint16_t themeSubText = theme.subText;

    // ── Background square bar running end-to-end ──────────────────────────
    display.fillRect(0, 0, SCREEN_WIDTH, 24, themeBg);
    display.drawFastHLine(0, 24, SCREEN_WIDTH, themeBorder);

    // ── Left zone: HH:MM ─────────────
    display.setTextSize(2);
    display.setTextColor(themeText);
    char tBuf[6];
    snprintf(tBuf, sizeof(tBuf), "%02d:%02d", hour, minute);
    display.setCursor(16, 4);
    display.print(tBuf);

    // ── Right zone: battery and connectivity ─────────
    int bx = SCREEN_WIDTH - 36;

    
    // Draw battery outline
    display.drawRect(bx, 6, 20, 12, themeText);
    display.fillRect(bx + 20, 9, 2, 6, themeText);
    
    // Calculate battery percentage
    int batteryPct = getBatteryPercentage(batteryVolts);

    
    int fillWidth = (batteryPct * 16) / 100;
    uint16_t batteryColor = 0x07E0; // Neon Green
    if (batteryPct < 20) {
      batteryColor = 0xF800; // Bright Red
    } else if (batteryPct < 55) {
      batteryColor = 0xFFE0; // Bright Yellow
    }
    if (fillWidth > 0) {
      display.fillRect(bx + 2, 8, fillWidth, 8, batteryColor);
    }

    display.setTextColor(themeText);
    display.setTextSize(2);
    String pctStr = String(batteryPct) + "%";
    int pctStrW = pctStr.length() * 12;
    display.setCursor(bx - 6 - pctStrW, 4);
    display.print(pctStr);

    // BLE Icon (Vector line drawing instead of simple circle)
    int bleX = bx - 14 - pctStrW;
    int bleY = 12;
    uint16_t bleColor = bleConnectedStatus ? (negativeDisplay ? 0x07FF : 0x197A) : themeBorder; // Bright Cyan/Royal Blue or border
    if (bleConnectedStatus) {
      display.drawLine(bleX, bleY - 5, bleX, bleY + 5, bleColor);
      display.drawLine(bleX, bleY - 5, bleX + 3, bleY - 2, bleColor);
      display.drawLine(bleX + 3, bleY - 2, bleX - 2, bleY + 2, bleColor);
      display.drawLine(bleX - 2, bleY - 2, bleX + 3, bleY + 2, bleColor);
      display.drawLine(bleX + 3, bleY + 2, bleX, bleY + 5, bleColor);
    } else {
      display.drawCircle(bleX, bleY, 2, bleColor);
    }

    // WiFi Icon (cellular/signal bars style instead of dot)
    int wifiX = bx - 26 - pctStrW;
    int wifiY = 9;
    uint16_t wifiColor = wifiConnectedStatus ? 0x07E0 : themeBorder; // Bright Green or border
    display.fillRect(wifiX,     wifiY + 4, 2, 2, wifiColor);
    display.fillRect(wifiX + 3, wifiY + 2, 2, 4, wifiColor);
    display.fillRect(wifiX + 6, wifiY,     2, 6, wifiColor);

    // Silent mode status indicator in status bar
    if (silentMode) {
      int silentX = wifiX - 14;
      int silentY = wifiY + 3;
      uint16_t silentColor = 0xF800; // Red indicator
      // Speaker body
      display.fillRect(silentX, silentY - 2, 2, 4, silentColor);
      display.fillTriangle(silentX + 2, silentY - 4, silentX + 2, silentY + 4, silentX + 4, silentY, silentColor);
      // Small line/cross representing mute
      display.drawLine(silentX + 6, silentY - 2, silentX + 8, silentY, silentColor);
      display.drawLine(silentX + 8, silentY - 2, silentX + 6, silentY, silentColor);
    }

    // ── Centre zone: screen name ─────
    display.setTextColor(themeAccent);
    display.setTextSize(1);
    const char* nm = "LUNA";
    switch (currentScreen) {
      case SCREEN_CLOCK:         nm = "CLOCK";     break;
      case SCREEN_NOTIFICATIONS: nm = "NOTIFS";    break;
      case SCREEN_CALENDAR:      nm = "CAL";       break;
      case SCREEN_GAMES:         nm = "ARCADE";    break;
      case SCREEN_FACE:          nm = "FACE";      break;
      case SCREEN_MAPS:          nm = "MAPS";      break;
      case SCREEN_CARD:          nm = "MY CARD";   break;
      case SCREEN_SETTINGS:      nm = "SETTINGS";  break;
      case SCREEN_LEVEL:         nm = "LEVEL";     break;
      case SCREEN_POMODORO:      nm = "POMO";      break;
      default:                   nm = "LUNA";      break;

    }
    int nmLen  = strlen(nm) * 6;
    int leftEdge  = 70;
    int rightEdge = bx - 30 - pctStrW;
    int nmX = leftEdge + (rightEdge - leftEdge - nmLen) / 2;
    if (nmX < leftEdge) nmX = leftEdge;
    if (nmX + nmLen > rightEdge) nmX = rightEdge - nmLen;
    
    // Draw capsule bg for title
    display.fillRoundRect(nmX - 6, 4, nmLen + 12, 16, 4, themeCardBg);
    display.drawRoundRect(nmX - 6, 4, nmLen + 12, 16, 4, themeBorder);
    
    display.setCursor(nmX, 8);
    display.print(nm);
  }

  void drawPopup() {
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104; // Charcoal/black
    uint16_t themeCardBg = (robotVariant == "mr_luna") ? 0xE7FC : 0xFDF2; // Light Pastel
    uint16_t themeBorder = 0xD69A; // Light Grey

    uint16_t LUNA_CYAN   = themeAccent;
    uint16_t LUNA_PINK   = 0xF8B8;
    uint16_t LUNA_DARK   = themeCardBg;
    uint16_t LUNA_GLASS  = themeBorder;

    // 1. Premium Card Container
    display.drawRoundRect(4, 4, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 8, 12, LUNA_CYAN);
    display.drawRoundRect(5, 5, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10, 11, LUNA_PINK);
    display.fillRoundRect(8, 8, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 16, 9, LUNA_DARK);

    // 2. Cute Header Bar (Pink Heart Mascot)
    int hx = 24, hy = 22;
    display.fillCircle(hx - 2, hy, 3, LUNA_PINK);
    display.fillCircle(hx + 2, hy, 3, LUNA_PINK);
    display.fillTriangle(hx - 5, hy + 1, hx + 5, hy + 1, hx, hy + 6, LUNA_PINK);

    // Title Capsule
    display.fillRoundRect(36, 13, 100, 18, 9, LUNA_GLASS);
    display.setTextColor(themeText);
    display.setTextSize(1);
    display.setCursor(44, 18);
    display.print("NEW ALERT  *");

    display.drawFastHLine(12, 38, SCREEN_WIDTH - 24, LUNA_GLASS);
    
    // 3. Title & Content
    display.setTextColor(themeText);
    display.setTextSize(2);
    display.setCursor(16, 48);
    String title = popupTitle;
    if (title.length() > 16) title = title.substring(0, 14) + "...";
    display.print(title);
    
    display.setTextColor(themeText);
    display.setTextSize(2);
    int yStart = 72;
    int charsPerLine = (SCREEN_WIDTH - 32) / 12;
    int line = 0;
    int maxLines = (SCREEN_HEIGHT - 110) / 20;
    if (maxLines < 3) maxLines = 3;
    for (unsigned int i = 0; i < popupBody.length() && line < maxLines; i += charsPerLine) {
      unsigned int endIdx = i + charsPerLine;
      if (endIdx > popupBody.length()) endIdx = popupBody.length();
      String lineStr = popupBody.substring(i, endIdx);
      display.setCursor(16, yStart + line * 20);
      display.print(lineStr);
      line++;
    }
    
    // 4. Dismiss indicator
    display.setTextColor(themeAccent);
    display.setTextSize(1);
    display.setCursor((SCREEN_WIDTH - 96) / 2, SCREEN_HEIGHT - 22);
    display.print("[Tap to Dismiss]");
  }

  void drawNotificationPanel() {
    ThemeColors theme = getTheme();
    uint16_t themeAccent = theme.accent;
    uint16_t themeBg     = theme.bg;
    uint16_t themeText   = theme.text;
    uint16_t themeCardBg = theme.cardBg;
    uint16_t themeBorder = theme.border;
    uint16_t themeSubText = theme.subText;

    // Clear display below the status bar
    display.fillRect(0, 24, SCREEN_WIDTH, SCREEN_HEIGHT - 24, themeBg);

    if (notificationCount == 0) {
      // Sleek moon and sleep animation
      int centerX = SCREEN_WIDTH / 2;
      
      // Crescent Moon
      uint16_t moonColor = !negativeDisplay ? themeAccent : 0xFFE0; // Accent in light, Yellow in dark
      display.fillCircle(centerX - 10, 70, 20, moonColor);
      display.fillCircle(centerX - 16, 70, 20, themeBg); // Shadow
      
      // Floating Zzz
      display.setTextColor(themeAccent);
      display.setTextSize(2);
      display.setCursor(centerX + 16, 44);
      display.print("Z");
      display.setTextSize(1);
      display.setCursor(centerX + 28, 36);
      display.print("z");
      display.setCursor(centerX + 36, 48);
      display.print("z");
      
      display.setTextColor(themeText);
      display.setTextSize(2);
      int lblW1 = 11 * 12;
      display.setCursor((SCREEN_WIDTH - lblW1) / 2, 116);
      display.print("INBOX CLEAR");
      
      display.setTextColor(themeSubText);
      display.setTextSize(1);
      int lblW2 = 20 * 6;
      display.setCursor((SCREEN_WIDTH - lblW2) / 2, 142);
      display.print("No new notifications");
      return;
    }
    
    // Draw Detail View or List View depending on selection
    if (notificationSelected) {
      NotificationItem& notif = notificationHistory[currentNotifViewIdx];
      
      // Glowing Card Container
      display.drawRoundRect(6, 26, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 32, 10, themeAccent);
      display.fillRoundRect(8, 28, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 36, 8, themeCardBg);
      
      // Header
      display.setTextColor(themeText);
      display.setTextSize(2);
      display.setCursor(14, 36);
      String title = notif.title;
      if (title.length() > 10) title = title.substring(0, 8) + "...";
      display.print(title);
      
      display.setTextColor(themeAccent);
      display.setTextSize(2);
      display.setCursor(SCREEN_WIDTH - 76, 36);
      display.print(notif.timeStr);
      
      display.drawFastHLine(12, 56, SCREEN_WIDTH - 24, themeBorder);
      
      // Body Text
      display.setTextColor(themeText);
      display.setTextSize(2);
      int yStart = 68;
      int charsPerLine = (SCREEN_WIDTH - 28) / 12;
      int line = 0;
      int maxLines = (SCREEN_HEIGHT - 110) / 20;
      for (unsigned int i = 0; i < notif.body.length() && line < maxLines; i += charsPerLine) {
        unsigned int endIdx = i + charsPerLine;
        if (endIdx > notif.body.length()) endIdx = notif.body.length();
        String lineStr = notif.body.substring(i, endIdx);
        display.setCursor(14, yStart + line * 20);
        display.print(lineStr);
        line++;
      }
      
      // Indicator
      display.setTextColor(themeAccent);
      display.setTextSize(1);
      char footerBuf[16];
      snprintf(footerBuf, sizeof(footerBuf), "[%d / %d]", currentNotifViewIdx + 1, notificationCount);
      int footerW = strlen(footerBuf) * 6;
      display.setCursor((SCREEN_WIDTH - footerW) / 2, SCREEN_HEIGHT - 20);
      display.print(footerBuf);
    } else {
      // List View: Draw list of up to 5 stored notifications
      display.setTextColor(themeText);
      display.setTextSize(2);
      display.setCursor(14, 30);
      display.print("Notifications");
      
      display.drawFastHLine(12, 50, SCREEN_WIDTH - 24, themeBorder);
      
      for (int i = 0; i < notificationCount && i < 5; i++) {
        int y = 54 + i * 36;
        NotificationItem& notif = notificationHistory[i];
        
        bool isSel = (notificationsActive && i == currentNotifViewIdx);
        
        // Card bg and border
        display.fillRoundRect(10, y, SCREEN_WIDTH - 20, 32, 6, isSel ? 0x10A2 : themeCardBg);
        display.drawRoundRect(10, y, SCREEN_WIDTH - 20, 32, 6, isSel ? themeAccent : themeBorder);
        
        if (isSel) {
          display.fillRect(10, y + 4, 4, 24, themeAccent); // Premium left accent bar
        }
        
        // Title/Sender text
        display.setCursor(isSel ? 22 : 18, y + 3);
        display.setTextSize(2);
        display.setTextColor(themeText);
        String shortTitle = notif.title;
        if (shortTitle.length() > 11) shortTitle = shortTitle.substring(0, 9) + "..";
        display.print(shortTitle);
        
        // Time text
        display.setCursor(SCREEN_WIDTH - 55, y + 3);
        display.setTextSize(1);
        display.setTextColor(themeAccent);
        display.print(notif.timeStr);
        
        // Body snippet text
        display.setCursor(isSel ? 22 : 18, y + 18);
        display.setTextSize(1);
        display.setTextColor(themeSubText);
        String snippet = notif.body;
        if (snippet.length() > 28) snippet = snippet.substring(0, 26) + "...";
        display.print(snippet);
      }
      
      // Bottom Tip / Footer
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      const char* tip = "Tap to Read | Swipe to Scroll";
      int tipW = strlen(tip) * 6;
      display.setCursor((SCREEN_WIDTH - tipW) / 2, SCREEN_HEIGHT - 20);
      display.print(tip);

    }
  }

  void drawCalendarEvents() {
    ThemeColors theme = getTheme();
    uint16_t themeAccent = theme.accent;
    uint16_t themeBg     = theme.bg;
    uint16_t themeText   = theme.text;
    uint16_t themeCardBg = theme.cardBg;
    uint16_t themeBorder = theme.border;
    uint16_t themeSubText = theme.subText;

    // Clear display below the status bar
    display.fillRect(0, 24, SCREEN_WIDTH, SCREEN_HEIGHT - 24, themeBg);

    // Curved border container
    display.drawRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 12, themeAccent);
    display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 10, themeCardBg);

    if (calendarEventCount == 0) {
      int centerX = SCREEN_WIDTH / 2;
      display.drawRect(centerX - 10, 62, 20, 20, themeSubText);
      display.drawFastHLine(centerX - 10, 68, 20, themeSubText);
      display.fillRect(centerX - 6, 58, 2, 6, themeSubText);
      display.fillRect(centerX + 4, 58, 2, 6, themeSubText);
      
      display.setTextColor(themeSubText);
      display.setTextSize(2);
      int lblW1 = 9 * 12;
      display.setCursor((SCREEN_WIDTH - lblW1) / 2, 98);
      display.print("No Events");
      display.setTextSize(1);
      int lblW2 = 19 * 6;
      display.setCursor((SCREEN_WIDTH - lblW2) / 2, 130);
      display.print("Sync events via BLE");
      return;
    }
    
    CalendarEventItem& ev = calendarEvents[currentCalViewIdx];
    
    if (ev.type.indexOf("birthday") >= 0 || ev.type.indexOf("bday") >= 0) {
      display.fillRoundRect(16, 38, 90, 20, 6, 0xF97F); // Pink label
      display.setTextColor(TFT_WHITE);
      display.setTextSize(1);
      display.setCursor(22, 44);
      display.print("BIRTHDAY");
    } else {
      display.fillRoundRect(16, 38, 90, 20, 6, 0xFFE0); // Yellow label
      display.setTextColor(TFT_BLACK);
      display.setTextSize(1);
      display.setCursor(22, 44);
      display.print("MEETING");
    }
    
    display.setTextColor(themeAccent);
    display.setTextSize(2);
    int timeW = ev.timeStr.length() * 12;
    display.setCursor(SCREEN_WIDTH - 16 - timeW, 40);
    display.print(ev.timeStr);
    
    display.drawFastHLine(12, 66, SCREEN_WIDTH - 24, themeBorder);
    
    display.setTextColor(themeText);
    display.setTextSize(2);
    int yStart = 76;
    int charsPerLine = (SCREEN_WIDTH - 32) / 12;
    int line = 0;
    int maxLines = (SCREEN_HEIGHT - 120) / 20;
    for (unsigned int i = 0; i < ev.title.length() && line < maxLines; i += charsPerLine) {
      unsigned int endIdx = i + charsPerLine;
      if (endIdx > ev.title.length()) endIdx = ev.title.length();
      String lineStr = ev.title.substring(i, endIdx);
      display.setCursor(16, yStart + line * 20);
      display.print(lineStr);
      line++;
    }
    
    display.setTextColor(themeSubText);
    display.setTextSize(2);
    char footerBuf[16];
    snprintf(footerBuf, sizeof(footerBuf), "[%d / %d]", currentCalViewIdx + 1, calendarEventCount);
    int footerW = strlen(footerBuf) * 12;
    display.setCursor((SCREEN_WIDTH - footerW) / 2, SCREEN_HEIGHT - 26);
    display.print(footerBuf);
  }

  void parseDateInfo(String dateStr, String dayStr, int& dayOut, int& monthOut, int& yearOut, int& startWeekdayOut, int& daysInMonthOut) {
    dayOut = 12;
    monthOut = 7;
    yearOut = 2026;
    
    dateStr.trim();
    int spaceIdx = dateStr.indexOf(' ');
    if (spaceIdx > 0) {
      dayOut = dateStr.substring(0, spaceIdx).toInt();
      if (dayOut <= 0) dayOut = 12;
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
    if (monthOut == 2) {
      if ((yearOut % 4 == 0 && yearOut % 100 != 0) || (yearOut % 400 == 0)) {
        daysInMonthOut = 29;
      } else {
        daysInMonthOut = 28;
      }
    } else if (monthOut >= 1 && monthOut <= 12) {
      daysInMonthOut = daysPerMonth[monthOut];
    } else {
      daysInMonthOut = 30;
    }
    
    int currentWeekday = 0;
    dayStr.trim();
    dayStr.toUpperCase();
    if (dayStr.startsWith("SUN")) currentWeekday = 0;
    else if (dayStr.startsWith("MON")) currentWeekday = 1;
    else if (dayStr.startsWith("TUE")) currentWeekday = 2;
    else if (dayStr.startsWith("WED")) currentWeekday = 3;
    else if (dayStr.startsWith("THU")) currentWeekday = 4;
    else if (dayStr.startsWith("FRI")) currentWeekday = 5;
    else if (dayStr.startsWith("SAT")) currentWeekday = 6;
    
    startWeekdayOut = (currentWeekday - (dayOut - 1) % 7 + 7) % 7;
  }

  void drawCalendarGrid(String dateStr, String dayStr) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent = theme.accent;
    uint16_t themeBg     = theme.bg;
    uint16_t themeText   = theme.text;
    uint16_t themeCardBg = theme.cardBg;
    uint16_t themeBorder = theme.border;
    uint16_t themeSubText = theme.subText;

    int curDay, curMonth, curYear, startWeekday, daysInMonth;
    parseDateInfo(dateStr, dayStr, curDay, curMonth, curYear, startWeekday, daysInMonth);
    
    // Clear display below status bar
    display.fillRect(0, 24, SCREEN_WIDTH, SCREEN_HEIGHT - 24, themeBg);

    // Draw Curved Border
    display.drawRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 12, themeAccent);
    display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 10, themeCardBg);
    
    // Month / Year header — size 2
    display.setTextSize(2);
    display.setTextColor(themeText);
    char headerBuf[32];
    const char* monthNames[] = { "", "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER" };
    snprintf(headerBuf, sizeof(headerBuf), "%s %d", (curMonth >= 1 && curMonth <= 12) ? monthNames[curMonth] : "JULY", curYear);
    int headerW = strlen(headerBuf) * 12;
    display.setCursor((SCREEN_WIDTH - headerW) / 2, 38);
    display.print(headerBuf);

    // Day-of-week header
    int colWidth = 31;
    int startX = 12;
    int startY = 60;
    display.setTextColor(themeAccent);
    display.setTextSize(2);
    const char* dayLabels[] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };
    for (int i = 0; i < 7; i++) {
      display.setCursor(startX + i * colWidth + 4, startY);
      display.print(dayLabels[i]);
    }
    display.drawFastHLine(8, startY + 16, SCREEN_WIDTH - 16, themeBorder);
    
    int col = startWeekday;
    int row = 0;
    int rowHeight = 22;
    
    for (int d = 1; d <= daysInMonth; d++) {
      int x = startX + col * colWidth;
      int y = startY + 22 + row * rowHeight;

      if (d == curDay) {
        display.fillRoundRect(x + 2, y - 2, 26, 18, 5, themeAccent); // Bold accent pill for today
        display.setTextColor(TFT_WHITE);
      } else {
        display.setTextColor(themeText);
      }

      display.setCursor(d < 10 ? x + 10 : x + 4, y);
      display.print(d);

      col++;
      if (col >= 7) {
        col = 0;
        row++;
      }
    }

    // Bottom hint
    display.setTextSize(1);
    display.setTextColor(themeSubText);
    const char* calTip = "Tap: View Agenda | Swipe: Scroll";
    int calTipW = strlen(calTip) * 6;
    display.setCursor((SCREEN_WIDTH - calTipW) / 2, SCREEN_HEIGHT - 20);
    display.print(calTip);
  }

  void drawSettingsMenuLandscape(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent = theme.accent;
    uint16_t themeBg     = theme.bg;
    uint16_t themeText   = theme.text;
    uint16_t themeCardBg = theme.cardBg;
    uint16_t themeBorder = theme.border;
    uint16_t themeSubText = theme.subText;

    // Clear display below the status bar
    display.fillRect(0, 24, SCREEN_WIDTH, SCREEN_HEIGHT - 24, themeBg);

    // Clean border
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 12, themeAccent);
    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, themeBg);

    // Header label
    display.setTextSize(2);
    display.setTextColor(themeAccent);
    display.setCursor(72, 34);
    display.print("SETTINGS");
    display.drawFastHLine(12, 52, SCREEN_WIDTH - 24, themeBorder);

    display.setTextSize(2);
    int itemsPerPage = 4;
    int scrollOffset = 0;
    if (option >= itemsPerPage) {
      scrollOffset = option - itemsPerPage + 1;
    }

    int itemHeight = 44; // Taller rows, inspired by S3Watch list layout

    for (int pageIdx = 0; pageIdx < itemsPerPage; pageIdx++) {
      int optIdx = pageIdx + scrollOffset;
      if (optIdx >= 8) break;

      int yPos = 58 + pageIdx * itemHeight;

      bool isCurrent = (option == optIdx) && settingsActive;
      
      // Select box colors
      uint16_t boxBg = selected ? themeAccent : themeCardBg;
      uint16_t boxText = selected ? TFT_WHITE : themeText;
      uint16_t itemAccent = isCurrent ? boxText : themeAccent;

      if (isCurrent) {
        display.fillRoundRect(10, yPos, SCREEN_WIDTH - 20, 38, 10, boxBg);
        display.drawRoundRect(10, yPos, SCREEN_WIDTH - 20, 38, 10, themeAccent);
        display.setTextColor(boxText);
      } else {
        display.fillRoundRect(10, yPos, SCREEN_WIDTH - 20, 38, 10, themeCardBg);
        display.setTextColor(themeText);
      }

      // Draw vector icon on the left
      int iconX = 18;
      int textX = 34;
      
      switch (optIdx) {
        case 0: // BLE
          {
            int ix = iconX + 4;
            int iy = yPos + 18;
            display.drawLine(ix, iy - 6, ix, iy + 6, itemAccent);
            display.drawLine(ix, iy - 6, ix + 3, iy - 3, itemAccent);
            display.drawLine(ix + 3, iy - 3, ix - 3, iy + 3, itemAccent);
            display.drawLine(ix - 3, iy - 3, ix + 3, iy + 3, itemAccent);
            display.drawLine(ix + 3, iy + 3, ix, iy + 6, itemAccent);
          }
          break;
        case 1: // Speed
          {
            int ix = iconX + 4;
            int iy = yPos + 18;
            display.drawCircle(ix, iy, 6, itemAccent);
            display.drawLine(ix, iy, ix + 4, iy - 3, itemAccent);
          }
          break;
        case 2: // Clock Style
          {
            int ix = iconX + 4;
            int iy = yPos + 18;
            display.drawRoundRect(ix - 4, iy - 6, 9, 13, 2, itemAccent);
            display.drawCircle(ix, iy, 3, itemAccent);
          }
          break;
        case 3: // Invert Screen
          {
            int ix = iconX + 4;
            int iy = yPos + 18;
            display.drawCircle(ix, iy, 6, itemAccent);
            display.fillRect(ix - 5, iy - 5, 5, 11, itemAccent);
          }
          break;
        case 4: // Brightness
          {
            int ix = iconX + 4;
            int iy = yPos + 18;
            display.drawCircle(ix, iy, 3, itemAccent);
            display.drawFastVLine(ix, iy - 6, 2, itemAccent);
            display.drawFastVLine(ix, iy + 5, 2, itemAccent);
            display.drawFastHLine(ix - 6, iy, 2, itemAccent);
            display.drawFastHLine(ix + 5, iy, 2, itemAccent);
          }
          break;
        case 5: // Buzzer Sound
          {
            int ix = iconX + 1;
            int iy = yPos + 18;
            display.fillRect(ix, iy - 3, 3, 7, itemAccent);
            display.drawLine(ix + 3, iy - 3, ix + 6, iy - 6, itemAccent);
            display.drawLine(ix + 6, iy - 6, ix + 6, iy + 6, itemAccent);
            display.drawLine(ix + 6, iy + 6, ix + 3, iy + 3, itemAccent);
          }
          break;
        case 6: // Save Settings
          {
            int ix = iconX + 4;
            int iy = yPos + 18;
            display.drawRect(ix - 5, iy - 5, 11, 11, itemAccent);
            display.fillRect(ix - 3, iy - 5, 6, 4, itemAccent);
            display.fillRect(ix - 2, iy + 2, 4, 3, itemAccent);
          }
          break;
        case 7: // Exit Menu
          {
            int ix = iconX + 4;
            int iy = yPos + 18;
            display.drawLine(ix - 5, iy, ix + 5, iy, itemAccent);
            display.drawLine(ix - 5, iy, ix - 2, iy - 3, itemAccent);
            display.drawLine(ix - 5, iy, ix - 2, iy + 3, itemAccent);
          }
          break;
      }

      display.setCursor(textX, yPos + 11);
      
      uint16_t activeSwitchColor = !negativeDisplay ? 0x03E0 : 0x07E0;
      uint16_t activeBrightnessColor = !negativeDisplay ? 0xD560 : 0xFFE0;

      switch (optIdx) {
        case 0:
          display.print("BLE Connected");
          {
            int sx = SCREEN_WIDTH - 50;
            int sy = yPos + 11;
            display.drawRoundRect(sx, sy, 32, 16, 8, isCurrent ? boxText : themeBorder);
            if (bleOn) {
              display.fillRoundRect(sx, sy, 32, 16, 8, activeSwitchColor); // Green ON
              display.fillCircle(sx + 24, sy + 8, 6, TFT_WHITE);
            } else {
              display.fillCircle(sx + 8, sy + 8, 6, themeSubText);
            }
          }
          break;
        case 1:
          display.print("Speed");
          {
            String val = String(speed) + "ms";
            display.setCursor(SCREEN_WIDTH - 24 - (val.length() * 12), yPos + 11);
            display.print(val);
          }
          break;
        case 2:
          display.print("Clock Style");
          {
            String val = String(clockStyle);
            display.setCursor(SCREEN_WIDTH - 24 - (val.length() * 12), yPos + 11);
            display.print(val);
          }
          break;
        case 3:
          display.print("Invert Color");
          {
            int sx = SCREEN_WIDTH - 50;
            int sy = yPos + 11;
            display.drawRoundRect(sx, sy, 32, 16, 8, isCurrent ? boxText : themeBorder);
            if (invertOn) {
              display.fillRoundRect(sx, sy, 32, 16, 8, activeSwitchColor);
              display.fillCircle(sx + 24, sy + 8, 6, TFT_WHITE);
            } else {
              display.fillCircle(sx + 8, sy + 8, 6, themeSubText);
            }
          }
          break;
        case 4:
          display.print("Brightness");
          {
            int bx = SCREEN_WIDTH - 38;
            int by = yPos + 23;
            for (int b = 0; b < 3; b++) {
              uint16_t col = (brightness > b) ? (isCurrent ? boxText : activeBrightnessColor) : 0x3186; // Yellow/Amber or Gray
              display.fillRect(bx + b * 6, by - (b + 1) * 4, 4, (b + 1) * 4, col);
            }
          }
          break;
        case 5:
          display.print("Buzzer Sound");
          {
            int sx = SCREEN_WIDTH - 50;
            int sy = yPos + 11;
            display.drawRoundRect(sx, sy, 32, 16, 8, isCurrent ? boxText : themeBorder);
            if (!silentMode) {
              display.fillRoundRect(sx, sy, 32, 16, 8, activeSwitchColor);
              display.fillCircle(sx + 24, sy + 8, 6, TFT_WHITE);
            } else {
              display.fillCircle(sx + 8, sy + 8, 6, themeSubText);
            }
          }
          break;
        case 6:
          {
            String val = "SAVE SETTINGS";
            int startX = textX + (SCREEN_WIDTH - 20 - textX - val.length() * 12) / 2;
            display.setCursor(startX, yPos + 11);
            if (!isCurrent) display.setTextColor(activeSwitchColor); // Theme-aware Green text
            display.print(val);
          }
          break;
        case 7:
          {
            String val = "EXIT MENU";
            int startX = textX + (SCREEN_WIDTH - 20 - textX - val.length() * 12) / 2;
            display.setCursor(startX, yPos + 11);
            if (!isCurrent) display.setTextColor(0xF800); // Red text
            display.print(val);
          }
          break;
      }
    }

    if (settingsActive) {
      // Scroll indicator dots at bottom
      int totalItems = 8;
      int dotAreaY = 240; // Moved lower down to accommodate taller rows
      int dotSpacing = 14;
      int dotsStartX = (SCREEN_WIDTH - totalItems * dotSpacing) / 2;
      for (int i = 0; i < totalItems; i++) {
        if (i == option) {
          display.fillRoundRect(dotsStartX + i * dotSpacing, dotAreaY, 8, 4, 2, themeAccent);
        } else {
          display.fillRoundRect(dotsStartX + i * dotSpacing + 1, dotAreaY + 1, 4, 2, 1, themeBorder);
        }
      }
    } else {
      // Hint text
      display.setTextSize(1);
      display.setTextColor(themeSubText);
      String hint = "Tap to Select | Swipe to Scroll";
      int navW = hint.length() * 6;
      display.setCursor((SCREEN_WIDTH - navW) / 2, SCREEN_HEIGHT - 22);
      display.print(hint);
    }
  }

  void drawPomodoroScreen(int remainingSec, int totalSec, int pomoState, int pomoMode, int completedSessions) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent = theme.accent;
    uint16_t themeBg     = theme.bg;
    uint16_t themeText   = theme.text;
    uint16_t themeCardBg = theme.cardBg;
    uint16_t themeBorder = theme.border;
    uint16_t themeSubText = theme.subText;

    // Clear display area below status bar
    display.fillRect(0, 24, SCREEN_WIDTH, SCREEN_HEIGHT - 24, themeBg);

    // Card border container
    display.drawRoundRect(6, 28, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 32, 12, themeAccent);
    display.fillRoundRect(8, 30, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 36, 10, themeCardBg);

    // Top Mode Capsule Pill
    const char* modeTitle = "FOCUS 25M";
    uint16_t modeColor = themeAccent;
    if (pomoMode == 1) {
      modeTitle = "SHORT BREAK 5M";
      modeColor = 0x07E0; // Neon Green
    } else if (pomoMode == 2) {
      modeTitle = "LONG BREAK 15M";
      modeColor = 0x7BF0; // Cyan / Purple
    }

    int modeLen = strlen(modeTitle) * 6;
    int modeX = (SCREEN_WIDTH - modeLen - 16) / 2;
    display.fillRoundRect(modeX, 34, modeLen + 16, 18, 5, modeColor);
    display.setTextColor(TFT_WHITE);
    display.setTextSize(1);
    display.setCursor(modeX + 8, 39);
    display.print(modeTitle);

    // --- Circular Clock Dial Display ---
    int cx = SCREEN_WIDTH / 2;
    int cy = 126;
    int radius = 54;

    // Outer clock face ring
    display.drawCircle(cx, cy, radius, themeBorder);
    display.drawCircle(cx, cy, radius - 1, themeBorder);
    display.drawCircle(cx, cy, radius - 5, themeBorder);
    display.fillCircle(cx, cy, radius - 6, themeBg);

    // 12 Clock Hour Ticks
    for (int i = 0; i < 12; i++) {
      float angle = i * (2.0f * M_PI / 12.0f) - (M_PI / 2.0f);
      int x1 = cx + (int)(cos(angle) * (radius - 5));
      int y1 = cy + (int)(sin(angle) * (radius - 5));
      int x2 = cx + (int)(cos(angle) * (radius - 1));
      int y2 = cy + (int)(sin(angle) * (radius - 1));
      display.drawLine(x1, y1, x2, y2, themeBorder);
    }

    // Radial Progress Arc around Clock Dial
    float progressPct = 0.0f;
    if (totalSec > 0) {
      progressPct = (float)(totalSec - remainingSec) / (float)totalSec;
    }
    progressPct = constrain(progressPct, 0.0f, 1.0f);

    int arcDots = (int)(progressPct * 48.0f);
    for (int s = 0; s < arcDots; s++) {
      float angle = s * (2.0f * M_PI / 48.0f) - (M_PI / 2.0f);
      int px = cx + (int)(cos(angle) * (radius - 3));
      int py = cy + (int)(sin(angle) * (radius - 3));
      display.fillCircle(px, py, 2, modeColor);
    }

    // Countdown Display inside Clock Dial: MM:SS
    int mins = remainingSec / 60;
    int secs = remainingSec % 60;
    char timeBuf[8];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", mins, secs);

    display.setTextSize(3);
    uint16_t timeColor = (pomoState == 3) ? 0x07E0 : ((pomoState == 1) ? themeText : themeSubText);
    display.setTextColor(timeColor);
    int timeW = 5 * 18;
    display.setCursor(cx - timeW / 2, cy - 10);
    display.print(timeBuf);

    // State badge inside clock dial bottom
    const char* stateLabel = "READY";
    if (pomoState == 1)      stateLabel = "RUNNING";
    else if (pomoState == 2) stateLabel = "PAUSED";
    else if (pomoState == 3) stateLabel = "DONE!";
    display.setTextSize(1);
    display.setTextColor(modeColor);
    int slW = strlen(stateLabel) * 6;
    display.setCursor(cx - slW / 2, cy + 18);
    display.print(stateLabel);

    // Action Control Pill Button at Bottom
    int btnW = 120;
    int btnH = 24;
    int btnX = (SCREEN_WIDTH - btnW) / 2;
    int btnY = 194;
    display.fillRoundRect(btnX, btnY, btnW, btnH, 8, modeColor);
    display.setTextColor(TFT_WHITE);
    display.setTextSize(1);

    const char* actText = "START";
    if (pomoState == 1)      actText = "PAUSE";
    else if (pomoState == 2) actText = "RESUME";
    else if (pomoState == 3) actText = "RESET";

    int actW = strlen(actText) * 6;
    display.setCursor(btnX + (btnW - actW) / 2, btnY + 8);
    display.print(actText);

    // Sessions Completed Counter
    char sessBuf[32];
    snprintf(sessBuf, sizeof(sessBuf), "Sessions: %d", completedSessions);
    display.setTextColor(themeSubText);
    int sessW = strlen(sessBuf) * 6;
    display.setCursor((SCREEN_WIDTH - sessW) / 2, 226);
    display.print(sessBuf);

    // Clean Touch Tip
    const char* navTip = "Tap: Start/Pause | Swipe: Mode";
    display.setTextColor(themeSubText);
    int navW = strlen(navTip) * 6;
    display.setCursor((SCREEN_WIDTH - navW) / 2, SCREEN_HEIGHT - 20);
    display.print(navTip);
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
      // Draw live Google Maps screen bitmap frame!
      display.drawRGBBitmap(0, 0, liveMapBuffer, SCREEN_WIDTH, SCREEN_HEIGHT);

      // Google Maps dark green status banner at top (#0F9D58)
      display.fillRect(0, 0, SCREEN_WIDTH, 26, 0x04C0);
      display.drawFastHLine(0, 26, SCREEN_WIDTH, TFT_WHITE);

      display.setTextSize(1);
      display.setTextColor(TFT_WHITE, 0x04C0);
      String topText = "GOOGLE MAPS LIVE";
      if (mapDirection.length() > 0 && mapDirection != "STRAIGHT") {
        topText = mapDirection + " " + mapDistance;
      }
      int txtW = topText.length() * 6;
      display.setCursor((SCREEN_WIDTH - txtW) / 2, 8);
      display.print(topText);
      return;
    }

    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104; // Charcoal/black
    uint16_t themeCardBg = (robotVariant == "mr_luna") ? 0xE7FC : 0xFDF2; // Light Pastel
    uint16_t themeBorder = 0xD69A; // Light Grey
    uint16_t themeSubText = 0x7BCF; // Muted grey

    // Clear display below the status bar
    display.fillRect(0, 24, SCREEN_WIDTH, SCREEN_HEIGHT - 24, themeBg);

    display.setTextWrap(false);

    // ── Background card ───────────────────────────────────────────────
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 12, themeAccent);
    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, themeCardBg);

    // ── Top label: "NAVIGATION" ───────────────────────────────────────
    display.setTextSize(1);
    display.setTextColor(themeText, themeCardBg);
    int navW = 10 * 6;
    display.setCursor((SCREEN_WIDTH - navW) / 2, 35);
    display.print("NAVIGATION");
    display.drawFastHLine(12, 45, SCREEN_WIDTH - 24, themeBorder);

    // ── Arrow area (centered, 60x60px arrow in the middle) ───────────
    int cx = SCREEN_WIDTH / 2;
    int cy = 108;  // vertical center of arrow area

    if (mapDirection.indexOf("LEFT") >= 0) {
      // LEFT arrow: large clear left-pointing arrow
      // Stem: horizontal bar going left from centre
      display.fillRect(cx - 30, cy - 8, 40, 16, themeText);
      // Arrowhead pointing LEFT
      display.fillTriangle(cx - 30, cy,
                           cx - 10, cy - 26,
                           cx - 10, cy + 26, themeText);
      // Small vertical stem going down at the right end (road continues straight then turns)
      display.fillRect(cx + 10, cy - 8, 14, 30, themeText);

    } else if (mapDirection.indexOf("RIGHT") >= 0) {
      // RIGHT arrow: large clear right-pointing arrow
      display.fillRect(cx - 10, cy - 8, 40, 16, themeText);
      // Arrowhead pointing RIGHT
      display.fillTriangle(cx + 30, cy,
                           cx + 10, cy - 26,
                           cx + 10, cy + 26, themeText);
      // Small vertical stem going down at the left end
      display.fillRect(cx - 24, cy - 8, 14, 30, themeText);

    } else if (mapDirection.indexOf("UTURN") >= 0 || mapDirection.indexOf("U-TURN") >= 0) {
      // U-TURN: thick U shape with downward arrow
      display.drawCircle(cx, cy - 14, 22, themeText);
      display.drawCircle(cx, cy - 14, 20, themeText);
      display.drawCircle(cx, cy - 14, 18, themeText);
      // Erase the bottom half of the circles to make a U
      display.fillRect(cx - 30, cy - 14, 60, 40, themeCardBg);
      // Left leg
      display.fillRect(cx - 24, cy - 14, 6, 32, themeText);
      // Right leg with downward arrow at bottom
      display.fillRect(cx + 18, cy - 14, 6, 24, themeText);
      display.fillTriangle(cx + 21, cy + 18,
                           cx + 10, cy + 8,
                           cx + 32, cy + 8, themeText);

    } else if (mapDirection.indexOf("ROUNDABOUT") >= 0 || mapDirection.indexOf("ROUND") >= 0) {
      // ROUNDABOUT: circle with an exit arrow
      display.drawCircle(cx, cy, 22, themeText);
      display.drawCircle(cx, cy, 20, themeText);
      // Fill inside card bg
      display.fillCircle(cx, cy, 17, themeCardBg);
      // Exit arrow pointing up-right
      display.fillRect(cx + 14, cy - 28, 6, 24, themeText);
      display.fillTriangle(cx + 17, cy - 34,
                           cx + 10, cy - 24,
                           cx + 24, cy - 24, themeText);
      // Entry from bottom
      display.fillRect(cx - 6, cy + 14, 12, 16, themeText);

    } else {
      // STRAIGHT: tall upward arrow
      display.fillRect(cx - 8, cy - 20, 16, 44, themeText);
      display.fillTriangle(cx, cy - 40,
                           cx - 22, cy - 20,
                           cx + 22, cy - 20, themeText);
    }

    // ── Direction label text below arrow ────────────────────────────
    display.setTextSize(2);
    display.setTextColor(themeText, themeCardBg);
    String dirLabel = "Go Straight";
    if      (mapDirection.indexOf("LEFT")       >= 0) dirLabel = "Turn Left";
    else if (mapDirection.indexOf("RIGHT")      >= 0) dirLabel = "Turn Right";
    else if (mapDirection.indexOf("UTURN")      >= 0 ||
             mapDirection.indexOf("U-TURN")     >= 0) dirLabel = "Make U-Turn";
    else if (mapDirection.indexOf("ROUNDABOUT") >= 0 ||
             mapDirection.indexOf("ROUND")      >= 0) dirLabel = "Roundabout";
    int lblW = dirLabel.length() * 12;
    display.setCursor((SCREEN_WIDTH - lblW) / 2, 152);
    display.print(dirLabel);

    // ── Bottom info bar ─────────────────────────────────────────────
    display.drawFastHLine(8, 170, SCREEN_WIDTH - 16, themeBorder);
    display.fillRoundRect(6, 172, SCREEN_WIDTH - 12, 50, 8, themeCardBg);
    display.drawRoundRect(6, 172, SCREEN_WIDTH - 12, 50, 8, themeBorder);

    // Distance — left side, large accent
    display.setTextSize(3);
    display.setTextColor(themeAccent, themeCardBg);
    String distStr = (mapDistance == "" || mapDistance == "--") ? "---" : mapDistance;
    display.setCursor(12, 179);
    display.print(distStr);

    // ETA / description — right side, size 2 text
    if (mapDescription != "") {
      display.setTextSize(2);
      display.setTextColor(themeText, themeCardBg);
      int etaW = mapDescription.length() * 12;
      int etaX = SCREEN_WIDTH - 12 - etaW;
      if (etaX < 12) etaX = 12;
      display.setCursor(etaX, 185);
      display.print(mapDescription);
    }
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
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104;
    uint16_t themeCardBg = (robotVariant == "mr_luna") ? 0xE7FC : 0xFDF2;
    uint16_t themeBorder = 0xD69A;
    uint16_t themeSubText = 0x7BCF;

    // Clear display below the status bar
    display.fillRect(0, 24, SCREEN_WIDTH, SCREEN_HEIGHT - 24, themeBg);

    if (style == 0) {
      // Style 0: Cyberpunk Dashboard (Light/Clean version)
      display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 12, themeAccent);
      display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 8, themeCardBg);

      // Horizontal divider line
      display.drawFastHLine(12, SCREEN_HEIGHT / 2 + 10, SCREEN_WIDTH - 24, themeBorder);

      // Large Digital Time
      display.setTextSize(4);
      display.setTextColor(themeText, themeCardBg);
      char timeStr[6];
      if (!timeSynced) {
        snprintf(timeStr, sizeof(timeStr), "--:--");
      } else {
        int dispHour = hour;
        if (is12Hour) {
          dispHour = hour % 12;
          if (dispHour == 0) dispHour = 12;
        }
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispHour, minute);
      }
      int timeW = 5 * 24;
      display.setCursor((SCREEN_WIDTH - timeW) / 2 - 10, SCREEN_HEIGHT / 2 - 28);
      display.print(timeStr);

      // AM/PM or Seconds
      display.setTextSize(1);
      display.setTextColor(themeAccent, themeCardBg);
      if (is12Hour) {
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        display.setCursor((SCREEN_WIDTH - timeW) / 2 + timeW + 4, SCREEN_HEIGHT / 2 - 22);
        display.print(ampm);
      }
      char secStr[16];
      snprintf(secStr, sizeof(secStr), "%02d", second);
      display.setCursor((SCREEN_WIDTH - timeW) / 2 + timeW + 4, SCREEN_HEIGHT / 2 - 10);
      display.print(secStr);

      // Date and Day — truncated to keep inside right margin
      display.setTextSize(2);
      String dayDate = day + " " + date;
      if ((int)dayDate.length() * 12 > SCREEN_WIDTH - 36) {
        dayDate = day;  // fallback to just weekday abbreviation
      }
      display.setTextColor(themeText, themeCardBg);
      display.setCursor(18, SCREEN_HEIGHT / 2 + 18);
      display.print(dayDate);

      // Steps widget — right-aligned, won't overlap day text
      display.setTextColor(themeText, themeCardBg);
      String stepStr = String(steps);
      int stepW = (int)stepStr.length() * 12 + 14; // extra for foot icon
      int stepX = SCREEN_WIDTH - 16 - stepW;
      display.setCursor(stepX + 14, SCREEN_HEIGHT / 2 + 18);
      display.print(stepStr);
      // foot icon dots
      int fx = stepX + 6;
      int fy = SCREEN_HEIGHT / 2 + 24;
      display.fillCircle(fx,     fy - 4, 2, themeAccent);
      display.fillCircle(fx + 4, fy - 2, 2, themeAccent);
      display.fillCircle(fx - 3, fy + 2, 1, themeAccent);

    } else if (style == 1) {
      // Style 1: Minimalist Radial Gauge
      display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 16, themeAccent);
      display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 12, themeBg);

      // Center card
      int cardW = 130;
      int cardH = 46;
      int cardX = (SCREEN_WIDTH - cardW) / 2;
      int cardY = (SCREEN_HEIGHT - cardH) / 2 + 8;
      display.fillRoundRect(cardX, cardY, cardW, cardH, 8, themeCardBg);
      display.drawRoundRect(cardX, cardY, cardW, cardH, 8, themeBorder);

      // Time
      display.setTextSize(3);
      display.setTextColor(themeText, themeCardBg);
      char timeStr[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispHour, minute);
      display.setCursor(cardX + 16, cardY + 12);
      display.print(timeStr);

      display.setTextSize(1);
      display.setTextColor(themeAccent, themeCardBg);
      if (is12Hour) {
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        display.setCursor(cardX + 104, cardY + 14);
        display.print(ampm);
      }
      char secStr[6];
      snprintf(secStr, sizeof(secStr), "%02d", second);
      display.setCursor(cardX + 104, cardY + 24);
      display.print(secStr);

      // Sweeping Ring arc
      int progressWidth = (second * (SCREEN_WIDTH - 48)) / 60;
      display.drawRoundRect(24, 38, SCREEN_WIDTH - 48, 6, 3, themeBorder);
      display.fillRoundRect(24, 38, progressWidth, 6, 3, themeAccent);

      // Date
      display.fillRoundRect(24, SCREEN_HEIGHT - 32, SCREEN_WIDTH - 48, 24, 6, themeCardBg);
      display.drawRoundRect(24, SCREEN_HEIGHT - 32, SCREEN_WIDTH - 48, 24, 6, themeBorder);
      display.setTextSize(2);
      display.setTextColor(themeText, themeCardBg);
      String dStr = day + " " + date;
      int dW = dStr.length() * 12;
      display.setCursor((SCREEN_WIDTH - dW) / 2, SCREEN_HEIGHT - 28);
      display.print(dStr);
    } else if (style == 1) {
      // Style 1: Minimalist Radial Gauge
      display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 16, themeAccent);
      display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 12, themeBg);

      // Center card
      int cardW = 130;
      int cardH = 46;
      int cardX = (SCREEN_WIDTH - cardW) / 2;
      int cardY = (SCREEN_HEIGHT - cardH) / 2 + 8;
      display.fillRoundRect(cardX, cardY, cardW, cardH, 8, themeCardBg);
      display.drawRoundRect(cardX, cardY, cardW, cardH, 8, themeBorder);

      // Time
      display.setTextSize(3);
      display.setTextColor(themeText, themeCardBg);
      char timeStr[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispHour, minute);
      display.setCursor(cardX + 16, cardY + 12);
      display.print(timeStr);

      display.setTextSize(1);
      display.setTextColor(themeAccent, themeCardBg);
      if (is12Hour) {
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        display.setCursor(cardX + 104, cardY + 14);
        display.print(ampm);
      }
      char secStr[6];
      snprintf(secStr, sizeof(secStr), "%02d", second);
      display.setCursor(cardX + 104, cardY + 24);
      display.print(secStr);

      // Sweeping Ring arc
      int progressWidth = (second * (SCREEN_WIDTH - 48)) / 60;
      display.drawRoundRect(24, 38, SCREEN_WIDTH - 48, 6, 3, themeBorder);
      display.fillRoundRect(24, 38, progressWidth, 6, 3, themeAccent);

      // Date
      display.fillRoundRect(24, SCREEN_HEIGHT - 32, SCREEN_WIDTH - 48, 24, 6, themeCardBg);
      display.drawRoundRect(24, SCREEN_HEIGHT - 32, SCREEN_WIDTH - 48, 24, 6, themeBorder);
      display.setTextSize(2);
      display.setTextColor(themeText, themeCardBg);
      String dStr = day + " " + date;
      int dW = dStr.length() * 12;
      display.setCursor((SCREEN_WIDTH - dW) / 2, SCREEN_HEIGHT - 28);
      display.print(dStr);

    } else {
      // Style 2+: Watch OS Grid Clock (clean, no wallpaper overlay)
      display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 12, themeAccent);
      display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 8, themeBg);

      // Grid lines
      int gridSpacing = SCREEN_WIDTH / 5;
      for (int gx = gridSpacing; gx < SCREEN_WIDTH; gx += gridSpacing) {
        display.drawFastVLine(gx, 28, SCREEN_HEIGHT - 28, themeBorder);
      }
      for (int gy = 28; gy < SCREEN_HEIGHT; gy += gridSpacing) {
        display.drawFastHLine(0, gy, SCREEN_WIDTH, themeBorder);
      }

      // Title Card
      display.fillRoundRect(12, 34, 130, 20, 4, themeCardBg);
      display.setTextColor(themeText, themeCardBg);
      display.setTextSize(1);
      display.setCursor(18, 40);
      display.print("WATCH OS v3.0");

      // Time
      display.setTextSize(4);
      display.setTextColor(themeText, themeBg);
      char timeStr2[6];
      int dispHour2 = hour;
      if (is12Hour) {
        dispHour2 = hour % 12;
        if (dispHour2 == 0) dispHour2 = 12;
      }
      snprintf(timeStr2, sizeof(timeStr2), "%d:%02d", dispHour2, minute);
      display.setCursor(14, 64);
      display.print(timeStr2);

      display.setTextSize(2);
      if (is12Hour) {
        display.setTextColor(themeAccent, themeBg);
        display.setCursor(120 + (dispHour2 >= 10 ? 24 : 0), 64);
        display.print((hour >= 12) ? "PM" : "AM");
      }

      // Steps widget
      display.fillRoundRect(14, 110, SCREEN_WIDTH - 28, 28, 6, themeCardBg);
      display.drawRoundRect(14, 110, SCREEN_WIDTH - 28, 28, 6, themeBorder);
      display.setTextColor(themeText, themeCardBg);
      display.setTextSize(2);
      display.setCursor(20, 116);
      display.print("STEPS: ");
      display.print(steps);

      // Date widget
      display.fillRoundRect(14, 146, SCREEN_WIDTH - 28, 28, 6, themeCardBg);
      display.drawRoundRect(14, 146, SCREEN_WIDTH - 28, 28, 6, themeBorder);
      display.setTextColor(themeText, themeCardBg);
      display.setTextSize(2);
      display.setCursor(20, 152);
      display.print("DATE: ");
      display.print(day);
      display.print(", ");
      display.print(date);

      // Flashing block
      display.fillRoundRect(SCREEN_WIDTH - 30, SCREEN_HEIGHT - 30, 12, 12, 3,
                            (second % 2 == 0) ? themeAccent : themeBorder);
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
    uint16_t themeAccent = theme.accent;
    uint16_t themeBg     = theme.bg;
    uint16_t themeText   = theme.text;
    uint16_t themeCardBg = theme.cardBg;
    uint16_t themeBorder = theme.border;
    uint16_t themeSubText = theme.subText;

    // Clear display below the status bar
    display.fillRect(0, 24, SCREEN_WIDTH, SCREEN_HEIGHT - 24, themeBg);

    // Draw main frame border
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, themeAccent);

    // Header title
    display.setTextSize(2);
    display.setTextColor(themeAccent);
    display.setCursor(36, 34);
    display.print("LEVEL ANALYSER");
    display.drawFastHLine(12, 50, SCREEN_WIDTH - 24, themeBorder);

    // Read IMU data (fall back to simulated values if hardware not initialized)
    float ax = 0.0f, ay = 0.0f, az = 1.0f;
    float gx = 0.0f, gy = 0.0f, gz = 0.0f;
    bool hasData = false;

    static float offsetX = 0.0f;
    static float offsetY = 0.0f;
    static float offsetZ = 0.0f;

    if (imu.isInitialized()) {
      hasData = imu.readMotion(ax, ay, az, gx, gy, gz);
      static unsigned long lastPrint = 0;
      if (millis() - lastPrint > 500) {
        Serial.printf("[IMU] Data: ax=%.3f, ay=%.3f, az=%.3f, gx=%.1f, gy=%.1f, gz=%.1f\n", ax, ay, az, gx, gy, gz);
        lastPrint = millis();
      }
    }

    if (!hasData) {
      // Simulation mode — generate beautiful waving values
      float t = millis() / 1000.0f;
      ax = sin(t * 1.5f) * 0.4f;
      ay = cos(t * 1.2f) * 0.3f;
      az = sqrt(max(0.0f, 1.0f - ax*ax - ay*ay));
      gx = cos(t * 2.0f) * 80.0f;
      gy = sin(t * 2.5f) * 60.0f;
      gz = sin(t * 1.0f) * 40.0f;
    }

    // Apply calibration request if requested
    if (calibrateRequest) {
      offsetX = ax;
      offsetY = ay;
      offsetZ = az - 1.0f; // treat current position as flat (1g on Z axis)
      calibrateRequest = false;
      audio.playSound(SOUND_POWERUP);
    }

    // Apply offsets
    ax -= offsetX;
    ay -= offsetY;
    az -= offsetZ;

    // Compute Pitch & Roll in degrees
    float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 57.29578f;
    float roll = atan2(ay, az) * 57.29578f;

    // Center coordinates for Bubble Level
    int cx = 120;
    int cy = 115;
    int maxRadius = 38;

    // Draw Bubble Level Target Crosshair
    display.drawCircle(cx, cy, maxRadius, themeBorder);
    display.drawCircle(cx, cy, 12, themeBorder);
    display.drawFastHLine(cx - maxRadius - 4, cy, (maxRadius + 4) * 2, themeBorder);
    display.drawFastVLine(cx, cy - maxRadius - 4, (maxRadius + 4) * 2, themeBorder);

    // Calculate bubble position by swapping physical X and Y axes to match vertical screen rotation
    int bx = cx - (int)(ay * maxRadius);
    int by = cy - (int)(ax * maxRadius);

    // Constrain bubble within maxRadius boundary
    float dist = sqrt((bx - cx) * (bx - cx) + (by - cy) * (by - cy));
    if (dist > (maxRadius - 6)) {
      float angle = atan2(by - cy, bx - cx);
      bx = cx + (int)(cos(angle) * (maxRadius - 6));
      by = cy + (int)(sin(angle) * (maxRadius - 6));
    }

    // Color code the bubble: Green if perfectly level, else Theme Accent
    uint16_t bubbleColor = (abs(ax) < 0.05f && abs(ay) < 0.05f) ? 0x07E0 : themeAccent;
    display.fillCircle(bx, by, 6, bubbleColor);
    display.drawCircle(bx, by, 6, themeText);

    // ─── CARD 1: PITCH & ROLL ANGLES ───
    display.fillRoundRect(10, 168, SCREEN_WIDTH - 20, 44, 6, themeCardBg);
    display.drawRoundRect(10, 168, SCREEN_WIDTH - 20, 44, 6, themeBorder);

    display.setTextSize(1);
    display.setTextColor(themeSubText);
    display.setCursor(20, 174);
    display.print("PITCH ANGLE");
    display.setCursor(130, 174);
    display.print("ROLL ANGLE");

    display.setTextSize(2);
    display.setTextColor(themeText);
    
    char pitchStr[10];
    char rollStr[10];
    snprintf(pitchStr, sizeof(pitchStr), "%+.1f", pitch);
    snprintf(rollStr, sizeof(rollStr), "%+.1f", roll);
    display.setCursor(20, 188);
    display.print(pitchStr);
    display.print((char)247); // Degree symbol
    
    display.setCursor(130, 188);
    display.print(rollStr);
    display.print((char)247); // Degree symbol

    // ─── CARD 2: GYROSCOPE TELEMETRY ───
    display.fillRoundRect(10, 222, SCREEN_WIDTH - 20, 48, 6, themeCardBg);
    display.drawRoundRect(10, 222, SCREEN_WIDTH - 20, 48, 6, themeBorder);

    // Visualise three axis rates
    int barY = 228;
    int barH = 5;
    int barW = 100;
    int barX = 90;

    uint16_t gyroXColor = !negativeDisplay ? 0xD560 : 0xFFE0; // Amber/Gold or Yellow
    uint16_t gyroZColor = !negativeDisplay ? 0xA014 : 0xF81F; // Rich Violet or Neon Magenta

    // Gyro X Bar
    display.setTextSize(1);
    display.setTextColor(gyroXColor);
    display.setCursor(20, barY - 1);
    display.print("GYRO X");
    
    // Draw horizontal bar (-250 to +250 dps range)
    display.drawRect(barX, barY, barW, barH, themeBorder);
    display.drawFastVLine(barX + barW/2, barY - 1, barH + 2, themeSubText);
    int valWX = (int)(gx / 250.0f * (barW/2));
    if (valWX > barW/2) valWX = barW/2;
    if (valWX < -barW/2) valWX = -barW/2;
    if (valWX >= 0) {
      display.fillRect(barX + barW/2, barY + 1, valWX, barH - 2, gyroXColor);
    } else {
      display.fillRect(barX + barW/2 + valWX, barY + 1, -valWX, barH - 2, gyroXColor);
    }

    // Gyro Y Bar
    barY += 12;
    display.setTextColor(themeAccent);
    display.setCursor(20, barY - 1);
    display.print("GYRO Y");
    display.drawRect(barX, barY, barW, barH, themeBorder);
    display.drawFastVLine(barX + barW/2, barY - 1, barH + 2, themeSubText);
    int valWY = (int)(gy / 250.0f * (barW/2));
    if (valWY > barW/2) valWY = barW/2;
    if (valWY < -barW/2) valWY = -barW/2;
    if (valWY >= 0) {
      display.fillRect(barX + barW/2, barY + 1, valWY, barH - 2, themeAccent);
    } else {
      display.fillRect(barX + barW/2 + valWY, barY + 1, -valWY, barH - 2, themeAccent);
    }

    // Gyro Z Bar
    barY += 12;
    display.setTextColor(gyroZColor);
    display.setCursor(20, barY - 1);
    display.print("GYRO Z");
    display.drawRect(barX, barY, barW, barH, themeBorder);
    display.drawFastVLine(barX + barW/2, barY - 1, barH + 2, themeSubText);
    int valWZ = (int)(gz / 250.0f * (barW/2));
    if (valWZ > barW/2) valWZ = barW/2;
    if (valWZ < -barW/2) valWZ = -barW/2;
    if (valWZ >= 0) {
      display.fillRect(barX + barW/2, barY + 1, valWZ, barH - 2, gyroZColor);
    } else {
      display.fillRect(barX + barW/2 + valWZ, barY + 1, -valWZ, barH - 2, gyroZColor);
    }
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
            uint16_t themeAccent = theme.accent;
            uint16_t themeBg     = theme.bg;
            uint16_t themeText   = theme.text;
            uint16_t themeCardBg = theme.cardBg;
            uint16_t themeBorder = theme.border;
            uint16_t themeSubText = theme.subText;

            // Clear display below the status bar
            display.fillRect(0, 24, SCREEN_WIDTH, SCREEN_HEIGHT - 24, themeBg);

            // Draw initial Games screen with prompt
            display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, themeCardBg);
            display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, themeAccent);

            // Large Title
            display.setTextSize(2);
            display.setTextColor(themeAccent);
            display.setCursor(54, 55);
            display.print("LUNA ARCADE");
            
            // Draw divider
            display.drawFastHLine(20, 80, SCREEN_WIDTH - 40, themeBorder);

            // Action Pill Button
            int btnW = 140;
            int btnH = 34;
            int btnX = (SCREEN_WIDTH - btnW) / 2;
            int btnY = 115;
            display.fillRoundRect(btnX, btnY, btnW, btnH, 8, themeAccent);
            display.setTextColor(TFT_WHITE);
            display.setTextSize(2);
            const char* stTxt = "START GAME";
            int stW = strlen(stTxt) * 12;
            display.setCursor(btnX + (btnW - stW) / 2, btnY + 9);
            display.print(stTxt);

            display.setTextColor(themeSubText);
            display.setTextSize(1);
            const char* hint = "Tap to Launch Arcade";
            int hW = strlen(hint) * 6;
            display.setCursor((SCREEN_WIDTH - hW) / 2, 175);
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
