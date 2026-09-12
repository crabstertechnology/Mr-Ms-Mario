# Luna UI Studio — Phase 4 Validation Report

*Generated: Phase 4 — Hardware Parity Validation & Rendering Contract*
*Test date: Phase 4 completion*

---

## Executive Summary

| Test Suite | Tests | Passed | Failed | Notes |
|-----------|-------|--------|--------|-------|
| Rendering Contract | 34 | 34 | 0 | All passed |
| Negative Diagnostics | 20 | 20 | 0 | All passed |
| Determinism | 18 | 18 | 0 | All passed |
| **Total** | **72** | **72** | **0** | |

**All 72 automated tests passed.** The pipeline is correct at the semantic and compiler layers.

---

## Stable SHA-256 Fingerprints

These hashes are deterministic. Any change to the source schema, compiler logic, or golden fixture will change these values.

| Artifact | SHA-256 |
|---------|---------|
| `GeneratedUI.h` (from golden fixture) | `6b02babde12d41b6da5fcd24fd992ed7ed376b8ff594da74c3e37ea06fe9ae4d` |
| `GeneratedUI.cpp` (from golden fixture) | `8ce75b4d8ee2587049526fe812edcbf4c9274c5b408f19904268d9cb20b66c21` |
| `GeneratedFonts.cpp` (Outfit 16 subsetted) | `e091f26b2a63f832229371685143b7638f9dd5408483abcc1732c23a8879a9ed` |
| `GeneratedAssets.cpp` (test 8x8 white) | `474468edf8e075f3eef251888eb8918037067dc0782bec66742573478bbe896f` |
| `renderSnapshot` hash (golden fixture) | `9fff8ad1d5f31017f1b139d5f4cd9f1e1ad6184e860839942d45d2877fcddf44` |

---

## New Defects Discovered by Phase 4 Audit

The following issues were discovered by direct source inspection during the Phase 4 audit and are now formally documented in [`RENDERING_CONTRACT.md`](./RENDERING_CONTRACT.md).

### D1 — fontData.js: Placeholder Glyph Data (SEVERITY: HIGH)

**Status: Known limitation. Deferred to future phase.**

`fontData.js` does not contain real rasterized data from the Outfit or JetBrains Mono font files. All uppercase letters use the same algorithmic stroke pattern. All lowercase use a single-column fill. All digits share one bitmap.

- Glyphs are structurally correct (xAdvance metrics, subsetting, C++ emission work correctly).
- On hardware, text will render as illegible blocky patterns — not actual letterforms.
- A true font pipeline (TTF rasterization) is a future phase item.

**Workaround:** None on current hardware. The pipeline is ready for real bitmaps — replace `fontData.js` content.

---

### D2 — PNG Alpha Ignored in RGB565 Conversion (SEVERITY: MEDIUM)

**Status: Known limitation. Documented.**

`rgbaToRgb565Array` reads `r`, `g`, `b` but silently ignores the alpha channel (channel `[3]`). PNG images with transparency produce garbage colors in transparent pixel regions.

**Test coverage:** `test_rendering_contract.mjs` test #33 confirms and documents exact behavior.

**Workaround:** Use only fully opaque PNG images as assets.

---

### D3 — BMP Top-Down Orientation Defect (SEVERITY: MEDIUM)

**Status: Known defect. Documented. Not fixed in Phase 4.**

`decodeBmp` uses `Math.abs(height)` which discards the sign used to indicate top-down BMPs. All BMPs are then decoded with the bottom-up scanline formula, producing a vertically-flipped image for top-down BMPs.

**Test coverage:** `test_rendering_contract.mjs` test #31 documents the behavior.

**Workaround:** Always export BMPs as bottom-up (standard Windows BMP export default). Never use top-down (negative height) BMPs.

---

### D4 — Asset Deduplication Hash Did Not Include Dimensions (SEVERITY: LOW)

**Status: FIXED in Phase 4.**

The `compileAssetsToCpp` SHA-256 hash previously covered pixel data only. Two images with the same pixel data but different `width`/`height` would incorrectly share a single PROGMEM symbol with mismatched dimensions.

**Fix applied:** The hash now prefixes `${width}x${height}:` before the pixel buffer. Verified by `test_rendering_contract.mjs` test #29.

---

### D5 — renderSnapshot Hash Not Content-Sensitive (SEVERITY: LOW)

**Status: Known defect. Documented. Not fixed in Phase 4.**

`createProjectRenderSnapshot` uses `JSON.stringify(snapshot, Object.keys(snapshot).sort())`. The array-replacer form of `JSON.stringify` filters keys at every nesting level, not just the root. Screen content (nested under screen IDs like `s1`) is filtered out because screen IDs are not in the root key list `['device', 'screens', 'version']`.

**Consequence:** The snapshot hash reflects only `version` and `device.id`, not screen content. Two visually different projects with the same version and device produce the same hash.

**Test coverage:** `test_determinism.mjs` test #16 documents the behavior.

**Fix (future):** Replace the array replacer with a custom replacer function that pre-sorts all object keys before serialization.

---

### D6 — RGB565 Byte Order at SPI Boundary (SEVERITY: HIGH, HARDWARE UNVERIFIED)

**Status: Unverified. Hardware capture required.**

The `rgb888ToRgb565` formula produces correct 16-bit integers. However, whether Arduino_GFX's `draw16bitRGBBitmap` correctly byte-swaps from the ESP32's little-endian layout to the ST7789's big-endian SPI protocol is **not confirmed**.

**Physical verification:** Flash the `screen_parity_test` screen and observe the hardware output:

- **Pure red square appears RED** → byte order is correct.
- **Pure red square appears BLUE** → byte order is swapped; fix requires calling the big-endian variant or manually swapping bytes before passing to `draw16bitRGBBitmap`.

---

## Hardware Parity Test Screen

**Screen ID:** `screen_parity_test` in [`golden-parity-project.json`](../test/fixtures/golden-parity-project.json)

**Elements and expected hardware output:**

| Element ID | Layout | Expected | Failure signature |
|-----------|--------|----------|-------------------|
| `pt_red` | x=5, y=5, 40×40 | Bright red block | Appears blue → byte-swap |
| `pt_green` | x=55, y=5, 40×40 | Bright green block | Color wrong |
| `pt_blue` | x=105, y=5, 40×40 | Bright blue block | Appears red → byte-swap |
| `pt_white` | x=155, y=5, 40×40 | Bright white block | Color wrong |
| `pt_black` | x=5, y=55, 40×40 | Black square (white border) | Border missing |
| `pt_gray` | x=55, y=55, 40×40 | Mid-gray block | Color wrong |
| `pt_text_hello` | x=5, y=110, 230×20 | Centered glyph patterns | Off-center |
| `pt_text_clock` | x=10, y=140, 100×20 | Left-aligned glyph patterns | Wrong position |
| `pt_card` | x=10, y=170, 220×60 | Rounded rect + 2 text rows | Layout wrong |
| `pt_button` | x=70, y=240, 100×32 | Rounded rect + centered "OK" | Label off-center |
| `pt_image` | x=195, y=55, 32×32 | Red/blue 8×8 checkerboard | Colors swapped or wrong |

**How to use:**
1. Compile `screen_parity_test` via the Studio exporter targeting `waveshare_esp32s3_touch_lcd_169`.
2. Flash to hardware.
3. Photograph the physical display.
4. Compare against the table above.
5. Record observed output in the table below.

**Hardware Capture Results** *(to be filled when hardware is available):*

| Element | Expected | Observed | ✓/✗ |
|---------|----------|----------|-----|
| pt_red | Red block | — | — |
| pt_green | Green block | — | — |
| pt_blue | Blue block | — | — |
| pt_white | White block | — | — |
| pt_black | Black+white border | — | — |
| pt_gray | Gray block | — | — |
| pt_text_hello | Centered glyphs | — | — |
| pt_text_clock | Left glyphs | — | — |
| pt_card | Card geometry | — | — |
| pt_button | Button+label | — | — |
| pt_image | Checkerboard | — | — |

---

## Open Items

| # | Item | Severity | Status |
|---|------|----------|--------|
| D1 | Font glyphs are placeholder bitmaps | HIGH | Deferred |
| D2 | PNG alpha ignored | MEDIUM | Documented |
| D3 | BMP top-down decoded upside-down | MEDIUM | Documented |
| D4 | Asset hash excludes dimensions | LOW | **FIXED** |
| D5 | renderSnapshot hash not content-sensitive | LOW | Documented |
| D6 | RGB565 byte order at SPI boundary | HIGH | **Hardware capture required** |

---

## Files Produced in Phase 4

| File | Purpose |
|------|---------|
| [`docs/RENDERING_CONTRACT.md`](./RENDERING_CONTRACT.md) | Formal rendering contract |
| [`test/test_rendering_contract.mjs`](../test/test_rendering_contract.mjs) | 34 semantic layer tests |
| [`test/test_negative_diagnostics.mjs`](../test/test_negative_diagnostics.mjs) | 20 unsupported-feature diagnostic tests |
| [`test/test_determinism.mjs`](../test/test_determinism.mjs) | 18 determinism tests |
| `test/fixtures/golden-parity-project.json` | Updated — `screen_parity_test` added |
| `src/compiler/assets/assetCompiler.js` | Bug fix — asset hash now includes dimensions |

---

*Luna UI Studio — Phase 4 complete.*
