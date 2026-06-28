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
  String notificationText;
  int scrollPos;
  unsigned long lastScrollTime;
  String stateLabel;
  int frameDelayMs;

public:
  MarioFace(Adafruit_SSD1306& disp) 
    : display(disp), currentExpr(EXPR_IDLE), targetExpr(EXPR_IDLE), defaultExpr(EXPR_IDLE), stateLabel("IDLE"), frameDelayMs(100) {
    currentFrame = 0;
    currentGifIndex = 0;
    lastFrameTime = 0;
    gifFinished = false;

    notificationText = "";
    scrollPos = SCREEN_WIDTH;
    lastScrollTime = 0;
  }

  void setDefaultExpression(Expression expr) {
    defaultExpr = expr;
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
  }

  Expression getExpression() {
    return currentExpr;
  }

  void setNotificationText(String text) {
    notificationText = text;
    setExpression(EXPR_TEXT);
  }

  void setStateLabel(String label) {
    stateLabel = label;
  }

  // Set which entry in ALL_GIFS_TABLE to display (for EXPR_ALL_GIF mode)
  void setGifIndex(int idx) {
    currentGifIndex = idx;
    currentFrame = 0;
    gifFinished = false;
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
        
        // Loop back to idle if scrolled off screen
        if (scrollPos < -textLength) {
          setExpression(EXPR_IDLE);
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

  void draw(int hour, int minute, int second) {
    display.clearDisplay();

    // 1. Draw Text Screen if active
    if (currentExpr == EXPR_TEXT) {
      drawTextScreen();
    } else if (currentExpr == EXPR_CLOCK) {
      drawClockScreen(hour, minute, second);
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
  void drawClockScreen(int hour, int minute, int second) {
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
    display.setCursor(16, 24);
    char timeStr[9];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hour, minute, second);
    display.print(timeStr);
    
    // Bottom status
    display.setTextSize(1);
    display.setCursor(10, 46);
    display.print("[Tap to close clock]");
  }
  void drawTextScreen() {
    // Draw small animated face at the top-left
    display.fillRoundRect(8, 4, 16, 12, 3, SSD1306_WHITE);
    display.fillCircle(12, 9, 2, SSD1306_BLACK); // Left eye
    display.fillCircle(20, 9, 2, SSD1306_BLACK); // Right eye
    display.drawFastHLine(14, 13, 4, SSD1306_WHITE); // Smile outline

    // Draw notification title
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(32, 6);
    display.print("Notification:");
    display.drawFastHLine(0, 20, SCREEN_WIDTH, SSD1306_WHITE);

    // Draw scrolling text
    display.setTextSize(2);
    display.setCursor(scrollPos, 35);
    display.print(notificationText);
  }
};

#endif // EXPRESSIONS_H
