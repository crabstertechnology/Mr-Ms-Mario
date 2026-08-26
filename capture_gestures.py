import sys
import time
import json
import serial
import msvcrt

def main():
    print("==================================================")
    print("          Luna Smartwatch Touch Capture           ")
    print("==================================================")
    
    port = "COM3"
    baud = 115200
    
    print(f"Connecting to {port} at {baud} baud...")
    try:
        ser = serial.Serial(port, baud, timeout=0.1)
        # Flush input buffer
        ser.reset_input_buffer()
        print("Connected successfully!")
    except Exception as e:
        print(f"Error connecting to serial port: {e}")
        print("Please check if the device is connected and the port is not in use.")
        return
        
    gestures_to_capture = ['Swipe Left', 'Swipe Right', 'Single Tap', 'Double Tap']
    captured_data = {}
    
    for gest in gestures_to_capture:
        print("\n" + "="*50)
        print(f" TARGET: {gest.upper()} ")
        print("="*50)
        print(f"Instructions: Please perform 10 separate '{gest}' gestures on the watch.")
        input("Press ENTER when you are ready to start capturing...")
        
        print(f"\nCapturing started! Perform your gestures now.")
        print("Press any key to STOP capturing this gesture...")
        
        gest_points = []
        ser.reset_input_buffer()
        
        while True:
            # Check for keyboard press (non-blocking)
            if msvcrt.kbhit():
                msvcrt.getch() # Consume the keypress
                break
                
            line = ser.readline()
            if not line:
                continue
                
            try:
                line_str = line.decode('utf-8', errors='ignore').strip()
                if line_str.startswith("[TOUCH_RAW]"):
                    # Example line: [TOUCH_RAW] G:03 F:1 X:120 Y:140 MS:53456
                    parts = line_str.split()
                    data_dict = {}
                    for part in parts[1:]:
                        k, v = part.split(':')
                        if k in ['G']:
                            data_dict[k] = v # keep hex string
                        else:
                            data_dict[k] = int(v)
                    
                    data_dict['time_recv'] = time.time()
                    gest_points.append(data_dict)
                    print(f"  [RAW POINT] G:{data_dict['G']} F:{data_dict['F']} X:{data_dict['X']} Y:{data_dict['Y']} MS:{data_dict['MS']}       ", end='\r')
            except Exception as e:
                # Silently ignore format errors
                pass
                
        print(f"\nStopped capturing for '{gest}'. Recorded {len(gest_points)} points.")
        captured_data[gest] = gest_points
        time.sleep(0.5)
        
    ser.close()
    
    # Save to file
    output_file = "touch_capture_data.json"
    with open(output_file, 'w') as f:
        json.dump(captured_data, f, indent=2)
        
    print("\n" + "="*50)
    print("               CAPTURE SUMMARY                    ")
    print("="*50)
    print(f"Data saved to '{output_file}' successfully!")
    
    for gest, points in captured_data.items():
        print(f"\n* {gest}:")
        if not points:
            print("  No points captured.")
            continue
        print(f"  Total raw coordinates: {len(points)}")
        gestures_seen = set(p['G'] for p in points)
        print(f"  Gestures reported by CST816T: {', '.join(gestures_seen)}")
        xs = [p['X'] for p in points]
        ys = [p['Y'] for p in points]
        print(f"  Coordinate Range: X:[{min(xs)}, {max(xs)}]  Y:[{min(ys)}, {max(ys)}]")
        print(f"  Average position: X:{sum(xs)/len(xs):.1f} Y:{sum(ys)/len(ys):.1f}")
        
    print("\nAll done!")

if __name__ == '__main__':
    main()
