import os
import sys
import glob
import json
import subprocess
import time

def get_video_dimensions(file_path):
    cmd = [
        "ffprobe", "-v", "error",
        "-select_streams", "v:0",
        "-show_entries", "stream=width,height",
        "-of", "json",
        file_path
    ]
    try:
        res = subprocess.run(cmd, capture_output=True, text=True, check=True)
        data = json.loads(res.stdout)
        width = int(data["streams"][0]["width"])
        height = int(data["streams"][0]["height"])
        return width, height
    except Exception:
        return 1920, 1080

def process_file(file_path):
    directory = os.path.dirname(file_path)
    base_name = os.path.splitext(os.path.basename(file_path))[0]
    
    # Skip already cropped or converted files
    if "_6x7" in base_name or "cropped" in base_name or "240x280" in base_name:
        return
        
    out_240x280 = os.path.join(directory, f"{base_name}_240x280_6x7.mp4")
    out_hd = os.path.join(directory, f"{base_name}_cropped_6x7.mp4")
    
    print(f"\n==========================================")
    print(f"Processing: {os.path.basename(file_path)}")
    print(f"==========================================")
    
    width, height = get_video_dimensions(file_path)
    
    # Calculate 6:7 aspect ratio crop
    if width == 1920 and height == 1080:
        # Standard landscape 16:9 -> 6:7
        crop_w, crop_h, crop_x, crop_y = 926, 1080, 497, 0
    elif width == 1080 and height == 1920:
        # Portrait 9:16 -> 6:7 (1080x1260 window)
        # Content (characters + logo) is situated between Y=770 and Y=1410
        # Optimal visual centering offset: Y=460 (1080x1260 window from Y=460 to Y=1720)
        crop_w, crop_h, crop_x, crop_y = 1080, 1260, 0, 460
    else:
        # General dynamic calculation
        target_aspect = 6.0 / 7.0
        current_aspect = width / height
        if current_aspect > target_aspect:
            # Video is wider than 6:7
            crop_h = height
            crop_w = int(round(height * target_aspect))
            if crop_w % 2 != 0:
                crop_w -= 1
            crop_x = (width - crop_w) // 2
            crop_y = 0
        else:
            # Video is taller than 6:7
            crop_w = width
            crop_h = int(round(width / target_aspect))
            if crop_h % 2 != 0:
                crop_h -= 1
            crop_x = 0
            crop_y = (height - crop_h) // 2

    crop_filter = f"crop={crop_w}:{crop_h}:{crop_x}:{crop_y}"
    print(f"Input: {width}x{height} -> Crop Filter: {crop_filter}")

    # 1. Hardware resolution 240x280
    cmd_hw = [
        "ffmpeg", "-y", "-i", file_path,
        "-filter:v", f"{crop_filter},scale=240:280:flags=lanczos",
        "-c:a", "copy",
        out_240x280
    ]
    t0 = time.time()
    res_hw = subprocess.run(cmd_hw, capture_output=True, text=True)
    if res_hw.returncode == 0:
        size_kb = os.path.getsize(out_240x280) / 1024
        print(f" -> Generated 240x280: {os.path.basename(out_240x280)} ({size_kb:.1f} KB, {time.time()-t0:.1f}s)")
    else:
        print(f" -> Error generating 240x280: {res_hw.stderr[-200:]}")
    
    # 2. Master HD crop
    cmd_hd = [
        "ffmpeg", "-y", "-i", file_path,
        "-filter:v", crop_filter,
        "-c:a", "copy",
        out_hd
    ]
    t0 = time.time()
    res_hd = subprocess.run(cmd_hd, capture_output=True, text=True)
    if res_hd.returncode == 0:
        size_kb = os.path.getsize(out_hd) / 1024
        print(f" -> Generated HD Crop: {os.path.basename(out_hd)} ({size_kb:.1f} KB, {time.time()-t0:.1f}s)")
    else:
        print(f" -> Error generating HD crop: {res_hd.stderr[-200:]}")

def main():
    if len(sys.argv) > 1:
        for arg in sys.argv[1:]:
            # If multiple paths separated by comma or space
            paths = [p.strip() for p in arg.split(",") if p.strip()]
            for target_path in paths:
                if os.path.isfile(target_path):
                    process_file(target_path)
                elif os.path.isdir(target_path):
                    files = sorted(glob.glob(os.path.join(target_path, "*.mp4")))
                    for f in files:
                        process_file(f)
                else:
                    print(f"Path not found: {target_path}")
    else:
        target_dirs = [
            r"W:\Mr.mario\1.69 Luna Firmware\ai video"
        ]
        for d in target_dirs:
            if not os.path.exists(d):
                continue
            print(f"\nScanning directory: {d}")
            files = sorted(glob.glob(os.path.join(d, "*.mp4")))
            for f in files:
                process_file(f)

if __name__ == "__main__":
    main()
