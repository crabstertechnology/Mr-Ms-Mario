import os
import sys
import ftplib
import time

FTP_HOST = "10.112.227.163"
FTP_PORT = 2221
FTP_USER = "android"
FTP_PASS = "android"

def upload_apk(apk_path):
    if not os.path.exists(apk_path):
        print(f"Error: APK not found at {apk_path}")
        return False

    file_size = os.path.getsize(apk_path)
    file_name = os.path.basename(apk_path)
    print(f"Connecting to phone FTP server at {FTP_HOST}:{FTP_PORT}...")

    ftp = ftplib.FTP()
    ftp.connect(FTP_HOST, FTP_PORT, timeout=15)
    ftp.login(FTP_USER, FTP_PASS)
    print("Logged in successfully.")

    # Target folder: Download
    target_dir = "Download"
    try:
        ftp.cwd(target_dir)
        print(f"Changed directory to /{target_dir}")
    except Exception as e:
        print(f"Could not change to /{target_dir}, uploading to root: {e}")

    print(f"Uploading {file_name} ({file_size / (1024*1024):.2f} MB)...")
    start_time = time.time()
    uploaded_bytes = 0

    def progress_callback(chunk):
        nonlocal uploaded_bytes
        uploaded_bytes += len(chunk)
        pct = (uploaded_bytes / file_size) * 100
        elapsed = time.time() - start_time
        speed = (uploaded_bytes / 1024) / max(elapsed, 0.001)
        sys.stdout.write(f"\rProgress: {pct:.1f}% ({uploaded_bytes // (1024*1024)}/{file_size // (1024*1024)} MB) @ {speed:.1f} KB/s")
        sys.stdout.flush()

    with open(apk_path, "rb") as f:
        ftp.storbinary(f"STOR {file_name}", f, blocksize=65536, callback=progress_callback)

    print("\nUpload complete!")
    ftp.quit()
    print(f"Successfully transferred {file_name} directly to your phone's /{target_dir} folder via WiFi!")
    return True

if __name__ == "__main__":
    apk = sys.argv[1] if len(sys.argv) > 1 else "mobile_flutter/build/app/outputs/flutter-apk/app-debug.apk"
    upload_apk(apk)
