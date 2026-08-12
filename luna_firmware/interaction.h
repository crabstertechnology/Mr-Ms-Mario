#ifndef INTERACTION_H
#define INTERACTION_H

#include <Arduino.h>
#include "config.h"

enum TouchEvent {
  TOUCH_NONE = 0,
  TOUCH_TAP,
  TOUCH_DOUBLE_TAP,
  TOUCH_TRIPLE_TAP,
  TOUCH_LONG_PRESS
};

class LunaInteraction {
private:
  int pinExpr;
  int pinSettings;

  bool lastStateExpr;
  bool debouncedStateExpr;
  unsigned long lastDebounceTimeExpr;

  bool lastStateSettings;
  bool debouncedStateSettings;
  unsigned long lastDebounceTimeSettings;

  unsigned long debounceDelay;

public:
  LunaInteraction(int exprPin, int settingsPin) : pinExpr(exprPin), pinSettings(settingsPin) {
    lastStateExpr = true;
    debouncedStateExpr = true;
    lastDebounceTimeExpr = 0;

    lastStateSettings = true;
    debouncedStateSettings = true;
    lastDebounceTimeSettings = 0;

    debounceDelay = 20; // 20ms debounce for mechanical switches

    pinMode(pinExpr, INPUT_PULLUP);
    pinMode(pinSettings, INPUT_PULLUP);
  }

  // Returns:
  // 1: Button Expression pressed (single tap equivalent)
  // 2: Button Settings pressed (short press)
  int update(bool &isLongPressSettings) {
    int event = 0;
    isLongPressSettings = false;
    unsigned long now = millis();

    // 1. Read Button Expression (Active LOW)
    bool rawExpr = digitalRead(pinExpr) == LOW;
    static bool prevDebouncedExpr = false;

    // Debounce Button Expression
    static bool lastRawExpr = false;
    static unsigned long lastDebTimeExpr = 0;
    if (rawExpr != lastRawExpr) {
      lastDebTimeExpr = now;
    }
    lastRawExpr = rawExpr;

    if ((now - lastDebTimeExpr) > debounceDelay) {
      if (rawExpr != prevDebouncedExpr) {
        prevDebouncedExpr = rawExpr;
        if (prevDebouncedExpr) {
          // Button pressed (falling edge of digital reading)
          event = 1; 
        }
      }
    }

    // 2. Read Button Settings (Active LOW)
    bool rawSettings = digitalRead(pinSettings) == LOW;
    static bool prevDebouncedSettings = false;
    static unsigned long pressStartSettings = 0;
    static bool longPressReported = false;

    static bool lastRawSettings = false;
    static unsigned long lastDebTimeSettings = 0;
    if (rawSettings != lastRawSettings) {
      lastDebTimeSettings = now;
    }
    lastRawSettings = rawSettings;

    if ((now - lastDebTimeSettings) > debounceDelay) {
      if (rawSettings != prevDebouncedSettings) {
        prevDebouncedSettings = rawSettings;
        if (prevDebouncedSettings) {
          // Button pressed
          pressStartSettings = now;
          longPressReported = false;
        } else {
          // Button released
          if (!longPressReported && (now - pressStartSettings < 1000)) {
            event = 2; // Short press settings
          }
        }
      }
    }

    // Check settings long press (hold for 1 second)
    if (prevDebouncedSettings && !longPressReported) {
      if ((now - pressStartSettings) >= 1000) {
        isLongPressSettings = true;
        longPressReported = true;
      }
    }

    return event;
  }
};

#endif // INTERACTION_H
