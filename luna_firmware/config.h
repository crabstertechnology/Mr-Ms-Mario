#ifndef CONFIG_H
#define CONFIG_H

#define FIRMWARE_VERSION "1.0.1"

// Pin Configurations for ESP32-C3 SuperMini with 1.3" IPS ST7789 TFT SPI Display
#define TFT_SCL 4     // Hardware SPI SCLK
#define TFT_SDA 6     // Hardware SPI MOSI (SDA)
#define TFT_RST 1    // Reset
#define TFT_DC 3      // Data/Command
#define TFT_BLK 7     // Backlight control

// --- Two Push Buttons Mappings ---
// Note: GPIO 8 and 9 are strapping pins on ESP32-C3.
// Since we configure them with internal pull-ups, they remain HIGH during reset (normal boot).
// Just avoid holding Button 2 (GPIO 9) down while plugging in USB or pressing Reset,
// otherwise the ESP32-C3 will enter serial bootloader/flasher mode instead of booting.
#define BTN_EXPR_PIN 8      // Button 1: Next Expression / Menu Navigate
#define BTN_SETTINGS_PIN 9  // Button 2: Enter Settings / Confirm Option

#define BUZZER_PIN 5        // Buzzer pin


// ST7789 Display Settings (Resolution: 240x240)
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define TFT_CS -1     // Chip Select pin (CS is tied to ground on display)

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
