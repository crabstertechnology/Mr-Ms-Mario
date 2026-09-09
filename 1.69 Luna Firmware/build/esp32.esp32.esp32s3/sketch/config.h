#line 1 "W:\\Mr.mario\\1.69 Luna Firmware\\config.h"
#ifndef CONFIG_H
#define CONFIG_H

#define FIRMWARE_VERSION "2.0.0"

// Pin Configurations for Waveshare ESP32-S3-Touch-LCD-1.69
#define TFT_SCL 6      // SPI SCK (SCLK)
#define TFT_SDA 7      // SPI MOSI (SDA)
#define TFT_RST 8      // LCD reset
#define TFT_DC  4      // LCD Data/Command
#define TFT_BLK 15     // LCD Backlight
#define TFT_CS  5      // LCD Chip Select

// I2C Capacitive Touch CST816T Pins
#define TOUCH_SDA 11
#define TOUCH_SCL 10
#define TOUCH_INT 14
#define TOUCH_RST 13

// Buzzer & Battery Pins
#define BUZZER_PIN 42       // Buzzer pin
#define BATTERY_PIN 1       // GPIO 1 for battery monitoring

// Battery monitoring voltage divider calibration
// Waveshare schematic indicates: VBAT = VADC * 3 (so multiplier is 3.0f)
#define BATTERY_CALIBRATION_MULTIPLIER 3.0f

// ST7789 1.69" TFT Display Settings (Resolution: 240x280)
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 280

// Touch Column Boundaries (0 to 240 pixels width)
#define TOUCH_LEFT_LIMIT 80
#define TOUCH_RIGHT_LIMIT 140

// BLE Service & Characteristic UUIDs
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define EXPRESSION_CHAR_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define AUDIO_CHAR_UUID        "d90e0c03-51ee-4c31-893c-cf572db85700"
#define TEXT_CHAR_UUID         "c8a00d04-62ff-4b32-843d-0f1c6db8a101"
#define STATUS_CHAR_UUID       "fb2f0e05-73ee-4f32-833d-1f2c6db8a102"
#define AUDIO_STREAM_CHAR_UUID "a823e50b-71ee-48c5-9276-2e8c6db8a103"
#define IMAGE_CHAR_UUID        "e1234501-1fb5-459e-8fcc-c5c9c331914b"

// Maximum wallpaper image size in bytes
#define IMG_MAX_BYTES (100 * 1024)

// Robot Expression States
enum Expression {
  EXPR_IDLE = 0,
  EXPR_HAPPY,
  EXPR_SAD,
  EXPR_ANGRY,
  EXPR_SURPRISED,
  EXPR_SLEEPING,
  EXPR_WINK,
  EXPR_TEXT,
  EXPR_CLOCK,
  EXPR_MAP,
  EXPR_ROBOT_EYE = 20  // Sprite AI Robot Eye Animation System
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
  SCREEN_LEVEL,  // Accelerometer & Gyroscope Level Analyzer
  SCREEN_POMODORO,// Pomodoro Focus Timer
  SCREEN_WALLPAPER, // Custom JPEG wallpaper (sent from phone)
  SCREEN_MAX
};
#endif // CONFIG_H

