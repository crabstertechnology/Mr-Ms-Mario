"""
Interactive Serial Controller for ESP32-C3 Video Player
Allows pausing, resuming, adjusting playback speed, rotating orientation, and toggling color inversion.
"""

import serial
import time
import sys

PORT = "COM13"
BAUD = 115200

def main():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"[OK] Connected to ESP32-C3 Video Player on {PORT}")
        print("-" * 55)
        print("Controls:")
        print("  [Space / 'p'] : Pause / Resume video playback")
        print("  ['+']         : Increase speed (decrease frame delay)")
        print("  ['-']         : Decrease speed (increase frame delay)")
        print("  ['o']         : Rotate orientation (0, 1, 2, 3)")
        print("  ['i']         : Toggle display color invert")
        print("  ['r']         : Reset to Frame 0")
        print("  ['q']         : Quit controller")
        print("-" * 55)

        while True:
            cmd = input("Command > ").strip().lower()
            if not cmd:
                continue
            if cmd == 'q':
                break
            elif cmd in ['p', ' ', '+', '-', 'o', 'i', 'r']:
                ser.write(cmd.encode('utf-8'))
                time.sleep(0.05)
                while ser.in_waiting:
                    line = ser.readline().decode('utf-8', errors='replace').strip()
                    if line:
                        print(f"Device: {line}")
            else:
                print("Unknown command. Options: p, +, -, o, i, r, q")
        ser.close()
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()
