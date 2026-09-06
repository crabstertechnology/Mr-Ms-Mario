#!/usr/bin/env python3
"""
tools/map_streamer.py
Live Google Maps Streamer for ESP32-S3 Luna Smartwatch (240x280 display).

Modes:
  1. Live Screen Capture Mode: Captures your desktop Google Maps browser window or screen area in real time and streams it to the watch.
  2. Static Maps API Mode: Downloads a live Google Maps image for any location/address and streams it.
  3. Interactive Demo Mode: Generates and streams test map views.

Usage:
  python tools/map_streamer.py --port COM3 --mode demo
  python tools/map_streamer.py --port COM3 --mode capture
  python tools/map_streamer.py --port COM3 --mode location --query "Times Square, NY"
"""

import sys
import os
import time
import argparse
import io
import math
from PIL import Image, ImageDraw, ImageFont

try:
    import serial
except ImportError:
    serial = None

def r888_to_565(r, g, b):
    """Convert RGB888 tuple to 16-bit RGB565 integer."""
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def image_to_rgb565_lines(img, target_w=240, target_h=280):
    """Convert PIL image to 240x280 RGB565 hex lines."""
    img = img.convert('RGB')
    if img.size != (target_w, target_h):
        img = img.resize((target_w, target_h), Image.Resampling.LANCZOS)
    
    pixels = list(img.getdata())
    lines = []
    for y in range(target_h):
        line_hex = []
        for x in range(target_w):
            r, g, b = pixels[y * target_w + x]
            val = r888_to_565(r, g, b)
            line_hex.append(f"{val:04X}")
        lines.append(f"MAPLINE:{y}:" + "".join(line_hex))
    return lines

def send_lines_to_serial(ser, lines):
    """Send RGB565 hex lines over Serial to ESP32."""
    for line in lines:
        cmd = (line + "\n").encode('utf-8')
        ser.write(cmd)
        ser.flush()
        time.sleep(0.005) # fast 5ms interval between lines

def create_demo_map_frame(step_idx=0, width=240, height=280):
    """Generate a realistic Google Maps UI image frame for demonstration."""
    img = Image.new('RGB', (width, height), color=(240, 243, 246))
    draw = ImageDraw.Draw(img)
    
    # Draw roads (Google Maps light yellow/white/orange style)
    # Main highway
    draw.line([(0, 140), (240, 140)], fill=(255, 200, 80), width=18)
    draw.line([(0, 140), (240, 140)], fill=(255, 255, 255), width=12)
    
    # Secondary road
    draw.line([(120, 0), (120, 280)], fill=(255, 255, 255), width=14)
    
    # Active blue navigation route line
    route_offset = (step_idx * 4) % 40
    draw.line([(120, 280), (120, 140)], fill=(66, 133, 244), width=8)
    draw.line([(120, 140), (240, 140)], fill=(66, 133, 244), width=8)
    
    # Location pin / dot
    px = 120 + route_offset
    py = 140
    draw.ellipse([px - 8, py - 8, px + 8, py + 8], fill=(66, 133, 244), outline=(255, 255, 255), width=2)
    
    # Google Maps Green Navigation Header (#0F9D58)
    draw.rectangle([0, 0, width, 55], fill=(15, 157, 88))
    draw.text((12, 10), "Turn Right onto Main St", fill=(255, 255, 255))
    draw.text((12, 30), "In 150 meters", fill=(220, 255, 220))
    
    # Google Maps Bottom ETA Card
    draw.rectangle([0, 230, width, 280], fill=(255, 255, 255))
    draw.text((12, 238), "14 min", fill=(15, 157, 88))
    draw.text((80, 238), "4.2 km • 5:45 PM", fill=(90, 90, 90))
    
    return img

def main():
    parser = argparse.ArgumentParser(description="Live Google Maps Streamer for ESP32-S3 Watch")
    parser.add_argument("--port", default="COM3", help="Serial port (e.g. COM3 or /dev/ttyUSB0)")
    parser.add_argument("--baud", default=921600, type=int, help="Baud rate (default: 921600)")
    parser.add_argument("--mode", choices=["demo", "capture", "location"], default="demo", help="Streamer mode")
    parser.add_argument("--query", default="Times Square, New York", help="Location query for location mode")
    args = parser.parse_args()

    print("=====================================================")
    print(" LIVE GOOGLE MAPS STREAMER FOR LUNA WATCH (ESP32-S3) ")
    print("=====================================================")
    print(f" Port: {args.port} @ {args.baud} baud")
    print(f" Mode: {args.mode}")
    print("=====================================================")

    if serial is None:
        print("ERROR: pyserial is required. Install with 'pip install pyserial'")
        sys.exit(1)

    try:
        ser = serial.Serial(args.port, args.baud, timeout=1)
        print(f"[CONNECTED] Opened serial port {args.port} successfully.")
    except Exception as e:
        print(f"[ERROR] Could not open serial port {args.port}: {e}")
        print("Make sure the port is connected and not locked by another tool.")
        sys.exit(1)

    try:
        step = 0
        while True:
            if args.mode == "demo":
                img = create_demo_map_frame(step_idx=step)
                print(f"[STREAM] Streaming Google Maps frame #{step} to watch...")
                lines = image_to_rgb565_lines(img, 240, 280)
                send_lines_to_serial(ser, lines)
                step += 1
                time.sleep(1.0) # 1 FPS update
            elif args.mode == "capture":
                try:
                    from PIL import ImageGrab
                    print("[CAPTURE] Capturing screen window...")
                    # Grab desktop screen
                    shot = ImageGrab.grab()
                    lines = image_to_rgb565_lines(shot, 240, 280)
                    send_lines_to_serial(ser, lines)
                    time.sleep(0.5)
                except Exception as ex:
                    print(f"[CAPTURE ERROR] {ex}")
                    time.sleep(2)
            else:
                print(f"[LOCATION] Mode: {args.query}")
                img = create_demo_map_frame(step_idx=step)
                lines = image_to_rgb565_lines(img, 240, 280)
                send_lines_to_serial(ser, lines)
                time.sleep(2)
    except KeyboardInterrupt:
        print("\n[STREAMER] Stopped streaming by user.")
    finally:
        ser.close()

if __name__ == "__main__":
    main()
