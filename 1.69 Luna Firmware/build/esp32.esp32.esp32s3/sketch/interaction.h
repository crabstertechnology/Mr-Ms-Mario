#line 1 "W:\\Mr.mario\\1.69 Luna Firmware\\interaction.h"
// =============================================================================
// interaction.h  —  Luna Firmware CST816T Touch Input (Left / Center / Right)
// =============================================================================
#ifndef INTERACTION_H
#define INTERACTION_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// ── Button events emitted by update() ────────────────────────────────────────
enum ButtonEvent {
  BTN_NONE    = 0,
  BTN1_SINGLE,      // Center tap  → select / interact
  BTN1_DOUBLE,      // (reserved)
  BTN1_LONG,        // Long press  → return to home (Face) screen
  BTN2_SINGLE,      // Right tap   → next screen
  BTN2_DOUBLE,      // Left tap    → previous screen
  BTN2_LONG,        // (reserved)
  BTN_SWIPE_UP,     // Swipe up    → scroll down
  BTN_SWIPE_DOWN,   // Swipe down  → scroll up
  BTN_SWIPE_LEFT,   // Swipe left  → next screen
  BTN_SWIPE_RIGHT,  // Swipe right → previous screen
};

// ── Keep backwards-compatible TouchEvent ─────────────────────────────────────
enum TouchEvent {
  TOUCH_NONE        = 0,
  TOUCH_TAP,
  TOUCH_DOUBLE_TAP,
  TOUCH_TRIPLE_TAP,
  TOUCH_LONG_PRESS
};

// ── Globals shared with games.h ───────────────────────────────────────────────
extern bool virtualBtn1;
extern bool virtualBtn2;
extern bool gamePlaying;
extern SmartwatchScreen currentScreen;

// ── Hardware interrupt flag ───────────────────────────────────────────────────
static volatile bool touchInterruptOccurred = false;
static void IRAM_ATTR touchISR() {
  touchInterruptOccurred = true;
}

// =============================================================================
class LunaInteraction {
private:
  // Raw last-known coordinates
  int   lastX, lastY;

  // Touch-down tracking
  bool          isDown;
  int           startX, startY;
  unsigned long startMs;
  unsigned long lastTouchMs;

public:
  int getLastX() const { return lastX; }
  int getLastY() const { return lastY; }

  // ---------------------------------------------------------------------------
  void begin() {
    lastX = 0; lastY = 0;
    isDown = false;
    startX = 0; startY = 0;
    startMs = 0;
    lastTouchMs = 0;

    // Hard-reset the CST816T (active-low reset pin)
    pinMode(TOUCH_RST, OUTPUT);
    digitalWrite(TOUCH_RST, LOW);
    delay(10);
    digitalWrite(TOUCH_RST, HIGH);
    delay(100);

    // I2C bus initialisation
    Wire.begin(TOUCH_SDA, TOUCH_SCL, 400000);

    // Interrupt pin — falling edge fires when screen is touched
    pinMode(TOUCH_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TOUCH_INT), touchISR, FALLING);
    touchInterruptOccurred = false;

    Serial.println("[Touch] CST816T ready — Left / Center / Right zones active.");
  }

  // ---------------------------------------------------------------------------
  // Read one packet from the CST816T over I2C.
  // Returns true when fresh data is available.
  // ---------------------------------------------------------------------------
  bool readTouch(uint8_t &gesture, uint8_t &fingerNum, int &x, int &y) {
    if (!touchInterruptOccurred) return false;
    touchInterruptOccurred = false;

    Wire.beginTransmission(0x15);   // CST816T I2C address
    Wire.write(0x01);               // GestureID register
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(0x15, 6) < 6) return false;

    gesture   = Wire.read();
    fingerNum = Wire.read();

    uint8_t xH = Wire.read();
    uint8_t xL = Wire.read();
    uint8_t yH = Wire.read();
    uint8_t yL = Wire.read();

    x = ((xH & 0x0F) << 8) | xL;
    y = ((yH & 0x0F) << 8) | yL;
    return true;
  }

  // ---------------------------------------------------------------------------
  // Call every loop() iteration.
  // Returns a ButtonEvent describing what the user just did.
  //
  //   Zone map (landscape 240-wide panel, portrait-reported coords):
  //   X <  TOUCH_LEFT_LIMIT   (40 px)  → Left  tap → BTN2_DOUBLE (prev screen)
  //   X >= TOUCH_RIGHT_LIMIT  (200 px) → Right tap → BTN2_SINGLE (next screen)
  //   Otherwise               (40–200) → Center tap→ BTN1_SINGLE (select)
  //   Hold ≥ 500 ms anywhere           → BTN1_LONG  (home screen)
  //
  // While a game is playing the same left/right split drives virtualBtn1/2.
  // ---------------------------------------------------------------------------
  ButtonEvent update() {
    uint8_t gesture   = 0;
    uint8_t fingerNum = 0;
    int  x = 0, y = 0;

    bool fresh = readTouch(gesture, fingerNum, x, y);
    unsigned long now = millis();
    ButtonEvent ev = BTN_NONE;

    // ── Finger DOWN / HELD ───────────────────────────────────────────────────
    if (fresh && fingerNum > 0) {
      Serial.printf("[TOUCH] G:%02X F:%d X:%d Y:%d\n", gesture, fingerNum, x, y);
      lastTouchMs = now;
      lastX = x;
      lastY = y;

      if (!isDown) {
        isDown  = true;
        startX  = x;
        startY  = y;
        startMs = now;
      }

      // Live game controls — split screen left / right
      if (gamePlaying && currentScreen == SCREEN_GAMES) {
        virtualBtn1 = (x < 120);
        virtualBtn2 = (x >= 120);
        return BTN_NONE;
      }

    // ── Finger UP / Timed-out ────────────────────────────────────────────────
    } else {
      bool explicitRelease = (fresh && fingerNum == 0);
      bool timedOut        = (isDown && (now - lastTouchMs > 60));

      if (isDown && (explicitRelease || timedOut)) {
        isDown      = false;
        virtualBtn1 = false;
        virtualBtn2 = false;

        if (gamePlaying && currentScreen == SCREEN_GAMES) {
          return BTN_NONE;
        }

        unsigned long held = now - startMs;

        // First, check gesture code from CST816T if available
        if (gesture == 0x01) {
          ev = BTN_SWIPE_UP;
          Serial.println("[Touch] Gesture: Swipe Up");
        } else if (gesture == 0x02) {
          ev = BTN_SWIPE_DOWN;
          Serial.println("[Touch] Gesture: Swipe Down");
        } else if (gesture == 0x03) {
          ev = BTN_SWIPE_LEFT;
          Serial.println("[Touch] Gesture: Swipe Left");
        } else if (gesture == 0x04) {
          ev = BTN_SWIPE_RIGHT;
          Serial.println("[Touch] Gesture: Swipe Right");
        } else {
          // If no hardware gesture detected, fallback to software delta tracking
          int deltaX = lastX - startX;
          int deltaY = lastY - startY;
          int absX = abs(deltaX);
          int absY = abs(deltaY);

          if (absY > 30 && absY > absX) {
            if (deltaY < 0) {
              ev = BTN_SWIPE_UP;
              Serial.printf("[Touch] Software Swipe Up (dY=%d)\n", deltaY);
            } else {
              ev = BTN_SWIPE_DOWN;
              Serial.printf("[Touch] Software Swipe Down (dY=%d)\n", deltaY);
            }
          } else if (absX > 30 && absX > absY) {
            if (deltaX < 0) {
              ev = BTN_SWIPE_LEFT;
              Serial.printf("[Touch] Software Swipe Left (dX=%d)\n", deltaX);
            } else {
              ev = BTN_SWIPE_RIGHT;
              Serial.printf("[Touch] Software Swipe Right (dX=%d)\n", deltaX);
            }
          } else {
            // Standard click/long press handling
            if (held >= 500) {
              ev = BTN1_LONG;
              Serial.println("[Touch] Long press → home screen");
            } else if (held >= 30) {
              if (currentScreen == SCREEN_FACE) {
                if (lastX < 40) {
                  ev = BTN2_DOUBLE;
                  Serial.printf("[Touch] LEFT tap (X=%d) → prev screen\n", lastX);
                } else if (lastX >= 200) {
                  ev = BTN2_SINGLE;
                  Serial.printf("[Touch] RIGHT tap (X=%d) → next screen\n", lastX);
                } else {
                  ev = BTN1_SINGLE;
                  Serial.printf("[Touch] CENTER tap (X=%d) → select\n", lastX);
                }
              } else {
                if (lastX < 15) {
                  ev = BTN2_DOUBLE;
                  Serial.printf("[Touch] Edge LEFT tap (X=%d) → prev screen\n", lastX);
                } else if (lastX >= 225) {
                  ev = BTN2_SINGLE;
                  Serial.printf("[Touch] Edge RIGHT tap (X=%d) → next screen\n", lastX);
                } else {
                  ev = BTN1_SINGLE;
                  Serial.printf("[Touch] Content tap (X=%d, Y=%d) → select\n", lastX, lastY);
                }
              }
            }
          }
        }
      }
    }

    return ev;
  }
};

#endif // INTERACTION_H
