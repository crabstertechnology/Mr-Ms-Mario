#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "mochi_bitmaps.h"

class LunaFace {
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
  LunaFace(Adafruit_SSD1306& disp) 
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
      if (now - lastScrollTime > 30) {
        lastScrollTime = now;
        scrollPos -= 2;
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
    display.setTextWrap(false);
    
    // 1. Draw header background and small animated face
    display.fillRoundRect(4, 2, 14, 10, 2, SSD1306_WHITE);
    display.fillCircle(7, 6, 1, SSD1306_BLACK); // Left eye
    display.fillCircle(14, 6, 1, SSD1306_BLACK); // Right eye
    display.drawFastHLine(9, 9, 3, SSD1306_BLACK); // Smile

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
    display.setTextSize(1); // Restore default text size
  }

  void drawMapScreen(int hour, int minute, bool is12Hour) {
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);

    // 1. Draw Giant Arrow in the upper/middle area (centered around x=64, y=25)
    if (mapDirection.indexOf("LEFT") >= 0) {
      // Bold LEFT Turn Arrow (up and left)
      display.fillTriangle(32, 22, 48, 6, 48, 38, SSD1306_WHITE); // Arrowhead pointing left
      display.fillRect(48, 16, 24, 12, SSD1306_WHITE); // Horizontal shaft (x: 48 to 72, y: 16 to 28)
      display.fillRect(60, 28, 12, 20, SSD1306_WHITE); // Vertical shaft (x: 60 to 72, y: 28 to 48)
    } else if (mapDirection.indexOf("RIGHT") >= 0) {
      // Bold RIGHT Turn Arrow (up and right)
      display.fillTriangle(96, 22, 80, 6, 80, 38, SSD1306_WHITE); // Arrowhead pointing right
      display.fillRect(56, 16, 24, 12, SSD1306_WHITE); // Horizontal shaft (x: 56 to 80, y: 16 to 28)
      display.fillRect(56, 28, 12, 20, SSD1306_WHITE); // Vertical shaft (x: 56 to 68, y: 28 to 48)
    } else if (mapDirection.indexOf("UTURN") >= 0 || mapDirection.indexOf("U-TURN") >= 0) {
      // Bold U-Turn
      display.drawCircle(64, 26, 16, SSD1306_WHITE);
      display.drawCircle(64, 26, 15, SSD1306_WHITE);
      display.drawCircle(64, 26, 14, SSD1306_WHITE);
      display.drawCircle(64, 26, 13, SSD1306_WHITE);
      display.drawCircle(64, 26, 12, SSD1306_WHITE);
      display.fillRect(44, 26, 40, 24, SSD1306_BLACK); // clear bottom half of circles
      display.fillRect(48, 26, 5, 12, SSD1306_WHITE); // left leg down
      display.fillRect(75, 26, 5, 22, SSD1306_WHITE); // right leg down
      display.fillTriangle(50, 48, 42, 38, 58, 38, SSD1306_WHITE); // arrowhead pointing down on left leg
    } else if (mapDirection.indexOf("ROUNDABOUT") >= 0 || mapDirection.indexOf("ROUND") >= 0 || mapDirection.indexOf("ROTARY") >= 0) {
      // Bold Roundabout
      display.drawCircle(64, 24, 14, SSD1306_WHITE);
      display.drawCircle(64, 24, 13, SSD1306_WHITE);
      display.drawCircle(64, 24, 12, SSD1306_WHITE);
      display.drawCircle(64, 24, 11, SSD1306_WHITE);
      display.drawCircle(64, 24, 10, SSD1306_WHITE);
      display.drawCircle(64, 24, 9, SSD1306_WHITE);
      display.fillRect(59, 19, 10, 10, SSD1306_BLACK); // clear center
      display.fillRect(60, 34, 8, 14, SSD1306_BLACK); // clear bottom entrance
      display.fillRect(74, 20, 8, 8, SSD1306_WHITE); // shaft connecting to the circle
      display.fillTriangle(94, 24, 80, 14, 80, 34, SSD1306_WHITE); // exit arrowhead pointing right
    } else { // STRAIGHT / default
      // Bold Straight Arrow
      display.fillRect(56, 22, 16, 26, SSD1306_WHITE); // thick vertical shaft
      display.fillTriangle(64, 2, 44, 22, 84, 22, SSD1306_WHITE); // giant filled arrowhead
    }

    // 2. Draw Bottom Status Info (Left: Distance, Right: Remaining Time)
    
    // Left side: Distance
    if (mapDistance != "" && mapDistance != "--") {
      drawMixedSizeText(mapDistance, 2, 48, 55);
    }

    // Right side: Remaining Time (stored in mapDescription)
    if (mapDescription != "") {
      int timeWidth = getMixedSizeTextWidth(mapDescription);
      int startX = SCREEN_WIDTH - timeWidth - 2;
      if (startX < 64) startX = 64; // Keep on right half
      drawMixedSizeText(mapDescription, startX, 48, 55);
    }
  }
};

#endif // EXPRESSIONS_H
