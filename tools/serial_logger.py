import serial
import time
import sys

print("Serial logger starting...")
try:
    ser = serial.Serial('COM3', 115200, timeout=0.1)
    print("Successfully connected to COM3.")
except Exception as e:
    print(f"Error opening COM3: {e}")
    sys.exit(1)

import os
log_file_path = os.path.join(os.path.dirname(__file__), "serial_log.txt")
log_file = open(log_file_path, "w", encoding="utf-8")
log_file.write("--- Start Log ---\n")
log_file.flush()

try:
    while True:
        line = ser.readline()
        if line:
            try:
                decoded = line.decode('utf-8', errors='ignore').strip()
                print(decoded)
                log_file.write(decoded + "\n")
                log_file.flush()
            except Exception as e:
                pass
        time.sleep(0.01)
except KeyboardInterrupt:
    print("Exiting...")
finally:
    ser.close()
    log_file.close()
