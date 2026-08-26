import os

def gif_to_c_header(gif_path, header_path, array_name="untitled_gif_map", dsc_name="untitled_gif_dsc", width=240, height=280):
    if not os.path.exists(gif_path):
        print(f"Error: {gif_path} not found.")
        return

    print(f"Reading {gif_path}...")
    with open(gif_path, 'rb') as f:
        gif_data = f.read()

    print(f"Converting {len(gif_data)} bytes to C array...")
    
    lines = []
    lines.append("// This file was auto-generated from a GIF animation for LVGL")
    lines.append("#ifndef UNTITLED_GIF_H")
    lines.append("#define UNTITLED_GIF_H")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("")
    lines.append("#ifdef __has_include")
    lines.append("    #if __has_include(\"lvgl.h\")")
    lines.append("        #ifndef LV_LVGL_H_INCLUDE_SIMPLE")
    lines.append("            #define LV_LVGL_H_INCLUDE_SIMPLE")
    lines.append("        #endif")
    lines.append("    #endif")
    lines.append("#endif")
    lines.append("")
    lines.append("#if defined(LV_LVGL_H_INCLUDE_SIMPLE)")
    lines.append("    #include \"lvgl.h\"")
    lines.append("#else")
    lines.append("    #include \"lvgl/lvgl.h\"")
    lines.append("#endif")
    lines.append("")
    lines.append(f"// Raw GIF binary data ({len(gif_data)} bytes)")
    lines.append(f"const uint8_t {array_name}[] = {{")
    
    # Write bytes in chunks of 16 for clean formatting
    for chunk_start in range(0, len(gif_data), 16):
        chunk = gif_data[chunk_start:chunk_start+16]
        hex_bytes = ", ".join(f"0x{b:02x}" for b in chunk)
        lines.append(f"  {hex_bytes},")
        
    lines.append("};")
    lines.append("")
    lines.append("// LVGL Image Descriptor for the GIF animation")
    lines.append(f"const lv_img_dsc_t {dsc_name} = {{")
    lines.append("  .header = {")
    lines.append("    .cf = LV_IMG_CF_RAW,")
    lines.append("    .always_zero = 0,")
    lines.append("    .reserved = 0,")
    lines.append(f"    .w = {width},")
    lines.append(f"    .h = {height},")
    lines.append("  },")
    lines.append(f"  .data_size = sizeof({array_name}),")
    lines.append(f"  .data = {array_name}")
    lines.append("};")
    lines.append("")
    lines.append("#endif // UNTITLED_GIF_H")

    print(f"Writing to {header_path}...")
    with open(header_path, 'w') as f:
        f.write("\n".join(lines) + "\n")
        
    print("Conversion complete!")

if __name__ == "__main__":
    current_dir = os.path.dirname(os.path.abspath(__file__))
    gif_file = os.path.join(current_dir, "Untitled file.gif")
    header_file = os.path.join(current_dir, "untitled_gif.h")
    gif_to_c_header(gif_file, header_file)
