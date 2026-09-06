// image_transfer.h — Luna Display BLE Wallpaper Transfer
// Receives JPEG image chunks over BLE, saves to LittleFS, and renders on ST7789.
// Protocol: IMG_START:<size>:<crc32hex> | raw binary chunks | IMG_END
#ifndef IMAGE_TRANSFER_H
#define IMAGE_TRANSFER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <Adafruit_ST7789.h>
#include "config.h"

// ───────────────────────────────────────────────────────────────────────────
// TJpgDec — lightweight JPEG decoder bundled with ESP32 Arduino core
// ───────────────────────────────────────────────────────────────────────────
#include <TJpg_Decoder.h>

// Forward-declared globals from the main sketch
extern Adafruit_ST7789 tft;

// ───────────────────────────────────────────────────────────────────────────
// CRC-32 (IEEE 802.3 / zlib-compatible) helper
// ───────────────────────────────────────────────────────────────────────────
static uint32_t _crc32Table[256];
static bool     _crc32TableReady = false;

static void _buildCrc32Table() {
  for (uint32_t i = 0; i < 256; i++) {
    uint32_t c = i;
    for (int j = 0; j < 8; j++) {
      c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
    }
    _crc32Table[i] = c;
  }
  _crc32TableReady = true;
}

static uint32_t crc32Update(uint32_t crc, const uint8_t* data, size_t len) {
  if (!_crc32TableReady) _buildCrc32Table();
  crc = ~crc;
  for (size_t i = 0; i < len; i++) {
    crc = _crc32Table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
  }
  return ~crc;
}

// ───────────────────────────────────────────────────────────────────────────
// File paths
// ───────────────────────────────────────────────────────────────────────────
#define WALLPAPER_TMP_PATH  "/wallpaper_tmp.jpg"
#define WALLPAPER_PATH      "/wallpaper.jpg"

// ───────────────────────────────────────────────────────────────────────────
// TJpgDec pixel output callback — blits a decoded MCU block to TFT, Cache, or Canvas
// ───────────────────────────────────────────────────────────────────────────
static GFXcanvas16* _targetCanvas = nullptr;
static uint16_t*    _targetCache  = nullptr;

static bool _tftOutputCallback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (_targetCache != nullptr) {
    for (int j = 0; j < h; j++) {
      int16_t cy = y + j;
      if (cy < 0 || cy >= SCREEN_HEIGHT) continue;
      for (int i = 0; i < w; i++) {
        int16_t cx = x + i;
        if (cx < 0 || cx >= SCREEN_WIDTH) continue;
        _targetCache[cy * SCREEN_WIDTH + cx] = bitmap[j * w + i];
      }
    }
  } else if (_targetCanvas != nullptr) {
    _targetCanvas->drawRGBBitmap(x, y, bitmap, w, h);
  } else {
    // Apply 20-px vertical offset for Waveshare 1.69 ST7789 display window
    tft.drawRGBBitmap(x, y + 20, bitmap, w, h);
  }
  return true;
}

// ───────────────────────────────────────────────────────────────────────────
// LunaImageTransfer — state machine
// ───────────────────────────────────────────────────────────────────────────
class LunaImageTransfer {
public:
  enum State {
    IDLE,
    RECEIVING,
    VERIFYING,
    DECODING,
    DONE_OK,
    DONE_FAIL
  };

private:
  State    _state         = IDLE;
  uint32_t _expectedSize  = 0;
  uint32_t _expectedCrc   = 0;
  uint32_t _receivedBytes = 0;
  uint32_t _runningCrc    = 0;   // CRC of received bytes so far
  File     _file;
  bool     _fsReady       = false;
  bool     _wallpaperExists = false;

  // RAM Wallpaper Cache (134.4 KB) for 60+ FPS zero-flicker clock screen rendering
  uint16_t* _wallpaperCache = nullptr;
  bool      _cacheValid     = false;

  void _initCache() {
    if (_wallpaperCache != nullptr) return;
    size_t sz = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t);
#if defined(BOARD_HAS_PSRAM) || defined(CONFIG_SPIRAM_SUPPORT)
    if (psramFound()) {
      _wallpaperCache = (uint16_t*)ps_malloc(sz);
    }
#endif
    if (!_wallpaperCache) {
      _wallpaperCache = (uint16_t*)malloc(sz);
    }
    if (_wallpaperCache) {
      Serial.println("[IMG] Allocated RAM wallpaper cache successfully.");
    }
  }

  // ── Progress broadcast (polled by main loop) ──────────────────────────
  float    _progress = 0.0f;  // 0.0 – 1.0

  void _setFail(const char* reason) {
    Serial.printf("[IMG] FAIL: %s\n", reason);
    if (_file) { _file.close(); }
    LittleFS.remove(WALLPAPER_TMP_PATH);
    _state = DONE_FAIL;
    _progress = 0.0f;
  }

  bool _initFS() {
    if (_fsReady) return true;
    if (!LittleFS.begin(true)) {
      Serial.println("[IMG] LittleFS mount failed");
      return false;
    }
    _fsReady = true;
    _wallpaperExists = LittleFS.exists(WALLPAPER_PATH);
    Serial.printf("[IMG] LittleFS ready. Wallpaper exists: %d\n", (int)_wallpaperExists);
    return true;
  }

public:

  // Call once in setup() to initialise LittleFS and check for existing wallpaper
  void begin() {
    _buildCrc32Table();
    if (_initFS()) {
      if (_wallpaperExists) {
        reloadCache();
      }
    }
  }

  void reloadCache() {
    if (!_initFS()) return;
    if (!_wallpaperExists || !LittleFS.exists(WALLPAPER_PATH)) {
      _cacheValid = false;
      return;
    }
    _initCache();
    if (_wallpaperCache == nullptr) return;

    _targetCache = _wallpaperCache;
    _targetCanvas = nullptr;
    _decodeAndDisplay();
    _targetCache = nullptr;
    _cacheValid = true;
    Serial.println("[IMG] Wallpaper cache reloaded into RAM successfully.");
  }

  // Called when TEXT char receives "IMG_START:<size>:<crc32hex>"
  bool startTransfer(uint32_t size, uint32_t crc) {
    if (!_initFS()) return false;
    if (size == 0 || size > IMG_MAX_BYTES) {
      Serial.printf("[IMG] Invalid size %u (max %d)\n", size, IMG_MAX_BYTES);
      return false;
    }

    // Clean up any previous temp file
    if (LittleFS.exists(WALLPAPER_TMP_PATH)) {
      LittleFS.remove(WALLPAPER_TMP_PATH);
    }

    _file = LittleFS.open(WALLPAPER_TMP_PATH, "w");
    if (!_file) {
      Serial.println("[IMG] Failed to open temp file for writing");
      return false;
    }

    _expectedSize  = size;
    _expectedCrc   = crc;
    _receivedBytes = 0;
    _runningCrc    = crc32Update(0, nullptr, 0);  // init = 0x00000000
    _runningCrc    = 0;
    _state         = RECEIVING;
    _progress      = 0.0f;
    Serial.printf("[IMG] Transfer started. Expected %u bytes, CRC=0x%08X\n", size, crc);
    return true;
  }

  // Called by BLE ImageCallbacks with each incoming chunk
  void onChunk(uint8_t* data, size_t len) {
    if (_state != RECEIVING) {
      Serial.println("[IMG] Chunk received but not in RECEIVING state — ignored");
      return;
    }
    if (_receivedBytes + len > _expectedSize) {
      _setFail("overflow — more data than declared size");
      return;
    }
    _file.write(data, len);
    _runningCrc    = crc32Update(_runningCrc, data, len);
    _receivedBytes += len;
    _progress       = (float)_receivedBytes / (float)_expectedSize;
  }

  // Called when TEXT char receives "IMG_END"
  bool finishTransfer(String& errorOut) {
    if (_state != RECEIVING) {
      errorOut = "not_receiving";
      return false;
    }

    _file.flush();
    _file.close();
    _state = VERIFYING;

    // 1. Size check
    if (_receivedBytes != _expectedSize) {
      char buf[64];
      snprintf(buf, sizeof(buf), "size_mismatch:%u!=%u", _receivedBytes, _expectedSize);
      errorOut = buf;
      _setFail(buf);
      return false;
    }

    // 2. CRC check
    if (_runningCrc != _expectedCrc) {
      char buf[64];
      snprintf(buf, sizeof(buf), "crc_mismatch:got=0x%08X exp=0x%08X", _runningCrc, _expectedCrc);
      errorOut = buf;
      _setFail(buf);
      return false;
    }

    // 3. Promote temp → permanent
    if (LittleFS.exists(WALLPAPER_PATH)) {
      LittleFS.remove(WALLPAPER_PATH);
    }
    if (!LittleFS.rename(WALLPAPER_TMP_PATH, WALLPAPER_PATH)) {
      errorOut = "rename_failed";
      _setFail("rename failed");
      return false;
    }

    _wallpaperExists = true;
    _cacheValid = false;
    _state = DECODING;
    _progress = 1.0f;
    Serial.println("[IMG] CRC OK — decoding and caching wallpaper...");

    reloadCache();

    _state = DONE_OK;
    errorOut = "";
    return true;
  }

  // Cancel an in-progress transfer
  void cancelTransfer() {
    if (_file) _file.close();
    LittleFS.remove(WALLPAPER_TMP_PATH);
    _state = IDLE;
    _progress = 0.0f;
    Serial.println("[IMG] Transfer cancelled.");
  }

  // Delete saved wallpaper and revert to face screen
  bool deleteWallpaper() {
    _wallpaperExists = false;
    _cacheValid = false;
    if (LittleFS.exists(WALLPAPER_PATH)) {
      LittleFS.remove(WALLPAPER_PATH);
      Serial.println("[IMG] Wallpaper deleted.");
      return true;
    }
    return false;
  }

  // Draw the saved wallpaper on boot (or on demand)
  bool drawWallpaper() {
    if (!_initFS()) return false;
    if (!LittleFS.exists(WALLPAPER_PATH)) {
      Serial.println("[IMG] No wallpaper saved.");
      return false;
    }
    _targetCanvas = nullptr;
    _targetCache  = nullptr;
    return _decodeAndDisplay();
  }

  // Draw saved wallpaper directly into GFXcanvas16 buffer using fast RAM memcpy
  bool drawWallpaperToCanvas(GFXcanvas16& canvas) {
    if (!_initFS()) return false;
    if (!_wallpaperExists) return false;

    if (!_cacheValid || _wallpaperCache == nullptr) {
      reloadCache();
    }

    if (_wallpaperCache != nullptr && _cacheValid) {
      memcpy(canvas.getBuffer(), _wallpaperCache, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));
      return true;
    }

    _targetCanvas = &canvas;
    _targetCache  = nullptr;
    bool ok = _decodeAndDisplay();
    _targetCanvas = nullptr;
    return ok;
  }

  bool hasWallpaper() {
    return _wallpaperExists;
  }

  float progress() const { return _progress; }
  State state()   const { return _state; }

  void resetState() {
    _state = IDLE;
    _progress = 0.0f;
  }

private:
  bool _decodeAndDisplay() {
    // Configure TJpgDec
    TJpgDec.setJpgScale(1);                          // 1:1 scale
    TJpgDec.setSwapBytes(false);                      // False: Adafruit_GFX drawRGBBitmap expects native RGB565
    TJpgDec.setCallback(_tftOutputCallback);

    // Clear hardware screen ONLY when rendering directly to physical TFT hardware
    if (_targetCanvas == nullptr && _targetCache == nullptr) {
      tft.fillScreen(ST77XX_BLACK);
    }

    // Decode directly from LittleFS file
    uint16_t imgW = 0, imgH = 0;
    JRESULT res = TJpgDec.getFsJpgSize(&imgW, &imgH, WALLPAPER_PATH, LittleFS);
    if (res != JDR_OK) {
      Serial.printf("[IMG] getFsJpgSize failed with result %d\n", res);
      return false;
    }

    // Center image if smaller than screen
    int16_t ox = (SCREEN_WIDTH  - imgW) / 2;
    int16_t oy = (SCREEN_HEIGHT - imgH) / 2;
    if (ox < 0) ox = 0;
    if (oy < 0) oy = 0;

    res = TJpgDec.drawFsJpg(ox, oy, WALLPAPER_PATH, LittleFS);
    if (res != JDR_OK) {
      Serial.printf("[IMG] drawFsJpg failed with result %d\n", res);
      return false;
    }

    Serial.printf("[IMG] Wallpaper decoded (%dx%d) at (%d,%d)\n", imgW, imgH, ox, oy);
    return true;
  }
};

#endif // IMAGE_TRANSFER_H
