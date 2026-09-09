#!/usr/bin/env python3
"""
tools/generate_figma_screens_header.py
Converts captured Figma React/Tailwind screens (240x280) and typography glyphs
into 16-bit RGB565 PROGMEM arrays for the Waveshare ESP32-S3 1.69" display.
"""

import os
import sys
from PIL import Image

def r888_to_565(r, g, b):
    """Convert RGB888 to RGB565 16-bit integer."""
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

screens_dir = r"W:\Mr.mario\scratch\screens"
output_header = r"W:\Mr.mario\1.69 Luna Firmware\figma_screens_data.h"

screen_files = [
    ("figma_screen_clock_minimal",       "clock_minimal_blank.png"),
    ("figma_screen_clock_retro",         "clock_retro.png"),
    ("figma_screen_clock_cyber",         "clock_cyber.png"),
    ("figma_screen_clock_analog",        "clock_analog.png"),
    ("figma_screen_notifications",       "notifications.png"),
    ("figma_screen_calendar",            "calendar.png"),
    ("figma_screen_arcade",              "arcade.png"),
    ("figma_screen_pomodoro",            "pomodoro.png"),
    ("figma_screen_navigation",          "navigation.png"),
    ("figma_screen_level",               "level.png"),
    ("figma_screen_settings",            "settings.png"),
]

print("[CONVERT] Generating figma_screens_data.h ...")

with open(output_header, "w", encoding="utf-8") as f:
    f.write("// =============================================================================\n")
    f.write("// figma_screens_data.h — Exact Pixel-Perfect Figma UI/UX Flow Design Bitmaps\n")
    f.write("// Captured directly from Luna React/Tailwind/Orbitron engine at 240x280 resolution\n")
    f.write("// =============================================================================\n\n")
    f.write("#ifndef FIGMA_SCREENS_DATA_H\n")
    f.write("#define FIGMA_SCREENS_DATA_H\n\n")
    f.write("#include <pgmspace.h>\n")
    f.write("#include <stdint.h>\n")
    f.write("#include <Adafruit_GFX.h>\n\n")
    f.write("#define FIGMA_SCREEN_W 240\n")
    f.write("#define FIGMA_SCREEN_H 280\n\n")

    # 1. Convert each full screen
    for var_name, filename in screen_files:
        p = os.path.join(screens_dir, filename)
        if not os.path.exists(p):
            print(f"[ERROR] Missing {p}")
            continue
        im = Image.open(p).convert("RGB")
        w, h = im.size
        print(f"[CONVERT] {var_name} from {filename} ({w}x{h}) ...")
        f.write(f"// ── {var_name} ({w}x{h}) ──\n")
        f.write(f"const uint16_t {var_name}[{w * h}] PROGMEM = {{\n")
        
        pixels = []
        for y in range(h):
            row_pixels = []
            for x in range(w):
                r, g, b = im.getpixel((x, y))
                val = r888_to_565(r, g, b)
                row_pixels.append(f"0x{val:04X}")
            pixels.append("  " + ", ".join(row_pixels))
        
        f.write(",\n".join(pixels))
        f.write("\n};\n\n")

    # 2. Extract Orbitron 58px Large Digits
    digits_img_path = os.path.join(screens_dir, "orbitron_digits_58.png")
    if os.path.exists(digits_img_path):
        dim = Image.open(digits_img_path).convert("RGB")
        w, h = dim.size
        cols = [any(sum(dim.getpixel((x, y))) > 80 for y in range(h)) for x in range(w)]
        glyphs = []
        in_g = False
        start = 0
        for x, v in enumerate(cols):
            if v and not in_g:
                in_g = True
                start = x
            elif not v and in_g:
                in_g = False
                glyphs.append((start, x))
        if in_g:
            glyphs.append((start, w))

        rows = [any(sum(dim.getpixel((x, y))) > 80 for x in range(w)) for y in range(h)]
        ymin = next(i for i, v in enumerate(rows) if v)
        ymax = max(i for i, v in enumerate(rows) if v)
        gh = ymax - ymin + 1

        f.write("// ── Orbitron 58px Digits (0-9 and colon) ──\n")
        glyph_names = ["0","1","2","3","4","5","6","7","8","9","colon"]
        for idx, (sx, ex) in enumerate(glyphs[:11]):
            gname = glyph_names[idx]
            gw = ex - sx
            f.write(f"const uint16_t orbitron_58_g_{gname}[{gw * gh}] PROGMEM = {{\n")
            gpixels = []
            for y in range(ymin, ymax + 1):
                row = []
                for x in range(sx, ex):
                    r, g, b = dim.getpixel((x, y))
                    row.append(f"0x{r888_to_565(r, g, b):04X}")
                gpixels.append("  " + ", ".join(row))
            f.write(",\n".join(gpixels))
            f.write("\n};\n\n")

        f.write("struct OrbitronGlyph {\n")
        f.write("  uint8_t w;\n")
        f.write("  uint8_t h;\n")
        f.write("  const uint16_t* data;\n")
        f.write("};\n\n")

        f.write("const OrbitronGlyph orbitron_58_glyphs[11] PROGMEM = {\n")
        for idx in range(min(11, len(glyphs))):
            gname = glyph_names[idx]
            gw = glyphs[idx][1] - glyphs[idx][0]
            f.write(f"  {{ {gw}, {gh}, orbitron_58_g_{gname} }},\n")
        f.write("};\n\n")

    # 3. Extract Orbitron 18px Seconds Digits
    sec_img_path = os.path.join(screens_dir, "orbitron_sec_18.png")
    if os.path.exists(sec_img_path):
        sim = Image.open(sec_img_path).convert("RGB")
        w, h = sim.size
        cols = [any(sum(sim.getpixel((x, y))) > 60 for y in range(h)) for x in range(w)]
        sglyphs = []
        in_g = False
        start = 0
        for x, v in enumerate(cols):
            if v and not in_g:
                in_g = True
                start = x
            elif not v and in_g:
                in_g = False
                sglyphs.append((start, x))
        if in_g:
            sglyphs.append((start, w))

        rows = [any(sum(sim.getpixel((x, y))) > 60 for x in range(w)) for y in range(h)]
        symin = next(i for i, v in enumerate(rows) if v)
        symax = max(i for i, v in enumerate(rows) if v)
        sgh = symax - symin + 1

        f.write("// ── Orbitron 18px Seconds Digits (0-9 and colon) ──\n")
        for idx, (sx, ex) in enumerate(sglyphs[:11]):
            gname = glyph_names[idx]
            gw = ex - sx
            f.write(f"const uint16_t orbitron_18_g_{gname}[{gw * sgh}] PROGMEM = {{\n")
            gpixels = []
            for y in range(symin, symax + 1):
                row = []
                for x in range(sx, ex):
                    r, g, b = sim.getpixel((x, y))
                    row.append(f"0x{r888_to_565(r, g, b):04X}")
                gpixels.append("  " + ", ".join(row))
            f.write(",\n".join(gpixels))
            f.write("\n};\n\n")

        f.write("const OrbitronGlyph orbitron_18_glyphs[11] PROGMEM = {\n")
        for idx in range(min(11, len(sglyphs))):
            gname = glyph_names[idx]
            gw = sglyphs[idx][1] - sglyphs[idx][0]
            f.write(f"  {{ {gw}, {sgh}, orbitron_18_g_{gname} }},\n")
        f.write("};\n\n")

    # Helper rendering functions
    f.write("""
// ── Helper: Draw a transparent-black RGB565 sprite onto GFXcanvas16 ───────────
inline void figmaDrawSprite(GFXcanvas16& d, int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* sprite) {
  uint16_t* buf = d.getBuffer();
  for (int16_t r = 0; r < h; r++) {
    int16_t cy = y + r;
    if (cy < 0 || cy >= 280) continue;
    for (int16_t c = 0; c < w; c++) {
      int16_t cx = x + c;
      if (cx < 0 || cx >= 240) continue;
      uint16_t pix = pgm_read_word(&sprite[r * w + c]);
      if (pix != 0x0000) {
        buf[cy * 240 + cx] = pix;
      }
    }
  }
}

// ── Helper: Draw live time in exact Orbitron font (58px main + 18px seconds) ─
inline void figmaDrawOrbitronLiveTime(GFXcanvas16& d, int hour, int minute, int second, bool is12Hour) {
  int dispHour = hour;
  if (is12Hour) {
    dispHour = hour % 12;
    if (dispHour == 0) dispHour = 12;
  }
  char timeStr[6];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", dispHour, minute);

  // Measure total width of the main time string
  int totalW = 0;
  for (int i = 0; i < 5; i++) {
    char ch = timeStr[i];
    int gIdx = (ch >= '0' && ch <= '9') ? (ch - '0') : 10;
    OrbitronGlyph g;
    memcpy_P(&g, &orbitron_58_glyphs[gIdx], sizeof(OrbitronGlyph));
    totalW += g.w;
    if (i < 4) totalW += 1; // 1px tracking
  }

  // Draw main time centered at Y=52
  int curX = (240 - (totalW + 28)) / 2 - 2;
  int curY = 52;
  for (int i = 0; i < 5; i++) {
    char ch = timeStr[i];
    int gIdx = (ch >= '0' && ch <= '9') ? (ch - '0') : 10;
    OrbitronGlyph g;
    memcpy_P(&g, &orbitron_58_glyphs[gIdx], sizeof(OrbitronGlyph));
    figmaDrawSprite(d, curX, curY, g.w, g.h, g.data);
    curX += g.w + 1;
  }

  // Draw seconds :ss in sky-blue (18px) to the right
  char secStr[4];
  snprintf(secStr, sizeof(secStr), ":%02d", second);
  int secX = curX + 2;
  int secY = curY + 6;
  for (int i = 0; i < 3; i++) {
    char ch = secStr[i];
    int gIdx = (ch >= '0' && ch <= '9') ? (ch - '0') : 10;
    OrbitronGlyph g;
    memcpy_P(&g, &orbitron_18_glyphs[gIdx], sizeof(OrbitronGlyph));
    figmaDrawSprite(d, secX, secY, g.w, g.h, g.data);
    secX += g.w + 1;
  }

  // AM/PM indicator if 12-hour
  if (is12Hour) {
    d.setTextSize(1);
    d.setTextColor(0x632C); // dimmed white
    d.setCursor(curX + 4, curY + 24);
    d.print(hour >= 12 ? "PM" : "AM");
  }
}

#endif // FIGMA_SCREENS_DATA_H
""")

print("[CONVERT] Finished generating figma_screens_data.h!")
