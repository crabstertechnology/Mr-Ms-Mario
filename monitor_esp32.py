import serial
import sys
import time

def monitor():
    port = "COM9"
    baud = 115200
    print(f"Monitoring ESP32 on {port} at {baud} baud. Press Ctrl+C to stop.\n")
    try:
        ser = serial.Serial(port, baud, timeout=1.0)
        ser.dtr = True
        ser.rts = False
    except Exception as e:
        print(f"Error opening serial port: {e}")
        return

    # Flush input buffer
    ser.reset_input_buffer()
    
    try:
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(line)
            time.sleep(0.01)
    except KeyboardInterrupt:
        print("\nStopped monitoring.")
    finally:
        ser.close()

if __name__ == "__main__":
    monitor()
