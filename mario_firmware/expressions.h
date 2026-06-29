#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "mochi_bitmaps.h"

class MarioFace {
private:
  Adafruit_SSD1306& display;
  Expression currentExpr;
  Expression targetExpr;
  Expression defaultExpr; // Custom default expression for Idle state

  // Animation frame control
  int currentFrame;
  int currentGifIndex;        // index into ALL_GIFS_TABLE for EXPR_ALL_GIF mode
  unsigned long lastFrameTime;
  bool gifFinished;

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

public:
  MarioFace(Adafruit_SSD1306& disp) 
    : display(disp), currentExpr(EXPR_IDLE), targetExpr(EXPR_IDLE), defaultExpr(EXPR_IDLE), stateLabel("IDLE"), frameDelayMs(100) {
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

  void setExpression(Expression expr) {
    if (currentExpr == expr) return;
    targetExpr = expr;
    currentExpr = expr; // Direct change for responsiveness
    currentFrame = 0;
    lastFrameTime = millis();
    gifFinished = false;
    
    // If text expression, reset scroll position
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

  void setMapNavigation(String direction, String distance) {
    mapDirection = direction;
    mapDistance = distance;
    setExpression(EXPR_MAP);
  }

  void setStateLabel(String label) {
    stateLabel = label;
  }

  String getStateLabel() {
    return stateLabel;
  }

  // Set which entry in ALL_GIFS_TABLE to display (for EXPR_ALL_GIF mode)
  void setGifIndex(int idx) {
    currentGifIndex = idx;
    currentFrame = 0;
    gifFinished = false;
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

  void update() {
    unsigned long now = millis();

    // 1. Frame Animation logic
    if (currentExpr != EXPR_TEXT) {
      int maxFrames = 1;
      Expression exprToUpdate = currentExpr;
      if (exprToUpdate == EXPR_IDLE) {
        exprToUpdate = defaultExpr;
      }
      switch (exprToUpdate) {
        case EXPR_IDLE: maxFrames = ep_relaxed_frame_count; break;  // relaxed.gif
        case EXPR_HAPPY: maxFrames = ep_happy_frame_count; break;
        case EXPR_SAD: maxFrames = ep_crying_frame_count; break;     // crying.gif
        case EXPR_ANGRY: maxFrames = ep_angry_frame_count; break;
        case EXPR_SURPRISED: maxFrames = ep_surprised_frame_count; break;
        case EXPR_SLEEPING: maxFrames = ep_sleepy_frame_count; break; // sleepy.gif
        case EXPR_WINK: maxFrames = ep_wink_frame_count; break;
        case EXPR_CLOCK: maxFrames = 1; break;
        case EXPR_ALL_GIF: {
          // Read frame count from PROGMEM master table (int = 4 bytes on ESP32)
          if (currentGifIndex < ALL_GIFS_COUNT) {
            maxFrames = (int)pgm_read_dword(&ALL_GIFS_TABLE[currentGifIndex].count);
          } else {
            maxFrames = 1;
          }
          break;
        }
        default: maxFrames = 1; break;
      }

      // Play Mochi GIF frames at custom dynamic frame speed
      if (now - lastFrameTime > frameDelayMs) {
        lastFrameTime = now;
        currentFrame++;
        if (currentFrame >= maxFrames) {
          currentFrame = 0;
          gifFinished = true;
        }
      }
    }

    // 2. Scroll text logic
    if (currentExpr == EXPR_TEXT) {
      if (now - lastScrollTime > 30) {
        lastScrollTime = now;
        scrollPos -= 2;
        int textLength = notificationText.length() * 12; // size-2 font: ~12px per char
        
        // Loop scrolling until duration timeout in main loop
        if (scrollPos < -textLength) {
          scrollPos = SCREEN_WIDTH;
        }
      }
    }
  }

  void drawSettingsMenu(int option, bool selected, bool bleOn, int speed) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    // Draw Title
    display.setTextSize(1);
    display.setCursor(20, 2);
    display.print("--- SETTINGS ---");
    display.drawFastHLine(0, 12, SCREEN_WIDTH, SSD1306_WHITE);
    
    // Draw BLE option
    display.setCursor(15, 18);
    if (option == 0) {
      display.print(selected ? ">> BLE: " : "> BLE: ");
    } else {
      display.print("   BLE: ");
    }
    display.print(bleOn ? "ON" : "OFF");
    
    // Draw Speed option
    display.setCursor(15, 30);
    if (option == 1) {
      display.print(selected ? ">> SPEED: " : "> SPEED: ");
    } else {
      display.print("   SPEED: ");
    }
    display.print(speed);
    display.print("ms");
    
    // Draw Save option
    display.setCursor(15, 42);
    if (option == 2) {
      display.print(selected ? ">> [ SAVE ]" : "> [ SAVE ]");
    } else {
      display.print("   [ SAVE ]");
    }
    
    // Draw Exit option
    display.setCursor(15, 54);
    if (option == 3) {
      display.print(selected ? ">> [ EXIT ]" : "> [ EXIT ]");
    } else {
      display.print("   [ EXIT ]");
    }
    
    display.display();
  }

  void draw(int hour, int minute, int second, bool is12Hour = false) {
    display.clearDisplay();

    // 1. Draw Text Screen if active
    if (currentExpr == EXPR_TEXT) {
      drawTextScreen();
    } else if (currentExpr == EXPR_CLOCK) {
      drawClockScreen(hour, minute, second, is12Hour);
    } else if (currentExpr == EXPR_MAP) {
      drawMapScreen();
    } else if (currentExpr == EXPR_ALL_GIF) {
      // ── All-GIF mode: render from the PROGMEM master table ──
      if (currentGifIndex < ALL_GIFS_COUNT) {
        const unsigned char* const* frames =
          (const unsigned char* const*)pgm_read_ptr(&ALL_GIFS_TABLE[currentGifIndex].frames);
        int frameCount = (int)pgm_read_dword(&ALL_GIFS_TABLE[currentGifIndex].count);
        int safeFrame = (currentFrame < frameCount) ? currentFrame : 0;
        const unsigned char* frameData =
          (const unsigned char*)pgm_read_ptr(&frames[safeFrame]);
        if (frameData) {
          display.drawBitmap(0, 0, frameData, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
        }
      }
    } else {
      // ── Standard 7-expression mode (also used for BLE-triggered expressions) ──
      const unsigned char* frameData = nullptr;
      int frameIdx = currentFrame;
      Expression exprToDraw = currentExpr;
      if (exprToDraw == EXPR_IDLE) {
        exprToDraw = defaultExpr;
      }

      switch (exprToDraw) {
        case EXPR_IDLE:     // relaxed.gif
          if (frameIdx < ep_relaxed_frame_count)
            frameData = (const unsigned char*)pgm_read_ptr(&ep_relaxed_frames[frameIdx]);
          break;
        case EXPR_HAPPY:
          if (frameIdx < ep_happy_frame_count)
            frameData = (const unsigned char*)pgm_read_ptr(&ep_happy_frames[frameIdx]);
          break;
        case EXPR_SAD:      // crying.gif
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
        case EXPR_SLEEPING: // sleepy.gif
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
        display.drawBitmap(0, 0, frameData, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
      }
    }

    display.display();
  }

private:
  void drawClockScreen(int hour, int minute, int second, bool is12Hour) {
    // Sleek premium border
    display.drawRoundRect(0, 0, 128, 64, 4, SSD1306_WHITE);
    display.drawRoundRect(2, 2, 124, 60, 2, SSD1306_WHITE);
    
    // Header
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(38, 8);
    display.print("- CLOCK -");
    
    // Time
    display.setTextSize(2);
    char timeStr[12];
    if (is12Hour) {
      int dispHour = hour % 12;
      if (dispHour == 0) dispHour = 12;
      const char* ampm = (hour >= 12) ? "PM" : "AM";
      snprintf(timeStr, sizeof(timeStr), "%2d:%02d %s", dispHour, minute, ampm);
      display.setCursor(14, 24);
    } else {
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hour, minute, second);
      display.setCursor(16, 24);
    }
    display.print(timeStr);
    
    // Bottom status
    display.setTextSize(1);
    display.setCursor(10, 46);
    display.print("[Tap to close clock]");
  }
  void drawTextScreen() {
    // 1. Draw header background and small animated face
    display.fillRoundRect(4, 2, 14, 10, 2, SSD1306_WHITE);
    display.fillCircle(7, 6, 1, SSD1306_BLACK); // Left eye
    display.fillCircle(14, 6, 1, SSD1306_BLACK); // Right eye
    display.drawFastHLine(9, 9, 3, SSD1306_WHITE); // Smile

    // 2. Draw notification title (truncated to fit screen size)
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(24, 3);
    
    String displayTitle = notificationTitle;
    if (displayTitle.length() == 0) {
      displayTitle = "Notification";
    }
    // Truncate title if too long (max ~16 chars at size 1 to fit next to face)
    if (displayTitle.length() > 16) {
      displayTitle = displayTitle.substring(0, 13) + "...";
    }
    display.print(displayTitle);
    
    // Draw divider line separating header from body
    display.drawFastHLine(0, 14, SCREEN_WIDTH, SSD1306_WHITE);

    // 3. Draw Body Text
    // Center vertically in the remaining space (y=15 to 64, height=49)
    // Size 2 text is 16px high, so centered y = 15 + (49 - 16)/2 = 31
    display.setTextSize(2);
    
    // If text is short, center it statically instead of scrolling!
    // Max characters that can fit statically on screen in size 2 is 10 chars (10 * 12 = 120px)
    if (notificationText.length() <= 10) {
      int textW = notificationText.length() * 12;
      int startX = (SCREEN_WIDTH - textW) / 2;
      display.setCursor(startX, 31);
      display.print(notificationText);
    } else {
      // Scroll long text
      display.setCursor(scrollPos, 31);
      display.print(notificationText);
    }
  }

  void drawMapScreen() {
    // 1. Draw header
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(4, 2);
    display.print("NAVIGATION");
    display.drawFastHLine(0, 12, SCREEN_WIDTH, SSD1306_WHITE);

    // 2. Draw Arrow based on mapDirection (centered around x=24)
    if (mapDirection == "LEFT") {
      display.fillRect(20, 24, 12, 6, SSD1306_WHITE); // horizontal shaft
      display.fillRect(26, 30, 6, 18, SSD1306_WHITE); // vertical shaft
      display.drawTriangle(20, 17, 20, 37, 8, 27, SSD1306_WHITE); // head pointing left
    } else if (mapDirection == "RIGHT") {
      display.fillRect(16, 24, 12, 6, SSD1306_WHITE); // horizontal shaft
      display.fillRect(16, 30, 6, 18, SSD1306_WHITE); // vertical shaft
      display.drawTriangle(28, 17, 28, 37, 40, 27, SSD1306_WHITE); // head pointing right
    } else if (mapDirection == "UTURN") {
      display.drawCircle(24, 32, 12, SSD1306_WHITE);
      display.fillRect(18, 32, 12, 20, SSD1306_BLACK); // clear bottom middle
      display.fillRect(12, 32, 6, 16, SSD1306_WHITE); // left leg
      display.fillRect(30, 32, 6, 16, SSD1306_WHITE); // right leg
      display.drawTriangle(12, 34, 12, 46, 4, 40, SSD1306_WHITE); // head pointing down
    } else if (mapDirection == "ROUNDABOUT") {
      display.drawCircle(24, 32, 12, SSD1306_WHITE);
      display.drawCircle(24, 32, 6, SSD1306_BLACK);
      display.drawTriangle(32, 24, 40, 30, 32, 36, SSD1306_WHITE); // exit arrow
    } else { // STRAIGHT / default
      display.fillRect(21, 28, 6, 20, SSD1306_WHITE); // vertical shaft
      display.drawTriangle(14, 28, 34, 28, 24, 16, SSD1306_WHITE); // head pointing up
    }

    // 3. Draw Distance / Meter Text on the right side
    display.setTextSize(1);
    display.setCursor(60, 22);
    display.print("Distance:");
    
    display.setTextSize(2);
    display.setCursor(60, 36);
    display.print(mapDistance);
  }
};

#endif // EXPRESSIONS_H
