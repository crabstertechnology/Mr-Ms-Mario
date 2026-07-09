#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <SPI.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "mochi_bitmaps.h"

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

  // Returns the correct frame delay (ms) for each named GIF,
  // measured from actual GIF file timing minus overhead.
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

  void setNotificationText(String text) {
    notificationTitle = "Notification";
    notificationText = text;
    scrollPos = SCREEN_WIDTH;
    lastScrollTime = millis();
    setExpression(EXPR_TEXT);
  }

  void setDetailedNotification(String title, String body) {
    notificationTitle = title;
    notificationText = body;
    scrollPos = SCREEN_WIDTH;
    lastScrollTime = millis();
    setExpression(EXPR_TEXT);
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

    // 1. Frame Animation logic
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

    // 2. Scroll text logic
    if (currentExpr == EXPR_TEXT) {
      if (now - lastScrollTime > 15) {
        lastScrollTime = now;
        scrollPos -= 2; // Scroll faster on color TFT
        int textLength = notificationText.length() * 12; // size-2 font: ~12px per char
        
        if (scrollPos < -textLength) {
          scrollPos = SCREEN_WIDTH;
        }
        changed = true;
      }
    }
    return changed;
  }

  // Draw Status Bar for Face Mode
  void drawStatusBar() {
    // Draw background top bar (y: 0 to 45)
    display.fillRect(0, 0, SCREEN_WIDTH, 44, TFT_BLACK);
    display.drawFastHLine(0, 44, SCREEN_WIDTH, TFT_DARKGREY);

    // Title text
    display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    display.setTextSize(1);
    display.setCursor(6, 6);
    display.print("LUNA OS");

    display.setCursor(6, 18);
    display.setTextColor(TFT_DARKCYAN, TFT_BLACK);
    display.print("v2.0.0");

    // Network / BLE statuses
    display.setTextSize(1);
    if (bleConnectedStatus) {
      display.setTextColor(TFT_BLUE, TFT_BLACK);
      display.setCursor(82, 6);
      display.print("BLE:OK");
    } else {
      display.setTextColor(TFT_DARKGREY, TFT_BLACK);
      display.setCursor(82, 6);
      display.print("BLE:DISC");
    }

    if (wifiConnectedStatus) {
      display.setTextColor(TFT_GREEN, TFT_BLACK);
      display.setCursor(82, 18);
      display.print("WIFI:OK");
    } else {
      display.setTextColor(TFT_DARKGREY, TFT_BLACK);
      display.setCursor(82, 18);
      display.print("WIFI:OFF");
    }
  }

  // Draw Footer for Face Mode
  void drawFooter() {
    display.fillRect(0, 112, SCREEN_WIDTH, 48, TFT_BLACK);
    display.drawFastHLine(0, 112, SCREEN_WIDTH, TFT_DARKGREY);

    // Centered label
    display.setTextSize(1);
    display.setTextColor(TFT_CYAN, TFT_BLACK);
    int labelW = stateLabel.length() * 6;
    int startX = (SCREEN_WIDTH - labelW) / 2;
    display.setCursor(startX, 126);
    display.print(stateLabel);

    // Cute border line accent
    display.drawRoundRect(4, 142, SCREEN_WIDTH - 8, 8, 4, TFT_DARKGREEN);
    display.fillRect(8, 144, SCREEN_WIDTH - 16, 4, TFT_GREEN);
  }

  void drawSettingsMenu(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    display.fillSprite(TFT_BLACK);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    
    // Draw Title Header (y: 0 to 22)
    display.setTextSize(1);
    display.setCursor(22, 6);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.print("=== SETTINGS ===");
    display.drawFastHLine(0, 18, SCREEN_WIDTH, TFT_YELLOW);
    
    // S3 screen has 160px height. We can display up to 8 rows comfortably!
    // We can display all 7 menu items simultaneously without scrolling!
    for (int optIdx = 0; optIdx < 7; optIdx++) {
      int yPos = 24 + optIdx * 18;
      
      bool isCurrent = (option == optIdx);
      if (isCurrent) {
        display.fillRect(2, yPos - 2, SCREEN_WIDTH - 4, 16, selected ? TFT_BLUE : TFT_DARKGREY);
        display.setTextColor(TFT_WHITE);
      } else {
        display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      }
      
      display.setCursor(8, yPos);
      switch (optIdx) {
        case 0:
          display.print("BLE Server: ");
          display.print(bleOn ? "ON" : "OFF");
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
          display.print("  [ SAVE SETTINGS ]");
          break;
        case 6:
          display.print("  [ EXIT MENU ]");
          break;
      }
    }
    display.pushSprite(0, 0);
  }

  void draw(int hour, int minute, int second, String day, String date, int style = 0, bool is12Hour = false) {
    display.fillSprite(TFT_BLACK);

    // Draw text screens or faces
    if (currentExpr == EXPR_TEXT) {
      drawTextScreen();
    } else if (currentExpr == EXPR_CLOCK) {
      drawClockScreen(hour, minute, second, day, date, style, is12Hour);
    } else if (currentExpr == EXPR_MAP) {
      drawMapScreen(hour, minute, is12Hour);
    } else {
      // Draw Face Frame in center (y-offset: 48)
      drawStatusBar();
      drawFooter();

      if (currentExpr == EXPR_ALL_GIF) {
        if (currentGifIndex < ALL_GIFS_COUNT) {
          const unsigned char* const* frames =
            (const unsigned char* const*)pgm_read_ptr(&ALL_GIFS_TABLE[currentGifIndex].frames);
          int frameCount = (int)pgm_read_dword(&ALL_GIFS_TABLE[currentGifIndex].count);
          int safeFrame = (currentFrame < frameCount) ? currentFrame : 0;
          const unsigned char* frameData =
            (const unsigned char*)pgm_read_ptr(&frames[safeFrame]);
          if (frameData) {
            display.drawBitmap(0, 48, frameData, 128, 64, getExpressionColor(currentExpr));
          }
        }
      } else {
        const unsigned char* frameData = nullptr;
        int frameIdx = currentFrame;
        Expression exprToDraw = currentExpr;
        if (exprToDraw == EXPR_IDLE) {
          exprToDraw = defaultExpr;
        }

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
          display.drawBitmap(0, 48, frameData, 128, 64, getExpressionColor(currentExpr));
        }
      }
    }

    display.pushSprite(0, 0); // Flush buffer to hardware display
  }

private:
  void drawClockScreen(int hour, int minute, int second, String day, String date, int style, bool is12Hour) {
    display.setTextColor(TFT_WHITE, TFT_BLACK);

    if (style == 1) {
      // Style 1: Minimalist (Sleek layout for 128x160)
      display.setTextSize(4);
      char timeNoSec[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeNoSec, sizeof(timeNoSec), "%02d:%02d", dispHour, minute);
      
      // Center digital time vertically
      display.setCursor(6, 42);
      display.setTextColor(TFT_CYAN, TFT_BLACK);
      display.print(timeNoSec);

      display.setTextSize(2);
      display.setTextColor(TFT_WHITE, TFT_BLACK);
      if (is12Hour) {
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        display.setCursor(48, 86);
        display.print(ampm);
      } else {
        char secStr[6];
        snprintf(secStr, sizeof(secStr), "%02d s", second);
        display.setCursor(44, 86);
        display.print(secStr);
      }

      // Date at bottom
      display.setTextSize(1);
      display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      String dayDateStr = day + ", " + date;
      int dateW = dayDateStr.length() * 6;
      display.setCursor((SCREEN_WIDTH - dateW) / 2, 128);
      display.print(dayDateStr);

    } else if (style == 2) {
      // Style 2: Vertical Analog Face
      // Center (64, 52), Radius 38
      display.drawCircle(64, 52, 38, TFT_WHITE);
      display.fillCircle(64, 52, 2, TFT_YELLOW);

      // Calculations
      float angleHour = (hour % 12 + minute / 60.0) * 30.0 * PI / 180.0;
      float angleMin = (minute + second / 60.0) * 6.0 * PI / 180.0;
      float angleSec = second * 6.0 * PI / 180.0;

      // Hour hand (length 18)
      int hx = 64 + (int)(18.0 * sin(angleHour));
      int hy = 52 - (int)(18.0 * cos(angleHour));
      display.drawLine(64, 52, hx, hy, TFT_WHITE);

      // Minute hand (length 28)
      int mx = 64 + (int)(28.0 * sin(angleMin));
      int my = 52 - (int)(28.0 * cos(angleMin));
      display.drawLine(64, 52, mx, my, TFT_LIGHTGREY);

      // Second hand (length 32)
      int sx = 64 + (int)(32.0 * sin(angleSec));
      int sy = 52 - (int)(32.0 * cos(angleSec));
      display.drawLine(64, 52, sx, sy, TFT_RED);

      // Digital display at the bottom (y: 104 to 160)
      display.setTextSize(2);
      display.setTextColor(TFT_GREEN, TFT_BLACK);
      char timeNoSec[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeNoSec, sizeof(timeNoSec), "%02d:%02d", dispHour, minute);
      int tW = strlen(timeNoSec) * 12;
      display.setCursor((SCREEN_WIDTH - tW) / 2, 108);
      display.print(timeNoSec);

      display.setTextSize(1);
      display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      String fullDate = day + " " + date;
      int dW = fullDate.length() * 6;
      display.setCursor((SCREEN_WIDTH - dW) / 2, 134);
      display.print(fullDate);

    } else if (style == 3) {
      // Style 3: Retro Grid
      display.drawFastHLine(0, 0, SCREEN_WIDTH, TFT_ORANGE);
      display.drawFastHLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH, TFT_ORANGE);
      display.drawFastVLine(0, 0, SCREEN_HEIGHT, TFT_ORANGE);
      display.drawFastVLine(SCREEN_WIDTH - 1, 0, SCREEN_HEIGHT, TFT_ORANGE);

      display.setTextSize(2);
      display.setTextColor(TFT_ORANGE, TFT_BLACK);
      display.setCursor(16, 20);
      display.print("CLOCK OS");
      display.drawFastHLine(12, 40, SCREEN_WIDTH - 24, TFT_DARKGREY);

      // Time
      display.setTextSize(3);
      char timeStr[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeStr, sizeof(timeStr), "%d:%02d", dispHour, minute);
      int tW = strlen(timeStr) * 18;
      display.setCursor((SCREEN_WIDTH - tW) / 2, 55);
      display.print(timeStr);

      display.setTextSize(1);
      display.setTextColor(TFT_YELLOW, TFT_BLACK);
      if (is12Hour) {
        display.setCursor(95, 82);
        display.print((hour >= 12) ? "PM" : "AM");
      }

      // Date
      display.setTextSize(1);
      display.setTextColor(TFT_GREEN, TFT_BLACK);
      String dayDateStr = day + ", " + date;
      int dateWidth = dayDateStr.length() * 6;
      display.setCursor((SCREEN_WIDTH - dateWidth) / 2, 98);
      display.print(dayDateStr);

      // Progress bar representing seconds
      int progressWidth = (second * 100) / 60;
      display.drawRect(14, 122, 100, 8, TFT_ORANGE);
      display.fillRect(16, 124, progressWidth, 4, TFT_YELLOW);

    } else {
      // Style 0: Classic Rounded Border (Full Screen 128x160)
      display.drawRoundRect(4, 4, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 8, 8, TFT_CYAN);
      display.drawRoundRect(8, 8, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 16, 6, TFT_CYAN);

      // Time
      display.setTextSize(3);
      display.setTextColor(TFT_WHITE, TFT_BLACK);
      char timeStr[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispHour, minute);
      int tW = strlen(timeStr) * 18;
      display.setCursor((SCREEN_WIDTH - tW) / 2, 42);
      display.print(timeStr);

      // Seconds & AM/PM
      display.setTextSize(1);
      display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      if (is12Hour) {
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        display.setCursor(55, 78);
        display.print(ampm);
      } else {
        char secStr[16];
        snprintf(secStr, sizeof(secStr), "%02d SEC", second);
        display.setCursor(48, 78);
        display.print(secStr);
      }

      // Divider
      display.drawFastHLine(24, 102, SCREEN_WIDTH - 48, TFT_CYAN);

      // Date
      display.setTextSize(1);
      display.setTextColor(TFT_CYAN, TFT_BLACK);
      String dayDateStr = day + ", " + date;
      int dateWidth = dayDateStr.length() * 6;
      display.setCursor((SCREEN_WIDTH - dateWidth) / 2, 116);
      display.print(dayDateStr);
    }
  }

  void drawTextScreen() {
    display.setTextWrap(false);
    
    // Draw rounded background notification container
    display.drawRoundRect(4, 4, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 8, 8, TFT_BLUE);
    display.drawRoundRect(6, 6, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 12, 6, TFT_NAVY);
    
    // 1. Draw Mini Logo Face
    display.fillRoundRect(12, 12, 16, 12, 3, TFT_WHITE);
    display.fillCircle(16, 17, 1, TFT_BLACK);
    display.fillCircle(24, 17, 1, TFT_BLACK);
    display.drawFastHLine(18, 21, 4, TFT_BLACK);

    // 2. Draw Title Header
    display.setTextSize(1);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.setCursor(34, 14);
    
    String displayTitle = notificationTitle;
    if (displayTitle.length() == 0) {
      displayTitle = "Alert";
    }
    // Truncate to fit
    if (displayTitle.length() > 14) {
      displayTitle = displayTitle.substring(0, 11) + "...";
    }
    display.print(displayTitle);
    
    // Separator line
    display.drawFastHLine(12, 28, SCREEN_WIDTH - 24, TFT_LIGHTGREY);

    // 3. Draw Body Text
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextSize(2);
    
    // Short text -> center statically. Long text -> scroll horizontally.
    if (notificationText.length() <= 8) {
      int textW = notificationText.length() * 12;
      int startX = (SCREEN_WIDTH - textW) / 2;
      display.setCursor(startX, 74);
      display.print(notificationText);
    } else {
      // Center y is 74 (vertical midpoint)
      display.setCursor(scrollPos, 74);
      display.print(notificationText);
    }

    // Footnote Accent
    display.setTextSize(1);
    display.setTextColor(TFT_DARKGREY, TFT_BLACK);
    display.setCursor(38, 136);
    display.print("Luna Notif");
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

  void drawMapScreen(int hour, int minute, bool is12Hour) {
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextWrap(false);

    // Draw Map Frame Header
    display.drawRoundRect(4, 4, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 8, 8, TFT_GREEN);
    display.setTextColor(TFT_GREEN, TFT_BLACK);
    display.setTextSize(1);
    display.setCursor(12, 10);
    display.print("NAVIGATION");
    display.drawFastHLine(10, 20, SCREEN_WIDTH - 20, TFT_GREEN);

    // Draw Giant Arrow in center (centered around x=64, y=70)
    display.setTextColor(TFT_WHITE);
    if (mapDirection.indexOf("LEFT") >= 0) {
      // Bold LEFT Turn Arrow
      display.fillTriangle(20, 70, 44, 46, 44, 94, TFT_WHITE); // Arrowhead
      display.fillRect(44, 60, 36, 20, TFT_WHITE);            // Horizontal shaft
      display.fillRect(60, 80, 20, 30, TFT_WHITE);            // Vertical shaft
    } else if (mapDirection.indexOf("RIGHT") >= 0) {
      // Bold RIGHT Turn Arrow
      display.fillTriangle(108, 70, 84, 46, 84, 94, TFT_WHITE); // Arrowhead
      display.fillRect(48, 60, 36, 20, TFT_WHITE);            // Horizontal shaft
      display.fillRect(48, 80, 20, 30, TFT_WHITE);            // Vertical shaft
    } else if (mapDirection.indexOf("UTURN") >= 0 || mapDirection.indexOf("U-TURN") >= 0) {
      // Bold U-Turn
      display.drawCircle(64, 70, 22, TFT_WHITE);
      display.drawCircle(64, 70, 20, TFT_WHITE);
      display.drawCircle(64, 70, 18, TFT_WHITE);
      display.fillRect(38, 70, 52, 34, TFT_BLACK);          // Clear bottom
      display.fillRect(42, 70, 6, 18, TFT_WHITE);            // Left leg
      display.fillRect(80, 70, 6, 32, TFT_WHITE);            // Right leg
      display.fillTriangle(45, 98, 35, 84, 55, 84, TFT_WHITE); // Left leg arrowhead down
    } else if (mapDirection.indexOf("ROUNDABOUT") >= 0 || mapDirection.indexOf("ROUND") >= 0 || mapDirection.indexOf("ROTARY") >= 0) {
      // Bold Roundabout
      display.drawCircle(64, 70, 18, TFT_WHITE);
      display.drawCircle(64, 70, 16, TFT_WHITE);
      display.fillRect(57, 63, 14, 14, TFT_BLACK);          // Center hole
      display.fillRect(58, 86, 12, 20, TFT_BLACK);          // Entrance hole
      display.fillRect(78, 66, 10, 8, TFT_WHITE);            // Exit shaft
      display.fillTriangle(102, 70, 88, 56, 88, 84, TFT_WHITE); // Exit arrowhead right
    } else { // STRAIGHT / default
      // Bold Straight Arrow
      display.fillRect(52, 60, 24, 40, TFT_WHITE);            // Shaft
      display.fillTriangle(64, 28, 34, 60, 94, 60, TFT_WHITE); // Arrowhead
    }

    // Divider at bottom
    display.drawFastHLine(10, 126, SCREEN_WIDTH - 20, TFT_GREEN);

    // Left side: Distance
    if (mapDistance != "" && mapDistance != "--") {
      display.setTextColor(TFT_YELLOW, TFT_BLACK);
      drawMixedSizeText(mapDistance, 8, 134, 141);
    }

    // Right side: Remaining Time (stored in mapDescription)
    if (mapDescription != "") {
      display.setTextColor(TFT_GREEN, TFT_BLACK);
      int timeWidth = getMixedSizeTextWidth(mapDescription);
      int startX = SCREEN_WIDTH - timeWidth - 8;
      if (startX < 64) startX = 64;
      drawMixedSizeText(mapDescription, startX, 134, 141);
    }
  }
};

#endif // EXPRESSIONS_H
