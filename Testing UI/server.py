import os
import json
import base64
from http.server import HTTPServer, SimpleHTTPRequestHandler

class SaveHandler(SimpleHTTPRequestHandler):
    def do_POST(self):
        if self.path == '/save':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            
            try:
                # The body will be JSON containing the base64 GIF
                payload = json.loads(post_data.decode('utf-8'))
                gif_data_b64 = payload['gif']
                
                # Remove the data URL header if present
                if gif_data_b64.startswith('data:'):
                    gif_data_b64 = gif_data_b64.split(',')[1]
                
                gif_bytes = base64.b64decode(gif_data_b64)
                
                output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'Untitled file.gif')
                with open(output_path, 'wb') as f:
                    f.write(gif_bytes)
                
                print(f"[SERVER] Successfully saved GIF to {output_path}")
                
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(b'{"status":"success"}')
                
                # Graceful shutdown after serving the request
                import threading
                def shutdown_server(server):
                    server.shutdown()
                threading.Thread(target=shutdown_server, args=(self.server,)).start()
                
            except Exception as e:
                print(f"[SERVER] Error: {e}")
                self.send_response(500)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(f'{{"status":"error", "message":"{str(e)}"}}'.encode('utf-8'))
        else:
            self.send_response(404)
            self.end_headers()

    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        super().end_headers()

def run(port=8000):
    server_address = ('', port)
    # Serve files from the directory where server.py is located
    current_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(current_dir)
    httpd = HTTPServer(server_address, SaveHandler)
    print(f"[SERVER] Serving files from {current_dir} on port {port}...")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    print("[SERVER] Stopped.")

if __name__ == '__main__':
    run()
