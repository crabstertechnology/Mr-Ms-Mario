#!/usr/bin/env python3
"""
Luna File Sharing Server — Share files across your local network.
Accessible from any laptop, phone, or tablet via a web browser.
"""

import os
import sys
import html
import urllib.parse
import mimetypes
import shutil
import socket
from http.server import HTTPServer, BaseHTTPRequestHandler

if sys.stdout and hasattr(sys.stdout, 'reconfigure'):
    try:
        sys.stdout.reconfigure(encoding='utf-8')
    except Exception:
        pass

PORT = 8000
ROOT_DIR = os.path.abspath(os.path.dirname(__file__))

def get_local_ip():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.settimeout(0.5)
        # Connect to an external address to get default outbound interface IP
        s.connect(('8.8.8.8', 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        return '127.0.0.1'

class FileShareHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        print(f"[{self.log_date_time_string()}] {self.address_string()} - {format % args}")

    def do_GET(self):
        url_path = urllib.parse.unquote(urllib.parse.urlparse(self.path).path)
        # Prevent directory traversal
        norm_path = os.path.normpath(url_path.lstrip('/\\'))
        full_path = os.path.join(ROOT_DIR, norm_path)

        # Check path security
        if not os.path.commonpath([ROOT_DIR, full_path]) == ROOT_DIR:
            self.send_error(403, "Access Denied")
            return

        if not os.path.exists(full_path):
            self.send_error(404, "File or Directory Not Found")
            return

        if os.path.isdir(full_path):
            self.send_directory_listing(full_path, url_path)
        else:
            self.send_file(full_path)

    def do_POST(self):
        """Handle file uploads from other laptops / devices."""
        url_path = urllib.parse.unquote(urllib.parse.urlparse(self.path).path)
        norm_path = os.path.normpath(url_path.lstrip('/\\'))
        target_dir = os.path.join(ROOT_DIR, norm_path)

        if not os.path.isdir(target_dir):
            target_dir = ROOT_DIR

        content_type = self.headers.get('Content-Type', '')
        if 'multipart/form-data' not in content_type:
            self.send_error(400, "Bad Request: Expected multipart/form-data")
            return

        boundary = content_type.split('boundary=')[-1].encode()
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length)

        parts = body.split(b'--' + boundary)
        uploaded_count = 0

        for part in parts:
            if b'filename="' in part:
                headers_part, file_data = part.split(b'\r\n\r\n', 1)
                file_data = file_data.rstrip(b'\r\n')
                headers_text = headers_part.decode('latin1', errors='ignore')
                filename_idx = headers_text.find('filename="')
                if filename_idx != -1:
                    raw_filename = headers_text[filename_idx + 10:].split('"')[0]
                    safe_filename = os.path.basename(raw_filename)
                    if safe_filename:
                        save_path = os.path.join(target_dir, safe_filename)
                        with open(save_path, 'wb') as f:
                            f.write(file_data)
                        uploaded_count += 1

        # Redirect back to the folder
        self.send_response(303)
        self.send_header('Location', url_path or '/')
        self.end_headers()

    def send_file(self, full_path):
        mime_type, _ = mimetypes.guess_type(full_path)
        if not mime_type:
            mime_type = 'application/octet-stream'

        try:
            file_size = os.path.getsize(full_path)
            self.send_response(200)
            self.send_header('Content-Type', mime_type)
            self.send_header('Content-Length', str(file_size))
            filename = os.path.basename(full_path)
            # Suggest download for binaries/archives
            if any(full_path.lower().endswith(ext) for ext in ['.zip', '.tar', '.gz', '.bin', '.exe', '.ino', '.hex']):
                self.send_header('Content-Disposition', f'attachment; filename="{filename}"')
            self.end_headers()

            with open(full_path, 'rb') as f:
                shutil.copyfileobj(f, self.wfile)
        except Exception as e:
            self.send_error(500, f"Error sending file: {e}")

    def send_directory_listing(self, dir_path, url_path):
        try:
            entries = os.listdir(dir_path)
        except Exception as e:
            self.send_error(403, f"Unable to read directory: {e}")
            return

        entries.sort(key=lambda s: (not os.path.isdir(os.path.join(dir_path, s)), s.lower()))

        # Parent directory link
        parent_url = None
        clean_url = url_path.rstrip('/')
        if clean_url:
            parent_url = os.path.dirname(clean_url) or '/'

        rows = []
        for name in entries:
            # Skip hidden files
            if name.startswith('.'):
                continue
            item_path = os.path.join(dir_path, name)
            is_dir = os.path.isdir(item_path)
            item_url = urllib.parse.quote(f"{clean_url}/{name}") if clean_url else urllib.parse.quote(f"/{name}")
            icon = "📁" if is_dir else self.get_icon_for_ext(name)

            if is_dir:
                size_str = "Folder"
            else:
                try:
                    sz = os.path.getsize(item_path)
                    size_str = self.format_size(sz)
                except Exception:
                    size_str = "--"

            try:
                mtime = os.path.getmtime(item_path)
                import datetime
                date_str = datetime.datetime.fromtimestamp(mtime).strftime('%Y-%m-%d %H:%M')
            except Exception:
                date_str = "--"

            rows.append(f"""
            <tr>
              <td class="icon">{icon}</td>
              <td class="name">
                <a href="{item_url}">{html.escape(name)}</a>
              </td>
              <td class="size">{size_str}</td>
              <td class="date">{date_str}</td>
              <td class="actions">
                {f'<a href="{item_url}" download class="btn-download">↓ Download</a>' if not is_dir else ''}
              </td>
            </tr>
            """)

        hostname = socket.gethostname()
        local_ip = get_local_ip()

        page_html = f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Shared Network Files — {html.escape(hostname)}</title>
  <style>
    :root {{
      --bg: #0f172a;
      --card-bg: #1e293b;
      --border: #334155;
      --text: #f8fafc;
      --text-dim: #94a3b8;
      --accent: #38bdf8;
      --accent-hover: #0284c7;
      --row-hover: #283548;
    }}
    * {{ box-sizing: border-box; margin: 0; padding: 0; }}
    body {{
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
      background: var(--bg);
      color: var(--text);
      min-height: 100vh;
      padding: 24px;
    }}
    .container {{
      max-width: 1000px;
      margin: 0 auto;
      background: var(--card-bg);
      border-radius: 12px;
      border: 1px solid var(--border);
      box-shadow: 0 10px 25px rgba(0,0,0,0.5);
      overflow: hidden;
    }}
    header {{
      background: #111827;
      padding: 20px 24px;
      border-bottom: 1px solid var(--border);
      display: flex;
      justify-content: space-between;
      align-items: center;
      flex-wrap: wrap;
      gap: 12px;
    }}
    .title-area h1 {{
      font-size: 20px;
      font-weight: 700;
      color: #fff;
      display: flex;
      align-items: center;
      gap: 10px;
    }}
    .title-area p {{
      font-size: 13px;
      color: var(--text-dim);
      margin-top: 4px;
    }}
    .badge {{
      display: inline-flex;
      align-items: center;
      gap: 6px;
      background: rgba(56, 189, 248, 0.15);
      color: var(--accent);
      border: 1px solid rgba(56, 189, 248, 0.4);
      padding: 6px 12px;
      border-radius: 20px;
      font-size: 12px;
      font-family: monospace;
    }}
    .upload-box {{
      padding: 16px 24px;
      background: rgba(15, 23, 42, 0.6);
      border-bottom: 1px solid var(--border);
      display: flex;
      align-items: center;
      justify-content: space-between;
      flex-wrap: wrap;
      gap: 12px;
    }}
    .upload-form {{
      display: flex;
      align-items: center;
      gap: 10px;
    }}
    .upload-form input[type="file"] {{
      font-size: 13px;
      color: var(--text-dim);
    }}
    .btn {{
      padding: 6px 14px;
      border-radius: 6px;
      border: none;
      font-size: 13px;
      font-weight: 600;
      cursor: pointer;
      text-decoration: none;
      transition: all 0.15s;
    }}
    .btn-primary {{
      background: var(--accent);
      color: #0f172a;
    }}
    .btn-primary:hover {{
      background: var(--accent-hover);
      color: #fff;
    }}
    .nav-bar {{
      padding: 12px 24px;
      background: #1e293b;
      border-bottom: 1px solid var(--border);
      font-size: 13px;
      display: flex;
      align-items: center;
      gap: 8px;
    }}
    .nav-bar a {{
      color: var(--accent);
      text-decoration: none;
    }}
    .nav-bar a:hover {{ text-decoration: underline; }}
    table {{
      width: 100%;
      border-collapse: collapse;
      text-align: left;
    }}
    th {{
      background: #111827;
      padding: 12px 18px;
      font-size: 12px;
      font-weight: 600;
      color: var(--text-dim);
      text-transform: uppercase;
      letter-spacing: 0.5px;
      border-bottom: 1px solid var(--border);
    }}
    td {{
      padding: 12px 18px;
      font-size: 14px;
      border-bottom: 1px solid rgba(51, 65, 85, 0.5);
    }}
    tr:hover td {{
      background: var(--row-hover);
    }}
    .icon {{ width: 32px; font-size: 16px; text-align: center; }}
    .name a {{
      color: #f1f5f9;
      text-decoration: none;
      font-weight: 500;
    }}
    .name a:hover {{
      color: var(--accent);
      text-decoration: underline;
    }}
    .size, .date {{
      color: var(--text-dim);
      font-size: 13px;
      font-family: monospace;
    }}
    .btn-download {{
      font-size: 12px;
      color: var(--accent);
      text-decoration: none;
      border: 1px solid var(--border);
      padding: 4px 8px;
      border-radius: 4px;
    }}
    .btn-download:hover {{
      background: rgba(56, 189, 248, 0.2);
      border-color: var(--accent);
    }}
    footer {{
      padding: 16px 24px;
      text-align: center;
      font-size: 12px;
      color: var(--text-dim);
      border-top: 1px solid var(--border);
      background: #111827;
    }}
  </style>
</head>
<body>
  <div class="container">
    <header>
      <div class="title-area">
        <h1>🌐 Laptop File Sharing Server</h1>
        <p>Serving files from <code>{html.escape(ROOT_DIR)}</code></p>
      </div>
      <div class="badge">
        <span>● LIVE:</span>
        <span>http://{local_ip}:{PORT}</span>
      </div>
    </header>

    <div class="upload-box">
      <div style="font-size: 13px; font-weight: 600; color: #cbd5e1;">
        📤 Send files to this laptop:
      </div>
      <form class="upload-form" method="POST" enctype="multipart/form-data">
        <input type="file" name="upload_files" multiple required>
        <button type="submit" class="btn btn-primary">Upload to this folder</button>
      </form>
    </div>

    <div class="nav-bar">
      <span>Location:</span>
      <a href="/">root</a>
      {' / '.join([f'<a href="{urllib.parse.quote("/" + "/".join(clean_url.strip("/").split("/")[:idx+1]))}">{html.escape(part)}</a>' for idx, part in enumerate(clean_url.strip("/").split("/")) if part]) if clean_url.strip("/") else ''}
    </div>

    <table>
      <thead>
        <tr>
          <th style="width: 40px;"></th>
          <th>Name</th>
          <th style="width: 120px;">Size</th>
          <th style="width: 160px;">Modified</th>
          <th style="width: 110px;"></th>
        </tr>
      </thead>
      <tbody>
        {f'<tr><td class="icon">⬆️</td><td class="name" colspan="4"><a href="{parent_url}">.. (Up to parent folder)</a></td></tr>' if parent_url else ''}
        {''.join(rows) if rows else '<tr><td colspan="5" style="text-align: center; color: var(--text-dim); padding: 30px;">Empty folder</td></tr>'}
      </tbody>
    </table>

    <footer>
      Connect from any other laptop or mobile device on the same Wi-Fi using <strong>http://{local_ip}:{PORT}</strong>
    </footer>
  </div>
</body>
</html>"""

        encoded = page_html.encode('utf-8')
        self.send_response(200)
        self.send_header('Content-Type', 'text/html; charset=utf-8')
        self.send_header('Content-Length', str(len(encoded)))
        self.end_headers()
        self.wfile.write(encoded)

    def get_icon_for_ext(self, name):
        ext = os.path.splitext(name)[-1].lower()
        if ext in ['.ino', '.cpp', '.c', '.h', '.hpp', '.py', '.js', '.jsx', '.json', '.html', '.css']:
            return "📄"
        if ext in ['.png', '.jpg', '.jpeg', '.gif', '.webp', '.bmp', '.svg', '.ico']:
            return "🖼️"
        if ext in ['.zip', '.tar', '.gz', '.7z', '.rar']:
            return "📦"
        if ext in ['.mp3', '.wav', '.ogg']:
            return "🎵"
        if ext in ['.mp4', '.mov', '.webm', '.avi']:
            return "🎬"
        if ext in ['.pdf', '.doc', '.docx', '.txt', '.md']:
            return "📝"
        return "📄"

    def format_size(self, size):
        for unit in ['B', 'KB', 'MB', 'GB', 'TB']:
            if size < 1024.0:
                return f"{size:.1f} {unit}" if unit != 'B' else f"{int(size)} B"
            size /= 1024.0
        return f"{size:.1f} PB"

def run_server():
    server_address = ('0.0.0.0', PORT)
    httpd = HTTPServer(server_address, FileShareHandler)
    local_ip = get_local_ip()
    hostname = socket.gethostname()

    print("=" * 65)
    print(">> Luna File Sharing Server is running!")
    print(f"Directory       : {ROOT_DIR}")
    print("=" * 65)
    print(f"Local access    : http://localhost:{PORT}")
    print(f"Network access  : http://{local_ip}:{PORT}")
    print(f"Hostname access : http://{hostname}:{PORT}")
    print("=" * 65)
    print("Press Ctrl+C to stop the server.\n")

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping File Sharing Server...")
        httpd.server_close()

if __name__ == '__main__':
    run_server()
