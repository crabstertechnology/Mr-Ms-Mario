#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <TJpg_Decoder.h>
#include "video_frames.h"

// Pin Definitions for ESP32-C3 SuperMini + 1.3" IPS ST7789 (240x240)
#define TFT_SCL 4     // SPI Clock (SCK)
#define TFT_SDA 6     // SPI MOSI (SDA)
#define TFT_RST 1     // Reset Pin
#define TFT_DC  3     // Data / Command Pin
#define TFT_BLK 7     // Backlight Control Pin
#define TFT_CS -1     // Chip Select (Tied to GND on board)

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240

// Initialize Display object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Fast block render callback for TJpg_Decoder
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  // Clip out-of-bounds rendering
  if (y >= tft.height()) return 0;
  tft.drawRGBBitmap(x, y, bitmap, w, h);
  return 1;
}

int currentFrame = 0;
unsigned long lastFrameTime = 0;
int playbackSpeedMs = VIDEO_FRAME_DELAY_MS; // ~41ms for 24 FPS
bool isPlaying = true;
int currentRotation = 2; // Default rotation 2 for correct right-side-up orientation
bool displayInvert = true; // IPS ST7789 requires true for standard color representation

void setup() {
  Serial.begin(115200);
  delay(100);

  // Turn ON Backlight
  pinMode(TFT_BLK, OUTPUT);
  digitalWrite(TFT_BLK, HIGH);

  // Initialize Custom SPI Pins for ESP32-C3
  SPI.begin(TFT_SCL, -1, TFT_SDA, TFT_CS);

  // Initialize ST7789 Display (240x240, SPI Mode 3)
  tft.init(SCREEN_WIDTH, SCREEN_HEIGHT, SPI_MODE3);
  tft.setSPISpeed(40000000); // 40 MHz high-speed SPI
  tft.setRotation(currentRotation); // Rotation 2 (Right-side up)
  tft.invertDisplay(displayInvert); // Fixes color inversion / distortion for IPS panel
  tft.fillScreen(ST77XX_BLACK);

  // Initialize TJpg_Decoder (swapBytes false for Adafruit_GFX drawRGBBitmap)
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(false);
  TJpgDec.setCallback(tft_output);

  Serial.println("=========================================");
  Serial.println("  ESP32-C3 Video Player - Corrected Color & Rotation ");
  Serial.printf("  Total Frames: %d @ %d FPS\n", TOTAL_VIDEO_FRAMES, VIDEO_FPS);
  Serial.println("=========================================");
}

void loop() {
  // Check for Serial Commands for real-time calibration/controls
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'p' || cmd == ' ') {
      isPlaying = !isPlaying;
      Serial.printf("Playback: %s\n", isPlaying ? "PLAYING" : "PAUSED");
    } else if (cmd == '+') {
      if (playbackSpeedMs > 10) playbackSpeedMs -= 5;
      Serial.printf("Speed Delay: %d ms\n", playbackSpeedMs);
    } else if (cmd == '-') {
      playbackSpeedMs += 5;
      Serial.printf("Speed Delay: %d ms\n", playbackSpeedMs);
    } else if (cmd == 'r') {
      currentFrame = 0;
      Serial.println("Reset to Frame 0");
    } else if (cmd == 'o') {
      // Toggle rotation between 0, 1, 2, 3
      currentRotation = (currentRotation + 1) % 4;
      tft.setRotation(currentRotation);
      Serial.printf("Display Rotation set to: %d\n", currentRotation);
    } else if (cmd == 'i') {
      // Toggle display inversion
      displayInvert = !displayInvert;
      tft.invertDisplay(displayInvert);
      Serial.printf("Display Invert set to: %s\n", displayInvert ? "TRUE" : "FALSE");
    }
  }

  if (isPlaying) {
    unsigned long now = millis();
    if (now - lastFrameTime >= (unsigned long)playbackSpeedMs) {
      lastFrameTime = now;

      // Draw current frame from Flash (PROGMEM)
      const uint8_t* frameData = video_frames[currentFrame];
      uint32_t frameSize = video_frame_sizes[currentFrame];

      TJpgDec.drawJpg(0, 0, frameData, frameSize);

      // Advance frame index in loop
      currentFrame = (currentFrame + 1) % TOTAL_VIDEO_FRAMES;
    }
  } else {
    delay(10);
  }
}
