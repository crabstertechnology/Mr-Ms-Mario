#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "config.h"
#include "mochi_bitmaps.h"

class LunaFace {
private:
  Adafruit_ST7789& display;
  Expression currentExpr;
  Expression targetExpr;
  Expression defaultExpr; // Custom default expression for Idle state
  Expression lastExpr;    // Track previous expression for screen clearing

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

public:
  String headerText;
  LunaFace(Adafruit_ST7789& disp) 
    : display(disp), currentExpr(EXPR_IDLE), targetExpr(EXPR_IDLE), defaultExpr(EXPR_IDLE), lastExpr(EXPR_TEXT), stateLabel("IDLE"), frameDelayMs(100), expressionChanged(true) {
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
    headerText = "";
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
  // measured from actual GIF file timing minus OLED I2C render overhead (~45ms).
  // Source: analyze_gifs.py on 63 animation files in mobile_app/animations/
  int getGifFrameDelay(int gifIndex) {
    // Per-GIF delays derived from GIF file frame timing (original - 45ms overhead)
    // Sorted by index matching ALL_GIFS_TABLE order:
    // ADORE ANGRY BLANK BLINDING BRAVE BUZZING CONTEMPT CRYING DANCING DEVIL
    // DISTRACTED DIZZY DOWN DROWSY ENCOURAGEMENT ENERGETIC ENRAGED EVIL FAST FIERCE
    // FURIOUS GIGGLE GLOWING GROWING HANDSOME HAPPY HELLO IRRITATED LAUGHING LEFT
    // LOVE MENACING MISTAKE PLAYFUL POLICE RAIN RELAXED RIGHT RUSH SCARED
    // SERENE SHRINK SHY SICK SLEEPY SMILE SMIRK SMOKE SNEEZE SOBBING
    // SPARKLE SPEED SPLASH SPRAYING SQUINT SURPRISED SUSHI SWINGING TEASING TOUGH
    // WEEPING WINK YAWN
    static const uint8_t gifDelays[] PROGMEM = {
      112, // 0  ADORE       (120ms orig)
      112, // 1  ANGRY       (120ms orig)
      52,  // 2  BLANK       ( 60ms orig)
      252, // 3  BLINDING    (260ms orig)
      112, // 4  BRAVE       (120ms orig)
      112, // 5  BUZZING     (120ms orig)
      92,  // 6  CONTEMPT    (100ms orig)
      112, // 7  CRYING      (120ms orig)
      112, // 8  DANCING     (120ms orig)
      92,  // 9  DEVIL       (100ms orig)
      52,  // 10 DISTRACTED  ( 60ms orig)
      112, // 11 DIZZY       (120ms orig)
      52,  // 12 DOWN        ( 60ms orig)
      112, // 13 DROWSY      (120ms orig)
      92,  // 14 ENCOURAGEMENT(100ms orig)
      52,  // 15 ENERGETIC   ( 60ms orig)
      112, // 16 ENRAGED     (120ms orig)
      92,  // 17 EVIL        (100ms orig)
      92,  // 18 FAST        (100ms orig)
      112, // 19 FIERCE      (120ms orig)
      92,  // 20 FURIOUS     (100ms orig)
      112, // 21 GIGGLE      (120ms orig)
      112, // 22 GLOWING     (120ms orig)
      112, // 23 GROWING     (120ms orig)
      112, // 24 HANDSOME    (120ms orig)
      112, // 25 HAPPY       (120ms orig)
      92,  // 26 HELLO       (100ms orig)
      112, // 27 IRRITATED   (120ms orig)
      92,  // 28 LAUGHING    (100ms orig)
      52,  // 29 LEFT        ( 60ms orig)
      112, // 30 LOVE        (120ms orig)
      92,  // 31 MENACING    (100ms orig)
      112, // 32 MISTAKE     (120ms orig)
      112, // 33 PLAYFUL     (120ms orig)
      112, // 34 POLICE      (120ms orig)
      112, // 35 RAIN        (120ms orig)
      92,  // 36 RELAXED     (100ms orig)
      52,  // 37 RIGHT       ( 60ms orig)
      92,  // 38 RUSH        (100ms orig)
      112, // 39 SCARED      (120ms orig)
      112, // 40 SERENE      (120ms orig)
      52,  // 41 SHRINK      ( 60ms orig)
      52,  // 42 SHY         ( 60ms orig)
      112, // 43 SICK        (120ms orig)
      92,  // 44 SLEEPY      (100ms orig)
      112, // 45 SMILE       (120ms orig)
      52,  // 46 SMIRK       ( 60ms orig)
      92,  // 47 SMOKE       (100ms orig)
      112, // 48 SNEEZE      (120ms orig)
      112, // 49 SOBBING     (120ms orig)
      112, // 50 SPARKLE     (120ms orig)
      52,  // 51 SPEED       ( 60ms orig)
      112, // 52 SPLASH      (120ms orig)
      112, // 53 SPRAYING    (120ms orig)
      52,  // 54 SQUINT      ( 60ms orig)
      112, // 55 SURPRISED   (120ms orig)
      112, // 56 SUSHI       (120ms orig)
      112, // 57 SWINGING    (120ms orig)
      112, // 58 TEASING     (120ms orig)
      122, // 59 TOUGH       (130ms orig)
      92,  // 60 WEEPING     (100ms orig)
      112, // 61 WINK        (120ms orig)
      112, // 62 YAWN        (120ms orig)
    };
    if (gifIndex >= 0 && gifIndex < ALL_GIFS_COUNT) {
      return pgm_read_byte(&gifDelays[gifIndex]);
    }
    return frameDelayMs; // fallback to global setting
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
    currentExpr = expr; // Direct change for responsiveness
    currentFrame = 0;
    lastFrameTime = millis();
    gifFinished = false;
    expressionChanged = true;
    
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

  // Set which entry in ALL_GIFS_TABLE to display (for EXPR_ALL_GIF mode)
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

      // Play Mochi GIF frames using accurate per-GIF timing.
      // Delay is determined by actual GIF file frame duration minus OLED render overhead.
      int activeDelay;
      if (exprToUpdate == EXPR_ALL_GIF) {
        activeDelay = getGifFrameDelay(currentGifIndex);
      } else {
        // Standard 7-expression GIFs: use their actual measured delays (original - 8ms)
        // relaxed=92, happy=112, crying=112, angry=112, surprised=112, sleepy=92, wink=112
        switch (exprToUpdate) {
          case EXPR_IDLE:      activeDelay = 92; break;  // relaxed.gif  100ms orig
          case EXPR_HAPPY:     activeDelay = 112; break;  // happy.gif    120ms orig
          case EXPR_SAD:       activeDelay = 112; break;  // crying.gif   120ms orig
          case EXPR_ANGRY:     activeDelay = 112; break;  // angry.gif    120ms orig
          case EXPR_SURPRISED: activeDelay = 112; break;  // surprised    120ms orig
          case EXPR_SLEEPING:  activeDelay = 92; break;  // sleepy.gif   100ms orig
          case EXPR_WINK:      activeDelay = 112; break;  // wink.gif     120ms orig
          default:             activeDelay = 100; break;
        }
      }
      // Scale activeDelay based on user-configured frameDelayMs (slider value from app)
      // frameDelayMs defaults to 100. Scale = frameDelayMs / 100.0
      if (frameDelayMs != 100) {
        activeDelay = (int)(activeDelay * (frameDelayMs / 100.0f));
      }
      activeDelay = max(20, activeDelay); // never go below 20ms to prevent choking the display

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
        scrollPos -= 1;
        int textLength = notificationText.length() * 12; // size-2 font: ~12px per char
        
        // Loop scrolling until duration timeout in main loop
        if (scrollPos < -textLength) {
          scrollPos = SCREEN_WIDTH;
        }
        changed = true;
      }
    }
    return changed;
  }

  void drawSettingsMenu(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    display.fillScreen(ST77XX_BLACK);
    display.setTextColor(ST77XX_WHITE);
    
    // Draw Title
    display.setTextSize(2);
    display.setCursor(45, 15);
    display.print("--- SETTINGS ---");
    display.drawFastHLine(0, 38, SCREEN_WIDTH, ST77XX_WHITE);
    
    // Scrolling menu window calculation (displays 5 items)
    int startOpt = 0;
    if (option >= 4) {
      startOpt = option - 4;
    }
    if (startOpt > 2) startOpt = 2; // total 7 options, so max start index is 7 - 5 = 2
    
    for (int i = 0; i < 5; i++) {
      int optIdx = startOpt + i;
      if (optIdx >= 7) break;
      
      int yPos = 55 + i * 32;
      display.setCursor(10, yPos);
      
      bool isCurrent = (option == optIdx);
      if (isCurrent) {
        display.print(selected ? ">> " : "> ");
      } else {
        display.print("   ");
      }
      
      switch (optIdx) {
        case 0:
          display.print("BLE: ALWAYS ON");
          break;
        case 1:
          display.print("SPEED: ");
          display.print(speed);
          display.print("ms");
          break;
        case 2:
          display.print("CLOCK: STYLE ");
          display.print(clockStyle);
          break;
        case 3:
          display.print("INVERT: ");
          display.print(invertOn ? "ON" : "OFF");
          break;
        case 4:
          display.print("BRIGHT: ");
          if (brightness == 1) display.print("LOW");
          else if (brightness == 2) display.print("MED");
          else display.print("HIGH");
          break;
        case 5:
          display.print("[ SAVE ]");
          break;
        case 6:
          display.print("[ EXIT ]");
          break;
      }
    }
  }

  void draw(int hour, int minute, int second, String day, String date, int style = 0, bool is12Hour = false) {
    // Determine if we need a full screen clear to prevent high-frequency flicker
    bool needsClear = (currentExpr != lastExpr) || 
                      (currentExpr == EXPR_CLOCK) || 
                      (currentExpr == EXPR_MAP) || 
                      (currentExpr == EXPR_TEXT);
    
    if (needsClear) {
      display.fillScreen(ST77XX_BLACK);
      lastExpr = currentExpr;
    }

    // Determine the dynamic expression color based on mood
    uint16_t exprColor = getExpressionColor(currentExpr, currentGifIndex);

    // 1. Draw Text Screen if active
    if (currentExpr == EXPR_TEXT) {
      drawTextScreen();
    } else if (currentExpr == EXPR_CLOCK) {
      drawClockScreen(hour, minute, second, day, date, style, is12Hour);
    } else if (currentExpr == EXPR_MAP) {
      drawMapScreen(hour, minute, is12Hour);
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
          // Stream scaled 1.875x color bitmap to fill the screen (240x120 centered)
          drawFastScaledBitmap(0, 60, frameData, 128, 64, exprColor, ST77XX_BLACK);
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
        // Stream scaled 1.875x color bitmap to fill the screen (240x120 centered)
        drawFastScaledBitmap(0, 60, frameData, 128, 64, exprColor, ST77XX_BLACK);
      }
    }

    // Draw/update header text only when it changes or when screen is cleared
    static String lastHeaderText = "";
    if (currentExpr != EXPR_TEXT) {
      if (headerText != lastHeaderText || needsClear) {
        if (!needsClear) {
          // Clear header area (240x23)
          display.fillRect(0, 0, SCREEN_WIDTH, 23, ST77XX_BLACK);
        }
        if (headerText.length() > 0) {
          display.setTextColor(ST77XX_WHITE);
          display.setTextSize(2);
          int textW = headerText.length() * 12;
          int startX = (SCREEN_WIDTH - textW) / 2;
          if (startX < 0) startX = 0;
          display.setCursor(startX, 4);
          display.print(headerText);
          display.drawFastHLine(0, 22, SCREEN_WIDTH, ST77XX_WHITE);
        }
        lastHeaderText = headerText;
      }
    }
  }

private:
  uint16_t getExpressionColor(Expression expr, int gifIndex) {
    if (expr == EXPR_CLOCK || expr == EXPR_MAP || expr == EXPR_TEXT) {
      return ST77XX_WHITE;
    }
    
    Expression exprToEvaluate = expr;
    if (exprToEvaluate == EXPR_IDLE) {
      exprToEvaluate = defaultExpr;
    }

    if (exprToEvaluate == EXPR_ALL_GIF) {
      if (gifIndex >= 0 && gifIndex < ALL_GIFS_COUNT) {
        char nameBuf[32];
        strcpy_P(nameBuf, (char*)pgm_read_ptr(&ALL_GIFS_TABLE[gifIndex].name));
        String name = String(nameBuf);
        name.toUpperCase();
        
        if (name.indexOf("ANGRY") >= 0 || name.indexOf("DEVIL") >= 0 || 
            name.indexOf("EVIL") >= 0 || name.indexOf("FIERCE") >= 0 || 
            name.indexOf("FURIOUS") >= 0 || name.indexOf("MENACING") >= 0 || 
            name.indexOf("ENRAGED") >= 0 || name.indexOf("TOUGH") >= 0 || 
            name.indexOf("RUSH") >= 0 || name.indexOf("SPEED") >= 0) {
          return 0xF800; // Red
        }
        if (name.indexOf("HAPPY") >= 0 || name.indexOf("HELLO") >= 0 || 
            name.indexOf("DANCING") >= 0 || name.indexOf("GIGGLE") >= 0 || 
            name.indexOf("LAUGHING") >= 0 || name.indexOf("PLAYFUL") >= 0 || 
            name.indexOf("SMILE") >= 0 || name.indexOf("SMIRK") >= 0 || 
            name.indexOf("TEASING") >= 0 || name.indexOf("WINK") >= 0 || 
            name.indexOf("SPARKLE") >= 0 || name.indexOf("GLOWING") >= 0) {
          return 0xFEE0; // Gold / Yellow
        }
        if (name.indexOf("SAD") >= 0 || name.indexOf("CRYING") >= 0 || 
            name.indexOf("SOBBING") >= 0 || name.indexOf("WEEPING") >= 0 || 
            name.indexOf("SICK") >= 0 || name.indexOf("DIZZY") >= 0 || 
            name.indexOf("DROWSY") >= 0 || name.indexOf("SLEEPY") >= 0 || 
            name.indexOf("YAWN") >= 0 || name.indexOf("RAIN") >= 0) {
          return 0x5DFF; // Sky Blue
        }
        if (name.indexOf("SCARED") >= 0 || name.indexOf("MISTAKE") >= 0 || 
            name.indexOf("DOWN") >= 0 || name.indexOf("SURPRISED") >= 0 || 
            name.indexOf("SHY") >= 0 || name.indexOf("CONTEMPT") >= 0) {
          return 0xFD20; // Orange
        }
        if (name.indexOf("RELAXED") >= 0 || name.indexOf("SERENE") >= 0 || 
            name.indexOf("BUZZING") >= 0 || name.indexOf("BRAVE") >= 0 || 
            name.indexOf("ENERGETIC") >= 0) {
          return 0x07E0; // Green
        }
        if (name.indexOf("LOVE") >= 0 || name.indexOf("ADORE") >= 0) {
          return 0xF81F; // Pink / Magenta
        }
      }
      return 0x07FF; // Cyan
    }

    switch (exprToEvaluate) {
      case EXPR_HAPPY:
        return 0xFEE0; // Gold / Yellow
      case EXPR_SAD:
        return 0x5DFF; // Sky Blue
      case EXPR_ANGRY:
        return 0xF800; // Red
      case EXPR_SURPRISED:
        return 0xFD20; // Orange
      case EXPR_SLEEPING:
        return 0xC81F; // Purple
      case EXPR_WINK:
        return 0xF81F; // Pink / Magenta
      default:
        return 0x07FF; // Cyan
    }
  }

  void drawFastScaledBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) {
    int16_t targetW = 240;
    int16_t targetH = 120;

    // Use GFX library setAddrWindow which handles proper rotation and screen offsets
    display.startWrite();
    display.setAddrWindow(x, y, targetW, targetH);
    
    // Ensure we are in SPI DATA mode
    digitalWrite(TFT_DC, HIGH);

    uint8_t colorHigh = color >> 8;
    uint8_t colorLow = color & 0xFF;
    uint8_t bgHigh = bg >> 8;
    uint8_t bgLow = bg & 0xFF;

    for (int16_t dy = 0; dy < targetH; dy++) {
      int16_t sy = (dy * 8) / 15;
      if (sy >= h) sy = h - 1;
      const uint8_t *rowPtr = bitmap + sy * ((w + 7) / 8);

      for (int16_t dx = 0; dx < targetW; dx++) {
        int16_t sx = (dx * 8) / 15;
        if (sx >= w) sx = w - 1;

        uint8_t byteVal = pgm_read_byte(rowPtr + (sx / 8));
        bool pixelVal = (byteVal & (0x80 >> (sx & 7))) != 0;

        if (pixelVal) {
          SPI.transfer(colorHigh);
          SPI.transfer(colorLow);
        } else {
          SPI.transfer(bgHigh);
          SPI.transfer(bgLow);
        }
      }
    }

    display.endWrite();
  }

  void drawClockScreen(int hour, int minute, int second, String day, String date, int style, bool is12Hour) {
    display.setTextColor(ST77XX_WHITE);

    if (style == 1) {
      // Style 1: Minimalist
      display.setTextSize(5);
      char timeNoSec[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeNoSec, sizeof(timeNoSec), "%02d:%02d", dispHour, minute);
      display.setCursor(30, 75);
      display.print(timeNoSec);

      // Suffix (AM/PM or seconds) in size 2
      display.setTextSize(2);
      if (is12Hour) {
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        display.setCursor(185, 75);
        display.print(ampm);
      } else {
        char secStr[3];
        snprintf(secStr, sizeof(secStr), "%02d", second);
        display.setCursor(185, 100);
        display.print(secStr);
      }

      // Date & Day at bottom
      display.setCursor(45, 150);
      String dayDateStr = day + ", " + date;
      display.print(dayDateStr);

    } else if (style == 2) {
      // Style 2: Analog Split Face
      // Left side: Analog Clock. Center (65, 120), Radius 50
      display.drawCircle(65, 120, 50, ST77XX_WHITE);
      display.fillCircle(65, 120, 3, ST77XX_WHITE);

      // Calculate hand angles
      float angleHour = (hour % 12 + minute / 60.0) * 30.0 * PI / 180.0;
      float angleMin = (minute + second / 60.0) * 6.0 * PI / 180.0;
      float angleSec = second * 6.0 * PI / 180.0;

      // Hour hand (length 22)
      int hx = 65 + (int)(22.0 * sin(angleHour));
      int hy = 120 - (int)(22.0 * cos(angleHour));
      display.drawLine(65, 120, hx, hy, ST77XX_WHITE);

      // Minute hand (length 35)
      int mx = 65 + (int)(35.0 * sin(angleMin));
      int my = 120 - (int)(35.0 * cos(angleMin));
      display.drawLine(65, 120, mx, my, ST77XX_WHITE);

      // Second hand (length 42)
      int sx = 65 + (int)(42.0 * sin(angleSec));
      int sy = 120 - (int)(42.0 * cos(angleSec));
      display.drawLine(65, 120, sx, sy, ST77XX_WHITE);

      // Right side: Digital Time & Date
      // Time
      display.setTextSize(3);
      char timeNoSec[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeNoSec, sizeof(timeNoSec), "%02d:%02d", dispHour, minute);
      display.setCursor(135, 75);
      display.print(timeNoSec);

      display.setTextSize(2);
      if (is12Hour) {
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        display.setCursor(135, 110);
        display.print(ampm);
      }

      // Weekday
      display.setCursor(135, 140);
      display.print(day);

      // Date
      display.setCursor(135, 165);
      display.print(date);

    } else if (style == 3) {
      // Style 3: Retro Grid (Grid borders and a progress bar)
      display.drawFastHLine(0, 10, 240, ST77XX_WHITE);
      display.drawFastHLine(0, 230, 240, ST77XX_WHITE);

      // Time
      display.setTextSize(4);
      char timeStr[12];
      if (is12Hour) {
        int dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        snprintf(timeStr, sizeof(timeStr), "%d:%02d %s", dispHour, minute, ampm);
      } else {
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hour, minute, second);
      }
      int timeWidth = strlen(timeStr) * 24;
      display.setCursor((240 - timeWidth) / 2, 70);
      display.print(timeStr);

      // Date
      display.setTextSize(2);
      String dayDateStr = day + ", " + date;
      int dateWidth = dayDateStr.length() * 12;
      display.setCursor((240 - dateWidth) / 2, 130);
      display.print(dayDateStr);

      // Progress bar representing seconds (0 to 59)
      int progressWidth = (second * 200) / 60;
      display.drawRect(20, 180, 200, 15, ST77XX_WHITE);
      display.fillRect(22, 182, progressWidth, 11, ST77XX_WHITE);

    } else {
      // Style 0: Classic Border
      display.drawRoundRect(0, 0, 240, 240, 10, ST77XX_WHITE);
      display.drawRoundRect(4, 4, 232, 232, 8, ST77XX_WHITE);

      // Time
      display.setTextSize(4);
      char timeStr[12];
      if (is12Hour) {
        int dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        snprintf(timeStr, sizeof(timeStr), "%d:%02d %s", dispHour, minute, ampm);
      } else {
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hour, minute, second);
      }
      int timeWidth = strlen(timeStr) * 24;
      display.setCursor((240 - timeWidth) / 2, 80);
      display.print(timeStr);

      // Date & Day
      display.setTextSize(2);
      String dayDateStr = day + ", " + date;
      int dateWidth = dayDateStr.length() * 12;
      display.setCursor((240 - dateWidth) / 2, 150);
      display.print(dayDateStr);
    }
  }

  void drawTextScreen() {
    display.setTextWrap(false);
    
    // 1. Draw header background and small animated face (Double size for 240x240 screen)
    display.fillRoundRect(10, 8, 28, 20, 4, ST77XX_WHITE);
    display.fillCircle(16, 16, 2, ST77XX_BLACK); // Left eye
    display.fillCircle(30, 16, 2, ST77XX_BLACK); // Right eye
    display.drawFastHLine(20, 22, 6, ST77XX_BLACK); // Smile

    // 2. Draw notification title (truncated to fit screen size)
    display.setTextSize(2);
    display.setTextColor(ST77XX_WHITE);
    display.setCursor(48, 10);
    
    String displayTitle = notificationTitle;
    if (displayTitle.length() == 0) {
      displayTitle = "Notification";
    }
    // Truncate title if too long (max ~13 chars at size 2)
    if (displayTitle.length() > 13) {
      displayTitle = displayTitle.substring(0, 10) + "...";
    }
    display.print(displayTitle);
    
    // Draw divider line separating header from body
    display.drawFastHLine(0, 36, SCREEN_WIDTH, ST77XX_WHITE);

    // 3. Draw Body Text
    // Size 3 text is 24px high, centered around y=130
    display.setTextSize(3);
    
    // If text is short, center it statically instead of scrolling!
    // Max characters that can fit statically on 240px width at size 3 (18px/char) is 13 chars (13 * 18 = 234px)
    if (notificationText.length() <= 13) {
      int textW = notificationText.length() * 18;
      int startX = (SCREEN_WIDTH - textW) / 2;
      display.setCursor(startX, 120);
      display.print(notificationText);
    } else {
      // Scroll long text
      display.setCursor(scrollPos, 120);
      display.print(notificationText);
    }
  }

  int getMixedSizeTextWidth(String text) {
    int w = 0;
    for (unsigned int i = 0; i < text.length(); i++) {
      char c = text.charAt(i);
      if (c == ' ') {
        w += 12; // Double width for size 2
      } else if ((c >= '0' && c <= '9') || c == '.' || c == ':') {
        w += 24; // Double width for size 4
      } else {
        w += 12; // Double width for size 2
      }
    }
    return w;
  }

  void drawMixedSizeText(String text, int startX, int y2, int y1) {
    int currentX = startX;
    for (unsigned int i = 0; i < text.length(); i++) {
      char c = text.charAt(i);
      if (c == ' ') {
        currentX += 12;
      } else if ((c >= '0' && c <= '9') || c == '.' || c == ':') {
        display.setTextSize(4);
        display.setCursor(currentX, y2);
        display.print(c);
        currentX += 24;
      } else {
        display.setTextSize(2);
        display.setCursor(currentX, y1);
        display.print(c);
        currentX += 12;
      }
    }
    display.setTextSize(2); // Restore default text size
  }

  void drawMapScreen(int hour, int minute, bool is12Hour) {
    display.setTextColor(ST77XX_WHITE);
    display.setTextWrap(false);

    // 1. Draw Giant Arrow in the upper/middle area (centered around x=120, y=80)
    if (mapDirection.indexOf("LEFT") >= 0) {
      // Bold LEFT Turn Arrow
      display.fillTriangle(60, 80, 95, 45, 95, 115, ST77XX_WHITE); // Arrowhead pointing left
      display.fillRect(95, 68, 55, 24, ST77XX_WHITE); // Horizontal shaft
      display.fillRect(122, 92, 28, 48, ST77XX_WHITE); // Vertical shaft
    } else if (mapDirection.indexOf("RIGHT") >= 0) {
      // Bold RIGHT Turn Arrow
      display.fillTriangle(180, 80, 145, 45, 145, 115, ST77XX_WHITE); // Arrowhead pointing right
      display.fillRect(90, 68, 55, 24, ST77XX_WHITE); // Horizontal shaft
      display.fillRect(90, 92, 28, 48, ST77XX_WHITE); // Vertical shaft
    } else if (mapDirection.indexOf("UTURN") >= 0 || mapDirection.indexOf("U-TURN") >= 0) {
      // Bold U-Turn
      display.drawCircle(120, 80, 40, ST77XX_WHITE);
      display.drawCircle(120, 80, 39, ST77XX_WHITE);
      display.drawCircle(120, 80, 38, ST77XX_WHITE);
      display.drawCircle(120, 80, 37, ST77XX_WHITE);
      display.drawCircle(120, 80, 36, ST77XX_WHITE);
      display.fillRect(70, 80, 100, 50, ST77XX_BLACK); // Clear bottom half
      display.fillRect(80, 80, 10, 40, ST77XX_WHITE); // Left leg down
      display.fillRect(150, 80, 10, 60, ST77XX_WHITE); // Right leg down
      display.fillTriangle(85, 130, 70, 110, 100, 110, ST77XX_WHITE); // Arrowhead pointing down on left leg
    } else if (mapDirection.indexOf("ROUNDABOUT") >= 0 || mapDirection.indexOf("ROUND") >= 0 || mapDirection.indexOf("ROTARY") >= 0) {
      // Bold Roundabout
      display.drawCircle(120, 80, 35, ST77XX_WHITE);
      display.drawCircle(120, 80, 34, ST77XX_WHITE);
      display.drawCircle(120, 80, 33, ST77XX_WHITE);
      display.drawCircle(120, 80, 32, ST77XX_WHITE);
      display.drawCircle(120, 80, 31, ST77XX_WHITE);
      display.fillRect(110, 70, 20, 20, ST77XX_BLACK); // Clear center
      display.fillRect(112, 105, 16, 30, ST77XX_BLACK); // Clear bottom entrance
      display.fillRect(142, 72, 20, 16, ST77XX_WHITE); // Shaft connecting to circle
      display.fillTriangle(190, 80, 162, 60, 162, 100, ST77XX_WHITE); // Exit arrowhead pointing right
    } else { // STRAIGHT / default
      // Bold Straight Arrow
      display.fillRect(104, 75, 32, 65, ST77XX_WHITE); // Thick vertical shaft
      display.fillTriangle(120, 25, 80, 75, 160, 75, ST77XX_WHITE); // Giant filled arrowhead
    }

    // 2. Draw Bottom Status Info (Left: Distance, Right: Remaining Time)
    
    // Left side: Distance
    if (mapDistance != "" && mapDistance != "--") {
      drawMixedSizeText(mapDistance, 10, 165, 180);
    }

    // Right side: Remaining Time (stored in mapDescription)
    if (mapDescription != "") {
      int timeWidth = getMixedSizeTextWidth(mapDescription);
      int startX = SCREEN_WIDTH - timeWidth - 10;
      if (startX < 120) startX = 120; // Keep on right half
      drawMixedSizeText(mapDescription, startX, 165, 180);
    }
  }
};

#endif // EXPRESSIONS_H
