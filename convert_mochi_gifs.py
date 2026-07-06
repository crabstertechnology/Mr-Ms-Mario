import os
from PIL import Image

# All 63 original Mochi GIFs from watcher-mochi/sd_content
ALL_GIFS = [
    "adore", "angry", "blank", "blinding", "brave", "buzzing",
    "contempt", "crying", "dancing", "devil", "distracted", "dizzy",
    "down", "drowsy", "encouragement", "energetic", "enraged", "evil",
    "fast", "fierce", "furious", "giggle", "glowing", "growing",
    "handsome", "happy", "hello", "irritated", "laughing", "left",
    "love", "menacing", "mistake", "playful", "police", "rain",
    "relaxed", "right", "rush", "scared", "serene", "shrink",
    "shy", "sick", "sleepy", "smile", "smirk", "smoke",
    "sneeze", "sobbing", "sparkle", "speed", "splash", "spraying",
    "squint", "surprised", "sushi", "swinging", "teasing", "tough",
    "weeping", "wink", "yawn"
]

output_path = "luna_firmware/mochi_bitmaps.h"
gif_dir = "watcher-mochi/sd_content"

print(f"Starting conversion of {len(ALL_GIFS)} GIFs...")

with open(output_path, "w") as f:
    f.write("#ifndef MOCHI_BITMAPS_H\n")
    f.write("#define MOCHI_BITMAPS_H\n\n")
    f.write("#include <pgmspace.h>\n\n")

    converted = []

    for name in ALL_GIFS:
        gif_path = os.path.join(gif_dir, f"{name}.gif")
        if not os.path.exists(gif_path):
            print(f"  [SKIP] {gif_path} not found!")
            continue

        print(f"  Converting {name}.gif ...")
        im = Image.open(gif_path)
        frames = []

        # Determine frame indices to sample (max 10 frames)
        total_frames = im.n_frames
        max_frames = 10
        if total_frames <= max_frames:
            indices = list(range(total_frames))
        else:
            indices = [int(i * (total_frames - 1) / (max_frames - 1)) for i in range(max_frames)]

        # Extract and convert frames
        for f_idx in indices:
            im.seek(f_idx)
            frame = im.convert("RGBA")
            frame_resized = frame.resize((128, 64), Image.Resampling.LANCZOS)
            gray = frame_resized.convert("L")

            byte_array = bytearray(1024)
            for y in range(64):
                for x in range(128):
                    val = gray.getpixel((x, y))
                    is_white = 1 if val > 45 else 0
                    if is_white:
                        byte_array[(y * 128 + x) // 8] |= (1 << (7 - (x % 8)))
            frames.append(byte_array)

        # Write individual frame arrays
        for i, frame_data in enumerate(frames):
            f.write(f"const unsigned char ep_{name}_frame{i}[] PROGMEM = {{\n")
            bytes_str = [f"0x{b:02x}" for b in frame_data]
            for idx in range(0, len(bytes_str), 16):
                line = ", ".join(bytes_str[idx:idx+16])
                f.write(f"  {line},\n")
            f.write("};\n\n")

        # Write pointer array
        f.write(f"const unsigned char* const ep_{name}_frames[] PROGMEM = {{\n")
        for i in range(len(frames)):
            f.write(f"  ep_{name}_frame{i},\n")
        f.write("};\n\n")
        f.write(f"const int ep_{name}_frame_count = {len(frames)};\n\n")

        converted.append(name)
        print(f"  Done: {name} ({len(frames)} frames)")

    # Write the master GIF lookup table used by firmware
    f.write("// ============================================================\n")
    f.write("// Master GIF lookup table - all converted animations\n")
    f.write("// ============================================================\n\n")
    f.write("struct GifEntry {\n")
    f.write("  const unsigned char* const* frames;\n")
    f.write("  int count;\n")
    f.write("  const char* name;\n")
    f.write("};\n\n")
    f.write("const GifEntry ALL_GIFS_TABLE[] PROGMEM = {\n")
    for name in converted:
        upper = name.upper()
        f.write(f"  {{ ep_{name}_frames, ep_{name}_frame_count, \"{upper}\" }},\n")
    f.write("};\n\n")
    f.write(f"const int ALL_GIFS_COUNT = {len(converted)};\n\n")

    f.write("#endif // MOCHI_BITMAPS_H\n")

print(f"\nConversion complete! {len(converted)}/{len(ALL_GIFS)} GIFs converted.")
print(f"Output: {output_path}")
