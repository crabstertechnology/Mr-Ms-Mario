import re
import os

src_file = os.path.join(os.path.dirname(__file__), "..", "1.69 Luna Firmware", "image_logo.h")
dst_file = os.path.join(os.path.dirname(__file__), "..", "1.3 Luna Firmware", "image_logo.h")

with open(src_file, 'r') as f:
    text = f.read()

vals = [int(x, 16) for x in re.findall(r'0x[0-9a-fA-F]+', text)]
print('Parsed 1.69 logo pixel count:', len(vals))

# 1.69 is 240x280
# Content is rows 48 to 253 (206 rows)
content_rows = []
for y in range(48, 254):
    content_rows.append(vals[y * 240 : (y + 1) * 240])

print('Content row count:', len(content_rows))

# Target 240x240:
# Top padding: 17 rows of 0xFFFF
# Bottom padding: 17 rows of 0xFFFF
out_pixels = []
for _ in range(17):
    out_pixels.extend([0xFFFF] * 240)

for row in content_rows:
    out_pixels.extend(row)

for _ in range(17):
    out_pixels.extend([0xFFFF] * 240)

print('Output pixel count:', len(out_pixels), 'Expected:', 240 * 240)
assert len(out_pixels) == 240 * 240

lines = []
lines.append('// Auto-generated Startup Logo Header from 1.69 logo.png (Centered 240x240 for 1.3" ST7789)')
lines.append('#ifndef IMAGE_LOGO_H')
lines.append('#define IMAGE_LOGO_H')
lines.append('')
lines.append('#include <pgmspace.h>')
lines.append('#include <Arduino.h>')
lines.append('')
lines.append('#define LOGO_WIDTH  240')
lines.append('#define LOGO_HEIGHT 240')
lines.append('')
lines.append('static const uint16_t PROGMEM image_logo_pixels[] = {')

for i in range(0, len(out_pixels), 16):
    chunk = out_pixels[i : i + 16]
    hex_strs = [f'0x{v:04X}' for v in chunk]
    lines.append('  ' + ', '.join(hex_strs) + ',')

lines.append('};')
lines.append('')
lines.append('#endif // IMAGE_LOGO_H')
lines.append('')

with open(dst_file, 'w') as f:
    f.write('\n'.join(lines))

print('Wrote new 1.3 image_logo.h successfully!')
