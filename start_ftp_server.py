#!/usr/bin/env python3
"""
Luna FTP Server for Windows File Explorer
Allows other laptops to access this PC via standard FTP: ftp://<IP>/
Matches the Network Locations setup in Windows "This PC".
"""

import os
import sys
import socket
import warnings

# Suppress pyftpdlib anonymous write warning
warnings.filterwarnings('ignore', category=RuntimeWarning)

from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import FTPServer

# Configuration
PORT = 21
SHARED_DIRECTORY = r'W:\Mr.mario'

def get_local_ip():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.settimeout(0.5)
        s.connect(('8.8.8.8', 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        return '172.16.49.234'

def main():
    local_ip = get_local_ip()
    hostname = socket.gethostname()

    # Setup permissions: e=cwd, l=list, r=read, a=append, d=delete, f=rename, m=mkdir, w=write, M=mode, T=time
    authorizer = DummyAuthorizer()
    
    # 1. Anonymous user with full read & write access
    authorizer.add_anonymous(SHARED_DIRECTORY, perm='elradfmwMT')

    # 2. Also add user/pass option if preferred
    authorizer.add_user('admin', 'admin', SHARED_DIRECTORY, perm='elradfmwMT')

    handler = FTPHandler
    handler.authorizer = authorizer
    handler.banner = "Luna FTP Server Ready."

    # Configure passive ports for Windows File Explorer compatibility
    handler.passive_ports = range(60000, 60050)
    handler.masquerade_address = local_ip

    print("=" * 65)
    print(">> Luna FTP Server is ACTIVE!")
    print(f"Directory Shared  : {SHARED_DIRECTORY}")
    print("=" * 65)
    print(f"FTP Address       : ftp://{local_ip}")
    print(f"Hostname Address  : ftp://{hostname}")
    print("Port              : 21 (Standard FTP)")
    print("Access Mode       : Anonymous (No password required)")
    print("=" * 65)
    print("\nHow to connect from another laptop:")
    print(f"1. Open File Explorer on the other laptop.")
    print(f"2. Right-click 'This PC' -> click 'Add a network location'.")
    print(f"3. Type: ftp://{local_ip}")
    print(f"4. Check 'Log on anonymously' -> click Next -> Finish!")
    print(f"5. It will appear directly under 'Network locations' in This PC!\n")
    print("Press Ctrl+C to stop the server.\n")

    try:
        server = FTPServer(('0.0.0.0', PORT), handler)
        server.serve_forever()
    except PermissionError:
        # Fallback to port 2121 if port 21 is blocked by system policy
        ALT_PORT = 2121
        print(f"[!] Port 21 requires elevation or is occupied. Switching to port {ALT_PORT}...")
        print(f"FTP Address: ftp://{local_ip}:{ALT_PORT}")
        server = FTPServer(('0.0.0.0', ALT_PORT), handler)
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping FTP server...")

if __name__ == '__main__':
    main()
