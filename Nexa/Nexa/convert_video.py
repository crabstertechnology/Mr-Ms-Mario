import cv2
import io
import os
import sys
from PIL import Image

def convert_video_to_header(video_path, output_header_path, quality=60, target_size=(240, 240)):
    if not os.path.exists(video_path):
        print(f"Error: Video file not found at {video_path}")
        return False

    cap = cv2.VideoCapture(video_path)
    fps = cap.get(cv2.CAP_PROP_FPS) or 24.0
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    print(f"Loading video: {video_path}")
    print(f"Total input frames: {total_frames}, FPS: {fps}")

    frame_bytes_list = []
    frame_idx = 0

    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        # Convert BGR to RGB
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        img = Image.fromarray(rgb)
        
        if img.size != target_size:
            img = img.resize(target_size, Image.Resampling.LANCZOS)
            
        buf = io.BytesIO()
        img.save(buf, format="JPEG", quality=quality, optimize=True)
        jpeg_bytes = buf.getvalue()
        frame_bytes_list.append(jpeg_bytes)
        frame_idx += 1

    cap.release()
    print(f"Encoded {len(frame_bytes_list)} frames. Generating C++ header...")

    total_size = sum(len(b) for b in frame_bytes_list)
    print(f"Total JPEG binary size: {total_size / 1024:.2f} KB ({total_size / (1024*1024):.2f} MB)")

    with open(output_header_path, "w", encoding="utf-8") as f:
        f.write("// Auto-generated video frames header\n")
        f.write("#ifndef VIDEO_FRAMES_H\n")
        f.write("#define VIDEO_FRAMES_H\n\n")
        f.write("#include <Arduino.h>\n")
        f.write("#include <pgmspace.h>\n\n")
        f.write(f"#define TOTAL_VIDEO_FRAMES {len(frame_bytes_list)}\n")
        f.write(f"#define VIDEO_FRAME_WIDTH {target_size[0]}\n")
        f.write(f"#define VIDEO_FRAME_HEIGHT {target_size[1]}\n")
        f.write(f"#define VIDEO_FPS {int(fps)}\n")
        f.write(f"#define VIDEO_FRAME_DELAY_MS {int(1000 / fps)}\n\n")

        # Write each frame array
        for i, f_bytes in enumerate(frame_bytes_list):
            f.write(f"const uint8_t frame_{i:03d}[] PROGMEM = {{\n")
            hex_data = [f"0x{b:02X}" for b in f_bytes]
            # Write 16 bytes per line
            for chunk_start in range(0, len(hex_data), 16):
                f.write("  " + ", ".join(hex_data[chunk_start:chunk_start+16]) + ",\n")
            f.write("};\n\n")

        # Write array of pointers and size table
        f.write("const uint8_t* const video_frames[TOTAL_VIDEO_FRAMES] PROGMEM = {\n")
        for i in range(len(frame_bytes_list)):
            f.write(f"  frame_{i:03d},\n")
        f.write("};\n\n")

        f.write("const uint32_t video_frame_sizes[TOTAL_VIDEO_FRAMES] PROGMEM = {\n")
        for i, f_bytes in enumerate(frame_bytes_list):
            f.write(f"  {len(f_bytes)},\n")
        f.write("};\n\n")

        f.write("#endif // VIDEO_FRAMES_H\n")

    print(f"Header generated successfully at: {output_header_path}")
    return True

if __name__ == "__main__":
    video_path = r"D:\FounderOS\Nexa\Animation\Character_performing_360.mp4"
    output_header = r"D:\FounderOS\Nexa\VideoPlayer\video_frames.h"
    convert_video_to_header(video_path, output_header, quality=58, target_size=(240, 240))
