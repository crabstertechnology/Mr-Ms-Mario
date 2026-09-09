#!/usr/bin/env python3
"""
Luna Display Studio - Local Web Launcher
Starts a lightweight local HTTP server and opens the studio in your browser.
"""

import http.server
import socketserver
import webbrowser
import os
import sys

PORT = 8080

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=os.path.dirname(os.path.abspath(__file__)), **kwargs)

    def log_message(self, format, *args):
        # Keep console output clean
        pass

def main():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    socketserver.TCPServer.allow_reuse_address = True
    try:
        with socketserver.TCPServer(("", PORT), Handler) as httpd:
            url = f"http://localhost:{PORT}"
            print(f"==================================================")
            print(f"  🌙 LUNA DISPLAY STUDIO (ESP32-S3 1.69\" UI MAKER)")
            print(f"  Running locally at: {url}")
            print(f"  Press Ctrl+C in terminal to stop.")
            print(f"==================================================")
            webbrowser.open(url)
            httpd.serve_forever()
    except OSError as e:
        if "Address already in use" in str(e) or "10048" in str(e):
            # Port in use, try alternate
            alt_port = 8081
            with socketserver.TCPServer(("", alt_port), Handler) as httpd:
                url = f"http://localhost:{alt_port}"
                print(f"Running locally at: {url}")
                webbrowser.open(url)
                httpd.serve_forever()
        else:
            raise e

if __name__ == "__main__":
    main()
