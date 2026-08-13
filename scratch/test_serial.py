import serial
import time
import sys

def main():
    port = "COM8"
    if len(sys.argv) > 1:
        port = sys.argv[1]
    
    # Reconfigure stdout to support UTF-8 (emojis in boot log)
    try:
        sys.stdout.reconfigure(encoding='utf-8')
    except AttributeError:
        pass
        
    print(f"[*] Opening {port} at 115200...")
    try:
        ser = serial.Serial(port, 115200, timeout=1.0)
    except Exception as e:
        print(f"[!] Error opening serial port: {e}")
        return

    # Try normal run mode sequence
    # For native USB CDC on ESP32-C3/S3:
    # EN (Reset) is controlled by RTS, GPIO9 (Boot) is controlled by DTR.
    # To boot normally: EN must go HIGH (RTS = False) while GPIO9 is HIGH (DTR = False).
    # Let's try setting both to False first, then pulse RTS.
    print("[*] State 1: DTR=False, RTS=True (Reset active)")
    ser.dtr = False
    ser.rts = True
    time.sleep(0.2)

    print("[*] State 2: DTR=False, RTS=False (Release Reset, GPIO9 HIGH)")
    ser.rts = False
    time.sleep(0.2)
    
    # Enable DTR/RTS to see if we receive data (often DTR=True is required for Windows to receive USB CDC serial data)
    print("[*] State 3: DTR=True (Enable Windows CDC RX)")
    ser.dtr = True
    time.sleep(0.2)
    
    print("[*] Monitoring serial output (Ctrl+C to stop)...")
    try:
        end_time = time.time() + 6.0
        while time.time() < end_time:
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting)
                sys.stdout.write(data.decode('utf-8', errors='replace'))
                sys.stdout.flush()
            time.sleep(0.01)
    except KeyboardInterrupt:
        print("\n[*] Interrupted by user.")
    finally:
        ser.close()
        print("\n[*] Port closed.")

if __name__ == '__main__':
    main()
