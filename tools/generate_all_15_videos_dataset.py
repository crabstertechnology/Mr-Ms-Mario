import os
import io
import cv2
from PIL import Image

VIDEO_DIR = r"W:\Mr.mario\1.69 Luna Firmware\new ai video"
LOGO_HEADER = r"W:\Mr.mario\1.69 Luna Firmware\logo_video_data.h"
PET_HEADER = r"W:\Mr.mario\1.69 Luna Firmware\video_frames_data.h"

TARGET_FPS = 10
FRAME_DELAY_MS = 105  # 105 ms delay = ~9.5 FPS -> 75 frames = 7.87s (calmer, slightly slower, snappy & natural)
JPEG_QUALITY = 58      # Higher quality + INTER_AREA completely eliminates edge speckling / ringing noise

def extract_frames(vpath, target_count, quality):
    cap = cv2.VideoCapture(vpath)
    count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    fps = cap.get(cv2.CAP_PROP_FPS)
    dur = count / fps if fps > 0 else 0
    indices = [int(round(i * (count - 1) / (target_count - 1))) for i in range(target_count)] if target_count > 1 else [0]
    idx_set = set(indices)
    
    extracted = {}
    f_idx = 0
    while True:
        ret, frame = cap.read()
        if not ret: break
        if f_idx in idx_set:
            rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            # Area integration downsampling: prevents sinc ringing / Gibbs phenomenon speckles on high-contrast edges
            resized = cv2.resize(rgb, (240, 280), interpolation=cv2.INTER_AREA)
            # Black clamp for LCD bezels
            mask = (resized[:, :, 0] < 8) & (resized[:, :, 1] < 8) & (resized[:, :, 2] < 8)
            resized[mask] = 0
            buf = io.BytesIO()
            Image.fromarray(resized).save(buf, format="JPEG", quality=quality, optimize=True)
            extracted[f_idx] = buf.getvalue()
        f_idx += 1
    cap.release()
    
    frames = [extracted[idx] for idx in indices if idx in extracted]
    return frames, dur


def build_pet_dataset_header():
    anim_definitions = [
        ("IDLE.mp4", "Luna Idle", 75),
        ("ANGRY.mp4", "Angry Face", 75),
        ("HUNGRY MENU.mp4", "Hungry Menu", 75),
        ("IDLE TO HUNGRY.mp4", "Getting Hungry", 75),
        ("FISH.mp4", "Eat Fish", 75),
        ("MILK.mp4", "Drink Milk", 75),
        ("SALAD.mp4", "Eat Salad", 75),
        ("IDLE TO SICK.mp4", "Getting Sick", 75),
        ("SICK LOOP.mp4", "Luna Sick", 75),
        ("SICK TO IDLE.mp4", "Recovered", 75),
        ("IDLE TO SLEEP.mp4", "Going to Sleep", 75),
        ("SLEEP LOOP.mp4", "Sleeping", 38),
        ("SLEEP TO IDLE.mp4", "Waking Up", 75),
        ("THINKING.mp4", "Luna Thinking", 75),
    ]
    
    print(f"\n[PET ANIMATIONS] Processing {len(anim_definitions)} videos (Clean Edge INTER_AREA, Q{JPEG_QUALITY})...")
    
    all_frames_per_anim = []
    all_durations = []
    all_counts = []
    grand_bytes = 0
    
    for vname, label, target_cnt in anim_definitions:
        vpath = os.path.join(VIDEO_DIR, vname)
        frames, dur = extract_frames(vpath, target_cnt, JPEG_QUALITY)
        all_frames_per_anim.append(frames)
        all_durations.append(dur)
        all_counts.append(len(frames))
        anim_sz = sum(len(f) for f in frames)
        grand_bytes += anim_sz
        print(f" -> {vname:22s} [{label:15s}]: {len(frames):2d} frames, {anim_sz/1024:6.1f} KB")
        
    print(f"\n[PET ANIMATIONS] Total: {len(anim_definitions)} animations, {sum(all_counts)} frames, {grand_bytes/(1024*1024):.2f} MB")
    
    with open(PET_HEADER, "w", encoding="utf-8") as out:
        out.write("// Auto-generated Complete 14-Animation Video Dataset for 1.69 Luna Firmware (Clean Edge INTER_AREA, Q58)\n")
        out.write("#ifndef VIDEO_FRAMES_DATA_H\n#define VIDEO_FRAMES_DATA_H\n\n")
        out.write("#include <Arduino.h>\n#include <pgmspace.h>\n\n")
        out.write(f"#define TOTAL_ANIMATIONS   {len(anim_definitions)}\n")
        out.write("#define VIDEO_FRAME_WIDTH  240\n")
        out.write("#define VIDEO_FRAME_HEIGHT 280\n")
        out.write(f"#define ANIM_0_FRAME_COUNT {all_counts[0]}\n")
        out.write(f"#define ANIM_0_FPS         {TARGET_FPS}\n")
        out.write(f"#define ANIM_0_DELAY_MS    {FRAME_DELAY_MS}\n\n")
        
        # Write individual frame arrays
        for v_idx, (vname, label, target_cnt) in enumerate(anim_definitions):
            frames = all_frames_per_anim[v_idx]
            cnt = len(frames)
            out.write(f"// === Animation {v_idx}: anim{v_idx} ({cnt} frames @ {TARGET_FPS} FPS, {cnt*FRAME_DELAY_MS/1000.0:.2f}s) [{vname} - {label}] ===\n")
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
    build_pet_dataset_header()
    print("\nALL CLEAN-EDGE PET DATASET HEADERS GENERATED SUCCESSFULLY!")
