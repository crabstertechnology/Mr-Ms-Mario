import http.server
import socketserver
import subprocess
import os
import sys
import json
import threading
import socket
import asyncio
import websockets
import urllib.parse
import time

PORT = 8000
DIRECTORY = os.path.dirname(os.path.abspath(__file__))

# Threading Lock and active robots dictionary
robots_lock = threading.Lock()
connected_robots = {} # MAC -> { "socket": websocket, "variant": variant, "status": "online" }
ws_loop = None

# Screen capture variables
last_screenshot_request = 0.0
latest_screen_frame = None
screen_frame_lock = threading.Lock()

def start_screen_capture_worker():
    def screen_capture_worker():
        global latest_screen_frame, last_screenshot_request
        cmd = ["C:\\Users\\sasit\\AppData\\Local\\Android\\Sdk\\platform-tools\\adb.exe", "-s", "VKEU5DJNIFY9JJFU", "exec-out", "screencap", "-p"]
        print("[ScreenCapturer] Background worker started...")
        while True:
            try:
                if time.time() - last_screenshot_request < 3.5:
                    data = subprocess.check_output(cmd)
                    if data:
                        with screen_frame_lock:
                            latest_screen_frame = data
                else:
                    time.sleep(0.5)
            except Exception as e:
                print(f"[ScreenCapturer] Error: {e}")
                time.sleep(1.0)

    t = threading.Thread(target=screen_capture_worker, daemon=True)
    t.start()

# ----------------- HTTP REQUEST HANDLER -----------------
class DualStackServer(http.server.SimpleHTTPRequestHandler):
    def translate_path(self, path):
        self.directory = os.path.join(DIRECTORY, "mobile_app")
        return super().translate_path(path)

    def do_GET(self):
        # Handle API to fetch list of online robots
        if self.path == '/api/robots':
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            with robots_lock:
                robots_list = [
                    {"mac": mac, "variant": data["variant"], "status": data["status"]}
                    for mac, data in connected_robots.items()
                ]
            
            self.wfile.write(json.dumps(robots_list).encode('utf-8'))
            return
            
        elif self.path.startswith('/api/screenshot'):
            global last_screenshot_request
            last_screenshot_request = time.time()
            
            self.send_response(200)
            self.send_header('Content-Type', 'image/png')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate, max-age=0')
            self.end_headers()
            
            with screen_frame_lock:
                frame = latest_screen_frame
                
            if frame:
                self.wfile.write(frame)
            else:
                try:
                    cmd = ["C:\\Users\\sasit\\AppData\\Local\\Android\\Sdk\\platform-tools\\adb.exe", "-s", "VKEU5DJNIFY9JJFU", "exec-out", "screencap", "-p"]
                    img_data = subprocess.check_output(cmd)
                    self.wfile.write(img_data)
                except Exception as e:
                    print(f"[SERVER] Sync screenshot fallback failed: {e}")
            return
            
        return super().do_GET()

    def do_POST(self):
        # Handle firmware compilation
        if self.path == '/api/compile':
            print("[SERVER] Received compile request...")
            try:
                powershell_path = "powershell.exe"
                script_path = os.path.join(DIRECTORY, "compile.ps1")
                
                # Execute powershell compile.ps1
                process = subprocess.Popen(
                    [powershell_path, "-ExecutionPolicy", "Bypass", "-File", script_path],
                    cwd=DIRECTORY,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True
                )
                stdout, stderr = process.communicate()
                
                success = process.returncode == 0
                print(f"[SERVER] Compilation finished with exit code {process.returncode}")
                
                response = {
                    "success": success,
                    "stdout": stdout,
                    "stderr": stderr
                }
                
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps(response).encode('utf-8'))
            except Exception as e:
                print(f"[SERVER] Compile execution error: {e}")
                response = {
                    "success": False,
                    "error": str(e)
                }
                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps(response).encode('utf-8'))
        
        # Handle remote control input events (taps, scrolls/swipes)
        elif self.path == '/api/control':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            try:
                payload = json.loads(post_data.decode('utf-8'))
                action = payload.get("action")
                
                if action == "tap":
                    x_pct = payload.get("x_pct", 0)
                    y_pct = payload.get("y_pct", 0)
                    x = int(x_pct * 1080)
                    y = int(y_pct * 2412)
                    cmd = ["C:\\Users\\sasit\\AppData\\Local\\Android\\Sdk\\platform-tools\\adb.exe", "-s", "VKEU5DJNIFY9JJFU", "shell", "input", "tap", str(x), str(y)]
                    subprocess.Popen(cmd)
                    
                elif action == "swipe":
                    x1_pct = payload.get("x1_pct", 0)
                    y1_pct = payload.get("y1_pct", 0)
                    x2_pct = payload.get("x2_pct", 0)
                    y2_pct = payload.get("y2_pct", 0)
                    duration = max(50, payload.get("duration", 300))
                    
                    x1 = int(x1_pct * 1080)
                    y1 = int(y1_pct * 2412)
                    x2 = int(x2_pct * 1080)
                    y2 = int(y2_pct * 2412)
                    cmd = ["C:\\Users\\sasit\\AppData\\Local\\Android\\Sdk\\platform-tools\\adb.exe", "-s", "VKEU5DJNIFY9JJFU", "shell", "input", "swipe", str(x1), str(y1), str(x2), str(y2), str(duration)]
                    subprocess.Popen(cmd)
                
                response = {"success": True}
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps(response).encode('utf-8'))
            except Exception as e:
                response = {"success": False, "error": str(e)}
                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps(response).encode('utf-8'))
            return

        # Handle remote trigger command to a robot
        elif self.path == '/api/trigger':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            try:
                payload = json.loads(post_data.decode('utf-8'))
                mac = payload.get("mac", "").upper()
                command = payload.get("command", "")
                
                success = False
                with robots_lock:
                    if mac in connected_robots:
                        ws_client = connected_robots[mac]["socket"]
                        if ws_loop:
                            # Send command via the active WebSocket connection
                            asyncio.run_coroutine_threadsafe(ws_client.send(command), ws_loop)
                            success = True
                
                response = {
                    "success": success,
                    "message": f"Command '{command}' sent to {mac}" if success else f"Robot {mac} is offline"
                }
                self.send_response(200 if success else 404)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps(response).encode('utf-8'))
            except Exception as e:
                response = {"success": False, "error": str(e)}
                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps(response).encode('utf-8'))
        else:
            self.send_error(404, "File not found")

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()


# ----------------- WEBSOCKET SERVER -----------------
async def ws_handler(websocket):
    path = getattr(websocket, 'path', '/')
    parsed_url = urllib.parse.urlparse(path)
    params = urllib.parse.parse_qs(parsed_url.query)
    
    mac = params.get('mac', [None])[0]
    variant = params.get('variant', ['mr_luna'])[0]
    
    if not mac:
        print("[WS Server] Closed connection: Missing 'mac' parameter.")
        await websocket.close()
        return
        
    mac = mac.upper()
    print(f"[WS Server] Robot {mac} ({variant}) connected!")
    
    with robots_lock:
        connected_robots[mac] = {
            "socket": websocket,
            "variant": variant,
            "status": "online"
        }
        
    try:
        async for message in websocket:
            print(f"[WS Server] Received from {mac}: {message}")
    except websockets.ConnectionClosed:
        pass
    finally:
        print(f"[WS Server] Robot {mac} disconnected.")
        with robots_lock:
            if mac in connected_robots:
                del connected_robots[mac]

async def ws_server_main():
    global ws_loop
    ws_loop = asyncio.get_running_loop()
    print("[WS Server] Listening on ws://0.0.0.0:8001")
    async with websockets.serve(ws_handler, "0.0.0.0", 8001):
        await asyncio.Future()  # Keep running forever

def start_ws_server():
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    loop.run_until_complete(ws_server_main())


# ----------------- UDP DISCOVERY SERVER -----------------
def start_udp_discovery():
    print("[UDP Discovery] Listening on UDP port 8002")
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        udp_socket.bind(("", 8002))
    except Exception as e:
        print(f"[UDP Discovery] Bind failed: {e}")
        return
        
    while True:
        try:
            data, addr = udp_socket.recvfrom(1024)
            message = data.decode('utf-8', errors='ignore').strip()
            if message == "MR_LUNA_DISCOVER":
                print(f"[UDP Discovery] Discover ping from {addr}")
                # Respond to indicate discovery server presence
                udp_socket.sendto(b"MR_LUNA_SERVER_HERE", addr)
        except Exception as e:
            print(f"[UDP Discovery] Error: {e}")


# ----------------- MAIN RUNNER -----------------
if __name__ == '__main__':
    print(f"==================================================")
    print(f"Starting Mr.&Ms Luna Cloud & Dev Server Stack")
    print(f"Serving static files from: {os.path.join(DIRECTORY, 'mobile_app')}")
    print(f"API endpoints: http://localhost:{PORT}")
    print(f"WebSocket broker: ws://localhost:8001")
    print(f"UDP Discovery: Port 8002")
    print(f"==================================================")
    
    # 1. Start WebSocket server thread
    ws_thread = threading.Thread(target=start_ws_server, daemon=True)
    ws_thread.start()
    
    # 2. Start UDP Discovery thread
    udp_thread = threading.Thread(target=start_udp_discovery, daemon=True)
    udp_thread.start()
    
    # 3. Start Screen Capture worker thread
    start_screen_capture_worker()
    
    # 3. Start HTTP server
    socketserver.ThreadingTCPServer.allow_reuse_address = True
    with socketserver.ThreadingTCPServer(("", PORT), DualStackServer) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down server.")
            httpd.server_close()
