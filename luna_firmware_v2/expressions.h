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
  String headerText;
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
    // Floating Pill-Shaped Glassmorphic Status Bar
    display.fillRoundRect(8, 4, SCREEN_WIDTH - 16, 22, 11, 0x10A2);
    display.drawRoundRect(8, 4, SCREEN_WIDTH - 16, 22, 11, 0x18E3);

    // Sleek digital clock on left
    display.setTextColor(TFT_WHITE, 0x10A2);
    display.setTextSize(2);
    display.setCursor(18, 7);
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
    display.setCursor(centerTextX, 7);
    display.print(screenName);

    // BLE status icon
    if (bleConnectedStatus) {
      display.setTextColor(0x5DFF, 0x10A2); // cyan
      display.setCursor(SCREEN_WIDTH - 90, 7);
      display.print("B");
    } else {
      display.setTextColor(TFT_DARKGREY, 0x10A2);
      display.setCursor(SCREEN_WIDTH - 90, 7);
      display.print("b");
    }

    // WiFi status icon
    if (wifiConnectedStatus) {
      display.setTextColor(TFT_GREEN, 0x10A2);
      display.setCursor(SCREEN_WIDTH - 70, 7);
      display.print("W");
    } else {
      display.setTextColor(TFT_DARKGREY, 0x10A2);
      display.setCursor(SCREEN_WIDTH - 70, 7);
      display.print("w");
    }

    // Battery icon
    display.drawRect(SCREEN_WIDTH - 38, 9, 20, 10, TFT_LIGHTGREY);
    display.fillRect(SCREEN_WIDTH - 18, 11, 2, 6, TFT_LIGHTGREY);
    display.fillRect(SCREEN_WIDTH - 35, 12, 14, 4, TFT_GREEN);
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
    // Curved border container
    display.drawRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 12, 0xF8B8);
    display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 10, 0x0821);

    if (calendarEventCount == 0) {
      int centerX = SCREEN_WIDTH / 2;
      display.drawRect(centerX - 10, 62, 20, 20, TFT_DARKGREY);
      display.drawFastHLine(centerX - 10, 68, 20, TFT_DARKGREY);
      display.fillRect(centerX - 6, 58, 2, 6, TFT_DARKGREY);
      display.fillRect(centerX + 4, 58, 2, 6, TFT_DARKGREY);
      
      display.setTextColor(TFT_LIGHTGREY, 0x0821);
      display.setTextSize(2);
      int lblW1 = 9 * 12;
      display.setCursor((SCREEN_WIDTH - lblW1) / 2, 98);
      display.print("No Events");
      display.setTextColor(TFT_DARKGREY, 0x0821);
      display.setTextSize(1);
      int lblW2 = 19 * 6;
      display.setCursor((SCREEN_WIDTH - lblW2) / 2, 120);
      display.print("Sync events via BLE");
      return;
    }
    
    CalendarEventItem& ev = calendarEvents[currentCalViewIdx];
    
    display.setTextColor(TFT_BLACK);
    if (ev.type.indexOf("birthday") >= 0 || ev.type.indexOf("bday") >= 0) {
      display.fillRoundRect(16, 38, 90, 20, 6, 0xF97F); // Pink label
      display.setTextSize(1);
      display.setCursor(22, 44);
      display.print("BIRTHDAY");
    } else {
      display.fillRoundRect(16, 38, 90, 20, 6, TFT_YELLOW); // Yellow label
      display.setTextSize(1);
      display.setCursor(22, 44);
      display.print("MEETING");
    }
    
    display.setTextColor(0x07FF, 0x0821);
    display.setTextSize(2);
    display.setCursor(SCREEN_WIDTH - 90, 40);
    display.print(ev.timeStr);
    
    display.drawFastHLine(12, 66, SCREEN_WIDTH - 24, 0x18E3);
    
    display.setTextColor(TFT_WHITE, 0x0821);
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
    
    display.setTextColor(TFT_DARKGREY, 0x0821);
    display.setTextSize(1);
    char footerBuf[16];
    snprintf(footerBuf, sizeof(footerBuf), "[%d / %d]", currentCalViewIdx + 1, calendarEventCount);
    int footerW = strlen(footerBuf) * 6;
    display.setCursor((SCREEN_WIDTH - footerW) / 2, SCREEN_HEIGHT - 22);
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
    
    // Draw Curved Border
    display.drawRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 12, 0x07FF);
    display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 10, 0x0821);
    
    // Month / Year header — size 1 so it fits
    display.setTextSize(1);
    display.setTextColor(TFT_YELLOW, 0x0821);
    char headerBuf[32];
    const char* monthNames[] = { "", "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER" };
    snprintf(headerBuf, sizeof(headerBuf), "%s %d", (curMonth >= 1 && curMonth <= 12) ? monthNames[curMonth] : "JULY", curYear);
    int headerW = strlen(headerBuf) * 6;
    display.setCursor((SCREEN_WIDTH - headerW) / 2, 36);
    display.print(headerBuf);

    // Day-of-week header
    int colWidth = (SCREEN_WIDTH - 16) / 7;  // ~32px per column on 240px screen
    int startX = 8;
    int startY = 48;
    display.setTextColor(0x5DFF, 0x0821);
    const char* dayLabels[] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };
    for (int i = 0; i < 7; i++) {
      display.setCursor(startX + i * colWidth + 2, startY);
      display.print(dayLabels[i]);
    }
    display.drawFastHLine(8, startY + 10, SCREEN_WIDTH - 16, 0x18E3);
    
    int col = startWeekday;
    int row = 0;
    
    display.setTextSize(1);
    int rowHeight = (SCREEN_HEIGHT - startY - 22) / 6;
    if (rowHeight < 14) rowHeight = 14;
    for (int d = 1; d <= daysInMonth; d++) {
      int x = startX + col * colWidth;
      int y = startY + 14 + row * rowHeight;

      if (d == curDay) {
        display.fillCircle(x + 3, y + 3, 6, TFT_RED);
        display.setTextColor(TFT_WHITE, TFT_RED);
      } else {
        display.setTextColor(TFT_WHITE, 0x0821);
      }

      display.setCursor(d < 10 ? x + 1 : x - 1, y);
      display.print(d);

      col++;
      if (col >= 7) {
        col = 0;
        row++;
      }
    }
  }

  void drawSettingsMenuLandscape(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    // Clean border
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, 0x07FF);
    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 8, 0x0821);

    // Header label
    display.setTextSize(1);
    display.setTextColor(0xFDA0, 0x0821);
    display.setCursor(12, 34);
    display.print("--- SETTINGS ---");
    display.drawFastHLine(12, 44, SCREEN_WIDTH - 24, 0x18E3);

    display.setTextSize(2);
    int itemsPerPage = 5;
    int scrollOffset = 0;
    if (option >= itemsPerPage) {
      scrollOffset = option - itemsPerPage + 1;
    }

    int itemHeight = 36;

    for (int pageIdx = 0; pageIdx < itemsPerPage; pageIdx++) {
      int optIdx = pageIdx + scrollOffset;
      if (optIdx >= 8) break;

      int yPos = 46 + pageIdx * itemHeight;

      bool isCurrent = (option == optIdx);
      if (isCurrent) {
        display.fillRoundRect(8, yPos, SCREEN_WIDTH - 16, itemHeight - 2, 6, selected ? 0x0248 : 0x18E3);
        display.drawRoundRect(8, yPos, SCREEN_WIDTH - 16, itemHeight - 2, 6, 0x07FF);
        display.setTextColor(TFT_WHITE, selected ? 0x0248 : 0x18E3);
      } else {
        display.setTextColor(TFT_LIGHTGREY, 0x0821);
      }

      uint16_t bg = isCurrent ? (selected ? 0x0248 : 0x18E3) : 0x0821;
      display.setCursor(14, yPos + 10);
      switch (optIdx) {
        case 0:
          display.setTextColor(isCurrent ? TFT_WHITE : 0x07FF, bg);
          display.print("BLE: ALWAYS ON");
          break;
        case 1:
          display.setTextColor(isCurrent ? TFT_WHITE : TFT_LIGHTGREY, bg);
          display.print("Speed: ");
          display.print(speed);
          display.print("ms");
          break;
        case 2:
          display.setTextColor(isCurrent ? TFT_WHITE : TFT_LIGHTGREY, bg);
          display.print("Clock: Style ");
          display.print(clockStyle);
          break;
        case 3:
          display.setTextColor(isCurrent ? TFT_WHITE : TFT_LIGHTGREY, bg);
          display.print("Invert: ");
          display.print(invertOn ? "ON" : "OFF");
          break;
        case 4:
          display.setTextColor(isCurrent ? TFT_WHITE : TFT_LIGHTGREY, bg);
          display.print("Bright: ");
          if (brightness == 1) display.print("LOW");
          else if (brightness == 2) display.print("MED");
          else display.print("HIGH");
          break;
        case 5:
          display.setTextColor(isCurrent ? TFT_WHITE : TFT_LIGHTGREY, bg);
          display.print("Loopback: ");
          display.print(hardwareLoopbackActive ? "ON" : "OFF");
          break;
        case 6:
          display.setTextColor(isCurrent ? TFT_WHITE : TFT_GREEN, bg);
          display.print("SAVE SETTINGS");
          break;
        case 7:
          display.setTextColor(isCurrent ? TFT_WHITE : 0xF8B8, bg);
          display.print("EXIT MENU");
          break;
      }
    }

    // Scroll indicator dots at bottom
    int totalItems = 8;
    int dotAreaY = SCREEN_HEIGHT - 14;
    int dotSpacing = 14;
    int dotsStartX = (SCREEN_WIDTH - totalItems * dotSpacing) / 2;
    for (int i = 0; i < totalItems; i++) {
      if (i == option) {
        display.fillRoundRect(dotsStartX + i * dotSpacing, dotAreaY, 8, 4, 2, 0x07FF);
      } else {
        display.fillRoundRect(dotsStartX + i * dotSpacing, dotAreaY + 1, 4, 2, 1, 0x18E3);
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
    display.setTextWrap(false);

    // ── Background card ───────────────────────────────────────────────
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 12, TFT_GREEN);
    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, 0x0821);

    // ── Top label: "NAVIGATION" ───────────────────────────────────────
    display.setTextSize(1);
    display.setTextColor(TFT_GREEN, 0x0821);
    int navW = 10 * 6;
    display.setCursor((SCREEN_WIDTH - navW) / 2, 35);
    display.print("NAVIGATION");
    display.drawFastHLine(12, 45, SCREEN_WIDTH - 24, 0x18E3);

    // ── Arrow area (centered, 60x60px arrow in the middle) ───────────
    int cx = SCREEN_WIDTH / 2;
    int cy = 108;  // vertical center of arrow area

    if (mapDirection.indexOf("LEFT") >= 0) {
      // LEFT arrow: large clear left-pointing arrow
      // Stem: horizontal bar going left from centre
      display.fillRect(cx - 30, cy - 8, 40, 16, TFT_WHITE);
      // Arrowhead pointing LEFT
      display.fillTriangle(cx - 30, cy,
                           cx - 10, cy - 26,
                           cx - 10, cy + 26, TFT_WHITE);
      // Small vertical stem going down at the right end (road continues straight then turns)
      display.fillRect(cx + 10, cy - 8, 14, 30, TFT_WHITE);

    } else if (mapDirection.indexOf("RIGHT") >= 0) {
      // RIGHT arrow: large clear right-pointing arrow
      display.fillRect(cx - 10, cy - 8, 40, 16, TFT_WHITE);
      // Arrowhead pointing RIGHT
      display.fillTriangle(cx + 30, cy,
                           cx + 10, cy - 26,
                           cx + 10, cy + 26, TFT_WHITE);
      // Small vertical stem going down at the left end
      display.fillRect(cx - 24, cy - 8, 14, 30, TFT_WHITE);

    } else if (mapDirection.indexOf("UTURN") >= 0 || mapDirection.indexOf("U-TURN") >= 0) {
      // U-TURN: thick U shape with downward arrow
      display.drawCircle(cx, cy - 14, 22, TFT_WHITE);
      display.drawCircle(cx, cy - 14, 20, TFT_WHITE);
      display.drawCircle(cx, cy - 14, 18, TFT_WHITE);
      // Erase the bottom half of the circles to make a U
      display.fillRect(cx - 30, cy - 14, 60, 40, 0x0821);
      // Left leg
      display.fillRect(cx - 24, cy - 14, 6, 32, TFT_WHITE);
      // Right leg with downward arrow at bottom
      display.fillRect(cx + 18, cy - 14, 6, 24, TFT_WHITE);
      display.fillTriangle(cx + 21, cy + 18,
                           cx + 10, cy + 8,
                           cx + 32, cy + 8, TFT_WHITE);

    } else if (mapDirection.indexOf("ROUNDABOUT") >= 0 || mapDirection.indexOf("ROUND") >= 0) {
      // ROUNDABOUT: circle with an exit arrow
      display.drawCircle(cx, cy, 22, TFT_WHITE);
      display.drawCircle(cx, cy, 20, TFT_WHITE);
      // Fill inside dark
      display.fillCircle(cx, cy, 17, 0x0821);
      // Exit arrow pointing up-right
      display.fillRect(cx + 14, cy - 28, 6, 24, TFT_WHITE);
      display.fillTriangle(cx + 17, cy - 34,
                           cx + 10, cy - 24,
                           cx + 24, cy - 24, TFT_WHITE);
      // Entry from bottom
      display.fillRect(cx - 6, cy + 14, 12, 16, TFT_WHITE);

    } else {
      // STRAIGHT: tall upward arrow
      display.fillRect(cx - 8, cy - 20, 16, 44, TFT_WHITE);
      display.fillTriangle(cx, cy - 40,
                           cx - 22, cy - 20,
                           cx + 22, cy - 20, TFT_WHITE);
    }

    // ── Direction label text below arrow ────────────────────────────
    display.setTextSize(2);
    display.setTextColor(TFT_WHITE, 0x0821);
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
    display.drawFastHLine(8, 170, SCREEN_WIDTH - 16, 0x18E3);
    display.fillRoundRect(6, 172, SCREEN_WIDTH - 12, 50, 8, 0x18E3);

    // Distance — left side, large yellow
    display.setTextSize(3);
    display.setTextColor(TFT_YELLOW, 0x18E3);
    String distStr = (mapDistance == "" || mapDistance == "--") ? "---" : mapDistance;
    display.setCursor(12, 179);
    display.print(distStr);

    // ETA / description — right side, white size 2
    if (mapDescription != "") {
      display.setTextSize(2);
      display.setTextColor(TFT_WHITE, 0x18E3);
      int etaW = mapDescription.length() * 12;
      int etaX = SCREEN_WIDTH - 12 - etaW;
      if (etaX < 12) etaX = 12;
      display.setCursor(etaX, 185);
      display.print(mapDescription);
    }
  }

  void drawClockScreen(int hour, int minute, int second, String day, String date, int style, bool is12Hour, int steps) {
    display.setTextColor(TFT_WHITE, TFT_BLACK);

    if (style == 0) {
      // Style 0: Cyberpunk Dashboard
      display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 12, 0x07FF);
      display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 8, 0x0821);

      // Horizontal divider line
      display.drawFastHLine(12, SCREEN_HEIGHT / 2 + 10, SCREEN_WIDTH - 24, 0x18E3);

      // Large Digital Time
      display.setTextSize(4);
      display.setTextColor(TFT_WHITE, 0x0821);
      char timeStr[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispHour, minute);
      int timeW = 5 * 24;
      display.setCursor((SCREEN_WIDTH - timeW) / 2 - 10, SCREEN_HEIGHT / 2 - 28);
      display.print(timeStr);

      // AM/PM or Seconds
      display.setTextSize(1);
      display.setTextColor(0xF8B8, 0x0821);
      if (is12Hour) {
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        display.setCursor((SCREEN_WIDTH - timeW) / 2 + timeW + 4, SCREEN_HEIGHT / 2 - 22);
        display.print(ampm);
      }
      char secStr[16];
      snprintf(secStr, sizeof(secStr), "%02d", second);
      display.setCursor((SCREEN_WIDTH - timeW) / 2 + timeW + 4, SCREEN_HEIGHT / 2 - 10);
      display.print(secStr);

      // Date and Day
      display.setTextSize(2);
      display.setTextColor(0x07FF, 0x0821);
      display.setCursor(18, SCREEN_HEIGHT / 2 + 18);
      display.print(day);
      display.print(" ");
      display.setTextColor(TFT_WHITE, 0x0821);
      display.print(date);

      // Steps widget
      display.setTextColor(0xFDA0, 0x0821);
      String stepStr = String(steps);
      int stepW = stepStr.length() * 12;
      display.setCursor(SCREEN_WIDTH - 18 - stepW, SCREEN_HEIGHT / 2 + 18);
      display.print(stepStr);
      
      int fx = SCREEN_WIDTH - 28 - stepW;
      int fy = SCREEN_HEIGHT / 2 + 24;
      display.fillCircle(fx, fy - 4, 2, 0xFDA0);
      display.fillCircle(fx + 4, fy - 2, 2, 0xFDA0);
      display.fillCircle(fx - 3, fy + 2, 1, 0xFDA0);

    } else if (style == 1) {
      // Style 1: Minimalist Radial Gauge
      display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 16, 0xF8B8);
      display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 12, 0x0821);

      // Center card
      int cardW = 130;
      int cardH = 46;
      int cardX = (SCREEN_WIDTH - cardW) / 2;
      int cardY = (SCREEN_HEIGHT - cardH) / 2 + 8;
      display.fillRoundRect(cardX, cardY, cardW, cardH, 8, 0x18E3);
      display.drawRoundRect(cardX, cardY, cardW, cardH, 8, 0x07FF);

      // Time
      display.setTextSize(3);
      display.setTextColor(TFT_WHITE, 0x18E3);
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
      display.setTextColor(0xF8B8, 0x18E3);
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
      display.drawRoundRect(24, 38, SCREEN_WIDTH - 48, 6, 3, 0x18E3);
      display.fillRoundRect(24, 38, progressWidth, 6, 3, 0xF8B8);

      // Date
      display.setTextSize(2);
      display.setTextColor(0x07FF, 0x0821);
      String dStr = day + " " + date;
      int dW = dStr.length() * 12;
      display.setCursor((SCREEN_WIDTH - dW) / 2, SCREEN_HEIGHT - 22);
      display.print(dStr);

    } else {
      // Style 2: Watch OS Grid (original style 3, but polished with curved edges)
      display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 12, 0xFDA0);
      display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 8, 0x0821);

      // Grid spacing
      int gridSpacing = SCREEN_WIDTH / 10;
      for (int x = gridSpacing; x < SCREEN_WIDTH; x += gridSpacing) {
        display.drawFastVLine(x, 32, SCREEN_HEIGHT - 32, 0x0100);
      }
      for (int y = 32; y < SCREEN_HEIGHT; y += gridSpacing) {
        display.drawFastHLine(0, y, SCREEN_WIDTH, 0x0100);
      }

      // Title Card
      display.fillRoundRect(12, 34, 110, 16, 4, 0x18E3);
      display.setTextColor(0x07E0, 0x18E3);
      display.setTextSize(1);
      display.setCursor(18, 38);
      display.print("WATCH OS v3.0");

      // Time
      display.setTextSize(4);
      display.setTextColor(TFT_WHITE, 0x0821);
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
        display.setTextColor(0xF8B8, 0x0821);
        display.setCursor(115, 66);
        display.print((hour >= 12) ? "PM" : "AM");
      }

      // Steps widget
      display.fillRoundRect(14, 86, 100, 14, 4, 0x18E3);
      display.setTextColor(0xFDA0, 0x18E3);
      display.setCursor(18, 89);
      display.print("STP: ");
      display.print(steps);

      // Date widget
      display.fillRoundRect(14, 102, SCREEN_WIDTH - 28, 14, 4, 0x18E3);
      display.setTextColor(TFT_LIGHTGREY, 0x18E3);
      display.setCursor(18, 105);
      display.print("DT: ");
      display.print(day);
      display.print(", ");
      display.print(date);

      // Flashing block
      display.fillRoundRect(SCREEN_WIDTH - 24, SCREEN_HEIGHT - 24, 8, 8, 2, (second % 2 == 0) ? 0x07E0 : 0x18E3);
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

    if (!popupActive && currentScreen == SCREEN_FACE && headerText.length() > 0) {
      display.fillRect(0, 0, SCREEN_WIDTH, 24, TFT_BLACK);
      display.setTextColor(TFT_WHITE);
      display.setTextSize(2);
      
      // Center the header text
      int textW = headerText.length() * 12;
      int startX = (SCREEN_WIDTH - textW) / 2;
      if (startX < 0) startX = 0;
      
      display.setCursor(startX, 4);
      display.print(headerText);
      display.drawFastHLine(0, 24, SCREEN_WIDTH, TFT_WHITE);
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
