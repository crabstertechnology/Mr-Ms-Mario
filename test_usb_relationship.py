import serial
import time
import threading
import sys

# Port configurations
PORT_V1 = "COM6"  # Luna v1 (ESP32-C3)
PORT_V2 = "COM9"  # Luna v2 (ESP32-S3)
BAUD_RATE = 115200

# Thread-safe flags
running = True

def reader_thread(port_name, serial_conn, target_conn, target_name):
    global running
    print(f"[*] Started monitoring {port_name}...")
    while running:
        try:
            if serial_conn.in_waiting > 0:
                line = serial_conn.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"[{port_name}] {line}")
                    
                    # Intercept touch events
                    if "TOUCH_REL:" in line:
                        payload = line.split("TOUCH_REL:")[-1]
                        parts = payload.split('|')
                        if len(parts) >= 3:
                            touch_type = parts[0]
                            expr_val = parts[1]
                            sound_val = parts[2]
                            
                            print(f"\n[EVENT] {port_name} detected a touch: {touch_type}!")
                            sender_label = "Luna v1" if "COM6" in port_name else "Luna v2"
                            
                            sound_cmd = f"AUDIO:{sound_val}\n"
                            action_label = "Single Tap"
                            if touch_type == "DOUBLE":
                                action_label = "Double Tap"
                            elif touch_type == "TRIPLE":
                                action_label = "Triple Tap"
                            elif touch_type == "LONG":
                                action_label = "Long Press"
                                
                            notif_cmd = f"NOTIF_EXPR:{sender_label}|{action_label}|{expr_val}\n"
                            print(f"[ACTION] Sending reaction to {target_name}: Face {expr_val} + Sound {sound_val} + Header message")
                            target_conn.write(sound_cmd.encode('utf-8'))
                            time.sleep(0.1)
                            target_conn.write(notif_cmd.encode('utf-8'))
                    elif "TOUCH:" in line:
                        touch_type = line.split("TOUCH:")[-1]
                        print(f"\n[EVENT] {port_name} detected a touch: {touch_type}!")
                        
                        sender_label = "Luna v1" if "COM6" in port_name else "Luna v2"
                        
                        # Direct relationship mapping (Fallback):
                        # Send reaction to the OTHER robot
                        if touch_type == "TAP":
                            # Make the other robot happy and play a coin sound!
                            sound_cmd = "AUDIO:2\n"  # SOUND_COIN
                            notif_cmd = f"NOTIF_EXPR:{sender_label}|Single Tap|1\n"
                            print(f"[ACTION] Sending reaction to {target_name}: Happy face + Coin sound + Header message")
                            target_conn.write(sound_cmd.encode('utf-8'))
                            time.sleep(0.1)
                            target_conn.write(notif_cmd.encode('utf-8'))
                        elif touch_type == "DOUBLE":
                            # Make the other robot wink and play a jump sound!
                            sound_cmd = "AUDIO:1\n"  # SOUND_JUMP
                            notif_cmd = f"NOTIF_EXPR:{sender_label}|Double Tap|6\n"
                            print(f"[ACTION] Sending reaction to {target_name}: Wink face + Jump sound + Header message")
                            target_conn.write(sound_cmd.encode('utf-8'))
                            time.sleep(0.1)
                            target_conn.write(notif_cmd.encode('utf-8'))
                        elif touch_type == "TRIPLE":
                            # Make the other robot surprised and play sound!
                            sound_cmd = "AUDIO:8\n"  # SOUND_THEMECHANGE
                            notif_cmd = f"NOTIF_EXPR:{sender_label}|Triple Tap|4\n"
                            print(f"[ACTION] Sending reaction to {target_name}: Surprised face + Sound + Header message")
                            target_conn.write(sound_cmd.encode('utf-8'))
                            time.sleep(0.1)
                            target_conn.write(notif_cmd.encode('utf-8'))
                        elif touch_type == "LONG":
                            # Make the other robot surprised and play theme change sound!
                            sound_cmd = "AUDIO:10\n"  # SOUND_THEMECHANGE
                            notif_cmd = f"NOTIF_EXPR:{sender_label}|Long Press|4\n"
                            print(f"[ACTION] Sending reaction to {target_name}: Surprised face + Theme change sound + Header message")
                            target_conn.write(sound_cmd.encode('utf-8'))
                            time.sleep(0.1)
                            target_conn.write(notif_cmd.encode('utf-8'))
                            
            time.sleep(0.01)
        except Exception as e:
            print(f"[!] Error on {port_name}: {e}")
            break

def main():
    global running
    print("==================================================")
    print("      LUNA ROBOT USB RELATIONSHIP TEST BRIDGER    ")
    print("==================================================")
    
    try:
        # Open serial connections
        print(f"Connecting to Luna v1 (C3) on {PORT_V1}...")
        conn_v1 = serial.Serial(PORT_V1, BAUD_RATE, timeout=1.0)
        # Toggle RTS/DTR to reset/init clean connection
        conn_v1.dtr = False
        conn_v1.rts = True
        time.sleep(0.1)
        conn_v1.rts = False
        
        print(f"Connecting to Luna v2 (S3) on {PORT_V2}...")
        conn_v2 = serial.Serial(PORT_V2, BAUD_RATE, timeout=1.0)
        conn_v2.dtr = False
        conn_v2.rts = True
        time.sleep(0.1)
        conn_v2.rts = False
        
        print("[+] Both robots connected successfully!")
        print("Tapping Luna v1 will trigger Luna v2. Tapping Luna v2 will trigger Luna v1.")
        print("Test running for 60 seconds. Touch your sensors now!")
        print("--------------------------------------------------")
        
        # Start reader threads
        t1 = threading.Thread(target=reader_thread, args=("LUNA v1 (COM6)", conn_v1, conn_v2, "LUNA v2 (COM9)"), daemon=True)
        t2 = threading.Thread(target=reader_thread, args=("LUNA v2 (COM9)", conn_v2, conn_v1, "LUNA v1 (COM6)"), daemon=True)
        t1.start()
        t2.start()
        
        # Countdown loop
        for remaining in range(60, 0, -1):
            if remaining % 10 == 0:
                print(f"[*] Test active. {remaining} seconds remaining...")
            time.sleep(1)
            
    except KeyboardInterrupt:
        print("\nStopping relationship bridge...")
    except Exception as e:
        print(f"[!] Initialization error: {e}")
    finally:
        running = False
        time.sleep(0.2)
        print("[*] Connections closed. Goodbye!")

if __name__ == "__main__":
    main()
