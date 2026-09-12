// ============================================================================
// Firmware Testing Launcher for Waveshare ESP32-S3-Touch-LCD-1.69 (240x280)
// Powered by Waveshare official Arduino_GFX display driver & CST816T Touch
// Supports: Full-Display Vertical Drag Scroll & Screen Gesture Navigation (Left/Right)
// ============================================================================
#include <Arduino.h>
#include <Wire.h>
#include "Arduino_GFX_Library.h"
#include "luna_gfx_compat.h"

// Hardware Pin Definitions for Waveshare ESP32-S3-Touch-LCD-1.69
#define LCD_DC      4    // ST7789 DC
#define LCD_CS      5    // ST7789 CS
#define LCD_SCL     6    // ST7789 SCLK
#define LCD_SDA     7    // ST7789 MOSI
#define LCD_RST     8    // ST7789 RST
#define LCD_BL      15   // Backlight
#define POWER_HOLD  41   // Power hold (must be HIGH)
#define TOUCH_SDA   11   // I2C SDA
#define TOUCH_SCL   10   // I2C SCL
#define TOUCH_INT   14   // CST816T Interrupt
#define TOUCH_RST   13   // CST816T Reset
#define CST816T_ADDR 0x15

#define LCD_WIDTH   240
#define LCD_HEIGHT  280

// Official Waveshare Arduino_GFX ST7789 display bus with exact 20-pixel panel offset
// Official Waveshare Arduino_GFX ST7789 display bus with exact 20-pixel panel offset
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCL, LCD_SDA);
Arduino_GFX *output_gfx = new Arduino_ST7789(bus, LCD_RST, 0 /* rotation */, true /* IPS */,
                                              LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 20);
// High-performance Double-Buffered Canvas in PSRAM/RAM for 60FPS tearing-free UI
Arduino_Canvas *canvas = new Arduino_Canvas(LCD_WIDTH, LCD_HEIGHT, output_gfx);

// Global wrapper instance required by Luna UI Studio generated code
LunaGFXWrapper tft;

// Forward declarations from Luna_MultiScreen_App.ino
extern void drawCurrentScreen();
extern void handleScreenTouch(int touchX, int touchY);
extern void handleScreenSwipe(uint8_t direction);
extern void handleScreenScroll(int deltaY);
extern unsigned long alertDismissMs;

// Touch tracking interrupt
static volatile bool touchInterruptOccurred = false;
static void IRAM_ATTR touchISR() {
  touchInterruptOccurred = true;
}

void initTouch() {
  Serial.println("[Touch] Resetting CST816T...");
  pinMode(TOUCH_RST, OUTPUT);
  digitalWrite(TOUCH_RST, LOW);
  delay(10);
  digitalWrite(TOUCH_RST, HIGH);
  delay(50);

  Serial.println("[Touch] Starting I2C Wire on SDA=11, SCL=10...");
  Wire.begin(TOUCH_SDA, TOUCH_SCL, 400000);
  Wire.setTimeOut(50);

  pinMode(TOUCH_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TOUCH_INT), touchISR, FALLING);
  touchInterruptOccurred = false;
  Serial.println("[Touch] CST816T hardened interaction engine ready.");
}

// Low-level packet reader: returns true when fresh I2C data is read
bool readTouchPacket(int &touchX, int &touchY, uint8_t &gesture, uint8_t &fingerNum) {
  if (!touchInterruptOccurred && digitalRead(TOUCH_INT) == HIGH) {
    return false;
  }
  touchInterruptOccurred = false;

  Wire.beginTransmission(CST816T_ADDR);
  Wire.write(0x01);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom((uint16_t)CST816T_ADDR, (uint8_t)6) < 6) {
    return false;
  }

  gesture   = Wire.read();
  fingerNum = Wire.read();
  uint8_t xH = Wire.read();
  uint8_t xL = Wire.read();
  uint8_t yH = Wire.read();
  uint8_t yL = Wire.read();

  if (fingerNum > 0) {
    int rawX = ((xH & 0x0F) << 8) | xL;
    int rawY = ((yH & 0x0F) << 8) | yL;
    touchX = constrain(rawX, 0, 239);
    touchY = constrain(rawY, 0, 279);
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(400);
  Serial.println("==========================================");
  Serial.println("  Luna UI Studio - ESP32-S3 1.69 Test App");
  Serial.println("  Hardened Gestures & Smooth 60FPS Pipeline");
  Serial.println("==========================================");

  // 1. Maintain hardware power hold
  Serial.println("[Boot] 1. Enabling Power Hold (GPIO 41)...");
  pinMode(POWER_HOLD, OUTPUT);
  digitalWrite(POWER_HOLD, HIGH);

  // 2. Hardware LCD Reset
  Serial.println("[Boot] 2. Hardware Reset LCD (GPIO 8)...");
  pinMode(LCD_DC, OUTPUT);
  pinMode(LCD_RST, OUTPUT);
  digitalWrite(LCD_RST, HIGH);
  delay(50);
  digitalWrite(LCD_RST, LOW);
  delay(100);
  digitalWrite(LCD_RST, HIGH);
  delay(150);

  // 3. Initialize Backlight
  Serial.println("[Boot] 3. Enabling Backlight (GPIO 15)...");
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);

  // 4. Initialize ST7789 Display at 80MHz SPI and Canvas Framebuffer
  Serial.println("[Boot] 4. Initializing Arduino_GFX ST7789 (80MHz SPI)...");
  if (!output_gfx->begin(80000000)) {
    Serial.println("[Boot] ERROR: output_gfx->begin() failed!");
  } else {
    Serial.println("[Boot] output_gfx->begin(80MHz) succeeded!");
  }

  Serial.println("[Boot] Initializing Double-Buffered Canvas in RAM/PSRAM...");
  if (!canvas->begin(GFX_SKIP_OUTPUT_BEGIN)) {
    Serial.println("[Boot] Canvas begin failed, falling back to direct display");
    tft.attach(output_gfx);
  } else {
    Serial.println("[Boot] Canvas ready! Ultra-smooth 60FPS enabled.");
    tft.attach(canvas);
  }

  // 5. Initialize Capacitive Touch
  Serial.println("[Boot] 5. Initializing Touch...");
  initTouch();

  // 6. Draw initial screen
  Serial.println("[Boot] 6. Rendering Luna UI Studio screen...");
  drawCurrentScreen();
  Serial.println("[Boot] Screen rendered successfully! Entering loop.");
}

// ── Gesture State Machine Definitions ──
enum InteractionState {
  STATE_IDLE = 0,
  STATE_TOUCH_DOWN,
  STATE_TRACKING,
  STATE_SCROLL_VERTICAL,
  STATE_SWIPE_TRIGGERED
};

static InteractionState touchState = STATE_IDLE;
static bool isFingerDown = false;
static int startX = 0, startY = 0;
static int lastX = 0, lastY = 0;
static int prevSampleY = 0;
static unsigned long touchStartTime = 0;
static unsigned long lastPacketTime = 0;
static unsigned long lastSwipeTransitionMs = 0;
static uint8_t hwGesture = 0;
static int pendingScrollDelta = 0;
static unsigned long lastScrollFlushMs = 0;

void loop() {
  int tx = 0, ty = 0;
  uint8_t g = 0;
  uint8_t fNum = 0;
  bool fresh = readTouchPacket(tx, ty, g, fNum);
  unsigned long now = millis();

  // Auto-dismiss transient alert overlays after 1.5 seconds
  if (alertDismissMs != 0 && now >= alertDismissMs) {
    alertDismissMs = 0;
    drawCurrentScreen();
  }

  // ── 1. FINGER DOWN / MOVING ──
  if (fresh && fNum > 0) {
    lastPacketTime = now;
    lastX = tx;
    lastY = ty;
    if (g != 0) {
      hwGesture = g;
    }

    if (!isFingerDown) {
      // Touch Down Event
      isFingerDown = true;
      startX = tx;
      startY = ty;
      prevSampleY = ty;
      touchStartTime = now;
      touchState = STATE_TOUCH_DOWN;
      pendingScrollDelta = 0;
    } else {
      // Finger is moving
      int dX = tx - startX;
      int dY = ty - startY;
      int absX = abs(dX);
      int absY = abs(dY);

      // Deadzone: remain in candidate tap while movement < 10px
      if (touchState == STATE_TOUCH_DOWN) {
        if (absX >= 10 || absY >= 10) {
          touchState = STATE_TRACKING;
        }
      }

      // Classification during active tracking
      if (touchState == STATE_TRACKING) {
        // Immediate Horizontal Swipe Triggering:
        // Triggers as soon as finger moves >= 30px horizontally with horizontal dominance!
        if ((absX >= 30 && absX > (absY * 6 / 5)) || hwGesture == 0x03 || hwGesture == 0x04) {
          if (now - lastSwipeTransitionMs >= 250) {
            lastSwipeTransitionMs = now;
            bool isSwipeLeft = (hwGesture == 0x03) || (hwGesture != 0x04 && dX < 0);
            if (isSwipeLeft) {
              Serial.printf("[Gesture] Immediate Swipe Left (dx=%d, hw=%02X)\n", dX, hwGesture);
              handleScreenSwipe(1); // Next screen
            } else {
              Serial.printf("[Gesture] Immediate Swipe Right (dx=%d, hw=%02X)\n", dX, hwGesture);
              handleScreenSwipe(2); // Previous screen
            }
            touchState = STATE_SWIPE_TRIGGERED; // Lock until finger release
          }
        // Vertical scroll wins when vertical distance dominates
        } else if (absY >= 14 && absY > absX) {
          touchState = STATE_SCROLL_VERTICAL;
        }
      }

      // Live vertical scroll handling with 60FPS rate-limiting
      if (touchState == STATE_SCROLL_VERTICAL) {
        int diffY = prevSampleY - ty; // drag finger up -> scroll content down
        prevSampleY = ty;
        pendingScrollDelta += diffY;

        if (abs(pendingScrollDelta) >= 2 && (now - lastScrollFlushMs >= 16)) {
          handleScreenScroll(pendingScrollDelta);
          pendingScrollDelta = 0;
          lastScrollFlushMs = now;
        }
      }
    }
  }

  // ── 2. FINGER UP / TIMEOUT RELEASE ──
  // CST816T might report fNum == 0 on lift, or interrupts cease (> 65ms)
  bool explicitRelease = (fresh && fNum == 0);
  bool timedOut = (isFingerDown && (now - lastPacketTime > 65));

  if (isFingerDown && (explicitRelease || timedOut)) {
    isFingerDown = false;
    unsigned long held = now - touchStartTime;
    int deltaX = lastX - startX;
    int deltaY = lastY - startY;
    int absX = abs(deltaX);
    int absY = abs(deltaY);

    // Flush any pending scroll delta
    if (touchState == STATE_SCROLL_VERTICAL && pendingScrollDelta != 0) {
      handleScreenScroll(pendingScrollDelta);
      pendingScrollDelta = 0;
    }

    // 1. Clean Button Tap: movement stayed within deadzone (<12px) and duration < 500ms
    if (touchState == STATE_TOUCH_DOWN) {
      if (held >= 15 && held < 500) {
        Serial.printf("[Touch] Click at X=%d, Y=%d (duration=%lums)\n", startX, startY, held);
        handleScreenTouch(startX, startY);
      }
    }
    // 2. Short / Fast flick that didn't reach 30px during tracking
    else if (touchState == STATE_TRACKING && absX >= 24 && absX > (absY * 6 / 5)) {
      if (now - lastSwipeTransitionMs >= 250) {
        lastSwipeTransitionMs = now;
        bool isSwipeLeft = (deltaX < 0);
        if (isSwipeLeft) {
          Serial.printf("[Gesture] Flick Left (dx=%d)\n", deltaX);
          handleScreenSwipe(1);
        } else {
          Serial.printf("[Gesture] Flick Right (dx=%d)\n", deltaX);
          handleScreenSwipe(2);
        }
      }
    }

    // Reset interaction state
    touchState = STATE_IDLE;
    hwGesture = 0;
  }

  // Minimal delay to prevent watchdog starvation without capping touch polling rate
  delay(1);
}
