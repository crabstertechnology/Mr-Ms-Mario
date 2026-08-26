// =============================================================================
// interaction.h  —  Luna Firmware CST816T Touch & QMI8658 IMU Input Handler
// =============================================================================
#ifndef INTERACTION_H
#define INTERACTION_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// ── Exposed button events (mapped to gestures) ───────────────────────────────
enum ButtonEvent {
  BTN_NONE         = 0,
  BTN1_SINGLE,          // Button 1 single click (Tap / Select / Next)
  BTN1_DOUBLE,          // Button 1 double click (Double tap / Go to Clock)
  BTN1_LONG,            // Button 1 long press (Long tap / Go to Face)
  BTN2_SINGLE,          // Button 2 single click (Swipe left / Change screen)
  BTN2_DOUBLE,          // Button 2 double click (Swipe right / Change screen back)
  BTN2_LONG,            // Button 2 long press
};

// ── Keep backwards-compatible TouchEvent ─────────────────────────────────────
enum TouchEvent {
  TOUCH_NONE        = 0,
  TOUCH_TAP,
  TOUCH_DOUBLE_TAP,
  TOUCH_TRIPLE_TAP,
  TOUCH_LONG_PRESS
};

// Global virtual button states for games (read by digitalRead macro interceptor)
extern bool virtualBtn1;
extern bool virtualBtn2;

// Extern references to main sketch state variables
extern bool gamePlaying;
extern bool settingsActive;
extern bool gamesActive;
extern int menuOption;
extern int gameMenuOption;
extern SmartwatchScreen currentScreen;

// Static volatile flag for touch interrupts
static volatile bool touchInterruptOccurred = false;
static void IRAM_ATTR touchISR() {
  touchInterruptOccurred = true;
}

class LunaInteraction {
private:
  unsigned long lastTouchMs;
  uint8_t lastGesture;
  bool isDown;
  int lastX, lastY;

  // Software Gesture State
  int startX, startY;
  unsigned long startMs;
  bool swipeTriggered;

  unsigned long lastTapMs;
  int lastTapX, lastTapY;

public:
  int getLastX() const { return lastX; }
  int getLastY() const { return lastY; }

  void begin() {
    lastTouchMs = 0;
    lastGesture = 0;
    isDown = false;
    lastX = 0;
    lastY = 0;
    
    startX = 0;
    startY = 0;
    startMs = 0;
    swipeTriggered = false;
    lastTapMs = 0;
    lastTapX = 0;
    lastTapY = 0;

    // Reset touch controller (CST816T reset pin is active low)
    pinMode(TOUCH_RST, OUTPUT);
    digitalWrite(TOUCH_RST, LOW);
    delay(10);
    digitalWrite(TOUCH_RST, HIGH);
    delay(100);

    // Initialize I2C (Wire) on the touch SDA/SCL pins
    Wire.begin(TOUCH_SDA, TOUCH_SCL, 400000);

    // Initialize Interrupt pin (active low when touched) with internal pull-up and falling edge interrupt
    pinMode(TOUCH_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TOUCH_INT), touchISR, FALLING);
    touchInterruptOccurred = false;
    Serial.println("CST816T touch interface initialized with hardware interrupt.");
  }

  bool readTouch(uint8_t &gesture, uint8_t &fingerNum, int &x, int &y) {
    if (!touchInterruptOccurred) {
      return false;
    }
    touchInterruptOccurred = false; // Reset trigger flag

    Wire.beginTransmission(0x15); // CST816T I2C Address
    Wire.write(0x01); // Register 0x01: GestureID
    if (Wire.endTransmission(false) != 0) {
      return false;
    }

    if (Wire.requestFrom(0x15, 6) < 6) {
      return false;
    }

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

  ButtonEvent update() {
    uint8_t gesture = 0;
    uint8_t fingerNum = 0;
    int x = 0;
    int y = 0;

    bool success = readTouch(gesture, fingerNum, x, y);
    unsigned long now = millis();

    ButtonEvent resolvedEvent = BTN_NONE;

    if (success && fingerNum > 0) {
      Serial.printf("[TOUCH_RAW] G:%02X F:%d X:%d Y:%d MS:%lu\n", gesture, fingerNum, x, y, now);
      lastTouchMs = now;
      lastX = x;
      lastY = y;

      if (!isDown) {
        // Touch down event detected
        isDown = true;
        startX = x;
        startY = y;
        startMs = now;
      }

      // Live gameplay controls virtualization
      if (gamePlaying && currentScreen == SCREEN_GAMES) {
        // Tap left half of screen = BTN1, Tap right half = BTN2
        if (x < 120) {
          virtualBtn1 = true;
          virtualBtn2 = false;
        } else {
          virtualBtn1 = false;
          virtualBtn2 = true;
        }
        return BTN_NONE;
      }

    } else {
      // Release touch state
      bool isExplicitRelease = (success && fingerNum == 0);
      if (isDown && (isExplicitRelease || (now - lastTouchMs > 50))) {
        isDown = false;
        virtualBtn1 = false;
        virtualBtn2 = false;

        unsigned long dt = now - startMs;
        if (dt < 300) {
          // Single Tap
          lastTapMs = now;
          lastTapX = lastX;
          lastTapY = lastY;
          
          if (lastX < TOUCH_LEFT_LIMIT) {
            // Physical Left Side Tap -> Previous Screen (Cycle Backward)
            resolvedEvent = BTN2_DOUBLE;
            Serial.println("[Touch] Physical Left Side Tap -> previous screen");
          } else if (lastX >= TOUCH_RIGHT_LIMIT) {
            // Physical Right Side Tap -> Next Screen (Cycle Forward)
            resolvedEvent = BTN2_SINGLE;
            Serial.println("[Touch] Physical Right Side Tap -> next screen");
          } else {
            // Center Column Tap -> Normal Single Tap / Select Option / Scroll
            resolvedEvent = BTN1_SINGLE;
            Serial.println("[Touch] Center Column Tap -> select/scroll");
          }
        } else if (dt >= 500) {
          // Long Press -> Return to Home (Face) Screen
          resolvedEvent = BTN1_LONG;
          Serial.println("[Touch] Long Press detected");
        }
      }
    }

    return resolvedEvent;
  }
};

#endif // INTERACTION_H
