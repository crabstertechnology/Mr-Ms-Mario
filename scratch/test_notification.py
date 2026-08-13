import serial
import time
import sys

def main():
    port = "COM8"
    if len(sys.argv) > 1:
        port = sys.argv[1]
        
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

    # Trigger boot / run mode
    ser.dtr = False
    ser.rts = True
    time.sleep(0.2)
    ser.rts = False
    time.sleep(0.2)
    ser.dtr = True
    time.sleep(2.5)

    # Clear incoming buffer
    ser.reset_input_buffer()
    
    # Send a notification command
    print("[*] Sending NOTIF command...")
    cmd = "NOTIF:Alert|This is a very long notification text that will scroll across the screen.\n"
    ser.write(cmd.encode('utf-8'))
    ser.flush()
    time.sleep(0.5)

    # Send a GET command to verify response
    print("[*] Sending GET command...")
    ser.write(b"GET\n")
    ser.flush()
    time.sleep(0.1)
    
    print("[*] Monitoring response for 5 seconds...")
    try:
        end_time = time.time() + 5.0
        while time.time() < end_time:
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting)
                sys.stdout.write(data.decode('utf-8', errors='replace'))
                sys.stdout.flush()
            time.sleep(0.01)
    except KeyboardInterrupt:
        print("\n[*] Interrupted.")
    finally:
        ser.close()
        print("\n[*] Port closed.")

if __name__ == '__main__':
    main()
