import argparse
import sys
import os
import subprocess
import time
import serial
import serial.tools.list_ports

# Predefined example templates
EXAMPLES = {
    "blink": {
        "name": "Blink LED",
        "description": "Blinks the built-in LED (GPIO 21 and GPIO 8 in parallel) to support different board revisions.",
        "code": """// Predefined Blink Example for ESP32-S3 Mini
#define LED1_PIN 21
#define LED2_PIN 8

void setup() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("ESP32-S3 Mini Blink Sketch Started!");
}

void loop() {
  // Turn LEDs ON
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  Serial.println("LEDs ON");
  delay(1000);
  
  // Turn LEDs OFF
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  Serial.println("LEDs OFF");
  delay(1000);
}
"""
    },
    "hello": {
        "name": "Hello Serial Output",
        "description": "Prints 'Hello from ESP32-S3 Mini!' and uptime statistics every 1 second over serial connection.",
        "code": """// Predefined Hello Serial Example for ESP32-S3 Mini
unsigned long startTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for USB CDC connection
  Serial.println("=========================================");
  Serial.println("ESP32-S3 Mini Diagnostic Sketch Initialized");
  Serial.println("=========================================");
  startTime = millis();
}

void loop() {
  unsigned long uptime = (millis() - startTime) / 1000;
  Serial.print("Hello from ESP32-S3 Mini! | Uptime: ");
  Serial.print(uptime);
  Serial.println(" seconds");
  delay(1000);
}
"""
    }
}

def scan_ports():
    ports_list = []
    print("[*] Scanning for serial ports...")
    for p in serial.tools.list_ports.comports():
        desc = p.description.lower()
        is_esp = ("esp32" in desc) or ("usb" in desc) or ("jtag" in desc) or ("serial" in desc) or ("silicon labs" in desc)
        ports_list.append({
            'port': p.device,
            'description': p.description,
            'is_esp': is_esp
        })
        print(f"    Found: {p.device} - {p.description} (ESP Candidate: {is_esp})")
    
    # Prioritize ESP candidates
    candidates = [p['port'] for p in ports_list if p['is_esp']]
    if candidates:
        return candidates[0]
    elif ports_list:
        return ports_list[0]['port']
    return None

def run_command(cmd, step_name):
    print(f"\n[*] Step: {step_name}")
    print(f"    Running: {' '.join(cmd)}")
    
    process = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        shell=True
    )
    
    # Read output in real-time
    while True:
        output = process.stdout.readline()
        if output == '' and process.poll() is not None:
            break
        if output:
            print(f"    > {output.strip()}")
            
    rc = process.poll()
    if rc != 0:
        print(f"\n[!] ERROR: {step_name} failed with exit code {rc}")
        return False
    print(f"[+] Success: {step_name} completed.")
    return True

def open_serial_monitor(port, baud=115200):
    print(f"\n[*] Opening Serial Monitor on {port} at {baud} baud...")
    print("[*] Press Ctrl+C to stop the monitor.")
    print("------------------------------------------------------------------")
    
    try:
        # Open port with CDC configuration: DTR=True (indicates host ready), RTS=False (releases EN pin to allow chip to run)
        ser = serial.Serial(port, baud, timeout=1)
        ser.dtr = True
        ser.rts = False
        time.sleep(0.1)
        
        # Reset the board once by pulsing RTS
        ser.rts = True
        time.sleep(0.1)
        ser.rts = False
        
        while True:
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting)
                sys.stdout.write(data.decode('utf-8', errors='replace'))
                sys.stdout.flush()
            time.sleep(0.01)
            
    except KeyboardInterrupt:
        print("\n\n[*] Serial Monitor stopped by user.")
    except Exception as e:
        print(f"\n[!] Serial Monitor error: {e}")
    finally:
        try:
            ser.close()
        except:
            pass

def main():
    parser = argparse.ArgumentParser(description="ESP32-S3 Mini CLI Compiler and Flash Utility")
    parser.add_argument("--port", help="COM port (e.g. COM9). If omitted, scans and auto-detects.")
    parser.add_argument("--fqbn", default="esp32:esp32:esp32s3:CDCOnBoot=cdc", help="FQBN parameter (default: esp32:esp32:esp32s3:CDCOnBoot=cdc)")
    parser.add_argument("--example", choices=list(EXAMPLES.keys()), help="Name of predefined example sketch to compile and flash (choices: blink, hello)")
    parser.add_argument("--file", help="Path to a custom local .ino sketch file to compile and flash.")
    parser.add_argument("--monitor", action="store_true", help="Open serial monitor automatically after successful upload.")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate for serial monitor (default: 115200)")
    
    args = parser.parse_args()
    
    # 1. Resolve code content
    code_content = None
    sketch_name = "cli_sketch"
    
    if args.example:
        example_data = EXAMPLES[args.example]
        print(f"[*] Selected predefined example: {example_data['name']}")
        print(f"    Description: {example_data['description']}")
        code_content = example_data["code"]
        sketch_name = f"example_{args.example}"
    elif args.file:
        if not os.path.exists(args.file):
            print(f"[!] Error: File '{args.file}' does not exist.")
            sys.exit(1)
        print(f"[*] Selected custom sketch file: {args.file}")
        with open(args.file, 'r', encoding='utf-8') as f:
            code_content = f.read()
        sketch_name = os.path.splitext(os.path.basename(args.file))[0]
    else:
        print("[!] Error: You must specify either --example (blink/hello) or --file <path_to_ino>")
        parser.print_help()
        sys.exit(1)
        
    # 2. Resolve port
    port = args.port
    if not port:
        port = scan_ports()
        if not port:
            print("[!] Error: No serial ports detected! Please connect your ESP32-S3 Mini.")
            sys.exit(1)
        print(f"[*] Auto-detected Target Port: {port}")
    else:
        print(f"[*] Target Port Specified: {port}")
        
    # 3. Create temporary sketch directory (arduino-cli requires folder name to match sketch name)
    cwd = os.getcwd()
    sketch_dir = os.path.join(cwd, sketch_name)
    os.makedirs(sketch_dir, exist_ok=True)
    sketch_path = os.path.join(sketch_dir, f"{sketch_name}.ino")
    
    with open(sketch_path, 'w', encoding='utf-8') as f:
        f.write(code_content)
        
    print(f"[*] Created temporary sketch: {sketch_path}")
    
    # 4. Compile sketch
    compile_cmd = [
        "arduino-cli", "compile",
        "--fqbn", args.fqbn,
        sketch_name
    ]
    
    compilation_success = run_command(compile_cmd, "Compilation")
    if not compilation_success:
        print("[!] Build aborted due to compilation failure.")
        sys.exit(1)
        
    # 5. Upload sketch
    upload_cmd = [
        "arduino-cli", "upload",
        "-p", port,
        "--fqbn", args.fqbn,
        sketch_name
    ]
    
    upload_success = run_command(upload_cmd, "Upload")
    if not upload_success:
        print("\n[!] UPLOAD FAILED.")
        print("[!] Troubleshooting tips for ESP32-S3 Mini:")
        print("    1. Enter Manual Bootloader Recovery:")
        print("       - Press and hold the BOOT button.")
        print("       - Click the RESET button once.")
        print("       - Release the BOOT button.")
        print("       - Run this script again.")
        print("    2. Verify the USB cable supports data transfer (not a power-only charging cable).")
        sys.exit(1)
        
    print("\n[+] SUCCESS! Sketch compiled and flashed successfully.")
    
    # 6. Serial Monitor
    if args.monitor:
        # Give board a split-second to start up
        time.sleep(0.5)
        open_serial_monitor(port, args.baud)

if __name__ == "__main__":
    main()
