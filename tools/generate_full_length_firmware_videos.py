import os
import io
import cv2
from PIL import Image

VIDEO_DIR = r"W:\Mr.mario\1.69 Luna Firmware\new ai video"
LOGO_HEADER = r"W:\Mr.mario\1.69 Luna Firmware\logo_video_data.h"
PET_HEADER = r"W:\Mr.mario\1.69 Luna Firmware\video_frames_data.h"

TARGET_FPS = 10
FRAME_DELAY_MS = 100  # 10 FPS = 100 ms per frame
JPEG_QUALITY = 38

def sample_video_frames(vpath, target_fps, quality):
    cap = cv2.VideoCapture(vpath)
    count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    fps = cap.get(cv2.CAP_PROP_FPS)
    duration = count / fps if fps > 0 else 0
    target_count = max(1, int(round(duration * target_fps)))
    
    # Evenly distributed indices covering the ENTIRE video from frame 0 to count-1
    indices = [int(round(i * (count - 1) / (target_count - 1))) for i in range(target_count)] if target_count > 1 else [0]
    idx_set = set(indices)
    
    # Read and extract frames
    extracted = {}
    f_idx = 0
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        if f_idx in idx_set:
            rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            if (rgb.shape[1], rgb.shape[0]) != (240, 280):
                rgb = cv2.resize(rgb, (240, 280), interpolation=cv2.INTER_AREA)
            # Black clamp for clean screen bezels
            mask = (rgb[:, :, 0] < 8) & (rgb[:, :, 1] < 8) & (rgb[:, :, 2] < 8)
            rgb[mask] = 0
            buf = io.BytesIO()
            Image.fromarray(rgb).save(buf, format="JPEG", quality=quality, optimize=True)
            extracted[f_idx] = buf.getvalue()
        f_idx += 1
    cap.release()
    
    # Return frames in chronological sequence
    frames = [extracted[idx] for idx in indices if idx in extracted]
    return frames, duration

def generate_logo_header():
    vpath = os.path.join(VIDEO_DIR, "LOGO.mp4")
    print(f"\n[LOGO] Processing 100% full duration of {vpath}...")
    frames, duration = sample_video_frames(vpath, TARGET_FPS, JPEG_QUALITY)
    count = len(frames)
    total_bytes = sum(len(f) for f in frames)
    print(f" -> Extracted {count} frames covering full {duration:.2f}s ({total_bytes / 1024:.1f} KB total)")
    
    with open(LOGO_HEADER, "w", encoding="utf-8") as out:
        out.write("// Auto-generated SYNAPS Full-Length Intro Logo Animation Header\n")
        out.write("#ifndef LOGO_VIDEO_DATA_H\n#define LOGO_VIDEO_DATA_H\n\n")
        out.write("#include <Arduino.h>\n#include <pgmspace.h>\n\n")
        out.write(f"#define INTRO_LOGO_FRAME_COUNT    {count}\n")
        out.write(f"#define INTRO_LOGO_FPS            {TARGET_FPS}\n")
        out.write(f"#define INTRO_LOGO_FRAME_DELAY_MS {FRAME_DELAY_MS}\n\n")
        
        for idx, fbytes in enumerate(frames):
            arr_name = f"intro_logo_frame_{idx:03d}"
            out.write(f"const uint8_t {arr_name}[] PROGMEM = {{\n")
            lines = []
            for b_idx in range(0, len(fbytes), 20):
                chunk = fbytes[b_idx:b_idx+20]
                lines.append(", ".join(f"0x{b:02X}" for b in chunk))
            out.write(",\n".join(lines))
            out.write("\n};\n\n")
            
        out.write(f"const uint8_t* const intro_logo_frames[{count}] PROGMEM = {{\n")
        for idx in range(count):
            out.write(f"  intro_logo_frame_{idx:03d},\n")
        out.write("};\n\n")
        
        out.write(f"const uint32_t intro_logo_frame_sizes[{count}] PROGMEM = {{\n")
        for idx in range(count):
            out.write(f"  {len(frames[idx])},\n")
        out.write("};\n\n")
        out.write("#endif // LOGO_VIDEO_DATA_H\n")
    print(f"[LOGO] Successfully wrote {LOGO_HEADER}")

def generate_pet_animations_header():
    anim_definitions = [
        ("IDLE_240x280_6x7.mp4", "Luna Idle"),
        ("ANGRY_240x280_6x7.mp4", "Angry Face"),
        ("HUNGRY MENU_240x280_6x7.mp4", "Hungry Menu"),
        ("IDLE TO HUNGRY_240x280_6x7.mp4", "Getting Hungry"),
        ("FISH.mp4", "Eat Fish"),
        ("MILK.mp4", "Drink Milk"),
        ("SALAD.mp4", "Eat Salad"),
        ("SICK LOOP_240x280_6x7.mp4", "Luna Sick"),
        ("SICK TO RECOVERY.mp4", "Recovering"),
        ("IDLE TO SLEEP.mp4", "Going to Sleep"),
        ("SLEEP LOOP_240x280_6x7.mp4", "Sleeping Loop"),
        ("SLEEPING.mp4", "Deep Sleep"),
        ("THINKING.mp4", "Luna Thinking")
    ]
    
    print(f"\n[PET ANIMATIONS] Processing {len(anim_definitions)} videos for 100% full-duration playback...")
    
    all_frames_per_anim = []
    all_durations = []
    all_counts = []
    grand_bytes = 0
    
    for vname, label in anim_definitions:
        vpath = os.path.join(VIDEO_DIR, vname)
        frames, dur = sample_video_frames(vpath, TARGET_FPS, JPEG_QUALITY)
        all_frames_per_anim.append(frames)
        all_durations.append(dur)
        all_counts.append(len(frames))
        anim_sz = sum(len(f) for f in frames)
        grand_bytes += anim_sz
        print(f" -> {vname:32s} [{label:15s}]: {len(frames):3d} frames, full {dur:5.2f}s, {anim_sz/1024:7.1f} KB")
        
    print(f"\n[PET ANIMATIONS] Total: {len(anim_definitions)} animations, {sum(all_counts)} frames, {grand_bytes/(1024*1024):.2f} MB")
    
    with open(PET_HEADER, "w", encoding="utf-8") as out:
        out.write("// Auto-generated Full-Length Video Dataset for 1.69 Luna Firmware\n")
        out.write("#ifndef VIDEO_FRAMES_DATA_H\n#define VIDEO_FRAMES_DATA_H\n\n")
        out.write("#include <Arduino.h>\n#include <pgmspace.h>\n\n")
        out.write(f"#define TOTAL_ANIMATIONS   {len(anim_definitions)}\n")
        out.write("#define VIDEO_FRAME_WIDTH  240\n")
        out.write("#define VIDEO_FRAME_HEIGHT 280\n")
        out.write(f"#define ANIM_0_FRAME_COUNT {all_counts[0]}\n")
        out.write(f"#define ANIM_0_FPS         {TARGET_FPS}\n")
        out.write(f"#define ANIM_0_DELAY_MS    {FRAME_DELAY_MS}\n\n")
        
        # Write individual frame arrays
        for v_idx, (vname, label) in enumerate(anim_definitions):
            frames = all_frames_per_anim[v_idx]
            cnt = len(frames)
            out.write(f"// === Animation {v_idx}: anim{v_idx} ({cnt} frames @ {TARGET_FPS} FPS, {all_durations[v_idx]:.2f}s) [{vname} - {label}] ===\n")
            out.write(f"#define ANIM_{v_idx}_FRAME_COUNT {cnt}\n")
            out.write(f"#define ANIM_{v_idx}_FPS         {TARGET_FPS}\n")
            out.write(f"#define ANIM_{v_idx}_DELAY_MS    {FRAME_DELAY_MS}\n\n")
            
            for f_idx, fbytes in enumerate(frames):
                arr_name = f"anim{v_idx}_frame_{f_idx:03d}"
                out.write(f"const uint8_t {arr_name}[] PROGMEM = {{\n")
                lines = []
                for b_idx in range(0, len(fbytes), 20):
                    chunk = fbytes[b_idx:b_idx+20]
                    lines.append(", ".join(f"0x{b:02X}" for b in chunk))
                out.write(",\n".join(lines))
                out.write("\n};\n\n")
                
            out.write(f"const uint8_t* const anim{v_idx}_frames[ANIM_{v_idx}_FRAME_COUNT] PROGMEM = {{\n")
            for f_idx in range(cnt):
                out.write(f"  anim{v_idx}_frame_{f_idx:03d},\n")
            out.write("};\n\n")
            
            out.write(f"const uint32_t anim{v_idx}_sizes[ANIM_{v_idx}_FRAME_COUNT] PROGMEM = {{\n")
            for f_idx in range(cnt):
                out.write(f"  {len(frames[f_idx])},\n")
            out.write("};\n\n")
            
        # Write master lookup tables
        out.write("// Master Animation Lookup Tables\n")
        out.write(f"const uint8_t* const* const anim_frame_pointers[{len(anim_definitions)}] PROGMEM = {{\n")
        for v_idx in range(len(anim_definitions)):
            out.write(f"  anim{v_idx}_frames,\n")
        out.write("};\n\n")
        
        out.write(f"const uint32_t* const anim_size_pointers[{len(anim_definitions)}] PROGMEM = {{\n")
        for v_idx in range(len(anim_definitions)):
            out.write(f"  anim{v_idx}_sizes,\n")
        out.write("};\n\n")
        
        out.write(f"const int anim_frame_counts[{len(anim_definitions)}] PROGMEM = {{\n")
        out.write("  " + ", ".join(str(c) for c in all_counts) + "\n")
        out.write("};\n\n")
        
        out.write(f"const int anim_fps_list[{len(anim_definitions)}] PROGMEM = {{\n")
        out.write("  " + ", ".join([str(TARGET_FPS)] * len(anim_definitions)) + "\n")
        out.write("};\n\n")
        
        out.write(f"const int anim_delays[{len(anim_definitions)}] PROGMEM = {{\n")
        out.write("  " + ", ".join([str(FRAME_DELAY_MS)] * len(anim_definitions)) + "\n")
        out.write("};\n\n")
        
        out.write("#endif // VIDEO_FRAMES_DATA_H\n")
        
    print(f"[PET ANIMATIONS] Successfully wrote {PET_HEADER}")

if __name__ == "__main__":
    generate_logo_header()
    generate_pet_animations_header()
    print("\nALL FULL-LENGTH VIDEO HEADERS GENERATED SUCCESSFULLY!")
