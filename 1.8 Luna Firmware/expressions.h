#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "config.h"
#include "mochi_bitmaps.h"
#include "image_logo.h"

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

class LunaFace {
private:
  Adafruit_ST7735& tft;
  GFXcanvas16& display;
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

  // Map navigation state
  String mapDirection;
  String mapDistance;
  String mapDescription;

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

public:
  String headerText;
  LunaFace(Adafruit_ST7735& tftDisp, GFXcanvas16& disp) 
    : tft(tftDisp), display(disp), currentExpr(EXPR_IDLE), targetExpr(EXPR_IDLE), defaultExpr(EXPR_IDLE), stateLabel("IDLE"), frameDelayMs(100), expressionChanged(true) {
    currentFrame = 0;
    currentGifIndex = 0;
    lastFrameTime = 0;
    gifFinished = false;

    notificationTitle = "";
    notificationText = "";
    scrollPos = SCREEN_WIDTH;
    lastScrollTime = 0;

    mapDirection = "STRAIGHT";
    mapDistance = "--";
    mapDescription = "";

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

    for (int i = 0; i < 5; i++) {
      notificationHistory[i].active = false;
      calendarEvents[i].active = false;
    }
  }

  uint16_t getExpressionColor(Expression expr) {
    Expression activeExpr = expr;
    if (activeExpr == EXPR_IDLE) {
      activeExpr = defaultExpr;
    }
    
    if (activeExpr == EXPR_ALL_GIF) {
      if (currentGifIndex >= 0 && currentGifIndex < ALL_GIFS_COUNT) {
        char nameBuf[32];
        strcpy_P(nameBuf, (char*)pgm_read_ptr(&ALL_GIFS_TABLE[currentGifIndex].name));
        String gifName = String(nameBuf);
        gifName.toUpperCase();
        
        if (gifName.indexOf("LOVE") >= 0 || gifName.indexOf("ADORE") >= 0 || gifName.indexOf("SPARKLE") >= 0 || gifName.indexOf("GLOWING") >= 0) {
          return 0xF97F; // Soft Pink
        }
        if (gifName.indexOf("ANGRY") >= 0 || gifName.indexOf("ENRAGE") >= 0 || gifName.indexOf("FURIOUS") >= 0 || gifName.indexOf("FIERCE") >= 0 || gifName.indexOf("DEVIL") >= 0 || gifName.indexOf("MENACING") >= 0 || gifName.indexOf("TOUGH") >= 0) {
          return TFT_RED;
        }
        if (gifName.indexOf("CRY") >= 0 || gifName.indexOf("SICK") >= 0 || gifName.indexOf("DIZZY") >= 0 || gifName.indexOf("RAIN") >= 0 || gifName.indexOf("SOB") >= 0 || gifName.indexOf("WEEP") >= 0) {
          return 0x5DFF; // Cyan/Blue
        }
        if (gifName.indexOf("SLEEP") >= 0 || gifName.indexOf("DROW") >= 0 || gifName.indexOf("YAWN") >= 0) {
          return 0x91FF; // Lavender/Purple
        }
        if (gifName.indexOf("BUZZ") >= 0 || gifName.indexOf("CONTEMPT") >= 0 || gifName.indexOf("IRRITATED") >= 0 || gifName.indexOf("MISTAKE") >= 0 || gifName.indexOf("SCARE") >= 0) {
          return TFT_ORANGE;
        }
        if (gifName.indexOf("DANCE") >= 0 || gifName.indexOf("ENERGETIC") >= 0 || gifName.indexOf("SPEED") >= 0 || gifName.indexOf("RUSH") >= 0 || gifName.indexOf("FAST") >= 0) {
          return TFT_GREEN;
        }
        if (gifName.indexOf("HAPPY") >= 0 || gifName.indexOf("LAUGH") >= 0 || gifName.indexOf("PLAY") >= 0 || gifName.indexOf("SMILE") >= 0 || gifName.indexOf("SMIRK") >= 0 || gifName.indexOf("TEAS") >= 0 || gifName.indexOf("GIGGLE") >= 0 || gifName.indexOf("HELLO") >= 0) {
          return TFT_YELLOW;
        }
      }
      return TFT_CYAN;
    }
    
    switch (activeExpr) {
      case EXPR_IDLE:      return TFT_CYAN;
      case EXPR_HAPPY:     return TFT_YELLOW;
      case EXPR_SAD:       return 0x5DFF; // Soft Blue
      case EXPR_ANGRY:     return TFT_RED;
      case EXPR_SURPRISED: return 0xF97F; // Pink/Magenta
      case EXPR_SLEEPING:  return 0x91FF; // Purple
      case EXPR_WINK:      return TFT_YELLOW;
      default:             return TFT_CYAN;
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
      if (currentGifIndex >= 0 && currentGifIndex < ALL_GIFS_COUNT) {
        char nameBuf[32];
        strcpy_P(nameBuf, (char*)pgm_read_ptr(&ALL_GIFS_TABLE[currentGifIndex].name));
        stateLabel = String(nameBuf);
      } else {
        stateLabel = "IDLE";
      }
    } else {
      switch (exprToLabel) {
        case EXPR_IDLE:      stateLabel = "IDLE"; break;
        case EXPR_HAPPY:     stateLabel = "HAPPY"; break;
        case EXPR_SAD:       stateLabel = "SAD"; break;
        case EXPR_ANGRY:     stateLabel = "ANGRY"; break;
        case EXPR_SURPRISED: stateLabel = "SURPRISE"; break;
        case EXPR_SLEEPING:  stateLabel = "SLEEP"; break;
        case EXPR_WINK:      stateLabel = "WINK"; break;
        case EXPR_CLOCK:     stateLabel = "CLOCK"; break;
        case EXPR_TEXT:      stateLabel = "TEXT"; break;
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

  int getGifFrameDelay(int gifIndex) {
    static const uint8_t gifDelays[] PROGMEM = {
      112, // 0  ADORE
      112, // 1  ANGRY
      52,  // 2  BLANK
      252, // 3  BLINDING
      112, // 4  BRAVE
      112, // 5  BUZZING
      92,  // 6  CONTEMPT
      112, // 7  CRYING
      112, // 8  DANCING
      92,  // 9  DEVIL
      52,  // 10 DISTRACTED
      112, // 11 DIZZY
      52,  // 12 DOWN
      112, // 13 DROWSY
      92,  // 14 ENCOURAGEMENT
      52,  // 15 ENERGETIC
      112, // 16 ENRAGED
      92,  // 17 EVIL
      92,  // 18 FAST
      112, // 19 FIERCE
      92,  // 20 FURIOUS
      112, // 21 GIGGLE
      112, // 22 GLOWING
      112, // 23 GROWING
      112, // 24 HANDSOME
      112, // 25 HAPPY
      92,  // 26 HELLO
      112, // 27 IRRITATED
      92,  // 28 LAUGHING
      52,  // 29 LEFT
      112, // 30 LOVE
      92,  // 31 MENACING
      112, // 32 MISTAKE
      112, // 33 PLAYFUL
      112, // 34 POLICE
      112, // 35 RAIN
      92,  // 36 RELAXED
      52,  // 37 RIGHT
      92,  // 38 RUSH
      112, // 39 SCARED
      112, // 40 SERENE
      52,  // 41 SHRINK
      52,  // 42 SHY
      112, // 43 SICK
      92,  // 44 SLEEPY
      112, // 45 SMILE
      52,  // 46 SMIRK
      92,  // 47 SMOKE
      112, // 48 SNEEZE
      112, // 49 SOBBING
      112, // 50 SPARKLE
      52,  // 51 SPEED
      112, // 52 SPLASH
      112, // 53 SPRAYING
      52,  // 54 SQUINT
      112, // 55 SURPRISED
      112, // 56 SUSHI
      112, // 57 SWINGING
      112, // 58 TEASING
      122, // 59 TOUGH
      92,  // 60 WEEPING
      112, // 61 WINK
      112, // 62 YAWN
    };
    if (gifIndex >= 0 && gifIndex < ALL_GIFS_COUNT) {
      return pgm_read_byte(&gifDelays[gifIndex]);
    }
    return frameDelayMs;
  }

  void setExpression(Expression expr) {
    if ((int)expr >= 100) {
      int gifIdx = (int)expr - 100;
      if (gifIdx >= 0 && gifIdx < ALL_GIFS_COUNT) {
        setGifIndex(gifIdx);
        expr = EXPR_ALL_GIF;
      }
    }
    if (currentExpr == expr) return;
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

  // ------------------ Map Navigation State ------------------
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
    currentGifIndex = idx;
    currentFrame = 0;
    lastFrameTime = millis();
    gifFinished = false;
    expressionChanged = true;
    updateLabelFromState();
  }

  int getGifIndex() {
    return currentGifIndex;
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
      int maxFrames = 1;
      Expression exprToUpdate = currentExpr;
      if (exprToUpdate == EXPR_IDLE) {
        exprToUpdate = defaultExpr;
      }
      switch (exprToUpdate) {
        case EXPR_IDLE:      maxFrames = ep_relaxed_frame_count; break;
        case EXPR_HAPPY:     maxFrames = ep_happy_frame_count; break;
        case EXPR_SAD:       maxFrames = ep_crying_frame_count; break;
        case EXPR_ANGRY:     maxFrames = ep_angry_frame_count; break;
        case EXPR_SURPRISED: maxFrames = ep_surprised_frame_count; break;
        case EXPR_SLEEPING:  maxFrames = ep_sleepy_frame_count; break;
        case EXPR_WINK:      maxFrames = ep_wink_frame_count; break;
        case EXPR_CLOCK:     maxFrames = 1; break;
        case EXPR_ALL_GIF: {
          if (currentGifIndex < ALL_GIFS_COUNT) {
            maxFrames = (int)pgm_read_dword(&ALL_GIFS_TABLE[currentGifIndex].count);
          } else {
            maxFrames = 1;
          }
          break;
        }
        default: maxFrames = 1; break;
      }

      int activeDelay;
      if (exprToUpdate == EXPR_ALL_GIF) {
        activeDelay = getGifFrameDelay(currentGifIndex);
      } else {
        switch (exprToUpdate) {
          case EXPR_IDLE:      activeDelay = 92; break;
          case EXPR_HAPPY:     activeDelay = 112; break;
          case EXPR_SAD:       activeDelay = 112; break;
          case EXPR_ANGRY:     activeDelay = 112; break;
          case EXPR_SURPRISED: activeDelay = 112; break;
          case EXPR_SLEEPING:  activeDelay = 92; break;
          case EXPR_WINK:      activeDelay = 112; break;
          default:             activeDelay = 100; break;
        }
      }
      
      if (frameDelayMs != 100) {
        activeDelay = (int)(activeDelay * (frameDelayMs / 100.0f));
      }
      activeDelay = max(20, activeDelay);

      if (now - lastFrameTime > (unsigned long)activeDelay) {
        lastFrameTime = now;
        currentFrame++;
        if (currentFrame >= maxFrames) {
          currentFrame = 0;
          gifFinished = true;
        }
        changed = true;
      }
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

  // UI STATUS BAR - 128x160 SCREEN
  // Status bar y=0..20, content starts y=22
  void drawStatusBar(int hour, int minute) {
    uint16_t accent  = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t barBg   = TFT_WHITE;
    uint16_t barText = 0x2104; // Charcoal gray

    // Header bg
    display.fillRect(0, 0, 128, 21, barBg);

    // Left: HH:MM
    display.setTextSize(1);
    display.setTextColor(barText, barBg);
    char tBuf[6];
    snprintf(tBuf, sizeof(tBuf), "%02d:%02d", hour, minute);
    display.setCursor(4, 6);
    display.print(tBuf);

    // Centre: screen title (Accent colored)
    const char* nm = "LUNA";
    switch (currentScreen) {
      case SCREEN_CLOCK:         nm = "CLOCK";   break;
      case SCREEN_NOTIFICATIONS: nm = "NOTIFS";  break;
      case SCREEN_CALENDAR:      nm = "CALENDAR"; break;
      case SCREEN_GAMES:         nm = "ARCADE";  break;
      case SCREEN_FACE:          nm = "FACE";    break;
      case SCREEN_MAPS:          nm = "MAPS";    break;
      default:                   nm = "LUNA";    break;
    }
    int nmLen = strlen(nm) * 6;
    display.setTextColor(accent, barBg);
    display.setCursor((128 - nmLen) / 2, 6);
    display.print(nm);

    // Right: battery & BLE
    int pct = 0;
    if      (batteryVolts >= 4.15f) pct = 100;
    else if (batteryVolts >= 4.05f) pct = 90;
    else if (batteryVolts >= 3.95f) pct = 80;
    else if (batteryVolts >= 3.87f) pct = 70;
    else if (batteryVolts >= 3.82f) pct = 60;
    else if (batteryVolts >= 3.79f) pct = 50;
    else if (batteryVolts >= 3.75f) pct = 40;
    else if (batteryVolts >= 3.72f) pct = 30;
    else if (batteryVolts >= 3.68f) pct = 20;
    else if (batteryVolts >= 3.60f) pct = 10;
    else                             pct = 0;

    display.drawRect(108, 6, 15, 8, barText);
    display.fillRect(123, 8, 1, 4, barText);
    int fw = (pct * 11) / 100;
    uint16_t bc = (pct < 20) ? (uint16_t)TFT_RED : (pct < 50) ? (uint16_t)0xFA60 : (uint16_t)0x05E0;
    if (fw > 0) display.fillRect(110, 8, fw, 4, bc);

    uint16_t bleC = bleConnectedStatus ? (uint16_t)0x001F : (uint16_t)0xB5B6;
    display.fillCircle(101, 10, 2, bleC);

    // Separator line
    display.drawFastHLine(0, 21, 128, 0xD69A);
  }

  void drawPopup() {
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg = TFT_WHITE;
    uint16_t themeText = 0x2104;
    uint16_t themeBorder = 0xD69A;

    // Single clean card container
    display.fillRoundRect(4, 4, 120, 152, 6, themeBg);
    display.drawRoundRect(4, 4, 120, 152, 6, themeAccent);

    // Mascot heart
    int hx = 24, hy = 16;
    display.fillCircle(hx - 2, hy, 2, themeAccent);
    display.fillCircle(hx + 2, hy, 2, themeAccent);
    display.fillTriangle(hx - 4, hy + 1, hx + 4, hy + 1, hx, hy + 5, themeAccent);

    // Title Capsule
    display.setTextColor(themeAccent);
    display.setTextSize(1);
    display.setCursor(40, 14);
    display.print("NEW ALERT");

    // Divider
    display.drawFastHLine(10, 26, 108, themeBorder);
    
    // Title
    display.setTextColor(themeText);
    display.setTextSize(1);
    display.setCursor(12, 34);
    String title = popupTitle;
    if (title.length() > 14) title = title.substring(0, 12) + "..";
    display.print(title);
    
    // Body Text
    display.setTextColor(themeText);
    display.setTextSize(1);
    int yStart = 48;
    int charsPerLine = 17; // fits inside 120px card width
    int line = 0;
    int maxLines = 8;
    for (unsigned int i = 0; i < popupBody.length() && line < maxLines; i += charsPerLine) {
      unsigned int endIdx = i + charsPerLine;
      if (endIdx > popupBody.length()) endIdx = popupBody.length();
      String lineStr = popupBody.substring(i, endIdx);
      display.setCursor(12, yStart + line * 10);
      display.print(lineStr);
      line++;
    }
    
    // Dismiss indicator
    display.setTextColor(themeAccent);
    display.setTextSize(1);
    display.setCursor(22, 142);
    display.print("[Tap to Dismiss]");
  }

  void drawNotificationPanel() {
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg = TFT_WHITE;
    uint16_t themeCardBg = (robotVariant == "mr_luna") ? 0xE7FC : 0xFDF2; // light pastel
    uint16_t themeText = 0x2104;
    uint16_t themeBorder = 0xD69A;

    // Clear content area
    display.fillRect(0, 22, 128, 138, themeBg);

    if (notificationCount == 0) {
      // Clean Envelope Icon
      int cx = 64;
      int cy = 60;
      display.drawRect(cx - 15, cy - 10, 30, 20, themeAccent);
      display.drawLine(cx - 15, cy - 10, cx, cy, themeAccent);
      display.drawLine(cx + 15, cy - 10, cx, cy, themeAccent);

      display.setTextColor(themeText);
      display.setTextSize(1);
      
      String msg1 = "No Notifications";
      display.setCursor((128 - msg1.length() * 6) / 2, 92);
      display.print(msg1);

      display.setTextColor(0x7BCF);
      String msg2 = "Inbox is empty";
      display.setCursor((128 - msg2.length() * 6) / 2, 108);
      display.print(msg2);
      return;
    }

    if (notificationSelected) {
      // DETAIL VIEW
      NotificationItem& notif = notificationHistory[currentNotifViewIdx];

      // Title & Time header
      display.setTextColor(themeAccent);
      display.setTextSize(1);
      display.setCursor(6, 28);
      String title = notif.title;
      if (title.length() > 10) title = title.substring(0, 8) + "..";
      display.print(title);

      display.setTextColor(0x7BCF);
      display.setCursor(76, 28);
      display.print(notif.timeStr);

      display.drawFastHLine(4, 38, 120, themeBorder);

      // Body Text
      display.setTextColor(themeText);
      display.setTextSize(1);
      int yStart = 44;
      int charsPerLine = 19; // 19 * 6 = 114px
      int line = 0;
      int maxLines = 8;
      for (unsigned int i = 0; i < notif.body.length() && line < maxLines; i += charsPerLine) {
        unsigned int endIdx = i + charsPerLine;
        if (endIdx > notif.body.length()) endIdx = notif.body.length();
        String lineStr = notif.body.substring(i, endIdx);
        display.setCursor(6, yStart + line * 10);
        display.print(lineStr);
        line++;
      }

      // Footer page indicator
      display.setTextColor(themeAccent);
      display.setTextSize(1);
      char footerBuf[16];
      snprintf(footerBuf, sizeof(footerBuf), "[%d / %d]", currentNotifViewIdx + 1, notificationCount);
      int footerW = strlen(footerBuf) * 6;
      display.setCursor((128 - footerW) / 2, 146);
      display.print(footerBuf);

    } else {
      // LIST VIEW
      // Rows start immediately at y=26
      int cardHeight = 22;
      int cardSpacing = 24;
      int startY = 26;
      int maxItems = 5;

      for (int i = 0; i < notificationCount && i < maxItems; i++) {
        int y = startY + i * cardSpacing;
        NotificationItem& notif = notificationHistory[i];

        if (notificationsActive && i == currentNotifViewIdx) {
          // Highlight card background
          display.fillRoundRect(4, y, 120, cardHeight, 4, themeCardBg);
          display.drawRoundRect(4, y, 120, cardHeight, 4, themeAccent);
        } else {
          // Subtle separator line below item
          display.drawFastHLine(4, y + cardHeight, 120, themeBorder);
        }

        // Sender text
        display.setCursor(8, y + 3);
        display.setTextSize(1);
        display.setTextColor(themeText);
        String shortTitle = notif.title;
        if (shortTitle.length() > 10) shortTitle = shortTitle.substring(0, 8) + "..";
        display.print(shortTitle);

        // Time text
        display.setCursor(84, y + 3);
        display.setTextColor(0x7BCF);
        display.print(notif.timeStr);
      }

      // Bottom Tip / Footer
      display.setTextSize(1);
      display.setTextColor(themeAccent);
      const char* tip = notificationsActive ? "B1:Read  B2:Next" : "B1: Read Messages";
      int tipW = strlen(tip) * 6;
      display.setCursor((128 - tipW) / 2, 146);
      display.print(tip);
    }
  }
  void drawCalendarEvents() {
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg = TFT_WHITE;
    uint16_t themeCardBg = (robotVariant == "mr_luna") ? 0xE7FC : 0xFDF2; // light pastel
    uint16_t themeText = 0x2104;
    uint16_t themeBorder = 0xD69A;

    display.fillRect(0, 22, 128, 138, themeBg);

    if (calendarEventCount == 0) {
      // Calendar icon
      int cx = 64;
      int cy = 60;
      display.drawRect(cx - 15, cy - 10, 30, 20, themeAccent);
      display.drawFastHLine(cx - 15, cy - 4, 30, themeAccent);
      display.fillRect(cx - 10, cy - 14, 2, 4, themeAccent);
      display.fillRect(cx + 8, cy - 14, 2, 4, themeAccent);

      display.setTextColor(themeText);
      display.setTextSize(1);
      String msg1 = "No Events";
      display.setCursor((128 - msg1.length() * 6) / 2, 92);
      display.print(msg1);

      display.setTextColor(0x7BCF);
      String msg2 = "Sync via BLE";
      display.setCursor((128 - msg2.length() * 6) / 2, 108);
      display.print(msg2);
      return;
    }

    CalendarEventItem& ev = calendarEvents[currentCalViewIdx];

    // Event Type pill
    bool isBday = (ev.type.indexOf("birthday") >= 0 || ev.type.indexOf("bday") >= 0);
    uint16_t badgeColor = isBday ? 0xFDF2 : themeCardBg;
    uint16_t badgeText = isBday ? 0xF8B8 : themeAccent;
    display.fillRoundRect(6, 28, 56, 14, 4, badgeColor);
    display.drawRoundRect(6, 28, 56, 14, 4, badgeText);
    display.setTextColor(badgeText);
    display.setTextSize(1);
    display.setCursor(10, 31);
    display.print(isBday ? "BDAY" : "MEET");

    // Time text on the right
    display.setTextColor(0x7BCF);
    int timeW = ev.timeStr.length() * 6;
    display.setCursor(122 - timeW, 31);
    display.print(ev.timeStr);

    display.drawFastHLine(6, 48, 116, themeBorder);

    // Title / Description
    display.setTextColor(themeText);
    display.setTextSize(1);
    int charsPerLine = 19;
    int line = 0;
    int maxLines = 8;
    int yStart = 54;
    for (unsigned int i = 0; i < ev.title.length() && line < maxLines; i += charsPerLine) {
      unsigned int endIdx = i + charsPerLine;
      if (endIdx > ev.title.length()) endIdx = ev.title.length();
      String lineStr = ev.title.substring(i, endIdx);
      display.setCursor(6, yStart + line * 10);
      display.print(lineStr);
      line++;
    }

    // Page indicator
    display.setTextColor(themeAccent);
    char footerBuf[16];
    snprintf(footerBuf, sizeof(footerBuf), "[%d / %d]", currentCalViewIdx + 1, calendarEventCount);
    int footerW = strlen(footerBuf) * 6;
    display.setCursor((128 - footerW) / 2, 146);
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
    int curDay, curMonth, curYear, startWeekday, daysInMonth;
    parseDateInfo(dateStr, dayStr, curDay, curMonth, curYear, startWeekday, daysInMonth);
    
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg = TFT_WHITE;
    uint16_t themeText = 0x2104;
    uint16_t themeBorder = 0xD69A;

    display.fillRect(0, 22, 128, 138, themeBg);

    // Month / Year header
    display.setTextSize(1);
    display.setTextColor(themeText);
    char headerBuf[32];
    const char* monthNames[] = { "", "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER" };
    snprintf(headerBuf, sizeof(headerBuf), "%s %d", (curMonth >= 1 && curMonth <= 12) ? monthNames[curMonth] : "JULY", curYear);
    int headerW = strlen(headerBuf) * 6;
    display.setCursor((128 - headerW) / 2, 28);
    display.print(headerBuf);

    // Day labels
    int colWidth = 17;
    int startX = 5;
    int startY = 42;
    display.setTextColor(themeAccent);
    display.setTextSize(1);
    const char* dayLabels[] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };
    for (int i = 0; i < 7; i++) {
      display.setCursor(startX + i * colWidth + 2, startY);
      display.print(dayLabels[i]);
    }
    display.drawFastHLine(4, startY + 10, 120, themeBorder);
    
    int col = startWeekday;
    int row = 0;
    int rowHeight = 15;
    int firstRowOffset = 14;
    
    for (int d = 1; d <= daysInMonth; d++) {
      int x = startX + col * colWidth;
      int y = startY + firstRowOffset + row * rowHeight;

      if (d == curDay) {
        display.fillCircle(x + 7, y + 3, 6, themeAccent);
        display.setTextColor(TFT_WHITE);
      } else {
        display.setTextColor(themeText);
      }

      display.setCursor(d < 10 ? x + 5 : x + 2, y);
      display.print(d);

      col++;
      if (col >= 7) {
        col = 0;
        row++;
      }
    }
  }

  void drawSettingsMenuLandscape(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg = TFT_WHITE;
    uint16_t themeCardBg = 0xF7BE;
    uint16_t themeText = 0x2104;
    uint16_t themeBorder = 0xCE79;

    // Clean border
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, themeAccent);
    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 8, themeBg);

    bool isSmall = (SCREEN_WIDTH < 200);

    // Header label
    display.setTextSize(1);
    display.setTextColor(themeAccent);
    display.setCursor((SCREEN_WIDTH - (isSmall ? 8 * 6 : 16 * 6)) / 2, 34);
    display.print(isSmall ? "SETTINGS" : "--- SETTINGS ---");
    display.drawFastHLine(12, 44, SCREEN_WIDTH - 24, themeBorder);

    int itemsPerPage = isSmall ? 4 : 5;
    int itemHeight = isSmall ? 22 : 36;
    int scrollOffset = 0;
    if (option >= itemsPerPage) {
      scrollOffset = option - itemsPerPage + 1;
    }

    int startY = isSmall ? 48 : 46;

    for (int pageIdx = 0; pageIdx < itemsPerPage; pageIdx++) {
      int optIdx = pageIdx + scrollOffset;
      if (optIdx >= 7) break;

      int yPos = startY + pageIdx * itemHeight;

      bool isCurrent = (option == optIdx);
      if (isCurrent) {
        display.fillRoundRect(8, yPos, SCREEN_WIDTH - 16, itemHeight - 2, 4, selected ? themeAccent : themeCardBg);
        display.drawRoundRect(8, yPos, SCREEN_WIDTH - 16, itemHeight - 2, 4, themeAccent);
        display.setTextColor(selected ? TFT_WHITE : themeText);
      } else {
        display.setTextColor(0x7BEF);
      }

      display.setTextSize(isSmall ? 1 : 2);
      display.setCursor(14, yPos + (isSmall ? 5 : 10));
      switch (optIdx) {
        case 0:
          display.print(isSmall ? "BLE: ON" : "BLE: ALWAYS ON");
          break;
        case 1:
          display.print("Speed: ");
          display.print(speed);
          display.print("ms");
          break;
        case 2:
          display.print(isSmall ? "Clk: Style " : "Clock: Style ");
          display.print(clockStyle);
          break;
        case 3:
          display.print("Invert: ");
          display.print(invertOn ? "ON" : "OFF");
          break;
        case 4:
          display.print(isSmall ? "Bright: " : "Bright: ");
          if (brightness == 1) display.print("LOW");
          else if (brightness == 2) display.print("MED");
          else display.print("HIGH");
          break;
        case 5:
          if (!isCurrent) display.setTextColor(0x03E0);
          display.print("SAVE SETTINGS");
          break;
        case 6:
          if (!isCurrent) display.setTextColor(TFT_RED);
          display.print("EXIT MENU");
          break;
      }
    }

    // Scroll indicator dots at bottom
    int totalItems = 7;
    int dotAreaY = SCREEN_HEIGHT - (isSmall ? 10 : 14);
    int dotSpacing = isSmall ? 8 : 14;
    int dotsStartX = (SCREEN_WIDTH - totalItems * dotSpacing) / 2;
    for (int i = 0; i < totalItems; i++) {
      if (i == option) {
        display.fillRoundRect(dotsStartX + i * dotSpacing, dotAreaY, isSmall ? 6 : 8, isSmall ? 3 : 4, 1, themeAccent);
      } else {
        display.fillRoundRect(dotsStartX + i * dotSpacing + 1, dotAreaY + 1, isSmall ? 3 : 4, isSmall ? 1 : 2, 1, themeBorder);
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
    Expression exprToDraw = currentExpr;
    if (exprToDraw == EXPR_IDLE) {
      exprToDraw = defaultExpr;
    }
    
    bool isSmall = (SCREEN_WIDTH < 200);
    uint16_t bgColor = TFT_BLACK;
    uint16_t color   = TFT_WHITE;
    
    if (robotVariant == "mr_luna") {
      bgColor = negativeDisplay ? TFT_WHITE : 0x001F; // Blue background when not inverted
      color   = negativeDisplay ? 0x001F : TFT_WHITE; // Blue eyes when inverted, white when blue background
    } else { // ms_luna
      bgColor = negativeDisplay ? TFT_WHITE : 0xF8B8; // Pink background when not inverted
      color   = negativeDisplay ? 0xF8B8 : TFT_WHITE; // Pink eyes when inverted, white when pink background
    }
    
    display.fillScreen(bgColor);

    // Keep aspect ratio (2:1) and fit safely
    int targetW = isSmall ? SCREEN_WIDTH : 210;
    int targetH = isSmall ? (SCREEN_WIDTH / 2) : 105;
    int xOffset = (SCREEN_WIDTH - targetW) / 2;
    int yOffset = (SCREEN_HEIGHT - targetH) / 2;
    
    if (exprToDraw == EXPR_ALL_GIF) {
      if (currentGifIndex < ALL_GIFS_COUNT) {
        const unsigned char* const* frames =
          (const unsigned char* const*)pgm_read_ptr(&ALL_GIFS_TABLE[currentGifIndex].frames);
        int frameCount = (int)pgm_read_dword(&ALL_GIFS_TABLE[currentGifIndex].count);
        int safeFrame = (currentFrame < frameCount) ? currentFrame : 0;
        const unsigned char* frameData =
          (const unsigned char*)pgm_read_ptr(&frames[safeFrame]);
        if (frameData) {
          drawBitmapScaled(xOffset, yOffset, frameData, 128, 64, targetW, targetH, color);
        }
      }
    } else {
      const unsigned char* frameData = nullptr;
      int frameIdx = currentFrame;
      switch (exprToDraw) {
        case EXPR_IDLE:
          if (frameIdx < ep_relaxed_frame_count)
            frameData = (const unsigned char*)pgm_read_ptr(&ep_relaxed_frames[frameIdx]);
          break;
        case EXPR_HAPPY:
          if (frameIdx < ep_happy_frame_count)
            frameData = (const unsigned char*)pgm_read_ptr(&ep_happy_frames[frameIdx]);
          break;
        case EXPR_SAD:
          if (frameIdx < ep_crying_frame_count)
            frameData = (const unsigned char*)pgm_read_ptr(&ep_crying_frames[frameIdx]);
          break;
        case EXPR_ANGRY:
          if (frameIdx < ep_angry_frame_count)
            frameData = (const unsigned char*)pgm_read_ptr(&ep_angry_frames[frameIdx]);
          break;
        case EXPR_SURPRISED:
          if (frameIdx < ep_surprised_frame_count)
            frameData = (const unsigned char*)pgm_read_ptr(&ep_surprised_frames[frameIdx]);
          break;
        case EXPR_SLEEPING:
          if (frameIdx < ep_sleepy_frame_count)
            frameData = (const unsigned char*)pgm_read_ptr(&ep_sleepy_frames[frameIdx]);
          break;
        case EXPR_WINK:
          if (frameIdx < ep_wink_frame_count)
            frameData = (const unsigned char*)pgm_read_ptr(&ep_wink_frames[frameIdx]);
          break;
        default:
          break;
      }
      
      if (frameData != nullptr) {
        drawBitmapScaled(xOffset, yOffset, frameData, 128, 64, targetW, targetH, color);
      }
    }
  }

  void drawMapScreenLandscape(int hour, int minute, bool is12Hour) {
    display.setTextWrap(false);
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg = TFT_WHITE;
    uint16_t themeText = 0x2104;
    uint16_t themeBorder = 0xD69A;

    // Clear content area
    display.fillRect(0, 22, 128, 138, themeBg);

    // Arrow center
    int cx = 64;
    int cy = 68;

    // Draw Arrow based on direction
    if (mapDirection.indexOf("LEFT") >= 0) {
      display.fillRect(cx - 15, cy - 5, 25, 10, themeAccent);
      display.fillTriangle(cx - 25, cy, cx - 12, cy - 12, cx - 12, cy + 12, themeAccent);
    } else if (mapDirection.indexOf("RIGHT") >= 0) {
      display.fillRect(cx - 10, cy - 5, 25, 10, themeAccent);
      display.fillTriangle(cx + 25, cy, cx + 12, cy - 12, cx + 12, cy + 12, themeAccent);
    } else if (mapDirection.indexOf("UTURN") >= 0 || mapDirection.indexOf("U-TURN") >= 0) {
      display.drawCircle(cx, cy - 5, 12, themeAccent);
      display.drawCircle(cx, cy - 5, 10, themeAccent);
      display.fillRect(cx - 15, cy - 5, 30, 20, themeBg);
      display.fillRect(cx - 12, cy - 5, 3, 14, themeAccent);
      display.fillRect(cx + 9, cy - 5, 3, 8, themeAccent);
      display.fillTriangle(cx + 10, cy + 8, cx + 5, cy + 2, cx + 16, cy + 2, themeAccent);
    } else if (mapDirection.indexOf("ROUNDABOUT") >= 0 || mapDirection.indexOf("ROUND") >= 0) {
      display.drawCircle(cx, cy, 14, themeAccent);
      display.drawCircle(cx, cy, 12, themeAccent);
      display.fillCircle(cx, cy, 10, themeBg);
      display.fillRect(cx + 10, cy - 18, 3, 15, themeAccent);
      display.fillTriangle(cx + 11, cy - 22, cx + 7, cy - 16, cx + 16, cy - 16, themeAccent);
    } else {
      // STRAIGHT
      display.fillRect(cx - 5, cy - 12, 10, 24, themeAccent);
      display.fillTriangle(cx, cy - 24, cx - 12, cy - 10, cx + 12, cy - 10, themeAccent);
    }

    // Direction text below arrow
    display.setTextSize(1);
    display.setTextColor(themeText);
    String dirLabel = "Go Straight";
    if      (mapDirection.indexOf("LEFT")       >= 0) dirLabel = "Turn Left";
    else if (mapDirection.indexOf("RIGHT")      >= 0) dirLabel = "Turn Right";
    else if (mapDirection.indexOf("UTURN")      >= 0 ||
             mapDirection.indexOf("U-TURN")     >= 0) dirLabel = "Make U-Turn";
    else if (mapDirection.indexOf("ROUNDABOUT") >= 0 ||
             mapDirection.indexOf("ROUND")      >= 0) dirLabel = "Roundabout";
    int lblW = dirLabel.length() * 6;
    display.setCursor((128 - lblW) / 2, 106);
    display.print(dirLabel);

    // Separator
    display.drawFastHLine(6, 120, 116, themeBorder);

    // Distance and description at bottom (split layout)
    String distStr = (mapDistance == "" || mapDistance == "--") ? "---" : mapDistance;
    display.setTextSize(2);
    display.setTextColor(themeAccent);
    display.setCursor(6, 130);
    display.print(distStr);

    if (mapDescription != "") {
      display.setTextSize(1);
      display.setTextColor(themeText);
      // Clean word wrap/truncation
      String desc = mapDescription;
      if (desc.length() > 10) desc = desc.substring(0, 8) + "..";
      int descW = desc.length() * 6;
      display.setCursor(122 - descW, 134);
      display.print(desc);
    }
  }

  void drawClockScreen(int hour, int minute, int second, String day, String date, int style, bool is12Hour, int steps) {
    uint16_t accent  = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t bg      = TFT_WHITE;
    uint16_t cardBg  = (robotVariant == "mr_luna") ? 0xE7FC : 0xFDF2; // light pastel
    uint16_t txt     = 0x2104; // charcoal
    uint16_t sub     = 0x7BCF; // grey
    uint16_t sep     = 0xD69A;

    int dispH = hour;
    if (is12Hour) { dispH = hour % 12; if (dispH == 0) dispH = 12; }
    const char* ampm = (hour >= 12) ? "PM" : "AM";

    // Clear content area
    display.fillRect(0, 22, 128, 138, bg);

    if (style == 0) {
      // Bold Stacked Design
      // Accent line on left
      display.fillRect(6, 32, 3, 62, accent);

      // Large Stacked Time
      char hrBuf[4]; snprintf(hrBuf, sizeof(hrBuf), "%02d", dispH);
      display.setTextSize(4);
      display.setTextColor(txt, bg);
      display.setCursor(16, 32);
      display.print(hrBuf);

      char minBuf[4]; snprintf(minBuf, sizeof(minBuf), "%02d", minute);
      display.setTextColor(accent, bg);
      display.setCursor(16, 64);
      display.print(minBuf);

      // Info
      display.setTextSize(1);
      display.setTextColor(sub, bg);
      display.setCursor(76, 38);
      display.print(ampm);
      char secBuf[4]; snprintf(secBuf, sizeof(secBuf), "%02d", second);
      display.setCursor(76, 50);
      display.print(secBuf);

      // Separator
      display.drawFastHLine(6, 104, 116, sep);

      // Date & Steps
      display.setTextColor(txt, bg);
      display.setCursor(8, 114);
      display.print(day + " " + date);

      display.setTextColor(0x05E0, bg);
      display.setCursor(8, 128);
      display.print("Steps: " + String(steps));

      // Bottom progress
      int prog = (second * 116) / 60;
      display.fillRect(6, 146, 116, 4, 0xEF5C);
      display.fillRect(6, 146, prog, 4, accent);

    } else if (style == 1) {
      // Elegant Dashboard Style
      // Time side-by-side
      char tBuf[6]; snprintf(tBuf, sizeof(tBuf), "%02d:%02d", dispH, minute);
      display.setTextSize(3);
      display.setTextColor(txt, bg);
      display.setCursor(8, 32);
      display.print(tBuf);

      // AM/PM & Seconds
      display.setTextSize(1);
      display.setTextColor(accent, bg);
      display.setCursor(102, 32);
      display.print(ampm);
      char secBuf[4]; snprintf(secBuf, sizeof(secBuf), "%02d", second);
      display.setCursor(102, 44);
      display.print(secBuf);

      // Info Card
      display.fillRoundRect(8, 64, 112, 60, 6, cardBg);
      display.drawRoundRect(8, 64, 112, 60, 6, accent);

      display.setTextColor(txt, cardBg);
      display.setCursor(16, 72);
      display.print(day + " " + date);

      display.setTextColor(0x05E0, cardBg);
      display.setCursor(16, 88);
      display.print("Steps: " + String(steps));

      display.setTextColor(sub, cardBg);
      display.setCursor(16, 104);
      display.print("LUNA OS ACTIVE");

      // Progress bar at bottom
      int prog = (second * 112) / 60;
      display.fillRect(8, 140, 112, 4, 0xEF5C);
      display.fillRect(8, 140, prog, 4, accent);

    } else {
      // Modern Grid Style
      // Subtle Grid lines
      for (int x = 16; x < 128; x += 32) {
        display.drawFastVLine(x, 22, 138, 0xF7BE);
      }
      for (int y = 50; y < 160; y += 30) {
        display.drawFastHLine(0, y, 128, 0xF7BE);
      }

      // Time
      char tBuf[6]; snprintf(tBuf, sizeof(tBuf), "%02d:%02d", dispH, minute);
      display.setTextSize(3);
      display.setTextColor(txt, bg);
      display.setCursor(12, 64);
      display.print(tBuf);

      display.setTextSize(1);
      display.setTextColor(accent, bg);
      display.setCursor(104, 66);
      display.print(ampm);
      char secBuf[4]; snprintf(secBuf, sizeof(secBuf), "%02d", second);
      display.setCursor(104, 78);
      display.print(secBuf);

      // Date Widget
      display.setTextColor(txt, bg);
      display.setCursor(8, 116);
      display.print(day + " " + date);

      // Steps Widget
      display.setTextColor(0x05E0, bg);
      display.setCursor(8, 132);
      display.print("Steps: " + String(steps));

      // Flashing dot
      if (second % 2 == 0) {
        display.fillCircle(112, 116, 3, accent);
      }
    }
  }





  // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
  void drawTextScreen() {
    uint16_t accent = (robotVariant == "mr_luna") ? 0x001F : 0xF000;
    uint16_t bg     = TFT_WHITE;
    uint16_t cardBg = 0xEF5C;
    uint16_t txt    = 0x2104;
    uint16_t sep    = 0xD69A;

    display.setTextWrap(false);
    display.fillRect(0, 26, 128, 134, bg);

    // Header strip
    display.fillRect(0, 26, 128, 14, accent);
    display.setTextSize(1);
    display.setTextColor(TFT_WHITE, accent);
    String displayTitle = notificationTitle.length() > 0 ? notificationTitle : "Alert";
    if ((int)displayTitle.length() > 14) displayTitle = displayTitle.substring(0, 11) + "...";
    int tw = displayTitle.length() * 6;
    display.setCursor((128 - tw) / 2, 30);
    display.print(displayTitle);

    // Separator
    display.drawFastHLine(0, 40, 128, sep);

    // Scrolling message body â€” textSize 2 (12px wide per char)
    display.setTextColor(txt, bg);
    display.setTextSize(2);
    int charW = 12;
    int textLength = notificationText.length() * charW;
    if (textLength <= 116) {
      display.setCursor((128 - textLength) / 2, 72);
    } else {
      display.setCursor(scrollPos, 72);
    }
    display.print(notificationText);

    // Footer label
    display.fillRoundRect(4, 142, 120, 14, 4, cardBg);
    display.setTextSize(1);
    display.setTextColor(accent, cardBg);
    display.setCursor((128 - 10*6)/2, 146);
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
    display.fillScreen(TFT_BLACK);
    
    if (popupActive && (millis() - popupStartTime > popupDuration)) {
      popupActive = false;
    }

    if (popupActive) {
      drawPopup();
    } else {
      if (currentScreen != SCREEN_FACE) {
        drawStatusBar(hour, minute);
      }
      
      switch (currentScreen) {
        case SCREEN_CLOCK:
          drawClockScreen(hour, minute, second, day, date, style, is12Hour, touchCount);
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
            uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
            uint16_t themeBg = TFT_WHITE;
            uint16_t themeText = 0x2104;
            uint16_t themeBorder = 0xD69A;

            // Clear screen
            display.fillRect(0, 22, 128, 138, themeBg);

            // Clean Gamepad graphics
            display.drawRoundRect(44, 48, 40, 24, 6, themeAccent);
            display.fillCircle(54, 60, 2, themeAccent);
            display.fillCircle(74, 60, 2, themeText);

            // Title
            display.setTextSize(1);
            display.setTextColor(themeText);
            String titleStr = "LUNA ARCADE";
            display.setCursor((128 - titleStr.length() * 6) / 2, 88);
            display.print(titleStr);

            // Divider
            display.drawFastHLine(6, 120, 116, themeBorder);

            // Help text
            display.setTextColor(0x7BCF);
            display.setCursor(12, 132);
            display.print("B1:Start  B2:Cycle");
          } else {
            if (!gamePlaying) {
              games.drawMenu(display);
            } else {
              if (gameSelected == 1) {
                games.updateAndDrawCoinCatcher(display, audio);
              } else if (gameSelected == 2) {
                games.updateAndDrawFlappyMochy(display, audio);
              } else if (gameSelected == 3) {
                games.updateAndDrawSnake(display, audio);
              } else if (gameSelected == 4) {
                games.updateAndDrawSpaceInvaders(display, audio);
              } else if (gameSelected == 5) {
                games.updateAndDrawPong(display, audio);
              } else if (gameSelected == 6) {
                games.updateAndDrawBreakout(display, audio);
              } else if (gameSelected == 7) {
                games.updateAndDrawMemoryMatch(display, audio);
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
      }
    }

    if (!popupActive && currentScreen == SCREEN_FACE && headerText.length() > 0) {
      uint16_t headerBg = TFT_BLACK;
      uint16_t headerFg = TFT_WHITE;
      if (robotVariant == "mr_luna") {
        headerBg = negativeDisplay ? TFT_WHITE : 0x001F;
        headerFg = negativeDisplay ? 0x001F : TFT_WHITE;
      } else {
        headerBg = negativeDisplay ? TFT_WHITE : 0xF8B8;
        headerFg = negativeDisplay ? 0xF8B8 : TFT_WHITE;
      }
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
    
    tft.drawRGBBitmap(0, 0, display.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  // Legacy compatibility
  void drawSettingsMenu(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    display.fillScreen(TFT_BLACK);
    drawStatusBar(12, 0);
    drawSettingsMenuLandscape(option, selected, bleOn, speed, clockStyle, invertOn, brightness);
    tft.drawRGBBitmap(0, 0, display.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
  }
};

#endif // EXPRESSIONS_H
