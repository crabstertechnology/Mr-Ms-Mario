import os
import re
from PIL import Image

def rgb565_to_rgb888(val):
    r = ((val >> 11) & 0x1F) * 255 // 31
    g = ((val >> 5) & 0x3F) * 255 // 63
    b = (val & 0x1F) * 255 // 31
    return (r, g, b)

def extract_gifs_from_header(header_path, output_dir):
    os.makedirs(output_dir, exist_ok=True)
    with open(header_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Find all arrays luna_sprite_ai_animX_frame_YY
    pattern = re.compile(r'const\s+uint16_t\s+luna_sprite_ai_anim(\d+)_frame_(\d+)\[\d+\]\s*PROGMEM\s*=\s*\{([^}]+)\};', re.DOTALL)
    matches = pattern.findall(content)

    anims = {}
    for anim_idx, frame_idx, data_str in matches:
        anim_idx = int(anim_idx)
        frame_idx = int(frame_idx)
        
        # Parse hex values
        raw_vals = [int(v.strip(), 16) for v in data_str.split(',') if v.strip()]
        if anim_idx not in anims:
            anims[anim_idx] = {}
        
        # Convert RGB565 uint16 to PIL Image (240x240)
        img = Image.new('RGB', (240, 240))
        pixels = []
        for v in raw_vals:
            pixels.append(rgb565_to_rgb888(v))
        img.putdata(pixels)
        anims[anim_idx][frame_idx] = img

    print(f"Parsed {len(anims)} animations from header.")

    for anim_idx, frames in anims.items():
        sorted_frame_indices = sorted(frames.keys())
        frame_images = [frames[i] for i in sorted_frame_indices]
        
        output_gif_path = os.path.join(output_dir, f"sprite_ai_{anim_idx}.gif")
        if frame_images:
            # Save animated GIF (duration 125ms per frame = 8 FPS)
            frame_images[0].save(
                output_gif_path,
                save_all=True,
                append_images=frame_images[1:],
                duration=125,
                loop=0
            )
            print(f"Saved {output_gif_path} with {len(frame_images)} frames.")

if __name__ == "__main__":
    header = r"w:\Mr.mario\1.69 Luna Firmware\sprite_ai_data.h"
    out_dir = r"w:\Mr.mario\mobile_flutter\assets\animations"
    extract_gifs_from_header(header, out_dir)
