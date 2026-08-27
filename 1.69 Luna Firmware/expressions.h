#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "config.h"
#include "mochi_bitmaps.h"
#include "image_logo.h"
#include "qr_card.h"
#include "wallpaper_image.h"
#include "imu.h"

extern LunaQR qrCard;
extern LunaIMU imu;
extern bool calibrateRequest;

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

class LunaFace {
private:
  Adafruit_ST7789& tft;
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
  LunaFace(Adafruit_ST7789& tftDisp, GFXcanvas16& disp) 
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
    display.setCursor(8, 4);
    display.print(tBuf);

    // ── Right zone: battery and connectivity ─────────
    int bx = SCREEN_WIDTH - 28;
    
    // Draw battery outline
    display.drawRect(bx, 6, 20, 12, themeText);
    display.fillRect(bx + 20, 9, 2, 6, themeText);
    
    // Calculate battery percentage
    int batteryPct = 0;
    if (batteryVolts >= 4.15f) batteryPct = 100;
    else if (batteryVolts >= 4.05f) batteryPct = 90 + (batteryVolts - 4.05f) * 100;
    else if (batteryVolts >= 3.95f) batteryPct = 80 + (batteryVolts - 3.95f) * 100;
    else if (batteryVolts >= 3.87f) batteryPct = 70 + (batteryVolts - 3.87f) * 125;
    else if (batteryVolts >= 3.82f) batteryPct = 60 + (batteryVolts - 3.82f) * 200;
    else if (batteryVolts >= 3.79f) batteryPct = 50 + (batteryVolts - 3.79f) * 333;
    else if (batteryVolts >= 3.75f) batteryPct = 40 + (batteryVolts - 3.75f) * 250;
    else if (batteryVolts >= 3.72f) batteryPct = 30 + (batteryVolts - 3.72f) * 333;
    else if (batteryVolts >= 3.68f) batteryPct = 20 + (batteryVolts - 3.68f) * 250;
    else if (batteryVolts >= 3.60f) batteryPct = 10 + (batteryVolts - 3.60f) * 125;
    else if (batteryVolts >= 3.30f) batteryPct = (batteryVolts - 3.30f) * 33.3f;
    else batteryPct = 0;
    if (batteryPct > 100) batteryPct = 100;
    if (batteryPct < 0) batteryPct = 0;
    
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
        int y = 56 + i * 33;
        NotificationItem& notif = notificationHistory[i];
        
        bool isSel = (notificationsActive && i == currentNotifViewIdx);
        
        // Card bg and border
        display.fillRoundRect(10, y, SCREEN_WIDTH - 20, 29, 6, isSel ? 0x10A2 : themeCardBg);
        display.drawRoundRect(10, y, SCREEN_WIDTH - 20, 29, 6, isSel ? themeAccent : themeBorder);
        
        if (isSel) {
          display.fillCircle(18, y + 14, 3, 0xFC10); // alert dot
        }
        
        // Title/Sender text
        display.setCursor(isSel ? 26 : 18, y + 2);
        display.setTextSize(2);
        display.setTextColor(themeText);
        String shortTitle = notif.title;
        if (shortTitle.length() > 11) shortTitle = shortTitle.substring(0, 9) + "..";
        display.print(shortTitle);
        
        // Time text
        display.setCursor(SCREEN_WIDTH - 55, y + 2);
        display.setTextSize(1);
        display.setTextColor(themeAccent);
        display.print(notif.timeStr);
        
        // Body snippet text
        display.setCursor(isSel ? 26 : 18, y + 18);
        display.setTextSize(1);
        display.setTextColor(themeSubText);
        String snippet = notif.body;
        if (snippet.length() > 28) snippet = snippet.substring(0, 26) + "...";
        display.print(snippet);
      }
      
      // Bottom Tip / Footer
      display.setTextSize(1);
      if (notificationsActive) {
        display.setTextColor(themeAccent);
        const char* tip = "B1: Read | B2: Next | B1 L: Exit";
        int tipW = strlen(tip) * 6;
        display.setCursor((SCREEN_WIDTH - tipW) / 2, SCREEN_HEIGHT - 16);
        display.print(tip);
      } else {
        display.setTextColor(themeSubText);
        const char* tip = "B1: Open | B2: Cycle";
        int tipW = strlen(tip) * 6;
        display.setCursor((SCREEN_WIDTH - tipW) / 2, SCREEN_HEIGHT - 16);
        display.print(tip);
      }
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
        display.fillCircle(x + 15, y + 7, 11, 0xF800); // Red circle for today
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
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, themeAccent);
    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 8, themeBg);

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

    int itemHeight = 32;

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
        display.fillRoundRect(10, yPos, SCREEN_WIDTH - 20, 30, 6, boxBg);
        display.drawRoundRect(10, yPos, SCREEN_WIDTH - 20, 30, 6, themeAccent);
        display.setTextColor(boxText);
      } else {
        display.fillRoundRect(10, yPos, SCREEN_WIDTH - 20, 30, 6, themeCardBg);
        display.setTextColor(themeText);
      }

      // Draw vector icon on the left
      int iconX = 18;
      int textX = 34;
      
      switch (optIdx) {
        case 0: // BLE
          {
            int ix = iconX + 4;
            int iy = yPos + 14;
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
            int iy = yPos + 14;
            display.drawCircle(ix, iy, 6, itemAccent);
            display.drawLine(ix, iy, ix + 4, iy - 3, itemAccent);
          }
          break;
        case 2: // Clock Style
          {
            int ix = iconX + 4;
            int iy = yPos + 14;
            display.drawRoundRect(ix - 4, iy - 6, 9, 13, 2, itemAccent);
            display.drawCircle(ix, iy, 3, itemAccent);
          }
          break;
        case 3: // Invert Screen
          {
            int ix = iconX + 4;
            int iy = yPos + 14;
            display.drawCircle(ix, iy, 6, itemAccent);
            display.fillRect(ix - 5, iy - 5, 5, 11, itemAccent);
          }
          break;
        case 4: // Brightness
          {
            int ix = iconX + 4;
            int iy = yPos + 14;
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
            int iy = yPos + 14;
            display.fillRect(ix, iy - 3, 3, 7, itemAccent);
            display.drawLine(ix + 3, iy - 3, ix + 6, iy - 6, itemAccent);
            display.drawLine(ix + 6, iy - 6, ix + 6, iy + 6, itemAccent);
            display.drawLine(ix + 6, iy + 6, ix + 3, iy + 3, itemAccent);
          }
          break;
        case 6: // Save Settings
          {
            int ix = iconX + 4;
            int iy = yPos + 14;
            display.drawRect(ix - 5, iy - 5, 11, 11, itemAccent);
            display.fillRect(ix - 3, iy - 5, 6, 4, itemAccent);
            display.fillRect(ix - 2, iy + 2, 4, 3, itemAccent);
          }
          break;
        case 7: // Exit Menu
          {
            int ix = iconX + 4;
            int iy = yPos + 14;
            display.drawLine(ix - 5, iy, ix + 5, iy, itemAccent);
            display.drawLine(ix - 5, iy, ix - 2, iy - 3, itemAccent);
            display.drawLine(ix - 5, iy, ix - 2, iy + 3, itemAccent);
          }
          break;
      }

      display.setCursor(textX, yPos + 7);
      
      uint16_t activeSwitchColor = !negativeDisplay ? 0x03E0 : 0x07E0;
      uint16_t activeBrightnessColor = !negativeDisplay ? 0xD560 : 0xFFE0;

      switch (optIdx) {
        case 0:
          display.print("BLE Connected");
          {
            int sx = SCREEN_WIDTH - 50;
            int sy = yPos + 7;
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
            display.setCursor(SCREEN_WIDTH - 24 - (val.length() * 12), yPos + 7);
            display.print(val);
          }
          break;
        case 2:
          display.print("Clock Style");
          {
            String val = String(clockStyle);
            display.setCursor(SCREEN_WIDTH - 24 - (val.length() * 12), yPos + 7);
            display.print(val);
          }
          break;
        case 3:
          display.print("Invert Color");
          {
            int sx = SCREEN_WIDTH - 50;
            int sy = yPos + 7;
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
            int by = yPos + 19;
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
            int sy = yPos + 7;
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
            display.setCursor(startX, yPos + 7);
            if (!isCurrent) display.setTextColor(activeSwitchColor); // Theme-aware Green text
            display.print(val);
          }
          break;
        case 7:
          {
            String val = "EXIT MENU";
            int startX = textX + (SCREEN_WIDTH - 20 - textX - val.length() * 12) / 2;
            display.setCursor(startX, yPos + 7);
            if (!isCurrent) display.setTextColor(0xF800); // Red text
            display.print(val);
          }
          break;
      }
    }

    if (settingsActive) {
      // Scroll indicator dots at bottom
      int totalItems = 8;
      int dotAreaY = 202;
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
      String hint = "B1: Enter | B2: Cycle";
      display.setCursor((SCREEN_WIDTH - hint.length() * 6) / 2, 206);
      display.print(hint);
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
    
    // For 1.3" display: normal mode has blue background and white drawing.
    // Inverted/Negative mode has white background and blue drawing.
    uint16_t bgColor = negativeDisplay ? TFT_WHITE : TFT_BLUE; // Blue when normal, White when inverted
    uint16_t color   = negativeDisplay ? TFT_BLUE : TFT_WHITE; // White when normal, Blue when inverted
    
    display.fillScreen(bgColor);

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

  void drawClockScreen(int hour, int minute, int second, String day, String date, int style, bool is12Hour, int steps) {
    ThemeColors theme = getTheme();
    uint16_t themeAccent = theme.accent;
    uint16_t themeBg     = theme.bg;
    uint16_t themeText   = theme.text;
    uint16_t themeCardBg = theme.cardBg;
    uint16_t themeBorder = theme.border;
    uint16_t themeSubText = theme.subText;

    // Clear display below the status bar
    display.fillRect(0, 24, SCREEN_WIDTH, SCREEN_HEIGHT - 24, themeBg);

    if (style == 0) {
      // Style 0: Neon Cyberpunk Dashboard (Dark Version)
      // Glowing outer frame
      display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 12, themeAccent);
      display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 8, themeCardBg);

      // Horizontal divider line
      display.drawFastHLine(12, SCREEN_HEIGHT / 2 + 10, SCREEN_WIDTH - 24, themeBorder);

      // Large Digital Time
      display.setTextSize(4);
      display.setTextColor(themeText);
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
      display.setTextColor(themeAccent);
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
      String dayDate = day + " " + date;
      if ((int)dayDate.length() * 12 > SCREEN_WIDTH - 36) {
        dayDate = day;
      }
      display.setTextColor(themeText);
      display.setCursor(18, SCREEN_HEIGHT / 2 + 18);
      display.print(dayDate);

      // Steps widget
      display.setTextColor(themeText);
      String stepStr = String(steps);
      int stepW = (int)stepStr.length() * 12 + 14;
      int stepX = SCREEN_WIDTH - 18 - stepW;
      display.setCursor(stepX + 14, SCREEN_HEIGHT / 2 + 18);
      display.print(stepStr);
      
      // foot icon vector drawing
      int fx = stepX + 6;
      int fy = SCREEN_HEIGHT / 2 + 24;
      display.fillCircle(fx,     fy - 4, 2, themeAccent);
      display.fillCircle(fx + 4, fy - 2, 2, themeAccent);
      display.fillCircle(fx - 3, fy + 2, 1, themeAccent);

      // Steps progress bar at the bottom
      int stepBarW = (steps * (SCREEN_WIDTH - 36)) / 10000;
      if (stepBarW > SCREEN_WIDTH - 36) stepBarW = SCREEN_WIDTH - 36;
      display.fillRect(18, SCREEN_HEIGHT - 16, SCREEN_WIDTH - 36, 4, themeBorder); // track
      display.fillRect(18, SCREEN_HEIGHT - 16, stepBarW, 4, themeAccent); // fill

    } else if (style == 1) {
      // Style 1: Minimalist Radial Dots (Modern Eclipse)
      display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 16, themeAccent);
      display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 12, themeBg);

      // Sweeping Ring of dots for seconds
      int centerX = SCREEN_WIDTH / 2;
      int centerY = SCREEN_HEIGHT / 2 + 10;
      int radius = 48;
      for (int i = 0; i < 12; i++) {
        float angle = (i * 30.0f - 90.0f) * DEG_TO_RAD;
        int dx = centerX + radius * cos(angle);
        int dy = centerY + radius * sin(angle);
        bool active = (second / 5) == i;
        display.fillCircle(dx, dy, active ? 5 : 2, active ? themeAccent : themeBorder);
        if (active) {
          display.drawCircle(dx, dy, 7, themeAccent); // outer glow ring
        }
      }

      // Time in center card
      int cardW = 120;
      int cardH = 46;
      int cardX = (SCREEN_WIDTH - cardW) / 2;
      int cardY = (SCREEN_HEIGHT - cardH) / 2 + 8;
      display.fillRoundRect(cardX, cardY, cardW, cardH, 8, themeCardBg);
      display.drawRoundRect(cardX, cardY, cardW, cardH, 8, themeBorder);

      display.setTextSize(3);
      display.setTextColor(themeText);
      char timeStr[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispHour, minute);
      display.setCursor(cardX + 12, cardY + 12);
      display.print(timeStr);

      display.setTextSize(1);
      display.setTextColor(themeAccent);
      if (is12Hour) {
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        display.setCursor(cardX + 102, cardY + 14);
        display.print(ampm);
      }
      char secStr[6];
      snprintf(secStr, sizeof(secStr), "%02d", second);
      display.setCursor(cardX + 102, cardY + 24);
      display.print(secStr);

      // Date at bottom
      display.fillRoundRect(24, SCREEN_HEIGHT - 32, SCREEN_WIDTH - 48, 24, 6, themeCardBg);
      display.drawRoundRect(24, SCREEN_HEIGHT - 32, SCREEN_WIDTH - 48, 24, 6, themeBorder);
      display.setTextSize(2);
      display.setTextColor(themeText);
      String dStr = day + " " + date;
      int dW = dStr.length() * 12;
      display.setCursor((SCREEN_WIDTH - dW) / 2, SCREEN_HEIGHT - 28);
      display.print(dStr);

    } else if (style == 2) {
      // Style 2: Watch OS Dashboard Grid (Dark Mode)
      display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 12, themeAccent);
      display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 8, themeBg);

      // Clean tech background grid lines
      display.drawFastVLine(SCREEN_WIDTH / 2, 28, SCREEN_HEIGHT - 28, themeBorder);
      display.drawFastHLine(0, 110, SCREEN_WIDTH, themeBorder);

      // Top Left: OS Header
      display.fillRoundRect(12, 36, 96, 18, 4, themeCardBg);
      display.setTextColor(themeText);
      display.setTextSize(1);
      display.setCursor(18, 42);
      display.print("LUNA OS v3");

      // Bottom Left: Steps widget
      display.fillRoundRect(12, 118, 100, 46, 6, themeCardBg);
      display.drawRoundRect(12, 118, 100, 46, 6, themeBorder);
      display.setTextColor(themeAccent);
      display.setTextSize(1);
      display.setCursor(18, 124);
      display.print("STEPS");
      display.setTextColor(themeText);
      display.setTextSize(2);
      display.setCursor(18, 138);
      display.print(steps);

      // Bottom Right: Date widget
      display.fillRoundRect(128, 118, 100, 46, 6, themeCardBg);
      display.drawRoundRect(128, 118, 100, 46, 6, themeBorder);
      display.setTextColor(themeAccent);
      display.setTextSize(1);
      display.setCursor(134, 124);
      display.print("DATE");
      display.setTextColor(themeText);
      display.setTextSize(2);
      display.setCursor(134, 138);
      display.print(day);

      // Top Right: Time
      display.setTextSize(4);
      display.setTextColor(themeText);
      char timeStr[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeStr, sizeof(timeStr), "%d:%02d", dispHour, minute);
      display.setCursor(128, 42);
      display.print(timeStr);

      display.setTextSize(1);
      display.setTextColor(themeAccent);
      display.setCursor(128, 86);
      display.print(date);

      // Flashing circular state indicator
      display.fillCircle(SCREEN_WIDTH - 24, SCREEN_HEIGHT - 24, 6, (second % 2 == 0) ? themeAccent : themeBorder);

    } else {
      // Style 3: Sci-Fi Tactical HUD Layout
      display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 12, themeAccent);
      display.fillRoundRect(8, 32, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 40, 8, themeBg);

      // Corner brackets (HUD Style)
      int d = 10;
      display.drawLine(12, 36, 12 + d, 36, themeAccent);
      display.drawLine(12, 36, 12, 36 + d, themeAccent);
      display.drawLine(SCREEN_WIDTH - 12, 36, SCREEN_WIDTH - 12 - d, 36, themeAccent);
      display.drawLine(SCREEN_WIDTH - 12, 36, SCREEN_WIDTH - 12, 36 + d, themeAccent);
      display.drawLine(12, SCREEN_HEIGHT - 12, 12 + d, SCREEN_HEIGHT - 12, themeAccent);
      display.drawLine(12, SCREEN_HEIGHT - 12, 12, SCREEN_HEIGHT - 12 - d, themeAccent);
      display.drawLine(SCREEN_WIDTH - 12, SCREEN_HEIGHT - 12, SCREEN_WIDTH - 12 - d, SCREEN_HEIGHT - 12, themeAccent);
      display.drawLine(SCREEN_WIDTH - 12, SCREEN_HEIGHT - 12, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 12 - d, themeAccent);

      // Time center
      display.setTextSize(4);
      display.setTextColor(themeText);
      char timeStr[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispHour, minute);
      display.setCursor(40, 56);
      display.print(timeStr);

      display.setTextSize(2);
      display.setTextColor(themeAccent);
      char secStr[6];
      snprintf(secStr, sizeof(secStr), "%02d", second);
      display.setCursor(168, 70);
      display.print(secStr);

      // Center Divider line
      display.drawFastHLine(20, 108, SCREEN_WIDTH - 40, themeBorder);

      // Left Gauge: battery outline and title
      int batX = 54, batY = 146;
      display.drawCircle(batX, batY, 18, themeBorder);
      display.setTextColor(themeSubText);
      display.setTextSize(1);
      display.setCursor(batX - 9, batY - 3);
      display.print("BAT");
      display.drawCircle(batX, batY, 18, themeAccent);

      // Right Gauge: steps outline and title
      int stpX = 186, stpY = 146;
      display.drawCircle(stpX, stpY, 18, themeBorder);
      display.setTextColor(themeSubText);
      display.setTextSize(1);
      display.setCursor(stpX - 9, stpY - 3);
      display.print("STP");
      display.drawCircle(stpX, stpY, 18, themeAccent);

      // Info in center
      display.setTextColor(themeText);
      display.setTextSize(1);
      int dw = day.length() * 6;
      display.setCursor((SCREEN_WIDTH - dw) / 2, 126);
      display.print(day);
      
      int dtw = date.length() * 6;
      display.setCursor((SCREEN_WIDTH - dtw) / 2, 142);
      display.print(date);

      int stw = String(steps).length() * 6;
      display.setCursor((SCREEN_WIDTH - stw) / 2, 158);
      display.print(steps);
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
    ThemeColors theme = getTheme();
    display.fillScreen(theme.bg);
    
    if (popupActive && (millis() - popupStartTime > popupDuration)) {
      popupActive = false;
    }

    if (popupActive) {
      drawPopup();
    } else {
      if (currentScreen != SCREEN_FACE && currentScreen != SCREEN_MAPS && currentScreen != SCREEN_GAMES && currentScreen != SCREEN_CARD) {
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
            display.setCursor(54, 60);
            display.print("LUNA ARCADE");
            
            // Draw divider
            display.drawFastHLine(20, 85, SCREEN_WIDTH - 40, themeBorder);

            // Subtitle instructions
            display.setTextSize(2);
            display.setTextColor(themeText);
            display.setCursor(24, 115);
            display.print("BTN1: START");
            
            display.setTextColor(themeSubText);
            display.setCursor(24, 155);
            display.print("BTN2: CYCLE");
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
