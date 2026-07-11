import http.server
import socketserver
import subprocess
import os
import sys
import json
import threading
import asyncio
import websockets
import time
import serial
import serial.tools.list_ports

HTTP_PORT = 8300
WS_PORT = 8301
DIRECTORY = os.path.dirname(os.path.abspath(__file__))

# Global states
connected_clients = set()
ws_loop = None
serial_monitor = None
serial_monitor_lock = threading.Lock()
serial_was_active_before_build = False
serial_active_config = {"port": None, "baud": 115200}

# ----------------- SERIAL MONITOR WRAPPER -----------------
class SerialMonitor:
    def __init__(self, port, baud, on_data_cb, on_disconnect_cb):
        self.port = port
        self.baud = baud
        self.on_data = on_data_cb
        self.on_disconnect = on_disconnect_cb
        self.ser = None
        self.running = False
        self.thread = None

    def start(self):
        try:
            # Open port with CDC configurations: DTR=True, RTS=False
            self.ser = serial.Serial(self.port, self.baud, timeout=0.1)
            self.ser.dtr = True
            self.ser.rts = False
            time.sleep(0.1)
            
            # Pulse RTS to trigger soft reset
            self.ser.rts = True
            time.sleep(0.1)
            self.ser.rts = False
            
            self.running = True
            self.thread = threading.Thread(target=self._run, daemon=True)
            self.thread.start()
            return True
        except Exception as e:
            print(f"[SerialMonitor] Failed to open {self.port}: {e}")
            return False

    def _run(self):
        print(f"[SerialMonitor] Monitoring started on {self.port}")
        while self.running:
            try:
                if self.ser and self.ser.is_open and self.ser.in_waiting > 0:
                    data = self.ser.read(self.ser.in_waiting)
                    if data:
                        text = data.decode('utf-8', errors='replace')
                        self.on_data(text)
                time.sleep(0.01)
            except Exception as e:
                print(f"[SerialMonitor] Read error on {self.port}: {e}")
                self.on_disconnect(str(e))
                break
        self.close()

    def write(self, data):
        if self.ser and self.ser.is_open:
            try:
                self.ser.write(data.encode('utf-8'))
                return True
            except Exception as e:
                print(f"[SerialMonitor] Write error: {e}")
        return False

    def reset_rts(self):
        if self.ser and self.ser.is_open:
            try:
                self.ser.rts = True
                time.sleep(0.1)
                self.ser.rts = False
                return True
            except Exception as e:
                print(f"[SerialMonitor] RTS reset error: {e}")
        return False

    def close(self):
        self.running = False
        if self.ser:
            try:
                self.ser.close()
                print(f"[SerialMonitor] Port {self.port} closed.")
            except Exception as e:
                print(f"[SerialMonitor] Error closing port: {e}")
            self.ser = None


# ----------------- UTILITY FUNCTIONS -----------------

def scan_serial_ports():
    ports_list = []
    for p in serial.tools.list_ports.comports():
        desc = p.description.lower()
        is_esp = ("esp32" in desc) or ("usb" in desc) or ("jtag" in desc) or ("serial" in desc) or ("silicon labs" in desc)
        ports_list.append({
            'port': p.device,
            'description': p.description,
            'is_esp': is_esp
        })
    return ports_list

def scan_workspace_sketches():
    sketches = []
    # Scan w:\Mr.mario and w:\esp s3 testing
    dirs_to_scan = [
        "w:\\Mr.mario",
        "w:\\esp s3 testing"
    ]
    for base_dir in dirs_to_scan:
        if not os.path.exists(base_dir):
            continue
        try:
            for item in os.listdir(base_dir):
                item_path = os.path.join(base_dir, item)
                if os.path.isdir(item_path):
                    # Check if there is an .ino file inside
                    for file_name in os.listdir(item_path):
                        if file_name.endswith('.ino'):
                            sketches.append({
                                "name": item,
                                "path": item_path
                            })
                            break
        except Exception as e:
            print(f"[Server] Error scanning {base_dir}: {e}")
    return sketches

def broadcast_message(msg_dict):
    if ws_loop and connected_clients:
        msg_str = json.dumps(msg_dict)
        for ws in list(connected_clients):
            asyncio.run_coroutine_threadsafe(ws.send(msg_str), ws_loop)


# ----------------- BACKGROUND WORKERS -----------------

def handle_serial_data(text):
    broadcast_message({
        "type": "serial_data",
        "data": text
    })

def handle_serial_disconnect(error_message):
    global serial_monitor
    with serial_monitor_lock:
        serial_monitor = None
    broadcast_message({
        "type": "serial_status",
        "connected": False,
        "reason": error_message
    })

def safe_close_serial_for_action():
    global serial_monitor, serial_was_active_before_build
    with serial_monitor_lock:
        if serial_monitor:
            print("[Server] Closing serial monitor for build/flash action.")
            serial_was_active_before_build = True
            serial_monitor.close()
            serial_monitor = None
            broadcast_message({
                "type": "serial_status",
                "connected": False,
                "reason": "Suspended for build/flash action"
            })
        else:
            serial_was_active_before_build = False

def run_compile_worker(sketch_path, fqbn):
    print(f"[Compiler] Compiling {sketch_path} for {fqbn}")
    safe_close_serial_for_action()
    
    cmd = ["arduino-cli", "compile", "--fqbn", fqbn, sketch_path]
    
    try:
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            shell=True,
            bufsize=1
        )
        
        # Read compilation output line-by-line in real time
        for line in iter(process.stdout.readline, ''):
            broadcast_message({
                "type": "compile_output",
                "data": line
            })
            
        process.stdout.close()
        rc = process.wait()
        
        success = (rc == 0)
        broadcast_message({
            "type": "compile_status",
            "status": "success" if success else "failed"
        })
        print(f"[Compiler] Finished with code {rc}")
    except Exception as e:
        broadcast_message({
            "type": "compile_output",
            "data": f"\nExecution Error: {str(e)}\n"
        })
        broadcast_message({
            "type": "compile_status",
            "status": "failed"
        })

def run_flash_worker(sketch_path, port, fqbn):
    print(f"[Flasher] Flashing {sketch_path} to {port} for {fqbn}")
    safe_close_serial_for_action()
    
    cmd = ["arduino-cli", "upload", "-p", port, "--fqbn", fqbn, sketch_path]
    
    try:
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            shell=True,
            bufsize=1
        )
        
        # Read flash output line-by-line in real time
        for line in iter(process.stdout.readline, ''):
            broadcast_message({
                "type": "flash_output",
                "data": line
            })
            
        process.stdout.close()
        rc = process.wait()
        
        success = (rc == 0)
        broadcast_message({
            "type": "flash_status",
            "status": "success" if success else "failed"
        })
        print(f"[Flasher] Finished with code {rc}")
    except Exception as e:
        broadcast_message({
            "type": "flash_output",
            "data": f"\nExecution Error: {str(e)}\n"
        })
        broadcast_message({
            "type": "flash_status",
            "status": "failed"
        })


# ----------------- WEBSOCKET HANDLER -----------------

async def ws_handler(websocket):
    global serial_monitor, serial_active_config
    connected_clients.add(websocket)
    print(f"[WS] Client connected. Total: {len(connected_clients)}")
    
    # Send current serial status on connect
    with serial_monitor_lock:
        is_conn = serial_monitor is not None
        port = serial_monitor.port if is_conn else None
        baud = serial_monitor.baud if is_conn else 115200
    await websocket.send(json.dumps({
        "type": "serial_status",
        "connected": is_conn,
        "port": port,
        "baud": baud
    }))
    
    try:
        async for message in websocket:
            try:
                payload = json.loads(message)
                action = payload.get("action")
                
                if action == "get_ports":
                    ports = scan_serial_ports()
                    await websocket.send(json.dumps({
                        "type": "ports_list",
                        "ports": ports
                    }))
                    
                elif action == "get_sketches":
                    sketches = scan_workspace_sketches()
                    await websocket.send(json.dumps({
                        "type": "sketches_list",
                        "sketches": sketches
                    }))
                    
                elif action == "connect_serial":
                    port = payload.get("port")
                    baud = int(payload.get("baud", 115200))
                    
                    with serial_monitor_lock:
                        if serial_monitor:
                            serial_monitor.close()
                        
                        serial_active_config["port"] = port
                        serial_active_config["baud"] = baud
                        serial_monitor = SerialMonitor(port, baud, handle_serial_data, handle_serial_disconnect)
                        success = serial_monitor.start()
                        
                    if success:
                        await websocket.send(json.dumps({
                            "type": "serial_status",
                            "connected": True,
                            "port": port,
                            "baud": baud
                        }))
                    else:
                        await websocket.send(json.dumps({
                            "type": "serial_status",
                            "connected": False,
                            "reason": "Failed to open serial port"
                        }))
                        
                elif action == "disconnect_serial":
                    with serial_monitor_lock:
                        if serial_monitor:
                            serial_monitor.close()
                            serial_monitor = None
                    await websocket.send(json.dumps({
                        "type": "serial_status",
                        "connected": False
                    }))
                    
                elif action == "send_serial":
                    data = payload.get("data", "")
                    with serial_monitor_lock:
                        if serial_monitor:
                            serial_monitor.write(data)
                            
                elif action == "reset_chip":
                    with serial_monitor_lock:
                        if serial_monitor:
                            serial_monitor.reset_rts()
                            
                elif action == "start_compile":
                    sketch = payload.get("sketch")
                    fqbn = payload.get("fqbn")
                    # Run compilation in a background thread to keep websocket responsive
                    t = threading.Thread(target=run_compile_worker, args=(sketch, fqbn), daemon=True)
                    t.start()
                    
                elif action == "start_flash":
                    sketch = payload.get("sketch")
                    port = payload.get("port")
                    fqbn = payload.get("fqbn")
                    # Run flashing in a background thread
                    t = threading.Thread(target=run_flash_worker, args=(sketch, port, fqbn), daemon=True)
                    t.start()
                    
            except Exception as e:
                print(f"[WS] Error processing message: {e}")
                await websocket.send(json.dumps({
                    "type": "error",
                    "message": str(e)
                }))
                
    except websockets.ConnectionClosed:
        pass
    finally:
        connected_clients.remove(websocket)
        print(f"[WS] Client disconnected. Total: {len(connected_clients)}")

async def ws_server_main():
    global ws_loop
    ws_loop = asyncio.get_running_loop()
    print(f"[WS Server] Listening on ws://0.0.0.0:{WS_PORT}")
    async with websockets.serve(ws_handler, "0.0.0.0", WS_PORT):
        await asyncio.Future()

def start_ws_server():
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    loop.run_until_complete(ws_server_main())


# ----------------- HTTP SERVER HANDLER -----------------

class FlashDashboardServer(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/' or self.path == '/index.html':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            dashboard_path = os.path.join(DIRECTORY, "flash_dashboard.html")
            try:
                with open(dashboard_path, 'r', encoding='utf-8') as f:
                    self.wfile.write(f.read().encode('utf-8'))
            except Exception as e:
                self.wfile.write(f"Error loading dashboard: {e}".encode('utf-8'))
            return
        return super().do_GET()


# ----------------- MAIN RUNNER -----------------

if __name__ == '__main__':
    print("==================================================")
    print("Starting Luna-Forge Compiler, Flasher & Serial Server")
    print(f"Web Dashboard: http://localhost:{HTTP_PORT}")
    print(f"WebSocket Server: ws://localhost:{WS_PORT}")
    print("==================================================")
    
    # 1. Start WebSocket server thread
    ws_thread = threading.Thread(target=start_ws_server, daemon=True)
    ws_thread.start()
    
    # 2. Start HTTP server
    socketserver.ThreadingTCPServer.allow_reuse_address = True
    with socketserver.ThreadingTCPServer(("", HTTP_PORT), FlashDashboardServer) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down Luna-Forge server.")
            httpd.server_close()
            with serial_monitor_lock:
                if serial_monitor:
                    serial_monitor.close()
