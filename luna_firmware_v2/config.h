#ifndef CONFIG_H
#define CONFIG_H

#define FIRMWARE_VERSION "2.0.0"

// Pin Configurations for ESP32-S3 Mini
#define SDA_PIN 17
#define SCL_PIN 18
#define TOUCH_PIN 1

// I2S Pins (MAX98357 DAC & INMP441 Microphone sharing clocks)
#define I2S_BCLK 4
#define I2S_WS   5
#define I2S_DOUT 6 // Audio out to MAX98357 DIN
#define I2S_DIN  7 // Audio in from INMP441 SD

// ST7789 1.3" TFT Display Settings (Resolution: 240x240)
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

// BLE Service & Characteristic UUIDs
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define EXPRESSION_CHAR_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define AUDIO_CHAR_UUID        "d90e0c03-51ee-4c31-893c-cf572db85700"
#define TEXT_CHAR_UUID         "c8a00d04-62ff-4b32-843d-0f1c6db8a101"
#define STATUS_CHAR_UUID       "fb2f0e05-73ee-4f32-833d-1f2c6db8a102"
#define AUDIO_STREAM_CHAR_UUID "a823e50b-71ee-48c5-9276-2e8c6db8a103"

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
  EXPR_ALL_GIF = 10  // Auto-cycle from master ALL_GIFS_TABLE (all 63 animations)
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
  SCREEN_SETTINGS,
  SCREEN_FACE,
  SCREEN_MAPS,
  SCREEN_MAX
};

#endif // CONFIG_H
