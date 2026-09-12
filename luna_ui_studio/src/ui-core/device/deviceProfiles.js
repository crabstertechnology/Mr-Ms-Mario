/**
 * Luna UI Studio — Device Profiles & Renderer Capabilities
 *
 * Centralized specifications for hardware targets and their supported capabilities.
 * Eliminates hardcoded display dimensions and allows validators to check for unsupported
 * features ahead of compile time.
 */

export const WAVESHARE_ESP32S3_169 = {
  id: 'waveshare_esp32s3_touch_lcd_169',
  name: 'Waveshare ESP32-S3 Touch LCD 1.69"',
  // Hardware specifications
  width: 240,
  height: 280,
  colorDepth: 16,
  orientation: 'portrait',
  touch: true,
  mcu: 'ESP32-S3',
  displayController: 'ST7789',
  touchController: 'CST816T',
  panelOffsetX: 0,
  panelOffsetY: 20,
  maxFps: 60,
  flashSizeMb: 16,
  hasPsram: true,
  psramType: 'OPI',

  // Semantic renderer capability contract (conservative current runtime truth)
  capabilities: {
    maxWidth: 240,
    maxHeight: 280,
    colorDepth: 16,
    supportsTouch: true,
    supportsDoubleBuffer: true,
    supportsImages: true,           // Supported via assetCompiler and LunaRenderer::drawImage
    supportsCustomFonts: true,      // Supported via fontCompiler and LunaRenderer::drawTextWithFont
    supportsOpacity: false,         // Hardware/runtime has no alpha blending compositing
    supportsRotation: false,        // No 2D affine rotation matrix in current renderer
    supportsScale: false,           // No dynamic vector scaling
    supportsAnimation: false,       // Current firmware loop lacks animation frame timer
    supports3DTransform: false,     // No 3D projection (e.g. rotateY) on hardware
    supportsBlur: false,            // No hardware/software backdrop filter
    supportsDirtyRects: false,      // Full screen redraw only
    supportsAntiAliasing: false,    // Aliased rasterization only
    supportsClipping: false,        // No hardware viewport clipping
  },
}

export const WAVESHARE_ESP32C3_13 = {
  id: 'waveshare_esp32c3_lcd_13',
  name: 'Waveshare ESP32-C3 LCD 1.3"',
  width: 240,
  height: 240,
  colorDepth: 16,
  orientation: 'portrait',
  touch: false,
  mcu: 'ESP32-C3',
  displayController: 'ST7789',
  panelOffsetX: 0,
  panelOffsetY: 0,
  maxFps: 30,
  flashSizeMb: 4,
  hasPsram: false,

  capabilities: {
    maxWidth: 240,
    maxHeight: 240,
    colorDepth: 16,
    supportsTouch: false,
    supportsDoubleBuffer: false,
    supportsImages: false,
    supportsCustomFonts: false,
    supportsOpacity: false,
    supportsRotation: false,
    supportsScale: false,
    supportsAnimation: false,
    supports3DTransform: false,
    supportsBlur: false,
    supportsDirtyRects: false,
    supportsAntiAliasing: false,
    supportsClipping: false,
  },
}

export const DEVICE_PROFILES = {
  [WAVESHARE_ESP32S3_169.id]: WAVESHARE_ESP32S3_169,
  [WAVESHARE_ESP32C3_13.id]: WAVESHARE_ESP32C3_13,
}

export function getDefaultDevice() {
  return WAVESHARE_ESP32S3_169
}

export function getDeviceProfile(id) {
  return DEVICE_PROFILES[id] || WAVESHARE_ESP32S3_169
}
