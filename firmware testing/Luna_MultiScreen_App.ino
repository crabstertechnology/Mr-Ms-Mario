// ============================================================================
// GENERATED FILE — DO NOT EDIT MANUALLY.
// SOURCE: Luna UI Studio
// REGENERATE FROM STUDIO.
// Multi-Screen Interactive UI for ESP32-S3 1.69" (240x280 ST7789)
// ============================================================================
#include "luna_gfx_compat.h"
#include "luna_ui_elements.h"

extern TFT_eSPI tft;

// ── Available Screens ──
enum LunaScreen {
  SCREEN_HOME__GLANCE_,
  SCREEN_NOTIFICATIONS,
  SCREEN_CALENDAR,
  SCREEN_GAMES_LAUNCHER,
  SCREEN_FOCUS___POMODORO,
  SCREEN_CARDS___UTILITY,
  SCREEN_SETTINGS,
  SCREEN_ABOUT___DEVICE,
  SCREEN_COMPONENT_LAB
};

LunaScreen currentScreen = SCREEN_HOME__GLANCE_;
int currentScrollY = 0;
unsigned long alertDismissMs = 0;

// ── Draw Function: Home (Glance) ──
void drawScreen_HOME__GLANCE_() {
  tft.fillScreen(0x0841);

  // [monolith_time] Monolith Time Hero
  drawMonolithTime(10, 15 - currentScrollY, 220, 90, 10, 0x10A4, 0x2167, "10:42", ":38", "WED 09 SEP", 94, "LUNA - READY", 0xEF7E, 0x3DFF);
  // [glance_bar] NEXT FOCUS
  drawGlanceBar(10, 112 - currentScrollY, 220, 42, 8, 0x10A4, 0x2167, "NEXT FOCUS", "Sprint Review @ 11:00", 0xFCE7, 0xEF7E);
  // [tactile_button] START FOCUS SESSION
  drawTactileButton(10, 205 - currentScrollY, 220, 50, 12, 0xFCE7, 0xFD8C, "START FOCUS SESSION", 0x0841);
}

// ── Draw Function: Notifications ──
void drawScreen_NOTIFICATIONS() {
  tft.fillScreen(0x0841);

  // [notification_block] Sensor Ready Notice
  drawNotificationBlock(10, 15 - currentScrollY, 220, 72, 10, 0x10A4, 0x2167, "SARAH CONNOR", "4m ago", "Firmware calibration complete. Sensor ready.", 1, 0x3DFF, 0xEF7E, 0x8494);
  // [notification_block] BLE Heartbeat Notice
  drawNotificationBlock(10, 95 - currentScrollY, 220, 72, 10, 0x10A4, 0x2167, "LUNA BLE MESH", "18m ago", "Connected to iPhone 15 Pro - RSSI -58dBm.", 0, 0x15D0, 0xEF7E, 0x8494);
  // [notification_block] Battery System Notice
  drawNotificationBlock(10, 175 - currentScrollY, 220, 72, 10, 0x10A4, 0x2167, "POWER MANAGER", "1h ago", "Battery charged to 94% - Est. 18 hrs remaining.", 0, 0xFCE7, 0xEF7E, 0x8494);
  // [tactile_button] CLEAR ALL NOTIFICATIONS
  drawTactileButton(10, 255 - currentScrollY, 220, 44, 10, 0x1947, 0x320A, "CLEAR ALL NOTIFICATIONS", 0xEF7E);
  // Display Scrollbar Indicator
  const int maxScrollH_NOTIFICATIONS = 120;
  int thumbH = max(24, (280 * 280) / 400);
  int thumbY = (currentScrollY * (280 - thumbH)) / maxScrollH_NOTIFICATIONS;
  tft.drawFastVLine(238, 0, 280, 0x18C3);
  tft.fillRoundRect(236, thumbY, 3, thumbH, 1, 0x07FF);
}

// ── Draw Function: Calendar ──
void drawScreen_CALENDAR() {
  tft.fillScreen(0x0841);

  // [agenda_block] Next Agenda Item
  drawAgendaBlock(10, 15 - currentScrollY, 220, 84, 10, 0x10A4, 0x2167, "IN 24m", "10:30 - 11:15", "Architecture Sync", "Lab 4 / BLE Orbit", 0x3DFF, 0xEF7E, 0x8494);
  // [agenda_block] Afternoon Item
  drawAgendaBlock(10, 106 - currentScrollY, 220, 84, 10, 0x10A4, 0x2167, "AT 14:00", "14:00 - 15:30", "Hardware Touch Testing", "Bench COM3 (ESP32-S3)", 0x15D0, 0xEF7E, 0x8494);
  // [agenda_block] Evening Review
  drawAgendaBlock(10, 198 - currentScrollY, 220, 84, 10, 0x10A4, 0x2167, "AT 17:30", "17:30 - 18:00", "Sprint Review & Demo", "Luna Core Studio", 0xFCE7, 0xEF7E, 0x8494);
  // Display Scrollbar Indicator
  const int maxScrollH_CALENDAR = 100;
  int thumbH = max(24, (280 * 280) / 380);
  int thumbY = (currentScrollY * (280 - thumbH)) / maxScrollH_CALENDAR;
  tft.drawFastVLine(238, 0, 280, 0x18C3);
  tft.fillRoundRect(236, thumbY, 3, thumbH, 1, 0x07FF);
}

// ── Draw Function: Games Launcher ──
void drawScreen_GAMES_LAUNCHER() {
  tft.fillScreen(0x0841);

  // [game_launcher] Retro Runner Hero
  drawGameLauncher(10, 15 - currentScrollY, 220, 170, 12, 0x10A4, 0x2167, "RETRO RUNNER", "CYBERPLATFORM - 60FPS", "12,480 PTS", 0xFCE7, 0xEF7E);
  // [tactile_button] LAUNCH GAME ▶
  drawTactileButton(10, 195 - currentScrollY, 220, 50, 12, 0xFCE7, 0xFD8C, "LAUNCH GAME ▶", 0x0841);
}

// ── Draw Function: Focus / Pomodoro ──
void drawScreen_FOCUS___POMODORO() {
  tft.fillScreen(0x0841);

  // [focus_chamber] Focus Ambient Chamber
  drawFocusChamber(10, 15 - currentScrollY, 220, 175, 12, 0x10A4, 0x2167, "24:50", "DEEP WORK", "SESSION 2 / 4", 75, 0xFCE7, 0xEF7E, 0x8494);
  // [tactile_button] PAUSE FOCUS SESSION
  drawTactileButton(10, 202 - currentScrollY, 220, 48, 12, 0x1947, 0x320A, "PAUSE FOCUS SESSION", 0xEF7E);
}

// ── Draw Function: Cards & Utility ──
void drawScreen_CARDS___UTILITY() {
  tft.fillScreen(0x0841);

  // [qr_utility_card] Luna ID QR Card
  drawQRUtilityCard(10, 15 - currentScrollY, 220, 155, 12, 0x10A4, 0x2167, "LUNA ID CARD", "ESP32-S3 - BLE PEER", "UID: LN-8842-X", 0x3DFF, 0xEF7E);
  // [imu_level_card] IMU Spirit Level
  drawIMULevelCard(10, 178 - currentScrollY, 220, 140, 12, 0x10A4, 0x2167, "IMU SPIRIT LEVEL", "+2.4°", "-0.8°", "STABLE", 0x15D0, 0xEF7E, 0x8494);
  // Display Scrollbar Indicator
  const int maxScrollH_CARDS___UTILITY = 60;
  int thumbH = max(24, (280 * 280) / 340);
  int thumbY = (currentScrollY * (280 - thumbH)) / maxScrollH_CARDS___UTILITY;
  tft.drawFastVLine(238, 0, 280, 0x18C3);
  tft.fillRoundRect(236, thumbY, 3, thumbH, 1, 0x07FF);
}

// ── Draw Function: Settings ──
void drawScreen_SETTINGS() {
  tft.fillScreen(0x0841);

  // [setting_row] BRIGHTNESS
  drawSettingRow(10, 15 - currentScrollY, 220, 48, 8, 0x10A4, 0x2167, "BRIGHTNESS", "85%", "Auto-dim in 30s", 1, 0x3DFF, 0xEF7E, 0x8494);
  // [setting_row] BLUETOOTH LE
  drawSettingRow(10, 70 - currentScrollY, 220, 48, 8, 0x10A4, 0x2167, "BLUETOOTH LE", "ACTIVE", "Advertising as Luna_Core", 1, 0x15D0, 0xEF7E, 0x8494);
  // [setting_row] HAPTIC & AUDIO
  drawSettingRow(10, 125 - currentScrollY, 220, 48, 8, 0x10A4, 0x2167, "HAPTIC & AUDIO", "CHIRP", "Buzzer pin GPIO 42", 1, 0xFCE7, 0xEF7E, 0x8494);
  // [tactile_button] DEVICE DIAGNOSTICS ›
  drawTactileButton(10, 185 - currentScrollY, 220, 48, 10, 0x1947, 0x320A, "DEVICE DIAGNOSTICS ›", 0xEF7E);
  // Display Scrollbar Indicator
  const int maxScrollH_SETTINGS = 80;
  int thumbH = max(24, (280 * 280) / 360);
  int thumbY = (currentScrollY * (280 - thumbH)) / maxScrollH_SETTINGS;
  tft.drawFastVLine(238, 0, 280, 0x18C3);
  tft.fillRoundRect(236, thumbY, 3, thumbH, 1, 0x07FF);
}

// ── Draw Function: About & Device ──
void drawScreen_ABOUT___DEVICE() {
  tft.fillScreen(0x0841);

  // [device_spec_card] Hardware Specs Card
  drawDeviceSpecCard(10, 15 - currentScrollY, 220, 160, 12, 0x10A4, 0x2167, "LUNA CORE 1.69", "ESP32-S3 Dual 240MHz", "16MB Flash - 8MB PSRAM", "VBAT 4.12V (94%)", "CST816T OK - ST7789 80MHz", 0x15D0, 0xEF7E, 0x8494);
  // [tactile_button] OPEN COMPONENT LAB ›
  drawTactileButton(10, 185 - currentScrollY, 220, 48, 12, 0x1947, 0x320A, "OPEN COMPONENT LAB ›", 0xEF7E);
  // [tactile_button] RETURN TO HOME
  drawTactileButton(10, 242 - currentScrollY, 220, 48, 12, 0x3DFF, 0x7E9F, "RETURN TO HOME", 0x0841);
  // Display Scrollbar Indicator
  const int maxScrollH_ABOUT___DEVICE = 60;
  int thumbH = max(24, (280 * 280) / 340);
  int thumbY = (currentScrollY * (280 - thumbH)) / maxScrollH_ABOUT___DEVICE;
  tft.drawFastVLine(238, 0, 280, 0x18C3);
  tft.fillRoundRect(236, thumbY, 3, thumbH, 1, 0x07FF);
}

// ── Draw Function: Component Lab ──
void drawScreen_COMPONENT_LAB() {
  tft.fillScreen(0x0841);

  // [luna_header] Lab Header
  drawLunaHeader(0, 0 - currentScrollY, 240, 44, "COMPONENT LAB", 0x0841, 0x1906, 0xEF7E);
  // [luna_button] PRIMARY BUTTON
  drawLunaButton(10, 52 - currentScrollY, 220, 48, 12, 0xFCE7, 0xFD8C, "PRIMARY BUTTON", 0x0841);
  // [luna_button] SECONDARY OUTLINE
  drawLunaButton(10, 108 - currentScrollY, 220, 48, 12, 0x10A4, 0x2167, "SECONDARY OUTLINE", 0xEF7E);
  // [luna_button] DESTRUCTIVE SLAB
  drawLunaButton(10, 164 - currentScrollY, 220, 48, 12, 0x98C3, 0xD924, "DESTRUCTIVE SLAB", 0xFFFF);
  // [luna_toggle] TOUCH VIBRATION
  drawLunaToggle(10, 220 - currentScrollY, 220, 48, 10, 0x10A4, 0x2167, "TOUCH VIBRATION", 1, 0x3DFF, 0xEF7E);
  // [luna_slider] DISPLAY BRIGHTNESS
  drawLunaSlider(10, 276 - currentScrollY, 220, 64, 10, 0x10A4, 0x2167, "DISPLAY BRIGHTNESS", 75, 0xFCE7, 0xEF7E);
  // [luna_progress] Progress Gauge
  drawLunaProgress(10, 348 - currentScrollY, 220, 48, 10, 0x10A4, 0x2167, 68, 0x3DFF, 0xEF7E);
  // [luna_status] ONLINE
  drawLunaStatus(10, 404 - currentScrollY, 90, 26, 6, 0x10A4, 0x2167, "ONLINE", 0x15D0);
  // [luna_indicator] SYNCING
  drawLunaIndicator(110, 404 - currentScrollY, 100, 30, "SYNCING", 0xFCE7);
  // [luna_number] CPU FREQ
  drawLunaNumber(10, 442 - currentScrollY, 100, 44, "240", "MHz", "CPU FREQ", 0x3DFF, 0x8494);
  // [luna_navigation] Carousel Navigation Dots
  drawLunaNavigation(10, 494 - currentScrollY, 220, 36, 7, 8, 0x3DFF, 0x2167);
  // [luna_surface] Monolith Surface Container
  drawLunaSurface(10, 538 - currentScrollY, 220, 74, 10, 0x10A4, 0x2167, "TACTILE MONOLITH", "Elevation level 1 container", 0xEF7E, 0x8494);
  // [luna_button] RETURN TO HOME
  drawLunaButton(10, 622 - currentScrollY, 220, 48, 12, 0xFCE7, 0xFD8C, "RETURN TO HOME", 0x0841);
  // Display Scrollbar Indicator
  const int maxScrollH_COMPONENT_LAB = 480;
  int thumbH = max(24, (280 * 280) / 760);
  int thumbY = (currentScrollY * (280 - thumbH)) / maxScrollH_COMPONENT_LAB;
  tft.drawFastVLine(238, 0, 280, 0x18C3);
  tft.fillRoundRect(236, thumbY, 3, thumbH, 1, 0x07FF);
}

// ── Main Screen Refresh Dispatcher ──
void drawCurrentScreen() {
  Serial.printf("[Screen] Rendering Screen ID: %d\n", currentScreen);
  switch(currentScreen) {
    case SCREEN_HOME__GLANCE_: drawScreen_HOME__GLANCE_(); break;
    case SCREEN_NOTIFICATIONS: drawScreen_NOTIFICATIONS(); break;
    case SCREEN_CALENDAR: drawScreen_CALENDAR(); break;
    case SCREEN_GAMES_LAUNCHER: drawScreen_GAMES_LAUNCHER(); break;
    case SCREEN_FOCUS___POMODORO: drawScreen_FOCUS___POMODORO(); break;
    case SCREEN_CARDS___UTILITY: drawScreen_CARDS___UTILITY(); break;
    case SCREEN_SETTINGS: drawScreen_SETTINGS(); break;
    case SCREEN_ABOUT___DEVICE: drawScreen_ABOUT___DEVICE(); break;
    case SCREEN_COMPONENT_LAB: drawScreen_COMPONENT_LAB(); break;
  }
  tft.flush(); // Push complete frame buffer to display at 80MHz
}

// ── Touch & Button Interaction Handler ──
void handleScreenTouch(int touchX, int touchY) {
  if (currentScreen == SCREEN_HOME__GLANCE_) {
    // Check touch on "NEXT FOCUS" (10, 112, 220, 42) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (112 - currentScrollY) && touchY <= (112 - currentScrollY + 42)) {
      // Action: Navigate to "Calendar"
      currentScreen = SCREEN_CALENDAR;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
    // Check touch on "START FOCUS SESSION" (10, 205, 220, 50) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (205 - currentScrollY) && touchY <= (205 - currentScrollY + 50)) {
      // Action: Navigate to "Focus / Pomodoro"
      currentScreen = SCREEN_FOCUS___POMODORO;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
  }
  if (currentScreen == SCREEN_NOTIFICATIONS) {
    // Check touch on "Sensor Ready Notice" (10, 15, 220, 72) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (15 - currentScrollY) && touchY <= (15 - currentScrollY + 72)) {
      // Action: Show Alert Notification
      drawAlertBadge(20, 20, 200, 50, 8, "LUNA ALERT", "Notification 1 dismissed.", 0x18E3, 0x07E0, 0xFFFF);
      alertDismissMs = millis() + 1500;
      tft.flush();
      return;
    }
    // Check touch on "CLEAR ALL NOTIFICATIONS" (10, 255, 220, 44) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (255 - currentScrollY) && touchY <= (255 - currentScrollY + 44)) {
      // Action: Show Alert Notification
      drawAlertBadge(20, 20, 200, 50, 8, "LUNA ALERT", "All stream notifications cleared.", 0x18E3, 0x07E0, 0xFFFF);
      alertDismissMs = millis() + 1500;
      tft.flush();
      return;
    }
  }
  if (currentScreen == SCREEN_GAMES_LAUNCHER) {
    // Check touch on "Retro Runner Hero" (10, 15, 220, 170) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (15 - currentScrollY) && touchY <= (15 - currentScrollY + 170)) {
      // Action: Show Alert Notification
      drawAlertBadge(20, 20, 200, 50, 8, "LUNA ALERT", "🎮 Launching Retro Runner on hardware!", 0x18E3, 0x07E0, 0xFFFF);
      alertDismissMs = millis() + 1500;
      tft.flush();
      return;
    }
    // Check touch on "LAUNCH GAME ▶" (10, 195, 220, 50) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (195 - currentScrollY) && touchY <= (195 - currentScrollY + 50)) {
      // Action: Show Alert Notification
      drawAlertBadge(20, 20, 200, 50, 8, "LUNA ALERT", "🎮 Launching Retro Runner!", 0x18E3, 0x07E0, 0xFFFF);
      alertDismissMs = millis() + 1500;
      tft.flush();
      return;
    }
  }
  if (currentScreen == SCREEN_FOCUS___POMODORO) {
    // Check touch on "Focus Ambient Chamber" (10, 15, 220, 175) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (15 - currentScrollY) && touchY <= (15 - currentScrollY + 175)) {
      // Action: Show Alert Notification
      drawAlertBadge(20, 20, 200, 50, 8, "LUNA ALERT", "⏸ Focus timer paused.", 0x18E3, 0x07E0, 0xFFFF);
      alertDismissMs = millis() + 1500;
      tft.flush();
      return;
    }
    // Check touch on "PAUSE FOCUS SESSION" (10, 202, 220, 48) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (202 - currentScrollY) && touchY <= (202 - currentScrollY + 48)) {
      // Action: Show Alert Notification
      drawAlertBadge(20, 20, 200, 50, 8, "LUNA ALERT", "Focus session paused.", 0x18E3, 0x07E0, 0xFFFF);
      alertDismissMs = millis() + 1500;
      tft.flush();
      return;
    }
  }
  if (currentScreen == SCREEN_SETTINGS) {
    // Check touch on "BRIGHTNESS" (10, 15, 220, 48) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (15 - currentScrollY) && touchY <= (15 - currentScrollY + 48)) {
      // Action: Toggle Setting
      drawAlertBadge(20, 20, 200, 50, 8, "SETTING UPDATED", "Toggled brightness", 0x18E3, 0x10B981, 0xFFFF);
      alertDismissMs = millis() + 1500;
      tft.flush();
      return;
    }
    // Check touch on "BLUETOOTH LE" (10, 70, 220, 48) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (70 - currentScrollY) && touchY <= (70 - currentScrollY + 48)) {
      // Action: Toggle Setting
      drawAlertBadge(20, 20, 200, 50, 8, "SETTING UPDATED", "Toggled BLE", 0x18E3, 0x10B981, 0xFFFF);
      alertDismissMs = millis() + 1500;
      tft.flush();
      return;
    }
    // Check touch on "HAPTIC & AUDIO" (10, 125, 220, 48) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (125 - currentScrollY) && touchY <= (125 - currentScrollY + 48)) {
      // Action: Toggle Setting
      drawAlertBadge(20, 20, 200, 50, 8, "SETTING UPDATED", "Toggled audio", 0x18E3, 0x10B981, 0xFFFF);
      alertDismissMs = millis() + 1500;
      tft.flush();
      return;
    }
    // Check touch on "DEVICE DIAGNOSTICS ›" (10, 185, 220, 48) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (185 - currentScrollY) && touchY <= (185 - currentScrollY + 48)) {
      // Action: Navigate to "About & Device"
      currentScreen = SCREEN_ABOUT___DEVICE;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
  }
  if (currentScreen == SCREEN_ABOUT___DEVICE) {
    // Check touch on "OPEN COMPONENT LAB ›" (10, 185, 220, 48) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (185 - currentScrollY) && touchY <= (185 - currentScrollY + 48)) {
      // Action: Navigate to "Component Lab"
      currentScreen = SCREEN_COMPONENT_LAB;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
    // Check touch on "RETURN TO HOME" (10, 242, 220, 48) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (242 - currentScrollY) && touchY <= (242 - currentScrollY + 48)) {
      // Action: Navigate to "Home (Glance)"
      currentScreen = SCREEN_HOME__GLANCE_;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
  }
  if (currentScreen == SCREEN_COMPONENT_LAB) {
    // Check touch on "PRIMARY BUTTON" (10, 52, 220, 48) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (52 - currentScrollY) && touchY <= (52 - currentScrollY + 48)) {
      // Action: Show Alert Notification
      drawAlertBadge(20, 20, 200, 50, 8, "LUNA ALERT", "Primary button tapped", 0x18E3, 0x07E0, 0xFFFF);
      alertDismissMs = millis() + 1500;
      tft.flush();
      return;
    }
    // Check touch on "SECONDARY OUTLINE" (10, 108, 220, 48) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (108 - currentScrollY) && touchY <= (108 - currentScrollY + 48)) {
      // Action: Show Alert Notification
      drawAlertBadge(20, 20, 200, 50, 8, "LUNA ALERT", "Secondary button tapped", 0x18E3, 0x07E0, 0xFFFF);
      alertDismissMs = millis() + 1500;
      tft.flush();
      return;
    }
    // Check touch on "DESTRUCTIVE SLAB" (10, 164, 220, 48) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (164 - currentScrollY) && touchY <= (164 - currentScrollY + 48)) {
      // Action: Show Alert Notification
      drawAlertBadge(20, 20, 200, 50, 8, "LUNA ALERT", "Destructive action tapped", 0x18E3, 0x07E0, 0xFFFF);
      alertDismissMs = millis() + 1500;
      tft.flush();
      return;
    }
    // Check touch on "TOUCH VIBRATION" (10, 220, 220, 48) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (220 - currentScrollY) && touchY <= (220 - currentScrollY + 48)) {
      // Action: Toggle Setting
      drawAlertBadge(20, 20, 200, 50, 8, "SETTING UPDATED", "Toggle changed", 0x18E3, 0x10B981, 0xFFFF);
      alertDismissMs = millis() + 1500;
      tft.flush();
      return;
    }
    // Check touch on "RETURN TO HOME" (10, 622, 220, 48) with scroll offset
    if (touchX >= 10 && touchX <= 230 && touchY >= (622 - currentScrollY) && touchY <= (622 - currentScrollY + 48)) {
      // Action: Navigate to "Home (Glance)"
      currentScreen = SCREEN_HOME__GLANCE_;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
  }
}

// ── Screen-Level Gesture Swipe Handler (Left / Right) ──
void handleScreenSwipe(uint8_t direction) {
  // direction: 1 = Swipe Left, 2 = Swipe Right
  if (currentScreen == SCREEN_HOME__GLANCE_) {
    if (direction == 1) { // Swipe Left -> "Notifications"
      currentScreen = SCREEN_NOTIFICATIONS;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
    if (direction == 2) { // Swipe Right -> "About & Device"
      currentScreen = SCREEN_ABOUT___DEVICE;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
  }
  if (currentScreen == SCREEN_NOTIFICATIONS) {
    if (direction == 1) { // Swipe Left -> "Calendar"
      currentScreen = SCREEN_CALENDAR;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
    if (direction == 2) { // Swipe Right -> "Home (Glance)"
      currentScreen = SCREEN_HOME__GLANCE_;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
  }
  if (currentScreen == SCREEN_CALENDAR) {
    if (direction == 1) { // Swipe Left -> "Games Launcher"
      currentScreen = SCREEN_GAMES_LAUNCHER;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
    if (direction == 2) { // Swipe Right -> "Notifications"
      currentScreen = SCREEN_NOTIFICATIONS;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
  }
  if (currentScreen == SCREEN_GAMES_LAUNCHER) {
    if (direction == 1) { // Swipe Left -> "Focus / Pomodoro"
      currentScreen = SCREEN_FOCUS___POMODORO;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
    if (direction == 2) { // Swipe Right -> "Calendar"
      currentScreen = SCREEN_CALENDAR;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
  }
  if (currentScreen == SCREEN_FOCUS___POMODORO) {
    if (direction == 1) { // Swipe Left -> "Cards & Utility"
      currentScreen = SCREEN_CARDS___UTILITY;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
    if (direction == 2) { // Swipe Right -> "Games Launcher"
      currentScreen = SCREEN_GAMES_LAUNCHER;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
  }
  if (currentScreen == SCREEN_CARDS___UTILITY) {
    if (direction == 1) { // Swipe Left -> "Settings"
      currentScreen = SCREEN_SETTINGS;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
    if (direction == 2) { // Swipe Right -> "Focus / Pomodoro"
      currentScreen = SCREEN_FOCUS___POMODORO;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
  }
  if (currentScreen == SCREEN_SETTINGS) {
    if (direction == 1) { // Swipe Left -> "About & Device"
      currentScreen = SCREEN_ABOUT___DEVICE;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
    if (direction == 2) { // Swipe Right -> "Cards & Utility"
      currentScreen = SCREEN_CARDS___UTILITY;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
  }
  if (currentScreen == SCREEN_ABOUT___DEVICE) {
    if (direction == 1) { // Swipe Left -> "Home (Glance)"
      currentScreen = SCREEN_HOME__GLANCE_;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
    if (direction == 2) { // Swipe Right -> "Settings"
      currentScreen = SCREEN_SETTINGS;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
  }
  if (currentScreen == SCREEN_COMPONENT_LAB) {
    if (direction == 1) { // Swipe Left -> "Home (Glance)"
      currentScreen = SCREEN_HOME__GLANCE_;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
    if (direction == 2) { // Swipe Right -> "About & Device"
      currentScreen = SCREEN_ABOUT___DEVICE;
      currentScrollY = 0;
      drawCurrentScreen();
      return;
    }
  }
}

// ── Full-Display Touch Drag Vertical Scrolling Handler ──
void handleScreenScroll(int deltaY) {
  if (currentScreen == SCREEN_NOTIFICATIONS) {
    int newScroll = constrain(currentScrollY + deltaY, 0, 120);
    if (newScroll != currentScrollY) {
      currentScrollY = newScroll;
      drawCurrentScreen();
    }
    return;
  }
  if (currentScreen == SCREEN_CALENDAR) {
    int newScroll = constrain(currentScrollY + deltaY, 0, 100);
    if (newScroll != currentScrollY) {
      currentScrollY = newScroll;
      drawCurrentScreen();
    }
    return;
  }
  if (currentScreen == SCREEN_CARDS___UTILITY) {
    int newScroll = constrain(currentScrollY + deltaY, 0, 60);
    if (newScroll != currentScrollY) {
      currentScrollY = newScroll;
      drawCurrentScreen();
    }
    return;
  }
  if (currentScreen == SCREEN_SETTINGS) {
    int newScroll = constrain(currentScrollY + deltaY, 0, 80);
    if (newScroll != currentScrollY) {
      currentScrollY = newScroll;
      drawCurrentScreen();
    }
    return;
  }
  if (currentScreen == SCREEN_ABOUT___DEVICE) {
    int newScroll = constrain(currentScrollY + deltaY, 0, 60);
    if (newScroll != currentScrollY) {
      currentScrollY = newScroll;
      drawCurrentScreen();
    }
    return;
  }
  if (currentScreen == SCREEN_COMPONENT_LAB) {
    int newScroll = constrain(currentScrollY + deltaY, 0, 480);
    if (newScroll != currentScrollY) {
      currentScrollY = newScroll;
      drawCurrentScreen();
    }
    return;
  }
}
