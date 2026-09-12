# LUNA SMARTWATCH UI INTERACTION SPECIFICATION

**Target Hardware**: Waveshare ESP32-S3 Touch LCD 1.69"  
**Resolution**: 240 × 280 pixels (Portrait)  
**Display Driver**: ST7789 via 80MHz SPI with double-buffered PSRAM canvas  
**Touch Controller**: Hynitron CST816T (I2C address 0x15)  
**UI Engine**: LVGL 8.3.x / EEZ Studio v3  
**Document Version**: 1.0 (Commercial Smartwatch Standard)  

---

## 1. Physical Touch Input Profile & Limits

The CST816T capacitive touch IC provides point coordinates $(X, Y)$ and event types (`DOWN`, `UP`, `MOVE`, or hardware gesture codes). 

On a 1.69" 240 × 280 display:
- **Pixel Pitch**: ~0.15 mm / px (~169 DPI).
- **Physical Finger Contact Area**: Typical adult fingertip spans 6–9 mm (~40–60 pixels).
- **Minimum Tap Target**:
  - Absolute physical minimum: **30 × 30 px** (4.5 × 4.5 mm).
  - Recommended commercial target: **36–44 px** height.
  - Critical action CTAs (e.g. `START`, `SAVE`, `CALIBRATE`): Minimum **36 px** height.
- **Visual vs. Hit Target Decoupling**:
  Small visual icons (e.g., 16×16 or 20×20 px) MUST be nested inside a touch container of at least **36 × 36 px** to guarantee single-attempt hit success.

---

## 2. Gesture Classification State Machine

Every touch sequence from `TOUCH_DOWN` to `TOUCH_UP` is classified by an explicit spatial and temporal discriminator to eliminate ambiguity between tapping, vertical scrolling, and horizontal screen navigation.

```
       [ TOUCH_DOWN ] (X0, Y0, T0)
             │
             ▼
     [ TRACK MOVEMENT ] (dX = X - X0, dY = Y - Y0)
             │
    ┌────────┴───────────────────────────┐
    │                                    │
    ▼                                    ▼
[ abs(dX) < 12 && abs(dY) < 12 ]    [ sqrt(dX² + dY²) >= 12 ]
    │ (Within Slop Deadband)             │ (Intent Detected)
    │                                    │
    ▼                                    ├──────────────────────────┐
[ TOUCH_UP ]                             │                          │
    │                                    ▼                          ▼
    ▼                           [ abs(dX) > 1.5 * abs(dY) ]    [ abs(dY) >= abs(dX) ]
[ TAP EVENT ]                   (Horizontal Dominance)         (Vertical Dominance)
• Dispatched to child control            │                          │
• Action on release ONLY                 │                          ▼
• Visual pressed feedback cleared        │                [ VERTICAL SCROLL ]
                                         │                • Claimed by scroll container
                                         │                • Page swipe locked out
                                         │                • Content follows finger
                                         │
                                         ▼
                             [ HORIZONTAL DISPLACEMENT ]
                             • Distance: abs(dX) >= 50 px
                             • Velocity: abs(dX) / dt >= 0.4 px/ms
                                         │
                            ┌────────────┴────────────┐
                            ▼                         ▼
                       [ dX < -50 ]              [ dX > +50 ]
                       Swipe Left                Swipe Right
                            │                         │
                            ▼                         ▼
                      NEXT SCREEN               PREV SCREEN
```

---

## 3. Detailed Gesture Rules

### 3.1. Tap Behavior
1. **Touch-Down**: Immediate visual state change (`MAIN.PRESSED` state activates, border or background lightens/darkens subtly).
2. **Deadband**: Finger movement $< 12\text{ px}$ from $(X_0, Y_0)$ keeps the gesture in candidate-tap state.
3. **Touch-Up**: If released inside the widget boundary and no swipe/scroll was triggered:
   - Play audio/haptic click (if buzzer enabled).
   - Execute associated action.
   - Return widget to `MAIN.DEFAULT` state.
4. **Touch-Cancellation**: If the finger moves $> 12\text{ px}$ into a scroll or swipe, the pressed state is immediately cancelled and no tap action fires.

### 3.2. Horizontal Swipe Navigation
1. **Primary Screen Order (8 Core Screens)**:
   $$\text{Home (Face)} \longleftrightarrow \text{Clock} \longleftrightarrow \text{Notifications} \longleftrightarrow \text{Calendar} \longleftrightarrow \text{Games} \longleftrightarrow \text{Card} \longleftrightarrow \text{Settings} \longleftrightarrow \text{Pomodoro}$$
2. **Swipe Left**:
   - Condition: $dX \le -50\text{ px}$ AND $|dX| > 1.5 \times |dY|$.
   - Effect: Navigates to the next primary screen ($i \to i + 1$).
   - Boundary: On the last screen (`Pomodoro`), swipe left stops with subtle resistance or rubber-banding; no wrapping to avoid user disorientation.
3. **Swipe Right**:
   - Condition: $dX \ge +50\text{ px}$ AND $|dX| > 1.5 \times |dY|$.
   - Effect: Navigates to the previous primary screen ($i \to i - 1$).
   - Boundary: On the first screen (`Home`), swipe right stops at edge boundary.
4. **Transition Dynamics**:
   - Animation: `LV_SCR_LOAD_ANIM_MOVE_LEFT` / `LV_SCR_LOAD_ANIM_MOVE_RIGHT`.
   - Duration: **200 ms** (snappy, responsive, under the 250ms perception threshold).

### 3.3. Child Screen & Back Navigation
1. **Sub-Screen Hierarchy**:
   - `NOTIFICATIONS` $\to$ `NOTIFICATION_DETAIL`
   - `CALENDAR` $\to$ `CALENDAR_EVENTS (Agenda)` $\to$ `CALENDAR_DETAIL`
   - `GAMES` $\to$ `GAME_OVERLAY`
2. **Back Invocation**:
   - **Explicit CTA**: Dedicated `BACK` / `EXIT` button at bottom of screen ($H \ge 34\text{ px}$).
   - **Edge Swipe Right**: Swiping right from the left screen boundary ($X_0 < 30\text{ px}$, $dX > 40\text{ px}$) returns to the parent screen.
   - **Deduping**: Navigation triggers are debounced (minimum 300ms lockout) to ensure a single transition per gesture.

---

## 4. Vertical Scrolling Specification

### 4.1. Scroll Container Rules
1. **Scroll Ownership**:
   - Only dedicated list containers have `LV_OBJ_FLAG_SCROLLABLE` set.
   - Root screens, status bars, header panels, cards, and labels have `LV_OBJ_FLAG_SCROLLABLE` **explicitly removed**.
2. **Axis Restriction**:
   - Scroll containers enforce vertical-only scrolling: `lv_obj_set_scroll_dir(container, LV_DIR_VER)`.
   - Horizontal scrolling is completely disabled on vertical containers: `scroll_chain_hor = False`.
3. **Boundary Clamping & Elasticity**:
   - Top boundary: Scroll offset $= 0$. When dragged downward at the top, elastic resistance applies ($F_{\text{drag}} \propto \sqrt{dY}$), snapping back cleanly upon release.
   - Bottom boundary: The last item's bottom edge aligns with the bottom of the visible container plus $12\text{ px}$ safe breathing margin. Content never scrolls into empty space.
4. **Scrollbar Indicator**:
   - Thin 2px vertical pill on the right edge (`X = 236`), semi-transparent (`luna_muted` at 40% opacity).
   - Scrollbar mode: `LV_SCROLLBAR_MODE_AUTO` (appears only during active scroll movement, fades out after 500ms of inactivity).

---

## 5. Child Interaction Priority (Conflict Resolution)

When a touch gesture starts on an interactive child control inside a scrollable screen:

| Component | Touch-Down | Drag ($|dY| > 12$) | Drag ($|dX| > 12$) | Touch-Up ($< 12\text{ px}$) |
|---|---|---|---|---|
| **Button** | Pressed style active | Cancel press $\to$ Scroll | Cancel press $\to$ Screen swipe | Fire Button Action |
| **Switch / Toggle** | Knob pressed highlight | Scroll parent container | If horizontal $\to$ Toggle state | Toggle switch state |
| **Slider** | Knob active highlight | Ignored (Slider locks touch) | Adjust slider value | Apply setting value |
| **List Card** | Card surface tint | Scroll parent container | Screen swipe | Open Card Detail |
| **Arc (Pomodoro/Level)** | Arc focused | Scroll parent container | Adjust target (if unlocked) | Select mode tab |

**Core Invariant**:
$$\text{Slider / Switch interaction} > \text{Container Vertical Scroll} > \text{Global Horizontal Swipe}$$

---

## 6. Animation Safety & Frame Budget

- **Target Refresh Rate**: 30 FPS (~33.3 ms per frame budget).
- **Transition Budget**:
  - Screen slide transition: **200 ms** (6 frames total).
  - Button press feedback: **Instant** (0 ms delay, rendered on next frame).
  - Arc value animation: **150 ms** smooth ease-out.
- **Resource Management**:
  - All timers and active screen animations must be deleted or paused in `LV_EVENT_SCREEN_UNLOADED`.
  - When loading a new screen, double-buffered PSRAM canvas updates prevent any tearing on the ST7789 display.
