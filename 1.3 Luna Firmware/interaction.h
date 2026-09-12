  // =============================================================================
// interaction.h  —  Luna Firmware Dual-Button Input Handler (No Gesture Delay)
// =============================================================================
// Button 1 (BTN_EXPR_PIN):
//   • Click     : Next expression (Face) / Action / Cycle style / Next event
//   • Long press: Return to Face screen / Back
// Button 2 (BTN_SETTINGS_PIN):
//   • Click     : Advance to Next Screen (Face -> Clock -> Notifs -> Cal -> Games -> Card -> Setup)
//   • Long press: Confirm / Select in Settings
// =============================================================================
#ifndef INTERACTION_H
#define INTERACTION_H

#include <Arduino.h>
#include "config.h"

// ── Exposed button events ────────────────────────────────────────────────────
enum ButtonEvent {
  BTN_NONE         = 0,
  BTN1_SINGLE,          // Button 1 direct click (immediate on release)
  BTN1_DOUBLE,          // (kept for enum compat)
  BTN1_LONG,            // Button 1 long press (> 600ms)
  BTN2_SINGLE,          // Button 2 direct click (immediate on release)
  BTN2_DOUBLE,          // (kept for enum compat)
  BTN2_LONG             // Button 2 long press (> 600ms)
};

// ── Per-button immediate debounced state machine (no gesture delay) ─────────
struct BtnState {
  int           pin;
  bool          lastRaw;
  bool          debounced;
  unsigned long lastDebounceMs;
  unsigned long pressStartMs;
  bool          pressed;
  bool          longFired;

  static const unsigned long DEBOUNCE_MS   = 30;   // 30 ms debounce
  static const unsigned long LONG_PRESS_MS = 600;  // 600 ms long press

  void begin(int p, bool pullup = true) {
    pin = p;
    if (pullup) {
      pinMode(pin, INPUT_PULLUP);
    } else {
      pinMode(pin, INPUT);
    }
    bool initPressed = (digitalRead(pin) == LOW);
    lastRaw        = initPressed;
    debounced      = initPressed;
    lastDebounceMs = millis();
    pressStartMs   = 0;
    pressed        = initPressed;
    longFired      = false;
  }

  // Poll button: returns singleEvt immediately on release, or longEvt if held >= 600ms
  ButtonEvent poll(int singleEvt, int longEvt) {
    bool raw = (digitalRead(pin) == LOW); // active-low with INPUT_PULLUP
    unsigned long now = millis();

    if (raw != lastRaw) {
      lastDebounceMs = now;
    }
    lastRaw = raw;

    if ((now - lastDebounceMs) >= DEBOUNCE_MS) {
      if (raw != debounced) {
        debounced = raw;
        if (debounced) {
          // Press edge
          pressed      = true;
          pressStartMs = now;
          longFired    = false;
        } else {
          // Release edge — fire single click immediately with zero gesture delay!
          pressed = false;
          if (!longFired) {
            return (ButtonEvent)singleEvt;
          }
        }
      }
    }

    // Long press detection while held
    if (pressed && !longFired && (now - pressStartMs) >= LONG_PRESS_MS) {
      longFired = true;
      if (longEvt != BTN_NONE) {
        return (ButtonEvent)longEvt;
      }
    }

    return BTN_NONE;
  }
};

// ── LunaInteraction: manages both physical buttons ──────────────────────────
class LunaInteraction {
private:
  BtnState btn1;
  BtnState btn2;

public:
  void begin() {
    btn1.begin(BTN_EXPR_PIN,     true); // Button 1: Pin 8 (INPUT_PULLUP, active-low)
    btn2.begin(BTN_SETTINGS_PIN, true); // Button 2: Pin 9 (INPUT_PULLUP, active-low)
  }

  // Poll both buttons with priority: btn1 -> btn2
  ButtonEvent update() {
    ButtonEvent e1 = btn1.poll(BTN1_SINGLE, BTN1_LONG);
    if (e1 != BTN_NONE) return e1;

    ButtonEvent e2 = btn2.poll(BTN2_SINGLE, BTN2_LONG);
    return e2;
  }
};

// ── Keep backwards-compatible TouchEvent for legacy call sites ───────────────
enum TouchEvent {
  TOUCH_NONE        = 0,
  TOUCH_TAP,
  TOUCH_DOUBLE_TAP,
  TOUCH_TRIPLE_TAP,
  TOUCH_LONG_PRESS
};

#endif // INTERACTION_H
