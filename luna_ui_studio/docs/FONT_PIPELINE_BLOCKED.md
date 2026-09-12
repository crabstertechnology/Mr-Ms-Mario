# Font Pipeline Status: BLOCKED

**Status:** BLOCKED  
**Date:** 2026-09-10  
**Audit Finding:** True font rasterization pipeline cannot proceed due to missing source font binary assets.

---

## 1. Reason for Blocked Status

The Luna UI Studio embedded typography pipeline currently utilizes placeholder font metrics in `fontData.js` (`OUTFIT_16`, `OUTFIT_12`, and `JETBRAINS_MONO_9`). These glyphs were manually and algorithmically generated to establish the compiler, subsetter, and C++ PROGMEM emission pipelines.

Per the Phase 4 Correctness Mandate:
> "If no source font files are available, do NOT fabricate replacement glyphs.  
> Instead, document the exact missing source-font dependency, keep the current pipeline isolated, and report this as BLOCKED rather than pretending the font pipeline is complete.  
> Do not add a runtime font engine to the ESP32."

An exhaustive filesystem scan of the entire repository confirmed that **neither Outfit nor JetBrains Mono `.ttf` or `.otf` font files exist in the repository or project assets**.

---

## 2. Missing Font Dependencies

To enable the true deterministic offline rasterization pipeline:
```
True TTF/OTF File
      ↓
Deterministic Offline Rasterizer (Node.js script with opentype.js / canvas)
      ↓
Target Pixel-Grid Alignment (e.g. 16px, 12px, 9px)
      ↓
1-bit 8-pixel packed glyph bitmaps + metrics (width, height, xOffset, yOffset, xAdvance)
      ↓
fontData.js / GeneratedFonts.cpp
```

The following canonical font binary files must be provided:

| Font Family | Required Weights / Variants | Intended Target Bitmaps | Source |
|---|---|---|---|
| **Outfit** | `Outfit-Regular.ttf` (or `.otf`)<br>`Outfit-Medium.ttf`<br>`Outfit-SemiBold.ttf` | 16px (`OUTFIT_16`)<br>12px (`OUTFIT_12`) | [Google Fonts / Outfit](https://fonts.google.com/specimen/Outfit) (OFL License) |
| **JetBrains Mono** | `JetBrainsMono-Regular.ttf` (or `.otf`) | 9px (`JETBRAINS_MONO_9`) | [JetBrains Mono](https://www.jetbrains.com/lp/mono/) / [Google Fonts](https://fonts.google.com/specimen/JetBrains+Mono) (OFL License) |

---

## 3. Planned Offline Rasterization Architecture

Once the true `.ttf` or `.otf` files are placed into `luna_ui_studio/assets/fonts/`:

1. An offline rasterizer script (`tools/rasterize_fonts.mjs`) will:
   - Load each TTF via `opentype.js` (or pure vector glyph parser).
   - Render each ASCII glyph (`0x20` to `0x7E`) onto a 1-bit monochrome grid with pixel-hinting at the exact target sizes (16px, 12px, 9px).
   - Calculate exact typographical metrics:
     - `width`: bounding box width
     - `height`: bounding box height
     - `xOffset`: left bearing offset
     - `yOffset`: top baseline offset (negative or positive relative to baseline)
     - `xAdvance`: horizontal cursor step
   - Pack bitmap rows into 1-bit bytes (`uint8_t` PROGMEM byte arrays).
2. The rasterized tables will replace the placeholder metrics in `src/compiler/fonts/fontData.js`.
3. The existing subsetter (`subsetFont`), project character extractor (`extractCharactersFromProject`), and C++ emitter (`compileFontsToCpp`) will remain 100% untouched, as they already accept the canonical `LunaFont` struct format.

---

## 4. Current State & Isolation

- **Isolation:** The current placeholder fonts remain strictly isolated within `src/compiler/fonts/fontData.js`.
- **Contract:** The rendering contract (`docs/RENDERING_CONTRACT.md` §3) explicitly records that current font rendering produces placeholder glyph shapes and does not claim to represent true Outfit or JetBrains Mono typography.
- **Embedded Footprint:** No heavy FreeType or vector font engine is added to the ESP32 runtime; all typography remains static 1-bit PROGMEM bitmap tables.
