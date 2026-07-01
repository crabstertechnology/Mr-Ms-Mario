import serial
import time
import sys

def test_navigation():
    port = "COM6"
    baud = 115200
    print(f"Opening serial port {port} at {baud} baud...")
    
    try:
        ser = serial.Serial(port, baud, timeout=2.0)
    except Exception as e:
        print(f"Failed to open port {port}: {e}")
        return False
        
    time.sleep(2) # Wait for connection/reset stabilization
    
    test_cases = [
        ("MAP:LEFT,300 m,10 min", "[MAP] direction='LEFT' distance='300 m' desc='10 min'"),
        ("MAP:RIGHT,1.2 km,15 min", "[MAP] direction='RIGHT' distance='1.2 km' desc='15 min'"),
        ("MAP:UTURN,50 m,1 min", "[MAP] direction='UTURN' distance='50 m' desc='1 min'"),
        ("MAP:ROUNDABOUT,2nd exit,5 min", "[MAP] direction='ROUNDABOUT' distance='2nd exit' desc='5 min'"),
        ("MAP:STRAIGHT,Keep straight,12 min", "[MAP] direction='STRAIGHT' distance='Keep straight' desc='12 min'"),
        ("MAP:EXIT", "Maps Navigation Exited.")
    ]
    
    all_passed = True
    
    for cmd, expected in test_cases:
        print(f"\n---> Sending: {cmd}")
        # Flush input buffer before sending to avoid stale lines
        ser.reset_input_buffer()
        
        # Send command with newline character
        ser.write((cmd + "\n").encode('utf-8'))
        
        # Wait a short moment and read response lines
        time.sleep(0.5)
        
        lines = []
        start_time = time.time()
        matched = False
        
        # Poll for responses for up to 1.5 seconds
        while time.time() - start_time < 1.5:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"ESP32: {line}")
                    lines.append(line)
                    if expected in line:
                        matched = True
                        break
            time.sleep(0.05)
            
        if matched:
            print(f"PASSED: Found matching output line!")
        else:
            print(f"FAILED: Expected to find '{expected}' in output, but did not match.")
            all_passed = False
            
        # Keep the screen state on the hardware display for 2 seconds to let the user see it
        if "EXIT" not in cmd:
            time.sleep(2.0)
            
    ser.close()
    return all_passed

if __name__ == "__main__":
    success = test_navigation()
    if success:
        print("\nAll navigation and arrow display test cases successfully parsed and confirmed on ESP32!")
        sys.exit(0)
    else:
        print("\nSome test cases failed.")
        sys.exit(1)
