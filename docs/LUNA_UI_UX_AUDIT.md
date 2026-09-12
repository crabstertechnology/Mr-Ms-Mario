# LUNA EEZ STUDIO UI/UX COMPREHENSIVE AUDIT REPORT

**Project File**: `W:\Mr.mario\1.69 Luna Firmware\ui\luna.eez-project`  
**Target Board**: Waveshare ESP32-S3 Touch LCD 1.69" (240 × 280 portrait, ST7789, CST816T Touch)  
**Framework**: LVGL 8.3.x / EEZ Studio v3 (JSON project format)  
**Audit Date**: September 10, 2026  

---

## 1. Current Screen Structure

The project currently declares **15 screens** (`userPages`) and **12 reusable user widgets** (`userWidgets`). All screens are sized to 240 × 280 px.

### Screen Inventory:
1. **`FACE`** (Cute Robot Expression Dashboard): Root `LVGLScreenWidget` containing `status_bar_face` (240×22), screen title labels, expression state pill (`IDLE`), central face animation frame container (208×166) with nested mouth and labels, and a footer hint.
2. **`CLOCK`** (Smartwatch Clock): Digital time readout (`ui_time` in 32pt), full date (`ui_date` in 12pt), style badge (`24H`), clock face container (208×151), and style footer container (208×28).
3. **`NOTIFICATIONS`** (Notification Hub): Title bar and 5 statically stacked notification cards (`notification_item_0` to `4`) at fixed Y positions: 72, 110, 148, 186, 224.
4. **`NOTIFICATION_DETAIL`** (Expanded Notification): Header, detail content card (208×101), timestamp card (208×51), and two buttons (`MARK READ`, `BACK`) at Y=248.
5. **`NOTIFICATIONS_EMPTY`** (All-Clear Zero State): Header, large centered panel (208×132) with empty-state icon label and copy.
6. **`CALENDAR`** (Month View): Header, month badge, 7×5 numeric day label matrix (208×156) with highlighted day `06`, and bottom selected date panel (208×28).
7. **`CALENDAR_EVENTS`** (Agenda List): Header, date pill (`SEP 06`), 3 statically stacked event panels (at Y=78, 136, 194), and a `VIEW DAY` action button at Y=250.
8. **`CALENDAR_DETAIL`** (Event Detail View): Single event summary panel (208×126) and a `BACK TO AGENDA` button at Y=244.
9. **`GAMES`** (Arcade Launcher): Title, game counter pill (`7 GAMES`), 4 statically placed game cards (`Luna Racer`, `Luna Space`, `Flappy Mochy`, `Coin Catcher`) and a mini HUD preview container. **Games 5, 6, and 7 are missing from the layout.**
10. **`GAME_OVERLAY`** (Game Over Screen): Score summary card (208×106), `RETRY` and `EXIT` buttons at Y=212.
11. **`MAPS`** (HUD Navigation): Directional arrow (156×63), distance readout (`nav_distance`), instruction string (`nav_instruction`), live route status bar, and `EXIT MAP` button.
12. **`CARD`** (Digital Business Card): Central container (164×164) with `LVGLQRCodeWidget` (140×140), owner name label (`card_name`), and contact label (`card_contact`).
13. **`SETTINGS`** (System Configuration): Stacked panels for BLE switch, Silent mode switch, Brightness slider, Clock style readout, and `SAVE SETTINGS` button at Y=238.
14. **`LEVEL`** (Spirit Level Analyzer): 148×148 circular container with `LVGLArcWidget` (130×130), bubble dot panel (32×32), numeric pitch readout, gyro string, and `CALIBRATE` button at Y=250.
15. **`POMODORO`** (Focus Timer): Header with mode badge (`pomodoro_mode`), 170×170 timer panel with circular `LVGLArcWidget` (146×146), digital countdown (`25:00`), `START` and `RESET` buttons at Y=248.

---

## 2. Current Navigation Model

- **Existing State**: **Zero navigation actions or event handlers exist.**
  - Analysis of `luna.eez-project` reveals `actions: []` (empty list) and all components have `eventHandlers: []` (empty lists).
  - No transitions, screen load triggers, or EEZ Flow linkages are defined.
- **Deficiency**: The UI is currently a disconnected set of static layout mockups. There is no way for a user to move between screens via touch or swipe.

---

## 3. Current Scroll Behavior

- **Critical Bug — Ubiquitous `SCROLLABLE` Flag**:
  - Almost every single widget in the project has the default flag set:
    `SCROLLABLE|SCROLL_CHAIN_HOR|SCROLL_CHAIN_VER|SCROLL_ELASTIC|SCROLL_MOMENTUM|SCROLL_WITH_ARROW|SNAPPABLE`
  - Even single-line text labels (e.g., `title_luna_face`, `status_time`, day numbers in the calendar) and small static icons have `scrollable=True`.
  - In LVGL, this causes micro-drags to scroll individual labels inside their 12px bounding boxes instead of propagating to the screen or container.
- **Missing Native Scroll Containers**:
  - The multi-item screens (`NOTIFICATIONS`, `SETTINGS`, `GAMES`, `CALENDAR_EVENTS`) do **not** use an LVGL vertical scroll container (`lv_obj_set_scroll_dir(..., LV_DIR_VER)`).
  - Instead, items are placed with absolute coordinates directly on the screen widget. Content extending beyond Y=240 simply clips or is omitted entirely (e.g. Games 5, 6, 7).

---

## 4. Current Swipe Behavior

- **Existing State**: **No swipe gesture handling exists.**
- **Gesture Conflict**:
  - Because all child widgets currently have `GESTURE_BUBBLE` enabled alongside `SCROLLABLE`, any horizontal finger movement triggers LVGL's internal horizontal scroll inertia on child panels rather than bubbling a clean `LV_EVENT_GESTURE` or drag displacement.
  - No horizontal velocity or distance thresholding exists.

---

## 5. Current Touch Hitboxes

- **Sub-standard Touch Targets (Below 30×30 px)**:
  - Calendar Day Cells: `19 × 12 px` (impossible for capacitive finger input without false taps).
  - Switches & Toggles: `ble_toggle` is `42 × 22 px` with no surrounding touch margin.
  - Brightness Slider: Track height is `12 px` (`54 × 12 px`), making sliding extremely difficult.
  - Header Action Pill: `42 × 18 px` to `58 × 18 px`.
  - Level Calibrate Button: Height `26 px` (`128 × 26 px`).
  - Notification Action Buttons: Height `28 px` (`100 × 28 px`).
  - Pomodoro Action Buttons: Height `28 px` (`100 × 28 px`).
- **Target Standard**: In an embedded 240×280 screen, primary buttons must have at least **32–36 px** height (preferably 36–44 px for primary CTAs).

---

## 6. Existing Visual Inconsistencies

- **Arbitrary Corner Radii**:
  - Corner radii are scattered across 14 different values: `r=0, 4, 6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 22, 28, 74, 85`. There is no disciplined corner radius system.
- **Inconsistent Margins & Padding**:
  - Side margins fluctuate between 16px, 18px, 20px, and 24px across different screens.
  - Top margins below the status bar vary from 4px to 16px.
- **Visual Clutter & High Component Count**:
  - Subtitle labels ("expression frame supplied by firmware", "up to five firmware-supplied items", "controls are touch sized for 240 px") take up valuable vertical real estate with developer commentary rather than user-facing UI.

---

## 7. Existing Duplicated Styles

- **Lack of Global Styles**:
  - `data.lvglStyles.styles` is completely empty (`[]`).
  - Every single widget duplicates raw hex colors, font references, and border dimensions in its private `localStyles.definition` block.
- **Color Divergence**:
  - Light theme hardcodes `#EEF2F7`, `#D5DEE9`, `#DCE5EF`, `#17212C`, `#64748B`, `#2457D6`, `#E8468B`, `#021018`, `#0B8292`, `#3B5266` inline instead of referencing the clean design tokens.

---

## 8. Existing Interaction Bugs

1. **Missing Games in Arcade Launcher**:
   - `GAMES` screen contains only 4 cards (`game_card_0` to `game_card_3`).
   - Games 5 (`Mochy Jump`), 6 (`Stacker`), and 7 (`Memory Matrix`) are physically absent because the screen layout is static and cannot scroll.
2. **Negative Label Coordinates in Settings**:
   - In `SETTINGS`, the `settings_save` button contains a child label positioned at `@ (-13, -2)`! This causes off-center and clipped text on the physical display.
3. **QR Code Container Sizing**:
   - `CARD` container is 164×164, but the QR widget itself is 140×140 at offset `(50, 76)`, pushing it off-center relative to its bounding card.
4. **Header Collision**:
   - Screen headers place status pills at `X=184, W=56` (`184 + 56 = 240`), pushing them directly against the screen edge with 0px safe margin.

---

## 9. What Will Be Changed

1. **Design System & Palette Alignment**:
   - Implement the strict color tokens (Light: `#FFFFFF`, `#210421`, `#197AFA`, `#F8B8D0`, `#F5F7FA`, `#E5E7EB`, `#6B7280`; Cyberpunk Dark: `#0D0D11`, `#FFFFFF`, `#07FFFF`, `#F8B8D0`, `#16161D`, `#1D1D26`, `#292933`).
   - Unify corner radii into a 3-tier scale: Small (6px), Medium (10px), Large (14px).
   - Standardize typography scale across all screens.
2. **Navigation Hierarchy**:
   - Primary 8-screen horizontal carousel:
     `Home (Face)` ↔ `Clock` ↔ `Notifications` ↔ `Calendar` ↔ `Games` ↔ `Business Card` ↔ `Settings` ↔ `Pomodoro`
   - Child/sub-screen hierarchy with explicit back buttons:
     - `Notifications` → `Notification Detail`
     - `Calendar` → `Calendar Events (Agenda)` → `Calendar Detail`
     - `Games` → `Game Overlay`
3. **Scroll Architecture**:
   - Strip `SCROLLABLE` flag from all non-scrollable widgets, labels, icons, buttons, and headers.
   - Introduce dedicated vertical scroll containers with `LV_DIR_VER` and boundary clamping on `NOTIFICATIONS`, `SETTINGS`, `GAMES`, and `CALENDAR_EVENTS`.
   - Add all 7 games to the `GAMES` scroll container.
4. **Touch Target Expansion**:
   - Enforce minimum button height of 34–38 px.
   - Enforce minimum calendar touch areas of 28 × 24 px with comfortable tap feedback.
   - Add clear `MAIN.PRESSED` visual states (color shift / active tint).

---

## 10. What Must Remain Untouched

1. **Firmware Architecture**:
   - `1.69 Luna Firmware.ino`, drivers, FreeRTOS tasks, BLE, audio, and sensors remain strictly untouched.
2. **Dynamic Global Variables (All 29 bindings preserved)**:
   - `ui_time`, `ui_date`, `status_battery`, `status_ble`, `status_wifi`, `face_expression`, `face_frame`, `clock_style`, `notification_title`, `notification_preview`, `calendar_month`, `selected_date`, `nav_distance`, `nav_instruction`, `nav_alert`, `card_name`, `card_contact`, `qr_url`, `ble_enabled`, `silent_mode`, `brightness`, `level_pitch`, `level_roll`, `level_gyro`, `pomodoro_time`, `pomodoro_progress`, `pomodoro_mode`, `game_score`, `game_lives`.
3. **Target Device Hardware Profile**:
   - 240 × 280 portrait resolution, 18px display corner radius, 16-bit RGB565/BGR color format.
4. **Game Suite Completeness**:
   - All 7 games must remain fully accessible via the UI launcher.
