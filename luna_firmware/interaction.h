// =============================================================================
// interaction.h  —  Luna Firmware Dual-Button Input Handler
// =============================================================================
// Button 1 (BTN_EXPR_PIN):
//   • Main Screen: Single click = next expression, Double click = clock screen
//   • Settings    : Single click = navigate UP (menu option --)
// Button 2 (BTN_SETTINGS_PIN):
//   • Main Screen: Single click = open settings
//   • Settings    : Single click = navigate DOWN (menu option ++)
//                   Long press   = select / confirm highlighted option
// =============================================================================
#ifndef INTERACTION_H
#define INTERACTION_H

#include <Arduino.h>
#include "config.h"

// ── Exposed button events ────────────────────────────────────────────────────
enum ButtonEvent {
  BTN_NONE         = 0,
  BTN1_SINGLE,          // Button 1 single click
  BTN1_DOUBLE,          // Button 1 double click
  BTN1_LONG,            // Button 1 long press
  BTN2_SINGLE,          // Button 2 single click
  BTN2_LONG,            // Button 2 long press
};

// ── Per-button state machine ─────────────────────────────────────────────────
struct BtnState {
  int     pin;
  bool    lastRaw;
  bool    debounced;
  unsigned long lastDebounceMs;

  unsigned long pressStartMs;
  unsigned long releaseMs;
  bool    pressed;
  bool    longFired;
  int     clickCount;

  // config
  static const unsigned long DEBOUNCE_MS   = 40;   // 40 ms
  static const unsigned long LONG_PRESS_MS = 700;  // 700 ms
  static const unsigned long DBL_CLICK_MS  = 350;  // window after 1st release

  void begin(int p, bool pullup = true) {
    pin = p;
    lastRaw       = HIGH;
    debounced     = HIGH;
    lastDebounceMs = 0;
    pressStartMs  = 0;
    releaseMs     = 0;
    pressed       = false;
    longFired     = false;
    clickCount    = 0;
    if (pullup) {
      pinMode(pin, INPUT_PULLUP);
    } else {
      pinMode(pin, INPUT);
    }
  }

  // Call every loop(); returns a raw event or BTN_NONE
  // eventBase: the BTN_x_SINGLE value for this button (e.g. BTN1_SINGLE)
  ButtonEvent poll(int singleEvt, int doubleEvt, int longEvt) {
    bool raw    = (digitalRead(pin) == LOW); // active-low with INPUT_PULLUP
    unsigned long now = millis();

    // ── Debounce ──────────────────────────────────────────────────────────
    if (raw != lastRaw) {
      lastDebounceMs = now;
    }
    lastRaw = raw;

    bool stable = (now - lastDebounceMs) >= DEBOUNCE_MS;

    if (stable && raw != debounced) {
      debounced = raw;
      if (debounced) {
        // ── Press edge ────────────────────────────────────────────────────
        pressed      = true;
        pressStartMs = now;
        longFired    = false;
      } else {
        // ── Release edge ──────────────────────────────────────────────────
        pressed   = false;
        releaseMs = now;
        if (!longFired) {
          clickCount++;
          // Triple+ clicks treated as single to avoid user confusion
          if (clickCount > 2) clickCount = 1;
        }
      }
    }

    // ── Long press detection (fires while held) ───────────────────────────
    if (pressed && !longFired && (now - pressStartMs) >= LONG_PRESS_MS) {
      longFired  = true;
      clickCount = 0; // cancel any pending tap
      return (ButtonEvent)longEvt;
    }

    // ── Tap/double-tap timeout resolution (after release) ─────────────────
    if (!pressed && clickCount > 0 && (now - releaseMs) >= DBL_CLICK_MS) {
      int cnt    = clickCount;
      clickCount = 0;
      if (cnt == 1) return (ButtonEvent)singleEvt;
      if (cnt == 2) return (ButtonEvent)doubleEvt;
    }

    return BTN_NONE;
  }
};

// ── LunaInteraction: manages both buttons ────────────────────────────────────
class LunaInteraction {
private:
  BtnState btn1;
  BtnState btn2;

public:
  void begin() {
    btn1.begin(BTN_EXPR_PIN,     true); // INPUT_PULLUP, active-low
    btn2.begin(BTN_SETTINGS_PIN, true); // INPUT_PULLUP, active-low
  }

  // Poll both buttons; returns the first event found (priority: btn1 → btn2)
  ButtonEvent update() {
    ButtonEvent e1 = btn1.poll(BTN1_SINGLE, BTN1_DOUBLE, BTN1_LONG);
    if (e1 != BTN_NONE) return e1;

    ButtonEvent e2 = btn2.poll(BTN2_SINGLE, BTN_NONE, BTN2_LONG);
    return e2;
  }
};

// ── Keep backwards-compatible TouchEvent for any legacy call sites ───────────
enum TouchEvent {
  TOUCH_NONE        = 0,
  TOUCH_TAP,
  TOUCH_DOUBLE_TAP,
  TOUCH_TRIPLE_TAP,
  TOUCH_LONG_PRESS
};

#endif // INTERACTION_H
