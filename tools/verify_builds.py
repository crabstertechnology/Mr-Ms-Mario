import os
import sys
import subprocess
import time
import re

# ANSI Color codes for beautiful terminal output
GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
CYAN = "\033[96m"
MAGENTA = "\033[95m"
RESET = "\033[0m"
BOLD = "\033[1m"

def print_header(title):
    print(f"\n{BOLD}{CYAN}{'=' * 60}{RESET}")
    print(f"{BOLD}{CYAN}  {title}{RESET}")
    print(f"{BOLD}{CYAN}{'=' * 60}{RESET}")

def run_command(cmd):
    try:
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            shell=True
        )
        stdout, stderr = process.communicate()
        return process.returncode, stdout, stderr
    except Exception as e:
        return -1, "", str(e)

def parse_size_info(stdout):
    # Search for patterns like:
    # Sketch uses 731354 bytes (23%) of program storage space...
    # Global variables use 75560 bytes (23%) of dynamic memory...
    flash_size = "N/A"
    flash_percent = "N/A"
    ram_size = "N/A"
    ram_percent = "N/A"
    
    flash_match = re.search(r"Sketch uses (\d+) bytes \(([^)]+)\) of program storage", stdout)
    if flash_match:
        flash_size = f"{int(flash_match.group(1)) / 1024:.1f} KB"
        flash_percent = flash_match.group(2)
        
    ram_match = re.search(r"Global variables use (\d+) bytes \(([^)]+)\) of dynamic memory", stdout)
    if ram_match:
        ram_size = f"{int(ram_match.group(1)) / 1024:.1f} KB"
        ram_percent = ram_match.group(2)
        
    return flash_size, flash_percent, ram_size, ram_percent

def main():
    print_header("Luna Smartwatch Firmware Integrity Verification")
    
    # 1. Check if arduino-cli is installed
    code, stdout, stderr = run_command("arduino-cli version")
    if code != 0:
        print(f"{RED}{BOLD}[ERROR] arduino-cli is not installed or not in PATH.{RESET}")
        print("Please install arduino-cli to proceed.")
        sys.exit(1)
        
    version_str = stdout.strip()
    print(f"{GREEN}[FOUND] {version_str}{RESET}\n")
    
    # Define targets to build
    # Format: (Name, Sketch Path, FQBN, Extra Libraries Path)
    targets = [
        (
            "Luna Firmware 1.3\" (ST7789 C3)",
            "1.3 Luna Firmware/1.3 Luna Firmware.ino",
            "esp32:esp32:esp32c3:PartitionScheme=huge_app,CDCOnBoot=cdc",
            None
        ),
        (
            "Luna Firmware 1.8\" (ST7735 C3)",
            "1.8 Luna Firmware/1.8 Luna Firmware.ino",
            "esp32:esp32:esp32c3:PartitionScheme=huge_app,CDCOnBoot=cdc",
            None
        ),
        (
            "Luna Firmware 1.69\" (ST7789 S3 Touch)",
            "1.69 Luna Firmware/1.69 Luna Firmware.ino",
            "esp32:esp32:esp32s3:PartitionScheme=huge_app,CDCOnBoot=cdc",
            None
        ),
        (
            "LVGL Demo/Benchmark (S3 Touch)",
            "build_test/ESP32-S3-Touch-LCD-1.69/examples/arduino/11_LVGL_Arduino/11_LVGL_Arduino.ino",
            "esp32:esp32:esp32s3:PartitionScheme=huge_app,CDCOnBoot=cdc",
            "build_test/ESP32-S3-Touch-LCD-1.69/examples/arduino/libraries"
        )
    ]
    
    results = []
    
    for name, path, fqbn, lib_path in targets:
        print(f"{BOLD}Compiling {YELLOW}{name}{RESET}...")
        print(f"  Path: {path}")
        print(f"  FQBN: {fqbn}")
        if lib_path:
            print(f"  Libs: {lib_path}")
            
        start_time = time.time()
        
        # Build compile command
        cmd = ["arduino-cli", "compile", "--fqbn", fqbn]
        if lib_path:
            cmd.extend(["--libraries", lib_path])
        cmd.append(path)
        
        cmd_str = " ".join(f'"{c}"' if " " in c or "/" in c or "\\" in c else c for c in cmd)
        
        # Run compile
        code, stdout, stderr = run_command(cmd_str)
        elapsed = time.time() - start_time
        
        if code == 0:
            flash_size, flash_pct, ram_size, ram_pct = parse_size_info(stdout)
            print(f"  {GREEN}[OK] Success{RESET} in {elapsed:.1f}s | Flash: {flash_size} ({flash_pct}) | RAM: {ram_size} ({ram_pct})")
            results.append({
                "name": name,
                "status": "Success",
                "time": f"{elapsed:.1f}s",
                "flash": f"{flash_size} ({flash_pct})",
                "ram": f"{ram_size} ({ram_pct})",
                "error": ""
            })
        else:
            print(f"  {RED}[FAIL] Failed{RESET} in {elapsed:.1f}s")
            # Show a brief preview of the error
            err_preview = stderr.strip() if stderr.strip() else stdout.strip()
            err_lines = err_preview.split('\n')
            preview = "\n".join(err_lines[-5:]) if len(err_lines) > 5 else err_preview
            print(f"  {RED}Error Preview:{RESET}\n{preview}\n")
            results.append({
                "name": name,
                "status": "Failed",
                "time": f"{elapsed:.1f}s",
                "flash": "N/A",
                "ram": "N/A",
                "error": preview
            })
            
    # Print results summary table
    print_header("Build Verification Summary")
    
    # Header format
    row_fmt = "{:<32} | {:<10} | {:<8} | {:<16} | {:<16}"
    print(BOLD + row_fmt.format("Target Name", "Status", "Duration", "Flash Space", "Dynamic Memory") + RESET)
    print("-" * 92)
    
    failed_any = False
    for r in results:
        status_color = GREEN if r["status"] == "Success" else RED
        print(row_fmt.format(
            r["name"],
            status_color + r["status"] + RESET,
            r["time"],
            r["flash"],
            r["ram"]
        ))
        if r["status"] != "Success":
            failed_any = True
            
    print("-" * 92)
    
    if failed_any:
        print(f"\n{RED}{BOLD}[FAIL] Integrity Check FAILED. One or more firmware builds failed.{RESET}")
        sys.exit(1)
    else:
        print(f"\n{GREEN}{BOLD}[OK] Integrity Check PASSED. All firmware targets compiled successfully!{RESET}")
        sys.exit(0)

if __name__ == "__main__":
    main()
