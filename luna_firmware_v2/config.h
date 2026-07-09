#ifndef CONFIG_H
#define CONFIG_H

#define FIRMWARE_VERSION "2.0.0"

// Pin Configurations for ESP32-S3 Mini
#define SDA_PIN 17
#define SCL_PIN 18
#define TOUCH_PIN 1
#define BUZZER_PIN 2

// ST7735 1.8" TFT Display Settings (Resolution: 128x160)
// Note: SPI connections (MOSI, SCLK, CS, DC, RST) are configured in TFT_eSPI's User_Setup.h
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160

// BLE Service & Characteristic UUIDs
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define EXPRESSION_CHAR_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define AUDIO_CHAR_UUID        "d90e0c03-51ee-4c31-893c-cf572db85700"
#define TEXT_CHAR_UUID         "c8a00d04-62ff-4b32-843d-0f1c6db8a101"
#define STATUS_CHAR_UUID       "fb2f0e05-73ee-4f32-833d-1f2c6db8a102"

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

#endif // CONFIG_H
