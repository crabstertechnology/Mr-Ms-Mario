"""
Analyze all GIF files in mobile_app/animations/ to extract:
  - Frame count
  - Per-frame delay (from GIF header)
  - Total duration
  - Recommended frameDelayMs for ESP32 OLED playback
"""
import os
import struct

ANIM_DIR = os.path.join(os.path.dirname(os.path.dirname(__file__)), "mobile_app", "animations")

def read_gif_timing(path):
    """Extract frame count and average delay from a GIF file."""
    try:
        with open(path, "rb") as f:
            data = f.read()
    except Exception as e:
        return None, None, str(e)

    if data[:6] not in (b"GIF87a", b"GIF89a"):
        return None, None, "Not a GIF"

    delays = []
    i = 6
    # Read Logical Screen Descriptor (7 bytes)
    if i + 7 > len(data):
        return None, None, "Too short"
    packed = data[i + 4]
    has_gct = (packed >> 7) & 1
    gct_size = packed & 0x07
    i += 7
    if has_gct:
        i += 6 * (2 ** (gct_size + 1))

    frame_count = 0
    while i < len(data):
        if data[i] == 0x3B:  # Trailer
            break
        elif data[i] == 0x2C:  # Image Descriptor
            frame_count += 1
            i += 1
            if i + 9 > len(data):
                break
            packed2 = data[i + 8]
            has_lct = (packed2 >> 7) & 1
            lct_size = packed2 & 0x07
            i += 9
            if has_lct:
                i += 6 * (2 ** (lct_size + 1))
            # Skip LZW min code size + sub-blocks
            i += 1  # lzw minimum code size
            while i < len(data):
                block_size = data[i]
                i += 1 + block_size
                if block_size == 0:
                    break
        elif data[i] == 0x21:  # Extension
            i += 1
            if i >= len(data):
                break
            ext_type = data[i]
            i += 1
            if ext_type == 0xF9 and i < len(data):  # Graphic Control Extension
                block_size = data[i]
                i += 1
                if block_size >= 4 and i + block_size <= len(data):
                    delay_cs = struct.unpack_from("<H", data, i + 1)[0]  # centiseconds
                    if delay_cs > 0:
                        delays.append(delay_cs * 10)  # to ms
                i += block_size
                # skip terminator
                while i < len(data) and data[i] != 0:
                    block_size2 = data[i]
                    i += 1 + block_size2
                if i < len(data):
                    i += 1  # block terminator
            else:
                # skip other extensions
                while i < len(data):
                    block_size = data[i]
                    i += 1
                    if block_size == 0:
                        break
                    i += block_size
        else:
            i += 1

    if frame_count == 0:
        frame_count = 1
    avg_delay = int(sum(delays) / len(delays)) if delays else 100
    total_ms = sum(delays) if delays else avg_delay * frame_count
    return frame_count, avg_delay, total_ms

print(f"{'Name':<20} {'Frames':>6} {'Avg Delay':>10} {'Total ms':>10} {'Rec OLED ms':>12}")
print("-" * 62)

results = []
for fname in sorted(os.listdir(ANIM_DIR)):
    if not fname.lower().endswith(".gif"):
        continue
    name = fname[:-4].upper()
    path = os.path.join(ANIM_DIR, fname)
    frames, avg_delay, total = read_gif_timing(path)
    if frames is None:
        print(f"{name:<20} ERROR: {total}")
        continue
    # For OLED: the ESP32 needs to render each bitmap AND call display()
    # Adafruit SSD1306 display() takes ~40-50ms on I2C
    # So we subtract ~45ms from the GIF delay to keep correct speed
    RENDER_OVERHEAD_MS = 45
    oled_delay = max(10, avg_delay - RENDER_OVERHEAD_MS)
    results.append((name, frames, avg_delay, total, oled_delay))
    print(f"{name:<20} {frames:>6} {avg_delay:>9}ms {total:>9}ms {oled_delay:>11}ms")

print()
print("// ── ESP32 per-GIF frame delay table (for expressions.h) ──")
print("// Format: { \"NAME\", frames, oled_delay_ms }")
for name, frames, avg_delay, total, oled_delay in results:
    print(f'  {{ "{name}", {frames:3d}, {oled_delay:3d} }},  // orig={avg_delay}ms/frame, total={total}ms')
