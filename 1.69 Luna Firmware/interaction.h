// =============================================================================
// interaction.h  —  Luna Firmware CST816T Touch Input (Left / Center / Right)
// =============================================================================
#ifndef INTERACTION_H
#define INTERACTION_H

#include "config.h"
#include <Arduino.h>
#include <Wire.h>


// ── Button events emitted by update() ────────────────────────────────────────
enum ButtonEvent {
  BTN_NONE = 0,
  BTN1_SINGLE,     // Center tap  → select / interact
  BTN1_DOUBLE,     // (reserved)
  BTN1_LONG,       // Long press  → return to home (Face) screen
  BTN2_SINGLE,     // Right tap   → next screen
  BTN2_DOUBLE,     // Left tap    → previous screen
  BTN2_LONG,       // (reserved)
  BTN_SWIPE_UP,    // Swipe up    → scroll down
  BTN_SWIPE_DOWN,  // Swipe down  → scroll up
  BTN_SWIPE_LEFT,  // Swipe left  → next screen
  BTN_SWIPE_RIGHT, // Swipe right → previous screen
  BTN_CIRCLE_LEFT, // Hungry menu circle 0 (Left - Salad)
  BTN_CIRCLE_MID,  // Hungry menu circle 1 (Middle - Milk)
  BTN_CIRCLE_RIGHT // Hungry menu circle 2 (Right - Fish)
};

// ── Keep backwards-compatible TouchEvent ─────────────────────────────────────
enum TouchEvent {
  TOUCH_NONE = 0,
  TOUCH_TAP,
  TOUCH_DOUBLE_TAP,
  TOUCH_TRIPLE_TAP,
  TOUCH_LONG_PRESS
};

// ── Globals shared with games.h
// ───────────────────────────────────────────────
extern bool virtualBtn1;
extern bool virtualBtn2;
extern bool gamePlaying;
extern bool gameOverActive;
extern SmartwatchScreen currentScreen;
class LunaFace;
extern LunaFace face;

// ── Hardware interrupt flag
// ───────────────────────────────────────────────────
static volatile bool touchInterruptOccurred = false;
static void IRAM_ATTR touchISR() { touchInterruptOccurred = true; }

// ── Gesture State Machine states ─────────────────────────────────────────────
enum GestureState {
  STATE_IDLE = 0,
  STATE_TOUCH_DOWN,
  STATE_TRACKING,
  STATE_SCROLL_VERTICAL,
  STATE_SWIPE_HORIZONTAL,
  STATE_CANCELLED
};

// =============================================================================
class LunaInteraction {
private:
  // Raw last-known coordinates
  int lastX, lastY;

  // Touch-down tracking
  bool isDown;
  int startX, startY;
  unsigned long startMs;
  unsigned long lastTouchMs;

  // Gesture State Machine & Debounce
  GestureState gestureState;
  bool _hasScrolled;
  unsigned long lastSwipeTransitionMs;

public:
  int getLastX() const { return lastX; }
  int getLastY() const { return lastY; }
  int getMappedY() const { return constrain(lastY - 20, 0, 279); }
  GestureState getGestureState() const { return gestureState; }
  bool hasScrolled() const { return _hasScrolled; }
  bool isTouchDown() const { return isDown; }

  // ---------------------------------------------------------------------------
  void begin() {
    lastX = 0;
    lastY = 0;
    isDown = false;
    startX = 0;
    startY = 0;
    startMs = 0;
    lastTouchMs = 0;
    gestureState = STATE_IDLE;
    _hasScrolled = false;
    lastSwipeTransitionMs = 0;

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

    Serial.println("[Touch] CST816T hardened interaction engine initialized "
                   "(TouchSlop=10px, ScrollLatch=ON).");
  }

  // ---------------------------------------------------------------------------
  // Read one packet from the CST816T over I2C.
  // Returns true when fresh data is available.
  // ---------------------------------------------------------------------------
  bool readTouch(uint8_t &gesture, uint8_t &fingerNum, int &x, int &y,
                 bool allowPoll = false) {
    if (!touchInterruptOccurred && !allowPoll)
      return false;
    touchInterruptOccurred = false;

    Wire.beginTransmission(0x15); // CST816T I2C address
    Wire.write(0x01);             // GestureID register
    if (Wire.endTransmission(false) != 0)
      return false;
    if (Wire.requestFrom(0x15, 6) < 6)
      return false;

    gesture = Wire.read();
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
  // ---------------------------------------------------------------------------
  ButtonEvent update() {
    uint8_t gesture = 0;
    uint8_t fingerNum = 0;
    int x = 0, y = 0;

    unsigned long now = millis();
    // Allow I2C polling if finger is currently down and >25ms elapsed since
    // last packet
    bool allowPoll = (isDown && (now - lastTouchMs >= 25));
    bool fresh = readTouch(gesture, fingerNum, x, y, allowPoll);
    ButtonEvent ev = BTN_NONE;

    // ── On Character Animation Screen: only the 3 action circles are
    // interactive ──
    if (currentScreen == SCREEN_FACE) {
      if (fresh) {
        if (fingerNum > 0) {
          lastTouchMs = now;
          lastX = x;
          lastY = y;
          if (!isDown) {
            isDown = true;
            startX = x;
            startY = y;
            startMs = now;
            _hasScrolled = false;
          } else {
            if (abs(x - startX) >= 10 || abs(y - startY) >= 10) {
              _hasScrolled = true;
            }
          }
        } else {
          // Finger released
          if (isDown) {
            isDown = false;
            unsigned long held = now - startMs;
            int dx = lastX - startX;
            int dy = lastY - startY;
            // Clean stationary tap ONLY while displaying Hungry Menu (Anim 2)
            // While playing eating animations (Milk, Fish, Salad), touch is
            // completely ignored
            if (face.getRobotEyeAnim().getAnimationIndex() == 2 &&
                !_hasScrolled && abs(dx) < 16 && abs(dy) < 16 && held >= 20 &&
                held < 600) {
              int touchY = getMappedY(); // 0..279 canvas coordinate
              // Circles are centered at Y = 242, radius = 24 -> target Y range
              // 195..275
              if (touchY >= 195 && touchY <= 275) {
                if (lastX >= 5 && lastX <= 72) {
                  ev = BTN_CIRCLE_LEFT;
                  Serial.printf("[Touch] Hungry Menu: Circle LEFT (Salad) "
                                "tapped (X=%d, Y=%d)\n",
                                lastX, touchY);
                } else if (lastX >= 78 && lastX <= 146) {
                  ev = BTN_CIRCLE_MID;
                  Serial.printf("[Touch] Hungry Menu: Circle MID (Milk) tapped "
                                "(X=%d, Y=%d)\n",
                                lastX, touchY);
                } else if (lastX >= 152 && lastX <= 222) {
                  ev = BTN_CIRCLE_RIGHT;
                  Serial.printf("[Touch] Hungry Menu: Circle RIGHT (Fish) "
                                "tapped (X=%d, Y=%d)\n",
                                lastX, touchY);
                }
              }
            }
          }
          gestureState = STATE_IDLE;
          _hasScrolled = false;
        }
      }
      return ev;
    }

    // ── Finger DOWN / HELD ───────────────────────────────────────────────────
    if (fresh && fingerNum > 0) {
      lastTouchMs = now;
      lastX = x;
      lastY = y;

      if (!isDown) {
        isDown = true;
        startX = x;
        startY = y;
        startMs = now;
        gestureState = STATE_TOUCH_DOWN;
        _hasScrolled = false;
      } else {
        int dX = x - startX;
        int dY = y - startY;
        int absX = abs(dX);
        int absY = abs(dY);

        // Immediate touch slop threshold: 10 pixels
        if (absX >= 10 || absY >= 10) {
          _hasScrolled = true;
        }

        // Active tracking classification with immediate vertical lock
        if (gestureState == STATE_TOUCH_DOWN ||
            gestureState == STATE_TRACKING) {
          if (absY >= 10 && absY > absX) {
            gestureState = STATE_SCROLL_VERTICAL;
          } else if (absX >= 35 && absX > (absY * 3 / 2)) {
            gestureState = STATE_SWIPE_HORIZONTAL;
          } else if (absX >= 10 || absY >= 10) {
            gestureState = STATE_TRACKING;
          }
        }
      }

      // Live game controls — split screen left / right
      if (gamePlaying && currentScreen == SCREEN_GAMES) {
        if (!gameOverActive) {
          virtualBtn1 = (x < 120);
          virtualBtn2 = (x >= 120);
          return BTN_NONE;
        }
      }

      // ── Finger UP / Timed-out
      // ────────────────────────────────────────────────
    } else {
      bool explicitRelease = (fresh && fingerNum == 0);
      bool timedOut =
          (isDown && (now - lastTouchMs > 180)); // 180ms safe headroom

      if (isDown && (explicitRelease || timedOut)) {
        isDown = false;
        virtualBtn1 = false;
        virtualBtn2 = false;

        if (gamePlaying && currentScreen == SCREEN_GAMES) {
          if (!gameOverActive) {
            gestureState = STATE_IDLE;
            _hasScrolled = false;
            return BTN_NONE;
          }
          // Game Over is active: evaluate swipe-to-exit vs tap-to-replay
          int deltaX = lastX - startX;
          int deltaY = lastY - startY;
          int absX = abs(deltaX);
          int absY = abs(deltaY);
          gestureState = STATE_IDLE;
          _hasScrolled = false;

          // Swipe detected in any direction -> exit to arcade menu
          if (absX >= 18 || absY >= 18) {
            if (absY >= absX) {
              return (deltaY < 0) ? BTN_SWIPE_UP : BTN_SWIPE_DOWN;
            } else {
              return (deltaX < 0) ? BTN_SWIPE_LEFT : BTN_SWIPE_RIGHT;
            }
          }
          // Stationary tap -> replay game
          if (absX < 15 && absY < 15) {
            virtualBtn1 = true;
            return BTN1_SINGLE;
          }
          return BTN_NONE;
        }

        unsigned long held = now - startMs;
        int deltaX = lastX - startX;
        int deltaY = lastY - startY;
        int absX = abs(deltaX);
        int absY = abs(deltaY);

        // ── CASE 1: SCROLL OR DRAG OCCURRED ──
        // Once scrolling has engaged, strictly suppress all tap / selection
        // events!
        if (_hasScrolled || gestureState == STATE_SCROLL_VERTICAL ||
            gestureState == STATE_SWIPE_HORIZONTAL) {
          // Horizontal screen swipe
          if (gestureState == STATE_SWIPE_HORIZONTAL ||
              (absX >= 35 && absX > (absY * 3 / 2))) {
            if (currentScreen == SCREEN_FACE) {
              // Strictly suppress screen transition swipes while playing
              // character animation
              ev = BTN_NONE;
              Serial.println(
                  "[Touch] Horizontal swipe suppressed on animation screen");
            } else if (now - lastSwipeTransitionMs >= 300) {
              lastSwipeTransitionMs = now;
              ev = (deltaX < 0) ? BTN_SWIPE_LEFT : BTN_SWIPE_RIGHT;
            }
            // Vertical scroll completion
          } else if (gestureState == STATE_SCROLL_VERTICAL ||
                     (absY >= 16 && absY > absX)) {
            ev = (deltaY < 0) ? BTN_SWIPE_UP : BTN_SWIPE_DOWN;
          } else {
            ev = BTN_NONE;
          }

          // ── CASE 2: CLEAN STATIONARY TAP / LONG PRESS (NO SCROLLING) ──
        } else if (!_hasScrolled && absX < 12 && absY < 12) {
          if (held >= 500) {
            ev = BTN1_LONG;
            Serial.println("[Touch] Long press (>=500ms) -> Home Screen");
          } else if (held >= 20) {
            if (currentScreen == SCREEN_FACE) {
              // On animation screen: Left and Right side touches are DISABLED!
              // Only Center tap (X between 40 and 200) is recognized to
              // navigate to Clock.
              if (lastX >= 40 && lastX < 200) {
                ev = BTN1_SINGLE;
                Serial.printf(
                    "[Touch] CENTER tap on ANIMATION (X=%d) -> Go to Clock\n",
                    lastX);
              } else {
                Serial.printf("[Touch] Side tap ignored on ANIMATION (X=%d)\n",
                              lastX);
                ev = BTN_NONE;
              }
            } else if (currentScreen == SCREEN_CLOCK) {
              if (lastX >= 150) {
                ev = BTN2_SINGLE;
                Serial.printf("[Touch] Clock RIGHT tap (X=%d) -> Open Menu\n",
                              lastX);
              } else {
                ev = BTN1_SINGLE;
                Serial.printf("[Touch] Clock tap (X=%d) -> toggle style\n",
                              lastX);
              }
            } else if (currentScreen == SCREEN_MENU) {
              if (lastX < 20) {
                ev = BTN2_DOUBLE;
                Serial.printf(
                    "[Touch] Menu LEFT tap (X=%d) -> Return to Clock\n", lastX);
              } else {
                ev = BTN1_SINGLE;
                Serial.printf("[Touch] Menu tile tap (X=%d, Y=%d)\n", lastX,
                              lastY);
              }
            } else {
              if (lastX < 16) {
                ev = BTN2_DOUBLE;
                Serial.printf(
                    "[Touch] Edge LEFT tap (X=%d) -> Return to Menu\n", lastX);
              } else {
                ev = BTN1_SINGLE;
                Serial.printf("[Touch] Content tap (X=%d, Y=%d) -> select\n",
                              lastX, lastY);
              }
            }
          }
        } else {
          ev = BTN_NONE;
        }

        gestureState = STATE_IDLE;
        _hasScrolled = false;
      }
    }

    return ev;
  }
};

#endif // INTERACTION_H
