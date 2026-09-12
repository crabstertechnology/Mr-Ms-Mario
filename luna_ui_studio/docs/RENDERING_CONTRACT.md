# Luna UI Studio — Rendering Contract
# Phase 4 · Hardware Parity Validation

> **This document is the authoritative specification of what the Luna UI pipeline
> guarantees and what it does not.**
>
> Claims here are derived from direct source inspection of:
> - `src/compiler/assets/assetCompiler.js`
> - `src/compiler/fonts/fontData.js` + `fontCompiler.js`
> - `src/compiler/embeddedCompiler.js`
> - `src/ui-core/validation/renderSnapshot.js`
> - `runtime/luna_runtime/LunaRenderer.cpp`
> - `runtime/luna_runtime/LunaTypes.h`
>
> No claim below is assumed. Every claim that cannot be derived from source
> inspection alone is explicitly marked **UNVERIFIED (hardware capture required)**.

---

## 1. Pipeline Overview

```
Canonical UIProject  (source of truth — JSON)
        │
        ▼
embeddedCompiler.js  (JS — deterministic)
        │
        ├─► GeneratedUI.h / GeneratedUI.cpp   (C++ static structs)
        ├─► GeneratedFonts.h / GeneratedFonts.cpp
        └─► GeneratedAssets.h / GeneratedAssets.cpp
                │
                ▼
        LunaRuntime (C++)
                │
                ▼
        LunaRenderer → Arduino_GFX → Arduino_Canvas
                │
                ▼
        ST7789 (SPI 80 MHz, 240×280, RGB565)
```

**Verification layers:**

| Layer | Tool | What is verified |
|-------|------|-----------------|
| Semantic | `renderSnapshot.js` | Draw command presence, layout coords, measured text positions |
| Compile-time | `featureValidator.js` | Capability contract (unsupported features caught before flashing) |
| Determinism | SHA-256 of C++ output | Same project → byte-identical files |
| Hardware | Physical display capture | Color correctness, geometry, text legibility |

---

## 2. What Is Guaranteed

### 2.1 Layout Geometry — GUARANTEED (semantic)

- Element `x`, `y`, `width`, `height` from the Canonical UIProject are emitted
  verbatim into `LunaNodeDef.layout` C++ structs.
- `LunaRenderer` places rectangles, cards, buttons, images at these exact pixel
  coordinates.
- No automatic scaling, padding, or margin injection is applied by the compiler
  or the runtime.
- **All coordinates are in display space.** `(0, 0)` is the physical top-left
  corner of the display. The ST7789 panel Y-offset (`panelOffsetY = 20`) is
  absorbed by the display driver at the hardware register level — UI coordinates
  do NOT require a `+20` adjustment.

### 2.2 Screen Navigation — GUARANTEED (semantic)

- `LunaAction { type: NAVIGATE, targetScreenId }` is correctly emitted for
  `onClick` events.
- `LunaRuntime` switches the active screen when a button touch event fires.
- Swipe gestures (`swipeLeftTarget`, `swipeRightTarget`) are compiled into the
  screen definition.

### 2.3 Scroll — GUARANTEED (semantic)

- `isScrollable` and `maxScrollY` are correctly compiled into `LunaScreenDef`.
- Scroll logic is the responsibility of `LunaRuntime`, not `LunaRenderer`.

### 2.4 Color — GUARANTEED at integer level (hardware UNVERIFIED)

The RGB888 → RGB565 conversion formula used throughout the pipeline:

```js
rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
```

This is the standard lossy RGB565 quantization. It is mathematically correct as
a 16-bit integer.

> [!WARNING]
> **UNVERIFIED (hardware capture required):** Whether `draw16bitRGBBitmap` in
> Arduino_GFX correctly handles byte ordering when transmitting `uint16_t` pixel
> arrays to the ST7789 via SPI. The ESP32 is little-endian. The ST7789 SPI
> protocol expects big-endian 16-bit color words (MSB first). Whether
> Arduino_GFX performs this byte swap internally is library-version-dependent
> and has **not been confirmed against a physical display capture**.
>
> Symptoms if byte order is wrong: red appears blue, blue appears red. All
> mid-tones show incorrect hue.

### 2.5 Determinism — GUARANTEED

Two successive calls to `compileProject()` with identical canonical input produce
byte-identical C++ output files. Verified by SHA-256 hash comparison of
`GeneratedUI.cpp`, `GeneratedFonts.cpp`, and `GeneratedAssets.cpp`.

---

## 3. What Is NOT Guaranteed

### 3.1 Font Glyph Shapes — NOT GUARANTEED ⚠️

> [!CAUTION]
> `fontData.js` does **NOT** contain real rasterized data from the Outfit or
> JetBrains Mono font files. The data is manually authored placeholder bitmaps.

Specific defects (verified by source inspection of `fontData.js`):

| Character class | Actual bitmap |
|----------------|---------------|
| Uppercase A–Z  | All use the same stroke pattern `0x81 \| (crossbar ? 0x7E : 0x00)`. Letters are indistinguishable except by width. |
| Lowercase a–z  | All use a single-column fill of `0x7C`. Every character looks like a vertical bar. |
| Digits 0–9     | All 10 digits share the same "zero" bitmap `[0x3C, 0x66, 0x6E, ...]`. Digit `1` and digit `9` look identical. |
| JetBrains Mono | Same algorithmic generation — not actual monospaced bitmaps. |

**What IS guaranteed about fonts:**

- Correct character slots are reserved (subsetting works correctly).
- Glyph `xAdvance` widths control text cursor advancement — text measurement is
  self-consistent for layout purposes.
- C++ glyph emission (`LunaGlyph` arrays, `LunaFont` structs) is structurally
  correct. The runtime will render something at the correct position.
- The pipeline is ready for real font bitmaps: replace `fontData.js` with
  actual rasterized data and the rest of the pipeline requires no changes.

**Tracking:** Real TTF→bitmap rasterization is deferred to a future phase.

### 3.2 PNG Transparency — NOT SUPPORTED

> [!WARNING]
> PNG images with transparency (alpha channel) are **not correctly handled**.

**Behaviour (source-verified, `assetCompiler.js` L29–42):**
- The PNG decoder correctly reads RGBA8888 data (alpha is in channel `[3]`).
- `rgbaToRgb565Array` reads only `r`, `g`, `b` and **ignores the alpha channel
  entirely**.
- Transparent pixel regions are converted using whatever raw `r/g/b` values the
  source pixel contains, which may be pre-multiplied or zero.

**Required workaround:** All PNG images used as assets must be **fully opaque**.
Export assets without transparency before compiling. Transparent PNG assets will
produce garbage color output in transparent regions.

### 3.3 BMP Restrictions

The `decodeBmp` function (`pngDecoder.js`) handles the following correctly:
- 24-bpp uncompressed BMP (BI_RGB), bottom-up scanlines ✓
- 32-bpp uncompressed BMP (BI_BITFIELDS), bottom-up scanlines ✓
- BGR → RGB channel swap ✓
- Bottom-up row order → top-down RGBA output ✓

**Known defects and restrictions:**

| Condition | Result |
|-----------|--------|
| Top-down BMP (negative height) | **Decoded upside-down.** `Math.abs()` discards the sign so the decoder always uses the bottom-up formula `srcY = height - 1 - y`. |
| 16-bpp BMP | Silently produces all-zero (black) pixels. No error thrown. |
| 8-bpp palettized BMP | Silently produces all-zero pixels. No error thrown. |
| Compressed BMP (RLE4/RLE8) | Undefined behavior — compressed data is treated as raw pixels. |

**Required workaround:** Export BMP assets as **24-bpp or 32-bpp, uncompressed,
bottom-up** (standard Windows BMP export default). Never use top-down BMPs.

### 3.4 Image Opacity / Blending — NOT SUPPORTED

`LunaRenderer::drawImage` calls `draw16bitRGBBitmap` directly. There is no
alpha blending against the background. Images are blit directly over whatever
is on the canvas at those coordinates.

### 3.5 Animation — NOT SUPPORTED in current runtime

`supportsAnimation: false` in the device capability contract. Animation metadata
is compiled into `LunaNodeDef` structs but the render loop does not step
animation timelines. The spinner is the only animated element (rotation is driven
by `millis()` in the runtime loop).

### 3.6 Opacity — NOT SUPPORTED

`supportsOpacity: false`. Opacity values on nodes have no effect on hardware.
Elements always render at 100% opacity.

### 3.7 Blur / Backdrop Filter — NOT SUPPORTED

No software or hardware blur is available. Any CSS `backdrop-filter` or blur
effect visible in the Studio preview does not appear on hardware.

### 3.8 Clipping / Overflow — NOT SUPPORTED

`supportsClipping: false`. Elements that exceed screen bounds are not clipped
by the runtime. Coordinates outside `[0, 239]` × `[0, 279]` will be silently
discarded by Arduino_GFX at the display driver boundary.

### 3.9 Anti-aliasing — NOT SUPPORTED

All geometry (rectangles, circles, rounded corners) uses aliased rasterization.
No sub-pixel rendering.

---

## 4. Asset Deduplication Contract

Assets with **identical pixel content AND identical dimensions** are stored once
in `PROGMEM` and aliased to the same C++ symbol. This reduces Flash usage.

Two assets with the same pixel data but **different dimensions** are treated as
distinct assets (each gets its own symbol). The SHA-256 hash includes both pixel
data and `width × height` to prevent aliasing of geometrically distinct assets.

---

## 5. Hardware Test Screen

The `screen_parity_test` screen in
[`test/fixtures/golden-parity-project.json`](../test/fixtures/golden-parity-project.json)
is designed to be flashed directly to the ESP32-S3. It contains:

| Element | Expected | Failure indicates |
|---------|----------|-------------------|
| Pure red 40×40 | Bright red block | RGB565 byte-swap issue |
| Pure green 40×40 | Bright green block | RGB565 byte-swap issue |
| Pure blue 40×40 | Bright blue block | RGB565 byte-swap issue |
| White 40×40 | Bright white `0xFFFF` | Terminal white |
| Black 40×40 | True black `0x0000` | Terminal black |
| Mid-gray 40×40 | Gray `0x8410` | Mid-tone rounding |
| Text "Hello" center | Placeholder glyphs, centered X | Layout / text position |
| Text "12:30" left | Placeholder glyphs, left edge | Digit slots |
| Card with title+subtitle | Rounded rect + two text rows | Card geometry |
| Button "OK" | Rounded rect + centered label | Button centering |
| Opaque 32×32 BMP | Correctly colored image block | Asset pipeline |

**How to use:**
1. Compile `screen_parity_test` via the Studio exporter.
2. Flash to hardware.
3. Photograph the physical display.
4. Compare the photograph to the expected layout.
5. Document any discrepancies in `PHASE_4_VALIDATION_REPORT.md`.

---

## 6. Coordinate System Reference

```
(0,0) ─────────────────────────────► X (0..239)
  │
  │   Display content area
  │   240 × 280 pixels
  │
  ▼
  Y (0..279)
```

- Panel hardware offset `panelOffsetY = 20` is configured in ST7789 register
  RASET/CASET by the Arduino_GFX driver. UI code does not apply this offset.
- Touch coordinates reported by CST816T are in the same display space as pixel
  coordinates. No remapping is required.

---

## 7. Known Open Items (Hardware-Unverified)

| Item | Status | Evidence Required |
|------|--------|-------------------|
| RGB565 byte order at SPI boundary | ⚠️ UNVERIFIED | Physical display capture — pure red square must show red |
| Font glyph shapes | ⚠️ PLACEHOLDER | Real TTF rasterization |
| PNG transparency blending | ❌ NOT SUPPORTED | — |
| Top-down BMP orientation | ❌ DEFECT | Fix `decodeBmp` top-down branch |
| Animation runtime step | ❌ NOT SUPPORTED | Future phase |

---

*Generated: Phase 4 — Luna UI Studio*
*Source verified against: `luna_ui_studio/` commit at Phase 4 audit point*
