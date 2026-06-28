import sys
import os
import subprocess
import io
import time
import tkinter as tk
from PIL import Image, ImageTk

# Configuration
ADB_PATH = r"C:\Users\sasit\AppData\Local\Android\Sdk\platform-tools\adb.exe"
DEVICE_ID = "VKEU5DJNIFY9JJFU"
TARGET_WIDTH = 400

class ScreenMirrorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Mr. Mario Companion App Mirror")
        self.root.geometry("400x820")
        self.root.configure(bg="#0E0D1A")
        
        # Title Bar / Status label
        self.status_label = tk.Label(
            root, 
            text="Initializing connection...", 
            fg="#00F0FF", 
            bg="#161526", 
            font=("Helvetica", 10, "bold"),
            pady=6
        )
        self.status_label.pack(fill=tk.X)
        
        # Image Display Label
        self.canvas_label = tk.Label(root, bg="#0E0D1A")
        self.canvas_label.pack(fill=tk.BOTH, expand=True)
        
        self.running = True
        self.last_frame_time = time.time()
        
        # Start update loop
        self.update_frame()
        
    def update_frame(self):
        if not self.running:
            return
            
        try:
            # Fetch screenshot via adb exec-out
            cmd = [ADB_PATH, "-s", DEVICE_ID, "exec-out", "screencap", "-p"]
            img_bytes = subprocess.check_output(cmd)
            
            if img_bytes:
                # Load image
                image = Image.open(io.BytesIO(img_bytes))
                
                # Calculate scale height
                w, h = image.size
                scale = TARGET_WIDTH / w
                target_height = int(h * scale)
                
                # Resize
                resized_image = image.resize((TARGET_WIDTH, target_height), Image.Resampling.LANCZOS)
                
                # Convert to PhotoImage
                photo = ImageTk.PhotoImage(resized_image)
                
                # Update UI
                self.canvas_label.configure(image=photo)
                self.canvas_label.image = photo # Keep reference
                
                # Calculate FPS
                now = time.time()
                fps = 1.0 / (now - self.last_frame_time) if (now - self.last_frame_time) > 0 else 0
                self.last_frame_time = now
                
                self.status_label.configure(
                    text=f"Mirroring live | {TARGET_WIDTH}x{target_height} | FPS: {fps:.1f}",
                    fg="#10B981"
                )
        except Exception as e:
            self.status_label.configure(text=f"Connection Error: {e}", fg="#EF4444")
            
        # Schedule next update (approx 15-20 FPS)
        if self.running:
            self.root.after(50, self.update_frame)

if __name__ == "__main__":
    root = tk.Tk()
    app = ScreenMirrorApp(root)
    
    # Close window handler
    def on_closing():
        app.running = False
        root.destroy()
        
    root.protocol("WM_DELETE_WINDOW", on_closing)
    root.mainloop()
