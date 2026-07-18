import serial
import time
import sys

def probe(port="COM6", baud=115200):
    print(f"[*] Probing {port} at {baud} baud...")
    try:
        ser = serial.Serial(port, baud, timeout=1.0)
        ser.dtr = False
        ser.rts = False
        time.sleep(0.1)
        
        # Reset the board by pulsing RTS/DTR
        ser.dtr = True
        ser.rts = True
        time.sleep(0.2)
        ser.dtr = False
        ser.rts = False
        time.sleep(0.5)
    except Exception as e:
        print(f"[!] Error opening serial port: {e}")
        return
        
    print("[*] Reading serial output for 3 seconds...")
    end_time = time.time() + 3.0
    lines = []
    
    while time.time() < end_time:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                print(f"  > {line}")
                lines.append(line)
        time.sleep(0.01)
        
    ser.close()
    
    # Analyze output
    print("\n[*] Analysis:")
    is_v2 = False
    for line in lines:
        if "smartwatch" in line.lower() or "clock screen" in line.lower() or "inactivity timeout" in line.lower():
            is_v2 = True
            break
            
    if is_v2:
        print("[+] Detected: Version 2 (Mr. Luna smartwatch UI)")
    else:
        print("[+] Detected: Version 1 (Standard Ms. Luna)")
        
if __name__ == "__main__":
    port = "COM6"
    if len(sys.argv) > 1:
        port = sys.argv[1]
    probe(port)
