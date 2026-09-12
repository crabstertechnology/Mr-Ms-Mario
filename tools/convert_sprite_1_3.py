# -*- coding: utf-8 -*-
"""
convert_sprite_1_3.py
Extracts all 12 animations (4 frames each) from the 1.69" sprite_ai_data.h,
downscales each frame from 240x240 → 120x120 using bilinear sampling,
and writes sprite_ai_data_1_3.h for the ESP32-C3 (4MB flash) 1.3" firmware.

Usage:
    python tools/convert_sprite_1_3.py

Output:
    1.3 Luna Firmware/sprite_ai_data_1_3.h   (~1.4MB)
"""

import re, os, sys, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

SRC_W, SRC_H = 240, 240
DST_W, DST_H = 120, 120
ANIM_COUNT   = 12
FRAME_COUNT  = 4

SRC_FILE = os.path.join(os.path.dirname(__file__), "..", "1.69 Luna Firmware", "sprite_ai_data.h")
DST_FILE = os.path.join(os.path.dirname(__file__), "..", "1.3 Luna Firmware", "sprite_ai_data_1_3.h")

# ---------------------------------------------------------------------------
def rgb565_bilinear(src_pixels, sx, sy):
    """
    Bilinear sample of src_pixels[sy][sx] in a flat list (SRC_W × SRC_H).
    Returns an RGB565 uint16_t.
    """
    x0 = int(sx);  y0 = int(sy)
    x1 = min(x0 + 1, SRC_W - 1)
    y1 = min(y0 + 1, SRC_H - 1)
    tx = sx - x0;  ty = sy - y0

    def rgb565_to_rgb(p):
        r = ((p >> 11) & 0x1F) << 3
        g = ((p >>  5) & 0x3F) << 2
        b = ( p        & 0x1F) << 3
        return r, g, b

    def interp(a, b, t):
        return a + (b - a) * t

    p00 = src_pixels[y0 * SRC_W + x0]
    p10 = src_pixels[y0 * SRC_W + x1]
    p01 = src_pixels[y1 * SRC_W + x0]
    p11 = src_pixels[y1 * SRC_W + x1]

    r = int(interp(interp(rgb565_to_rgb(p00)[0], rgb565_to_rgb(p10)[0], tx),
                   interp(rgb565_to_rgb(p01)[0], rgb565_to_rgb(p11)[0], tx), ty))
    g = int(interp(interp(rgb565_to_rgb(p00)[1], rgb565_to_rgb(p10)[1], tx),
                   interp(rgb565_to_rgb(p01)[1], rgb565_to_rgb(p11)[1], tx), ty))
    b = int(interp(interp(rgb565_to_rgb(p00)[2], rgb565_to_rgb(p10)[2], tx),
                   interp(rgb565_to_rgb(p01)[2], rgb565_to_rgb(p11)[2], tx), ty))

    r = min(255, max(0, r)) >> 3
    g = min(255, max(0, g)) >> 2
    b = min(255, max(0, b)) >> 3
    return (r << 11) | (g << 5) | b

# ---------------------------------------------------------------------------
def downscale_frame(flat_pixels):
    """Downscale a 240×240 flat list of uint16_t → 120×120."""
    result = []
    for dy in range(DST_H):
        sy = dy * (SRC_H - 1) / (DST_H - 1) if DST_H > 1 else 0
        for dx in range(DST_W):
            sx = dx * (SRC_W - 1) / (DST_W - 1) if DST_W > 1 else 0
            result.append(rgb565_bilinear(flat_pixels, sx, sy))
    return result

# ---------------------------------------------------------------------------
def parse_frame_array(text):
    """Extract uint16_t values from a PROGMEM array body text."""
    hex_vals = re.findall(r'0x([0-9A-Fa-f]{4})', text)
    return [int(h, 16) for h in hex_vals]

# ---------------------------------------------------------------------------
def write_array(f, name, pixels):
    f.write(f"const uint16_t {name}[{DST_W * DST_H}] PROGMEM = {{\n")
    per_line = 12
    for i, val in enumerate(pixels):
        if i % per_line == 0:
            f.write("  ")
        f.write(f"0x{val:04X}")
        if i < len(pixels) - 1:
            f.write(", ")
        if (i + 1) % per_line == 0:
            f.write("\n")
    if len(pixels) % per_line != 0:
        f.write("\n")
    f.write("};\n\n")

# ---------------------------------------------------------------------------
def main():
    print(f"Reading {SRC_FILE} ...")
    if not os.path.exists(SRC_FILE):
        print(f"ERROR: Source file not found:\n  {SRC_FILE}")
        sys.exit(1)

    with open(SRC_FILE, 'r', encoding='utf-8', errors='replace') as fh:
        content = fh.read()

    # Find all frame arrays by regex
    # Pattern: const uint16_t luna_sprite_ai_animN_frame_FF[57600] PROGMEM = { ... };
    pattern = re.compile(
        r'const\s+uint16_t\s+(luna_sprite_ai_anim(\d+)_frame_(\d+))\[57600\]\s+PROGMEM\s*=\s*\{([^}]+)\};',
        re.DOTALL
    )

    frames = {}  # (anim_idx, frame_idx) -> array_name
    arrays = {}  # array_name -> [pixels...]

    matches = list(pattern.finditer(content))
    if not matches:
        print("ERROR: No frames found — check pattern or source file.")
        sys.exit(1)

    total = len(matches)
    print(f"Found {total} frame arrays. Processing ...")

    for i, m in enumerate(matches):
        arr_name = m.group(1)
        anim_idx = int(m.group(2))
        frame_idx = int(m.group(3))
        body = m.group(4)

        src_pixels = parse_frame_array(body)
        if len(src_pixels) != SRC_W * SRC_H:
            print(f"  WARNING: {arr_name} has {len(src_pixels)} pixels (expected {SRC_W*SRC_H}), padding/truncating")
            src_pixels = (src_pixels + [0] * (SRC_W * SRC_H))[:SRC_W * SRC_H]

        print(f"  [{i+1}/{total}] Downscaling {arr_name} 240x240 -> 120x120 ...")
        dst_pixels = downscale_frame(src_pixels)

        new_name = f"luna_sprite_ai13_anim{anim_idx}_frame_{frame_idx:02d}"
        arrays[new_name] = dst_pixels
        frames[(anim_idx, frame_idx)] = new_name

    print(f"\nWriting {DST_FILE} ...")
    with open(DST_FILE, 'w', encoding='utf-8') as f:
        f.write("// ============================================================================\n")
        f.write("// GENERATED FILE — DO NOT EDIT MANUALLY.\n")
        f.write("// SOURCE: Luna UI Studio\n")
        f.write("// REGENERATE FROM STUDIO.\n")
        f.write("// Tool: tools/convert_sprite_1_3.py\n")
        f.write("// Downscaled from 240x240 → 120x120 RGB565 for ESP32-C3 4MB flash.\n")
        f.write("// ============================================================================\n\n")
        f.write("#ifndef SPRITE_AI_DATA_1_3_H\n")
        f.write("#define SPRITE_AI_DATA_1_3_H\n\n")
        f.write("#include <pgmspace.h>\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"#define SPRITE_AI13_FRAME_WIDTH      {DST_W}\n")
        f.write(f"#define SPRITE_AI13_FRAME_HEIGHT     {DST_H}\n")
        f.write(f"#define SPRITE_AI13_ANIMATION_COUNT  {ANIM_COUNT}\n")
        f.write(f"#define SPRITE_AI13_FRAME_COUNT      {FRAME_COUNT}\n")
        f.write(f"#define SPRITE_AI13_DEFAULT_FPS      8\n\n")

        # Write all frame arrays
        for (anim_idx, frame_idx) in sorted(frames.keys()):
            name = frames[(anim_idx, frame_idx)]
            write_array(f, name, arrays[name])

        # Write getSpriteAi13Frame() accessor
        f.write("inline const uint16_t* getSpriteAi13Frame(int animIdx, int frameIdx) {\n")
        f.write(f"  if (animIdx < 0 || animIdx >= {ANIM_COUNT}) animIdx = 0;\n")
        f.write(f"  if (frameIdx < 0 || frameIdx >= {FRAME_COUNT}) frameIdx = 0;\n")
        f.write("  switch (animIdx) {\n")
        for a in range(ANIM_COUNT):
            f.write(f"    case {a}:\n")
            f.write("      switch (frameIdx) {\n")
            for fr in range(FRAME_COUNT):
                name = frames.get((a, fr), frames.get((0, 0)))
                f.write(f"        case {fr}: return {name};\n")
            f.write("      }\n")
            f.write("      break;\n")
        f.write("  }\n")
        f.write(f"  return {frames.get((0,0), 'luna_sprite_ai13_anim0_frame_00')};\n")
        f.write("}\n\n")
        f.write("#endif // SPRITE_AI_DATA_1_3_H\n")

    size = os.path.getsize(DST_FILE)
    print(f"\nDone! Output: {DST_FILE}")
    print(f"File size: {size/1024:.1f} KB ({size/1024/1024:.2f} MB)")
    print(f"Pixel count: {DST_W}x{DST_H} x {ANIM_COUNT} anims x {FRAME_COUNT} frames = {DST_W*DST_H*ANIM_COUNT*FRAME_COUNT} uint16_t")
    print("Next step: include sprite_ai_data_1_3.h in 1.3 Luna Firmware/expressions.h")

if __name__ == "__main__":
    main()
