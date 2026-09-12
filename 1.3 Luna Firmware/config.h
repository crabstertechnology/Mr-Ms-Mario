#ifndef CONFIG_H
#define CONFIG_H

#define FIRMWARE_VERSION "2.0.0"

// Pin Configurations for ESP32-C3 SuperMini with 1.3" IPS ST7789 TFT SPI Display
#define TFT_SCL 4     // Hardware SPI SCLK
#define TFT_SDA 6     // Hardware SPI MOSI (SDA)
#define TFT_RST 1     // Reset
#define TFT_DC 3      // Data/Command
#define TFT_BLK 7     // Backlight control
#define TFT_CS -1     // Chip Select pin (CS is tied to ground on display)

// --- Two Push Buttons Mappings ---
#define BTN_EXPR_PIN 8      // Button 1: Next Expression / Menu Navigate
#define BTN_SETTINGS_PIN 9  // Button 2: Enter Settings / Confirm Option

#define BUZZER_PIN 5        // Buzzer pin
#define BATTERY_PIN 0       // GPIO 0 (ADC1_CH0) for battery monitoring

// Battery monitoring voltage divider calibration
// For standard 10k/10k Ohm divider, use 2.0f.
// For high-impedance 1M/1M Ohm divider, the ADC sampling capacitor pulls down the voltage,
// so a higher multiplier (typically between 2.2f and 2.4f) is needed to calibrate.
#define BATTERY_CALIBRATION_MULTIPLIER 2.3f

// ST7789 Display Settings (Resolution: 240x240)
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

// BLE Service & Characteristic UUIDs
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define EXPRESSION_CHAR_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define AUDIO_CHAR_UUID        "d90e0c03-51ee-4c31-893c-cf572db85700"
#define TEXT_CHAR_UUID         "c8a00d04-62ff-4b32-843d-0f1c6db8a101"
#define STATUS_CHAR_UUID       "fb2f0e05-73ee-4f32-833d-1f2c6db8a102"
#define AUDIO_STREAM_CHAR_UUID "a823e50b-71ee-48c5-9276-2e8c6db8a103"

// Robot Expression States (12 Full-Color Sprite AI animations)
enum Expression {
  EXPR_IDLE = 0,
  EXPR_HAPPY,
  EXPR_SAD,
  EXPR_ANGRY,
  EXPR_SURPRISED,
  EXPR_SLEEPING,
  EXPR_WINK,
  EXPR_EXCITED,
  EXPR_LOVE,
  EXPR_SCARED,
  EXPR_LAUGH,
  EXPR_PEACE,
  EXPR_TEXT,
  EXPR_CLOCK,
  EXPR_MAP,
  EXPR_ALL_GIF
};

// Buzzer Sound Effects
enum SoundEffect {
  SOUND_NONE = 0,
  SOUND_JUMP,
  SOUND_COIN,
  SOUND_POWERUP,
  SOUND_POWERDOWN,
  SOUND_GAMEOVER,
  SOUND_CHIRP,
  SOUND_STARTUP,
  SOUND_CASTLE,
  SOUND_UNDERWORLD,
  SOUND_THEMECHANGE
};

// Smartwatch UI Screen Modes
enum SmartwatchScreen {
  SCREEN_CLOCK = 0,
  SCREEN_NOTIFICATIONS,
  SCREEN_CALENDAR,
  SCREEN_GAMES,
  SCREEN_FACE,
  SCREEN_MAPS,
  SCREEN_CARD,   // Digital Business Card / QR Code screen
  SCREEN_SETTINGS,
  SCREEN_MAX
};

#endif // CONFIG_H
