#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <SPI.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "mochi_bitmaps.h"
#include "image_logo.h"

// External references to settings/status variables defined in the main sketch
extern bool bleActive;
extern int gifSpeed;
extern int clockStyle;
extern int oledBrightness;
extern bool negativeDisplay;
extern volatile bool hardwareLoopbackActive;
extern int menuOption;
extern bool optionSelected;
extern unsigned int touchCount;
extern SmartwatchScreen currentScreen;

class LunaFace {
private:
  TFT_eSprite& display;
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
  LunaFace(TFT_eSprite& disp) 
    : display(disp), currentExpr(EXPR_IDLE), targetExpr(EXPR_IDLE), defaultExpr(EXPR_IDLE), stateLabel("IDLE"), frameDelayMs(100), expressionChanged(true) {
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

  // ------------------ Smartwatch UI Drawing Methods ------------------
  
  void drawStatusBar(int hour, int minute) {
    display.fillRect(0, 0, SCREEN_WIDTH, 24, 0x10A2); // Slate grey/blue background
    display.drawFastHLine(0, 24, SCREEN_WIDTH, TFT_DARKGREY);

    // Sleek digital clock on left
    display.setTextColor(TFT_WHITE, 0x10A2);
    display.setTextSize(2);
    display.setCursor(6, 4);
    char tBuf[6];
    snprintf(tBuf, sizeof(tBuf), "%02d:%02d", hour, minute);
    display.print(tBuf);

    // Screen name in the center
    display.setTextColor(TFT_YELLOW, 0x10A2);
    String screenName = "";
    switch (currentScreen) {
      case SCREEN_CLOCK:         screenName = "CLOCK"; break;
      case SCREEN_NOTIFICATIONS: screenName = "NOTIFS"; break;
      case SCREEN_CALENDAR:      screenName = "CALENDAR"; break;
      case SCREEN_SETTINGS:      screenName = "SETTINGS"; break;
      case SCREEN_FACE:          screenName = "LUNA FACE"; break;
      case SCREEN_MAPS:          screenName = "MAPS"; break;
      default:                   screenName = "LUNA OS"; break;
    }
    int centerTextX = (SCREEN_WIDTH - (screenName.length() * 12)) / 2;
    display.setCursor(centerTextX, 4);
    display.print(screenName);

    // BLE status icon
    if (bleConnectedStatus) {
      display.setTextColor(0x5DFF, 0x10A2); // cyan
      display.setCursor(SCREEN_WIDTH - 84, 4);
      display.print("B");
    } else {
      display.setTextColor(TFT_DARKGREY, 0x10A2);
      display.setCursor(SCREEN_WIDTH - 84, 4);
      display.print("b");
    }

    // WiFi status icon
    if (wifiConnectedStatus) {
      display.setTextColor(TFT_GREEN, 0x10A2);
      display.setCursor(SCREEN_WIDTH - 64, 4);
      display.print("W");
    } else {
      display.setTextColor(TFT_DARKGREY, 0x10A2);
      display.setCursor(SCREEN_WIDTH - 64, 4);
      display.print("w");
    }

    // Battery icon
    display.drawRect(SCREEN_WIDTH - 32, 6, 22, 12, TFT_LIGHTGREY);
    display.fillRect(SCREEN_WIDTH - 10, 9, 3, 6, TFT_LIGHTGREY);
    display.fillRect(SCREEN_WIDTH - 29, 9, 16, 6, TFT_GREEN);
  }

  void drawPopup() {
    uint16_t LUNA_CYAN   = 0x07FF;
    uint16_t LUNA_PINK   = 0xF8B8;
    uint16_t LUNA_DARK   = 0x0842;
    uint16_t LUNA_GLASS  = 0x18E3;

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
    display.setTextColor(LUNA_CYAN);
    display.setTextSize(1);
    display.setCursor(44, 18);
    display.print("NEW ALERT  *");

    display.drawFastHLine(12, 38, SCREEN_WIDTH - 24, LUNA_GLASS);
    
    // 3. Title & Content
    display.setTextColor(TFT_WHITE);
    display.setTextSize(2);
    display.setCursor(16, 48);
    String title = popupTitle;
    if (title.length() > 16) title = title.substring(0, 14) + "...";
    display.print(title);
    
    display.setTextColor(0xDEDB);
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
    display.setTextColor(LUNA_PINK);
    display.setTextSize(1);
    display.setCursor((SCREEN_WIDTH - 96) / 2, SCREEN_HEIGHT - 22);
    display.print("[Tap to Dismiss]");
  }

  void drawNotificationPanel() {
    uint16_t LUNA_CYAN   = 0x07FF;
    uint16_t LUNA_PINK   = 0xF8B8;
    uint16_t LUNA_DARK   = 0x0842;
    uint16_t LUNA_GLASS  = 0x18E3;
    uint16_t LUNA_CORAL  = 0xFC10;

    if (notificationCount == 0) {
      // Sleeping face graphic
      int centerX = SCREEN_WIDTH / 2;
      display.fillCircle(centerX, 70, 36, LUNA_GLASS);
      display.drawCircle(centerX, 70, 36, LUNA_PINK);
      
      display.drawCircle(centerX - 12, 68, 6, TFT_WHITE);
      display.fillRect(centerX - 19, 60, 14, 8, LUNA_GLASS);
      display.drawCircle(centerX + 12, 68, 6, TFT_WHITE);
      display.fillRect(centerX + 5, 60, 14, 8, LUNA_GLASS);
      
      display.fillCircle(centerX - 18, 76, 4, LUNA_CORAL);
      display.fillCircle(centerX + 18, 76, 4, LUNA_CORAL);
      
      display.drawCircle(centerX, 76, 3, TFT_WHITE);
      display.fillRect(centerX - 4, 73, 8, 3, LUNA_GLASS);
      
      display.setTextColor(LUNA_CYAN);
      display.setTextSize(1);
      display.setCursor(centerX + 24, 40);
      display.print("Z");
      display.setCursor(centerX + 32, 32);
      display.print("z");
      
      display.setTextColor(TFT_WHITE);
      display.setTextSize(2);
      int lblW1 = 16 * 12;
      display.setCursor((SCREEN_WIDTH - lblW1) / 2, 126);
      display.print("No Notifications");
      
      display.setTextColor(0xAD55);
      display.setTextSize(1);
      int lblW2 = 18 * 6;
      display.setCursor((SCREEN_WIDTH - lblW2) / 2, 150);
      display.print("History is empty");
      return;
    }
    
    NotificationItem& notif = notificationHistory[currentNotifViewIdx];
    
    // Glowing Card Container
    display.drawRoundRect(6, 26, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 32, 10, LUNA_CYAN);
    display.drawRoundRect(7, 27, SCREEN_WIDTH - 14, SCREEN_HEIGHT - 34, 9, LUNA_GLASS);
    display.fillRoundRect(8, 28, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 36, 8, LUNA_DARK);
    
    // Header
    display.setTextColor(LUNA_PINK);
    display.setTextSize(2);
    display.setCursor(14, 34);
    String title = notif.title;
    if (title.length() > 10) title = title.substring(0, 8) + "...";
    display.print(title);
    
    display.setTextColor(LUNA_CYAN);
    display.setTextSize(2);
    display.setCursor(SCREEN_WIDTH - 76, 34);
    display.print(notif.timeStr);
    
    display.drawFastHLine(12, 54, SCREEN_WIDTH - 24, LUNA_GLASS);
    
    // Body Text
    display.setTextColor(TFT_WHITE);
    display.setTextSize(2);
    int yStart = 64;
    int charsPerLine = (SCREEN_WIDTH - 28) / 12;
    int line = 0;
    int maxLines = (SCREEN_HEIGHT - 106) / 20;
    if (maxLines < 4) maxLines = 4;
    for (unsigned int i = 0; i < notif.body.length() && line < maxLines; i += charsPerLine) {
      unsigned int endIdx = i + charsPerLine;
      if (endIdx > notif.body.length()) endIdx = notif.body.length();
      String lineStr = notif.body.substring(i, endIdx);
      display.setCursor(14, yStart + line * 20);
      display.print(lineStr);
      line++;
    }
    
    // Indicator
    display.setTextColor(LUNA_PINK);
    display.setTextSize(1);
    char footerBuf[16];
    snprintf(footerBuf, sizeof(footerBuf), "[%d / %d]", currentNotifViewIdx + 1, notificationCount);
    int footerW = strlen(footerBuf) * 6;
    display.setCursor((SCREEN_WIDTH - footerW) / 2, SCREEN_HEIGHT - 20);
    display.print(footerBuf);
  }

  void drawCalendarEvents() {
    if (calendarEventCount == 0) {
      int centerX = SCREEN_WIDTH / 2;
      display.drawRect(centerX - 10, 42, 20, 20, TFT_DARKGREY);
      display.drawFastHLine(centerX - 10, 48, 20, TFT_DARKGREY);
      display.fillRect(centerX - 6, 38, 2, 6, TFT_DARKGREY);
      display.fillRect(centerX + 4, 38, 2, 6, TFT_DARKGREY);
      
      display.setTextColor(TFT_LIGHTGREY);
      display.setTextSize(1);
      int lblW1 = 20 * 6;
      display.setCursor((SCREEN_WIDTH - lblW1) / 2, 82);
      display.print("No Events Scheduled");
      display.setTextColor(TFT_DARKGREY);
      int lblW2 = 19 * 6;
      display.setCursor((SCREEN_WIDTH - lblW2) / 2, 98);
      display.print("Sync events via BLE");
      return;
    }
    
    CalendarEventItem& ev = calendarEvents[currentCalViewIdx];
    
    display.drawRoundRect(6, 28, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 34, 6, TFT_GREEN);
    
    display.setTextColor(TFT_BLACK);
    if (ev.type.indexOf("birthday") >= 0 || ev.type.indexOf("bday") >= 0) {
      display.fillRoundRect(12, 34, 60, 14, 3, 0xF97F); // Pink label
      display.setCursor(16, 37);
      display.print("BIRTHDAY");
    } else {
      display.fillRoundRect(12, 34, 60, 14, 3, TFT_YELLOW); // Yellow label
      display.setCursor(16, 37);
      display.print("MEETING");
    }
    
    display.setTextColor(TFT_GREEN);
    display.setCursor(SCREEN_WIDTH - 80, 37);
    display.print(ev.timeStr);
    
    display.drawFastHLine(10, 54, SCREEN_WIDTH - 20, TFT_DARKGREY);
    
    display.setTextColor(TFT_WHITE);
    int yStart = 62;
    int charsPerLine = (SCREEN_WIDTH - 24) / 6;
    int line = 0;
    int maxLines = (SCREEN_HEIGHT - 86) / 10;
    if (maxLines < 4) maxLines = 4;
    for (unsigned int i = 0; i < ev.title.length() && line < maxLines; i += charsPerLine) {
      unsigned int endIdx = i + charsPerLine;
      if (endIdx > ev.title.length()) endIdx = ev.title.length();
      String lineStr = ev.title.substring(i, endIdx);
      display.setCursor(12, yStart + line * 10);
      display.print(lineStr);
      line++;
    }
    
    display.setTextColor(TFT_DARKGREY);
    char footerBuf[16];
    snprintf(footerBuf, sizeof(footerBuf), "[%d / %d]", currentCalViewIdx + 1, calendarEventCount);
    int footerW = strlen(footerBuf) * 6;
    display.setCursor((SCREEN_WIDTH - footerW) / 2, SCREEN_HEIGHT - 18);
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
    
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.setTextSize(1);
    char headerBuf[32];
    const char* monthNames[] = { "", "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER" };
    snprintf(headerBuf, sizeof(headerBuf), "%s %d", (curMonth >= 1 && curMonth <= 12) ? monthNames[curMonth] : "JULY", curYear);
    
    int headerW = strlen(headerBuf) * 6;
    display.setCursor((SCREEN_WIDTH - headerW) / 2, 28);
    display.print(headerBuf);
    
    display.setTextColor(0x5DFF, TFT_BLACK);
    int colWidth = (SCREEN_WIDTH - 24) / 7;
    int startX = (SCREEN_WIDTH - colWidth * 7) / 2 + 2;
    int startY = 42;
    display.setCursor(startX, startY);
    display.print(" S   M   T   W   T   F   S");
    
    display.drawFastHLine(8, startY + 10, SCREEN_WIDTH - 16, TFT_DARKGREY);
    
    int col = startWeekday;
    int row = 0;
    
    for (int d = 1; d <= daysInMonth; d++) {
      int x = startX + col * colWidth;
      int y = startY + 14 + row * ((SCREEN_HEIGHT - startY - 28) / 6);
      
      if (d == curDay) {
        display.fillCircle(x + 5, y + 3, 7, TFT_RED);
        display.setTextColor(TFT_WHITE, TFT_RED);
      } else {
        display.setTextColor(TFT_WHITE, TFT_BLACK);
      }
      
      display.setCursor(d < 10 ? x + 2 : x, y);
      display.print(d);
      
      col++;
      if (col >= 7) {
        col = 0;
        row++;
      }
    }
  }

  void drawSettingsMenuLandscape(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    display.setTextSize(2);
    int itemsPerPage = 5;
    int scrollOffset = 0;
    if (option >= itemsPerPage) {
      scrollOffset = option - itemsPerPage + 1;
    }
    
    int itemHeight = (SCREEN_HEIGHT - 40) / itemsPerPage;
    if (itemHeight > 40) itemHeight = 40;
    
    for (int pageIdx = 0; pageIdx < itemsPerPage; pageIdx++) {
      int optIdx = pageIdx + scrollOffset;
      if (optIdx >= 8) break;
      
      int yPos = 32 + pageIdx * itemHeight + 2;
      
      bool isCurrent = (option == optIdx);
      if (isCurrent) {
        display.fillRoundRect(4, yPos - 2, SCREEN_WIDTH - 8, itemHeight - 4, 4, selected ? TFT_BLUE : 0x4208);
        display.setTextColor(TFT_WHITE);
      } else {
        display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      }
      
      display.setCursor(10, yPos + (itemHeight - 16) / 2);
      switch (optIdx) {
        case 0:
          display.print("BLE: ALWAYS ON");
          break;
        case 1:
          display.print("Anim Speed: ");
          display.print(speed);
          display.print("ms");
          break;
        case 2:
          display.print("Clock Style:  ");
          display.print(clockStyle);
          break;
        case 3:
          display.print("Invert Colors: ");
          display.print(invertOn ? "ON" : "OFF");
          break;
        case 4:
          display.print("Brightness: ");
          if (brightness == 1) display.print("LOW");
          else if (brightness == 2) display.print("MED");
          else display.print("HIGH");
          break;
        case 5:
          display.print("Loopback Test: ");
          display.print(hardwareLoopbackActive ? "ON" : "OFF");
          break;
        case 6:
          display.print("  [ SAVE SETTINGS ]");
          break;
        case 7:
          display.print("  [ EXIT MENU ]");
          break;
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
    
    uint16_t color = getExpressionColor(currentExpr);

    // Keep aspect ratio (2:1) and fit safely within circular smartwatch screen (210x105)
    int targetW = 210;
    int targetH = 105;
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
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextWrap(false);

    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 56, 6, TFT_GREEN);
    
    int arrowX = SCREEN_WIDTH / 2;
    int arrowY = (SCREEN_HEIGHT - 28) / 2;
    display.setTextColor(TFT_WHITE);
    if (mapDirection.indexOf("LEFT") >= 0) {
      display.fillTriangle(arrowX - 24, arrowY, arrowX, arrowY - 20, arrowX, arrowY + 20, TFT_WHITE);
      display.fillRect(arrowX, arrowY - 8, 24, 16, TFT_WHITE);
      display.fillRect(arrowX + 12, arrowY + 8, 12, 16, TFT_WHITE);
    } else if (mapDirection.indexOf("RIGHT") >= 0) {
      display.fillTriangle(arrowX + 24, arrowY, arrowX, arrowY - 20, arrowX, arrowY + 20, TFT_WHITE);
      display.fillRect(arrowX - 24, arrowY - 8, 24, 16, TFT_WHITE);
      display.fillRect(arrowX - 24, arrowY + 8, 12, 16, TFT_WHITE);
    } else if (mapDirection.indexOf("UTURN") >= 0 || mapDirection.indexOf("U-TURN") >= 0) {
      display.drawCircle(arrowX, arrowY, 18, TFT_WHITE);
      display.drawCircle(arrowX, arrowY, 16, TFT_WHITE);
      display.fillRect(arrowX - 20, arrowY, 40, 24, TFT_BLACK);
      display.fillRect(arrowX - 18, arrowY, 4, 14, TFT_WHITE);
      display.fillRect(arrowX + 14, arrowY, 4, 22, TFT_WHITE);
      display.fillTriangle(arrowX - 16, arrowY + 20, arrowX - 22, arrowY + 10, arrowX - 10, arrowY + 10, TFT_WHITE);
    } else if (mapDirection.indexOf("ROUNDABOUT") >= 0 || mapDirection.indexOf("ROUND") >= 0) {
      display.drawCircle(arrowX, arrowY, 14, TFT_WHITE);
      display.fillRect(arrowX - 8, arrowY - 8, 16, 16, TFT_BLACK);
      display.fillRect(arrowX - 2, arrowY + 8, 4, 10, TFT_WHITE);
      display.fillRect(arrowX + 8, arrowY - 2, 8, 4, TFT_WHITE);
      display.fillTriangle(arrowX + 20, arrowY, arrowX + 12, arrowY - 6, arrowX + 12, arrowY + 6, TFT_WHITE);
    } else {
      display.fillRect(arrowX - 8, arrowY - 8, 16, 28, TFT_WHITE);
      display.fillTriangle(arrowX, arrowY - 24, arrowX - 20, arrowY - 8, arrowX + 20, arrowY - 8, TFT_WHITE);
    }

    display.drawFastHLine(10, SCREEN_HEIGHT - 22, SCREEN_WIDTH - 20, TFT_GREEN);

    if (mapDistance != "" && mapDistance != "--") {
      display.setTextColor(TFT_YELLOW, TFT_BLACK);
      drawMixedSizeText(mapDistance, 10, SCREEN_HEIGHT - 16, SCREEN_HEIGHT - 12);
    }

    if (mapDescription != "") {
      display.setTextColor(TFT_GREEN, TFT_BLACK);
      int timeWidth = getMixedSizeTextWidth(mapDescription);
      int sX = SCREEN_WIDTH - 10 - timeWidth;
      if (sX < arrowX) sX = arrowX;
      drawMixedSizeText(mapDescription, sX, SCREEN_HEIGHT - 16, SCREEN_HEIGHT - 12);
    }
  }

  void drawClockScreen(int hour, int minute, int second, String day, String date, int style, bool is12Hour, int steps) {
    display.setTextColor(TFT_WHITE, TFT_BLACK);

    if (style == 1) {
      display.setTextSize(5);
      char timeNoSec[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeNoSec, sizeof(timeNoSec), "%02d:%02d", dispHour, minute);
      
      int timeW = 5 * 30; // 5 characters of width (approx 30px per char at size 5)
      display.setCursor((SCREEN_WIDTH - timeW) / 2, SCREEN_HEIGHT / 2 - 40);
      display.setTextColor(0x5DFF, TFT_BLACK);
      display.print(timeNoSec);

      display.setTextSize(2);
      display.setTextColor(TFT_WHITE, TFT_BLACK);
      if (is12Hour) {
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        display.setCursor((SCREEN_WIDTH - timeW) / 2 + timeW + 4, SCREEN_HEIGHT / 2 - 20);
        display.print(ampm);
      } else {
        char secStr[6];
        snprintf(secStr, sizeof(secStr), "%02ds", second);
        display.setCursor((SCREEN_WIDTH - timeW) / 2 + timeW + 4, SCREEN_HEIGHT / 2 - 20);
        display.print(secStr);
      }

      display.setTextSize(2);
      display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      String dayDateStr = day + ", " + date;
      int dayDateW = dayDateStr.length() * 12;
      display.setCursor((SCREEN_WIDTH - dayDateW) / 2, SCREEN_HEIGHT / 2 + 16);
      display.print(dayDateStr);

      display.setTextColor(TFT_YELLOW, TFT_BLACK);
      String stepStr = "STEPS: " + String(steps);
      int stepW = stepStr.length() * 12;
      display.setCursor((SCREEN_WIDTH - stepW) / 2, SCREEN_HEIGHT / 2 + 40);
      display.print(stepStr);

      int progressWidth = (second * (SCREEN_WIDTH - 32)) / 60;
      display.drawRoundRect(16, SCREEN_HEIGHT - 22, SCREEN_WIDTH - 32, 6, 3, TFT_DARKGREY);
      display.fillRoundRect(18, SCREEN_HEIGHT - 20, progressWidth, 2, 1, 0x5DFF);

    } else if (style == 2) {
      int centerX = SCREEN_WIDTH / 2;
      int centerY = (SCREEN_HEIGHT + 16) / 2;
      int radius = (SCREEN_HEIGHT - 48) / 2;
      display.drawCircle(centerX, centerY, radius, TFT_WHITE);
      
      // Face ticks
      for (int i = 0; i < 12; i++) {
        float angle = i * 30 * PI / 180;
        int x1 = centerX + (radius - 4) * cos(angle);
        int y1 = centerY + (radius - 4) * sin(angle);
        int x2 = centerX + radius * cos(angle);
        int y2 = centerY + radius * sin(angle);
        display.drawLine(x1, y1, x2, y2, TFT_WHITE);
      }
      
      // Hour hand
      float hAngle = ((hour % 12) * 30 + minute * 0.5 - 90) * PI / 180;
      int hHandLen = radius * 0.5;
      display.drawLine(centerX, centerY, centerX + hHandLen * cos(hAngle), centerY + hHandLen * sin(hAngle), TFT_YELLOW);
      
      // Minute hand
      float mAngle = (minute * 6 - 90) * PI / 180;
      int mHandLen = radius * 0.75;
      display.drawLine(centerX, centerY, centerX + mHandLen * cos(mAngle), centerY + mHandLen * sin(mAngle), 0x5DFF);
      
      // Second hand
      float sAngle = (second * 6 - 90) * PI / 180;
      int sHandLen = radius * 0.85;
      display.drawLine(centerX, centerY, centerX + sHandLen * cos(sAngle), centerY + sHandLen * sin(sAngle), TFT_RED);
      
      display.fillCircle(centerX, centerY, 3, TFT_LIGHTGREY);

    } else if (style == 3) {
      int gridSpacing = SCREEN_WIDTH / 10;
      for (int x = gridSpacing; x < SCREEN_WIDTH; x += gridSpacing) {
        display.drawFastVLine(x, 24, SCREEN_HEIGHT - 24, 0x0100);
      }
      for (int y = 32; y < SCREEN_HEIGHT; y += gridSpacing) {
        display.drawFastHLine(0, y, SCREEN_WIDTH, 0x0100);
      }

      display.setTextSize(2);
      display.setTextColor(0x07E0, TFT_BLACK);
      display.setCursor(14, 32);
      display.print("WATCH OS v2.0");

      display.setTextSize(4);
      char timeStr[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeStr, sizeof(timeStr), "%d:%02d", dispHour, minute);
      display.setCursor(14, 52);
      display.print(timeStr);

      display.setTextSize(1);
      if (is12Hour) {
        display.setCursor(115, 66);
        display.print((hour >= 12) ? "PM" : "AM");
      }

      display.setTextColor(TFT_YELLOW, TFT_BLACK);
      display.setCursor(14, 86);
      display.print("STEPS: ");
      display.print(steps);

      display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      display.setCursor(14, 102);
      display.print("DATE: ");
      display.print(day);
      display.print(", ");
      display.print(date);

      display.fillRect(SCREEN_WIDTH - 24, SCREEN_HEIGHT - 24, 8, 8, 0x07E0);

    } else if (style == 4) {
      // Draw Lopaka custom background logo
      display.pushImage((SCREEN_WIDTH - LOGO_WIDTH) / 2, (SCREEN_HEIGHT - LOGO_HEIGHT) / 2, LOGO_WIDTH, LOGO_HEIGHT, image_logo_pixels);

      // Translucent look overlay card for time
      int cardW = 120;
      int cardH = 80;
      int cardX = (SCREEN_WIDTH - cardW) / 2;
      int cardY = (SCREEN_HEIGHT - cardH) / 2;
      display.fillRoundRect(cardX, cardY, cardW, cardH, 6, TFT_BLACK);
      display.drawRoundRect(cardX, cardY, cardW, cardH, 6, 0x5DFF); // Cyan border

      display.setTextSize(3);
      display.setTextColor(TFT_WHITE, TFT_BLACK);
      char timeStr[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispHour, minute);
      display.setCursor(cardX + 16, cardY + 10);
      display.print(timeStr);

      display.setTextSize(1);
      display.setTextColor(TFT_YELLOW, TFT_BLACK);
      display.setCursor(cardX + 12, cardY + 40);
      display.print("STEPS: ");
      display.print(steps);

      display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      display.setCursor(cardX + 12, cardY + 56);
      display.print(day);
      display.print(", ");
      display.print(date);

    } else {
      display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 8, 0x5DFF);
      display.drawRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 6, 0x0821);

      display.setTextSize(5);
      display.setTextColor(TFT_WHITE, TFT_BLACK);
      char timeStr[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispHour, minute);
      
      int timeW = 5 * 30; // ~150px wide
      display.setCursor((SCREEN_WIDTH - timeW) / 2, SCREEN_HEIGHT / 2 - 24);
      display.print(timeStr);

      display.setTextSize(1);
      display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      if (is12Hour) {
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        display.setCursor((SCREEN_WIDTH - timeW) / 2 + timeW + 4, SCREEN_HEIGHT / 2 - 16);
        display.print(ampm);
      }
      
      char secStr[16];
      snprintf(secStr, sizeof(secStr), "%02d SEC", second);
      display.setCursor(SCREEN_WIDTH - 54, SCREEN_HEIGHT - 36);
      display.print(secStr);

      display.setTextSize(1);
      display.setTextColor(0x5DFF, TFT_BLACK);
      String dayDateStr = day + ", " + date;
      display.setCursor(18, SCREEN_HEIGHT - 36);
      display.print(dayDateStr);
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

  // ------------------ Primary Smartwatch Draw Adapter ------------------
  void draw(int hour, int minute, int second, String day, String date, int style = 0, bool is12Hour = false) {
    display.fillSprite(TFT_BLACK);
    
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
        case SCREEN_SETTINGS:
          drawSettingsMenuLandscape(menuOption, optionSelected, bleActive, gifSpeed, clockStyle, negativeDisplay, oledBrightness);
          break;
        case SCREEN_FACE:
          drawRobotFaceScreen();
          break;
        case SCREEN_MAPS:
          drawMapScreenLandscape(hour, minute, is12Hour);
          break;
      }
    }
    
    display.pushSprite(0, 0);
  }

  // Legacy compatibility
  void drawSettingsMenu(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    display.fillSprite(TFT_BLACK);
    drawStatusBar(12, 0);
    drawSettingsMenuLandscape(option, selected, bleOn, speed, clockStyle, invertOn, brightness);
    display.pushSprite(0, 0);
  }
};

#endif // EXPRESSIONS_H
