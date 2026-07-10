#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#define SSD1306_WHITE ST77XX_CYAN
#define SSD1306_BLACK ST77XX_BLACK
#include "config.h"
#include "mochi_bitmaps.h"

class LunaFace {
private:
  Adafruit_ST7789& display;
  GFXcanvas16 canvas;
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

  uint16_t getExpressionColor(Expression expr) {
    Expression actual = expr;
    if (actual == EXPR_IDLE) actual = defaultExpr;
    switch (actual) {
      case EXPR_HAPPY:      return ST77XX_GREEN;
      case EXPR_SAD:        return ST77XX_BLUE;
      case EXPR_ANGRY:      return ST77XX_RED;
      case EXPR_SURPRISED:   return ST77XX_YELLOW;
      case EXPR_SLEEPING:   return ST77XX_MAGENTA;
      case EXPR_WINK:       return ST77XX_CYAN;
      default:              return ST77XX_CYAN;
    }
  }

  void drawBitmap2x(const unsigned char *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) {
    int16_t byteWidth = (w + 7) / 8;
    for (int16_t j = 0; j < h; j++) {
      for (int16_t rowRepeat = 0; rowRepeat < 2; rowRepeat++) {
        int16_t canvasY = 56 + j * 2 + rowRepeat;
        for (int16_t i = 4; i < 124; i++) {
          uint8_t byteVal = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
          bool bitSet = byteVal & (128 >> (i & 7));
          uint16_t pixelColor = bitSet ? color : bg;
          int16_t canvasX = (i - 4) * 2;
          canvas.drawPixel(canvasX, canvasY, pixelColor);
          canvas.drawPixel(canvasX + 1, canvasY, pixelColor);
        }
      }
    }
  }

public:
  LunaFace(Adafruit_ST7789& disp) 
    : display(disp), canvas(240, 240), currentExpr(EXPR_IDLE), targetExpr(EXPR_IDLE), defaultExpr(EXPR_IDLE), stateLabel("IDLE"), frameDelayMs(100), expressionChanged(true) {
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
      if (notificationText.length() > 10) { // Only scroll if it doesn't fit statically
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
    }
    return changed;
  }

  void drawSettingsMenu(int option, bool selected, bool bleOn, int speed, int clockStyle, bool invertOn, int brightness) {
    canvas.fillScreen(ST77XX_BLACK);
    canvas.setTextColor(ST77XX_WHITE);
    
    // Draw Title
    canvas.setTextSize(2);
    canvas.setCursor(40, 20);
    canvas.print("--- SETTINGS ---");
    canvas.drawFastHLine(0, 45, SCREEN_WIDTH, ST77XX_WHITE);
    
    // Scrolling menu window calculation (displays 4 items)
    int startOpt = 0;
    if (option >= 3) {
      startOpt = option - 3;
    }
    if (startOpt > 3) startOpt = 3;
    
    for (int i = 0; i < 4; i++) {
      int optIdx = startOpt + i;
      if (optIdx >= 7) break;
      
      int yPos = 65 + i * 35;
      canvas.setCursor(15, yPos);
      canvas.setTextSize(2);
      
      bool isCurrent = (option == optIdx);
      if (isCurrent) {
        canvas.print(selected ? ">> " : "> ");
      } else {
        canvas.print("   ");
      }
      
      switch (optIdx) {
        case 0:
          canvas.print("BLE: ");
          canvas.print(bleOn ? "ON" : "OFF");
          break;
        case 1:
          canvas.print("SPEED: ");
          canvas.print(speed);
          canvas.print("ms");
          break;
        case 2:
          canvas.print("CLOCK: STYLE ");
          canvas.print(clockStyle);
          break;
        case 3:
          canvas.print("INVERT: ");
          canvas.print(invertOn ? "ON" : "OFF");
          break;
        case 4:
          canvas.print("BRIGHT: ");
          if (brightness == 1) canvas.print("LOW");
          else if (brightness == 2) canvas.print("MED");
          else canvas.print("HIGH");
          break;
        case 5:
          canvas.print("[ SAVE ]");
          break;
        case 6:
          canvas.print("[ EXIT ]");
          break;
      }
    }
  
    display.drawRGBBitmap(0, 0, canvas.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);}

    void draw(int hour, int minute, int second, String day, String date, int style = 0, bool is12Hour = false) {
    canvas.fillScreen(ST77XX_BLACK);

    // 1. Draw Text Screen if active
    if (currentExpr == EXPR_TEXT) {
      drawTextScreen(true); // Draw static header
      drawTextScreen(false); // Draw scrolling text
    } else if (currentExpr == EXPR_CLOCK) {
      drawClockScreen(hour, minute, second, day, date, style, is12Hour);
    } else if (currentExpr == EXPR_MAP) {
      drawMapScreen(hour, minute, is12Hour);
    } else if (currentExpr == EXPR_ALL_GIF) {
      if (currentGifIndex < ALL_GIFS_COUNT) {
        const unsigned char* const* frames =
          (const unsigned char* const*)pgm_read_ptr(&ALL_GIFS_TABLE[currentGifIndex].frames);
        int frameCount = (int)pgm_read_dword(&ALL_GIFS_TABLE[currentGifIndex].count);
        int safeFrame = (currentFrame < frameCount) ? currentFrame : 0;
        const unsigned char* frameData =
          (const unsigned char*)pgm_read_ptr(&frames[safeFrame]);
        if (frameData) {
          drawBitmap2x(frameData, 128, 64, getExpressionColor(currentExpr), ST77XX_BLACK);
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
        drawBitmap2x(frameData, 128, 64, getExpressionColor(exprToDraw), ST77XX_BLACK);
      }
    }

    display.drawRGBBitmap(0, 0, canvas.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
  }

private:
  void drawClockScreen(int hour, int minute, int second, String day, String date, int style, bool is12Hour) {
    if (style == 1) {
      // Clear previous text regions to avoid overlapping
      canvas.fillRect(40, 60, 155, 40, ST77XX_BLACK);
      canvas.fillRect(200, 60, 35, 45, ST77XX_BLACK);
      canvas.fillRect(40, 140, 195, 20, ST77XX_BLACK);

      canvas.setTextSize(5);
      char timeNoSec[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeNoSec, sizeof(timeNoSec), "%02d:%02d", dispHour, minute);
      canvas.setCursor(45, 60);
      canvas.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
      canvas.print(timeNoSec);

      canvas.setTextSize(2);
      canvas.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
      if (is12Hour) {
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        canvas.setCursor(200, 60);
        canvas.print(ampm);
      } else {
        char secStr[3];
        snprintf(secStr, sizeof(secStr), "%02d", second);
        canvas.setCursor(200, 85);
        canvas.print(secStr);
      }

      canvas.setCursor(45, 140);
      canvas.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
      String dayDateStr = day + ", " + date;
      canvas.print(dayDateStr);

    } else if (style == 2) {
      canvas.fillCircle(70, 120, 56, ST77XX_BLACK);
      canvas.drawCircle(70, 120, 55, ST77XX_YELLOW);
      canvas.fillCircle(70, 120, 3, ST77XX_YELLOW);

      float angleHour = (hour % 12 + minute / 60.0) * 30.0 * PI / 180.0;
      float angleMin = (minute + second / 60.0) * 6.0 * PI / 180.0;
      float angleSec = second * 6.0 * PI / 180.0;

      int hx = 70 + (int)(30.0 * sin(angleHour));
      int hy = 120 - (int)(30.0 * cos(angleHour));
      canvas.drawLine(70, 120, hx, hy, ST77XX_CYAN);

      int mx = 70 + (int)(45.0 * sin(angleMin));
      int my = 120 - (int)(45.0 * cos(angleMin));
      canvas.drawLine(70, 120, mx, my, ST77XX_GREEN);

      int sx = 70 + (int)(50.0 * sin(angleSec));
      int sy = 120 - (int)(50.0 * cos(angleSec));
      canvas.drawLine(70, 120, sx, sy, ST77XX_RED);

      // Clear text areas on the right side
      canvas.fillRect(135, 80, 105, 85, ST77XX_BLACK);

      canvas.setTextSize(2);
      char timeNoSec[6];
      int dispHour = hour;
      if (is12Hour) {
        dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
      }
      snprintf(timeNoSec, sizeof(timeNoSec), "%02d:%02d", dispHour, minute);
      canvas.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
      canvas.setCursor(140, 85);
      canvas.print(timeNoSec);

      canvas.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
      if (is12Hour) {
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        canvas.setCursor(140, 110);
        canvas.print(ampm);
      }

      canvas.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
      canvas.setTextSize(1);
      canvas.setCursor(140, 135);
      canvas.print(day);

      canvas.setCursor(140, 150);
      canvas.print(date);

    } else if (style == 3) {
      // Clear time and date regions
      canvas.fillRect(0, 55, 240, 35, ST77XX_BLACK);
      canvas.fillRect(0, 110, 240, 25, ST77XX_BLACK);

      canvas.drawFastHLine(0, 10, 240, ST77XX_MAGENTA);
      canvas.drawFastHLine(0, 230, 240, ST77XX_MAGENTA);

      canvas.setTextSize(3);
      char timeStr[12];
      if (is12Hour) {
        int dispHour = hour % 12;
        if (dispHour == 0) dispHour = 12;
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        snprintf(timeStr, sizeof(timeStr), "%d:%02d %s", dispHour, minute, ampm);
      } else {
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hour, minute, second);
      }
      int timeWidth = strlen(timeStr) * 18;
      canvas.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
      canvas.setCursor((240 - timeWidth) / 2, 60);
      canvas.print(timeStr);

      canvas.setTextSize(2);
      canvas.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
      String dayDateStr = day + ", " + date;
      int dateWidth = dayDateStr.length() * 12;
      canvas.setCursor((240 - dateWidth) / 2, 115);
      canvas.print(dayDateStr);

      int progressWidth = (second * 200) / 60;
      canvas.fillRect(22, 172, 196, 8, ST77XX_BLACK);
      canvas.drawRect(20, 170, 200, 12, ST77XX_YELLOW);
      canvas.fillRect(22, 172, progressWidth, 8, ST77XX_GREEN);

    } else {
      // Clear time and date regions
      canvas.fillRect(12, 65, 216, 45, ST77XX_BLACK);
      canvas.fillRect(12, 135, 216, 25, ST77XX_BLACK);

      canvas.drawRoundRect(4, 4, 232, 232, 12, ST77XX_MAGENTA);
      canvas.drawRoundRect(8, 8, 224, 224, 8, ST77XX_MAGENTA);

      canvas.setTextSize(4);
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
      canvas.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
      canvas.setCursor((240 - timeWidth) / 2, 70);
      canvas.print(timeStr);

      canvas.setTextSize(2);
      canvas.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
      String dayDateStr = day + ", " + date;
      int dateWidth = dayDateStr.length() * 12;
      canvas.setCursor((240 - dateWidth) / 2, 140);
      canvas.print(dayDateStr);
    }
  }

  void drawTextScreen(bool drawHeaderOnly = false) {
    canvas.setTextWrap(false);
    
    if (drawHeaderOnly) {
      canvas.fillRoundRect(10, 8, 30, 20, 4, ST77XX_YELLOW);
      canvas.fillCircle(17, 16, 2, ST77XX_BLACK);
      canvas.fillCircle(31, 16, 2, ST77XX_BLACK);
      canvas.drawFastHLine(20, 22, 6, ST77XX_BLACK);

      canvas.setTextSize(2);
      canvas.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
      canvas.setCursor(50, 10);
      
      String displayTitle = notificationTitle;
      if (displayTitle.length() == 0) {
        displayTitle = "Notification";
      }
      if (displayTitle.length() > 14) {
        displayTitle = displayTitle.substring(0, 11) + "...";
      }
      canvas.print(displayTitle);
      
      canvas.drawFastHLine(0, 36, SCREEN_WIDTH, ST77XX_YELLOW);
      return;
    }

    // Clear dynamic text area to prevent scroll trace artifacts
    canvas.fillRect(0, 80, 240, 60, ST77XX_BLACK);

    canvas.setTextSize(3);
    canvas.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
    
    if (notificationText.length() <= 10) {
      int textW = notificationText.length() * 18;
      int startX = (SCREEN_WIDTH - textW) / 2;
      canvas.setCursor(startX, 110);
      canvas.print(notificationText);
    } else {
      canvas.setCursor(scrollPos, 110);
      canvas.print(notificationText);
    }
  }

  int getMixedSizeTextWidth(String text) {
    int w = 0;
    for (unsigned int i = 0; i < text.length(); i++) {
      char c = text.charAt(i);
      if (c == ' ') {
        w += 12;
      } else if ((c >= '0' && c <= '9') || c == '.' || c == ':') {
        w += 24;
      } else {
        w += 12;
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
        canvas.setTextSize(3);
        canvas.setCursor(currentX, y2);
        canvas.print(c);
        currentX += 24;
      } else {
        canvas.setTextSize(2);
        canvas.setCursor(currentX, y1);
        canvas.print(c);
        currentX += 12;
      }
    }
    canvas.setTextSize(1);
  }

  void drawMapScreen(int hour, int minute, bool is12Hour) {
    canvas.fillRect(0, 40, 240, 200, ST77XX_BLACK);
    canvas.setTextWrap(false);

    if (mapDirection.indexOf("LEFT") >= 0) {
      canvas.fillTriangle(60, 90, 90, 60, 90, 120, ST77XX_GREEN);
      canvas.fillRect(90, 80, 50, 20, ST77XX_GREEN);
      canvas.fillRect(120, 100, 20, 40, ST77XX_GREEN);
    } else if (mapDirection.indexOf("RIGHT") >= 0) {
      canvas.fillTriangle(180, 90, 150, 60, 150, 120, ST77XX_GREEN);
      canvas.fillRect(100, 80, 50, 20, ST77XX_GREEN);
      canvas.fillRect(100, 100, 20, 40, ST77XX_GREEN);
    } else if (mapDirection.indexOf("UTURN") >= 0 || mapDirection.indexOf("U-TURN") >= 0) {
      canvas.drawCircle(120, 90, 32, ST77XX_GREEN);
      canvas.drawCircle(120, 90, 30, ST77XX_GREEN);
      canvas.drawCircle(120, 90, 28, ST77XX_GREEN);
      canvas.fillRect(80, 90, 80, 50, ST77XX_BLACK);
      canvas.fillRect(88, 90, 10, 30, ST77XX_GREEN);
      canvas.fillRect(142, 90, 10, 30, ST77XX_GREEN);
      canvas.fillTriangle(93, 130, 78, 115, 108, 115, ST77XX_GREEN);
    } else if (mapDirection.indexOf("ROUNDABOUT") >= 0 || mapDirection.indexOf("ROUND") >= 0 || mapDirection.indexOf("ROTARY") >= 0) {
      canvas.drawCircle(120, 90, 28, ST77XX_GREEN);
      canvas.drawCircle(120, 90, 26, ST77XX_GREEN);
      canvas.fillRect(110, 80, 20, 20, ST77XX_BLACK);
      canvas.fillRect(112, 110, 16, 28, ST77XX_BLACK);
      canvas.fillRect(140, 82, 16, 16, ST77XX_GREEN);
      canvas.fillTriangle(170, 90, 150, 75, 150, 105, ST77XX_GREEN);
    } else {
      canvas.fillRect(110, 80, 20, 50, ST77XX_GREEN);
      canvas.fillTriangle(120, 40, 90, 80, 150, 80, ST77XX_GREEN);
    }

    canvas.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    if (mapDistance != "" && mapDistance != "--") {
      drawMixedSizeText(mapDistance, 10, 180, 190);
    }

    canvas.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
    if (mapDescription != "") {
      int timeWidth = getMixedSizeTextWidth(mapDescription);
      int startX = SCREEN_WIDTH - timeWidth - 10;
      if (startX < 120) startX = 120;
      drawMixedSizeText(mapDescription, startX, 180, 190);
    }
  }
};

#endif // EXPRESSIONS_H