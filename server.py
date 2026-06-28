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

PORT = 8000
DIRECTORY = os.path.dirname(os.path.abspath(__file__))

# Threading Lock and active robots dictionary
robots_lock = threading.Lock()
connected_robots = {} # MAC -> { "socket": websocket, "variant": variant, "status": "online" }
ws_loop = None

# ----------------- HTTP REQUEST HANDLER -----------------
class DualStackServer(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        # Serve the mobile_app directory by default
        super().__init__(*args, directory=os.path.join(DIRECTORY, "mobile_app"), **kwargs)

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
    variant = params.get('variant', ['mr_mario'])[0]
    
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
            if message == "MR_MARIO_DISCOVER":
                print(f"[UDP Discovery] Discover ping from {addr}")
                # Respond to indicate discovery server presence
                udp_socket.sendto(b"MR_MARIO_SERVER_HERE", addr)
        except Exception as e:
            print(f"[UDP Discovery] Error: {e}")


# ----------------- MAIN RUNNER -----------------
if __name__ == '__main__':
    print(f"==================================================")
    print(f"Starting Mr. Mario Cloud & Dev Server Stack")
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
    
    # 3. Start HTTP server
    socketserver.ThreadingTCPServer.allow_reuse_address = True
    with socketserver.ThreadingTCPServer(("", PORT), DualStackServer) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down server.")
            httpd.server_close()
