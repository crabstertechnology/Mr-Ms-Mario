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
  int touchPin;
  bool lastButtonState;
  bool debouncedState;
  unsigned long lastDebounceTime;
  unsigned long debounceDelay;

  // Gesture detection timings
  unsigned long touchStartTime;
  unsigned long touchEndTime;
  bool isPressed;
  bool isHolding;
  bool holdReported;
  
  int tapCount;
  unsigned long doubleTapTimeout;

public:
  LunaInteraction(int pin) : touchPin(pin) {
    lastButtonState = false;
    debouncedState = false;
    lastDebounceTime = 0;
    debounceDelay = 25; // 25ms debounce

    touchStartTime = 0;
    touchEndTime = 0;
    isPressed = false;
    isHolding = false;
    holdReported = false;

    tapCount = 0;
    doubleTapTimeout = 450; // 450ms to register double-taps
    
    pinMode(touchPin, INPUT);
  }

  TouchEvent update() {
    bool rawState = digitalRead(touchPin) == HIGH; // Active-HIGH capacitive touch sensor
    TouchEvent event = TOUCH_NONE;
    unsigned long now = millis();

    // 1. Debounce logic
    if (rawState != lastButtonState) {
      lastDebounceTime = now;
    }
    lastButtonState = rawState;

    if ((now - lastDebounceTime) > debounceDelay) {
      // Pin state has stabilized
      if (rawState != debouncedState) {
        debouncedState = rawState;

        if (debouncedState) {
          // Touch start (rising edge)
          isPressed = true;
          touchStartTime = now;
          isHolding = false;
          holdReported = false;
          Serial.println("[TOUCH] Pressed (HIGH on pin 1)");
        } else {
          // Touch release (falling edge)
          isPressed = false;
          touchEndTime = now;
          Serial.println("[TOUCH] Released (LOW on pin 1)");
          
          if (!isHolding) {
            tapCount++;
            if (tapCount >= 3) {
              event = TOUCH_TRIPLE_TAP;
              tapCount = 0; // Reset immediately
            }
          }
          isHolding = false;
        }
      }
    }

    // 2. Gesture parsing
    if (isPressed && !holdReported) {
      if ((now - touchStartTime) > 800) { // 800ms hold threshold
        isHolding = true;
        holdReported = true;
        tapCount = 0; // Clear tap queue on hold
        event = TOUCH_LONG_PRESS;
      }
    }

    // Process tap queue when release has occurred and no new touch starts
    if (tapCount > 0 && !isPressed) {
      if ((now - touchEndTime) > doubleTapTimeout) {
        if (tapCount == 1) {
          event = TOUCH_TAP;
        } else if (tapCount == 2) {
          event = TOUCH_DOUBLE_TAP;
        }
        tapCount = 0; // Reset tap count
      }
    }

    return event;
  }
};

#endif // INTERACTION_H
