// ============================================================================
// Luna UI Studio — Phase 2 Canonical Embedded Renderer Proof
// Target: Waveshare ESP32-S3 Touch LCD 1.69" (240x280 ST7789 + CST816T)
//
// This sketch proves that the Luna Embedded Runtime can directly consume
// the Canonical UI Schema compiled into GeneratedUI.h / GeneratedUI.cpp.
// ============================================================================

#include <Arduino.h>
#include <Wire.h>
#include "Arduino_GFX_Library.h"

#include "LunaTypes.h"
#include "LunaRuntime.h"
#include "GeneratedUI.h"

// Hardware Pin Configuration (Waveshare ESP32-S3 Touch LCD 1.69")
#define LCD_DC      4    // ST7789 Data/Command
#define LCD_CS      5    // ST7789 Chip Select
#define LCD_SCL     6    // ST7789 SPI Clock
#define LCD_SDA     7    // ST7789 SPI MOSI
#define LCD_RST     8    // ST7789 Reset
#define LCD_BL      15   // ST7789 Backlight
#define POWER_HOLD  41   // Power hold (must remain HIGH)

#define LCD_WIDTH   240
#define LCD_HEIGHT  280
#define LCD_OFFSET_Y 20  // Panel offset for ST7789 1.69"

// Official Waveshare Arduino_GFX display bus configuration
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCL, LCD_SDA);
Arduino_GFX *output_gfx = new Arduino_ST7789(bus, LCD_RST, 0 /* rotation */, true /* IPS */,
                                              LCD_WIDTH, LCD_HEIGHT, 0, LCD_OFFSET_Y, 0, LCD_OFFSET_Y);

// Double-Buffered PSRAM Canvas for smooth, tearing-free updates
Arduino_Canvas *canvas = new Arduino_Canvas(LCD_WIDTH, LCD_HEIGHT, output_gfx);

// Master Embedded Runtime Instance
LunaRuntime runtime;

void setup() {
  // Hold power pin HIGH to maintain battery power circuit
  pinMode(POWER_HOLD, OUTPUT);
  digitalWrite(POWER_HOLD, HIGH);

  // Initialize backlight pin
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);

  Serial.begin(115200);
  delay(400);

  Serial.println(F("\n=================================================="));
  Serial.println(F(" Luna UI Studio — Phase 2 Canonical Embedded Proof"));
  Serial.println(F(" Target: Waveshare ESP32-S3 Touch LCD 1.69\""));
  Serial.println(F("=================================================="));

  // 1. Initialize Display Controller
  if (!output_gfx->begin()) {
    Serial.println(F("[ERROR] Failed to initialize ST7789 display controller!"));
  } else {
    Serial.println(F("[Display] ST7789 240x280 (offset Y=20) initialized."));
  }

  // 2. Initialize Double-Buffered Canvas in PSRAM
  if (!canvas->begin()) {
    Serial.println(F("[ERROR] Failed to allocate double-buffered canvas in PSRAM!"));
  } else {
    Serial.println(F("[Canvas] Double-buffered canvas initialized successfully."));
  }

  // 3. Initialize LunaRuntime with Compiled Canonical UIProject
  Serial.println(F("[Runtime] Loading compiled canonical project: LUNA_COMPILED_PROJECT..."));
  if (!runtime.begin(&LUNA_COMPILED_PROJECT, canvas, LCD_WIDTH, LCD_HEIGHT)) {
    Serial.println(F("[ERROR] Failed to initialize runtime with canonical project definition!"));
  } else {
    Serial.printf("[Runtime] Successfully loaded project: \"%s\" (v%s)\n",
                  LUNA_COMPILED_PROJECT.name, LUNA_COMPILED_PROJECT.version);
    Serial.printf("[Runtime] Screens configured: %u | Initial screen: \"%s\"\n",
                  LUNA_COMPILED_PROJECT.screenCount, runtime.getActiveScreenId());
  }

  // Initial Memory Telemetry
  Serial.printf("[Memory] Free Internal SRAM: %u bytes\n", runtime.getFreeInternalHeap());
  Serial.printf("[Memory] Free PSRAM:         %u bytes\n", runtime.getFreePsram());
  Serial.println(F("[Runtime] Entering standard render & input loop.\n"));
}

void loop() {
  // 1. Poll Touch Input and Process Hit Testing
  runtime.updateInput();

  // 2. Update Dynamic State and Animations (e.g. Spinner Rotation)
  runtime.updateRuntime();

  // 3. Render and Present If Screen or Animation is Dirty
  runtime.renderIfNeeded();

  // 4. Periodic Telemetry Reporting (every 3 seconds)
  static uint32_t lastLogMillis = 0;
  if (millis() - lastLogMillis >= 3000) {
    lastLogMillis = millis();
    Serial.printf("[Telemetry] Active: %-16s | Frame: %5u us | FPS: %4.1f | SRAM: %6u B | PSRAM: %7u B\n",
                  runtime.getActiveScreenId(),
                  runtime.getLastRenderDurationUs(),
                  runtime.getMeasuredFps(),
                  runtime.getFreeInternalHeap(),
                  runtime.getFreePsram());
  }

  // Prevent watchdog starvation
  delay(1);
}
