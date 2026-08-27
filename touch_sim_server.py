"""
touch_sim_server.py  —  Luna Touch Zone Simulator + Serial Monitor
Reads COM3, parses [Touch] lines for zone events, and broadcasts
BOTH the parsed zone events AND every raw serial line to the browser.

Message types sent over WebSocket:
  {"type": "touch",  "zone": "LEFT|CENTER|RIGHT|LONG", "x": 45, "action": "..."}
  {"type": "serial", "line": "<raw text from watch>"}
"""

import asyncio, json, re, serial, threading, websockets

PORT    = "COM3"
BAUD    = 115200
WS_PORT = 8765
clients = set()

ZONE_RE = re.compile(r'\[Touch\]\s+(LEFT|CENTER|RIGHT)\s+tap\s+\(X=(\d+)\)', re.IGNORECASE)
LONG_RE = re.compile(r'\[Touch\]\s+Long\s+press', re.IGNORECASE)

ACTIONS = {
    "LEFT":   "Prev Screen",
    "CENTER": "Select / Interact",
    "RIGHT":  "Next Screen",
}

# ── broadcast helper (fire-and-forget) ───────────────────────────────────────
def broadcast_sync(payload: dict):
    msg = json.dumps(payload)
    for ws in list(clients):
        asyncio.run_coroutine_threadsafe(ws.send(msg), loop)

# ── serial reader thread ──────────────────────────────────────────────────────
def serial_reader():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=0.1)
        print(f"[Serial] Connected to {PORT} @ {BAUD}")
        broadcast_sync({"type": "serial", "line": f"-- Connected to {PORT} @ {BAUD} --"})
    except Exception as e:
        print(f"[Serial] ERROR: {e}")
        broadcast_sync({"type": "serial", "line": f"ERROR: Cannot open {PORT}: {e}"})
        return

    while True:
        try:
            raw = ser.readline()
            if not raw:
                continue
            line = raw.decode("utf-8", errors="ignore").strip()
            if not line:
                continue

            print(f"[Serial] {line}")

            # Always send the raw line to serial monitor
            broadcast_sync({"type": "serial", "line": line})

            # Also send a parsed zone event if applicable
            m = ZONE_RE.search(line)
            if m:
                zone = m.group(1).upper()
                x    = int(m.group(2))
                broadcast_sync({"type": "touch", "zone": zone, "x": x, "action": ACTIONS[zone]})
                continue

            if LONG_RE.search(line):
                broadcast_sync({"type": "touch", "zone": "LONG", "x": None, "action": "Home Screen"})

        except Exception:
            pass

# ── WebSocket handler ─────────────────────────────────────────────────────────
async def ws_handler(ws):
    clients.add(ws)
    print(f"[WS] Client connected ({len(clients)} total)")
    try:
        await ws.send(json.dumps({"type": "serial", "line": "-- Serial monitor ready. Waiting for touch events... --"}))
        await ws.wait_closed()
    finally:
        clients.discard(ws)
        print(f"[WS] Client disconnected ({len(clients)} total)")

# ── Entry point ───────────────────────────────────────────────────────────────
async def main():
    global loop
    loop = asyncio.get_running_loop()

    t = threading.Thread(target=serial_reader, daemon=True)
    t.start()

    print(f"[WS] Serving on ws://localhost:{WS_PORT}")
    print(f"     Open touch_sim.html in your browser.")
    async with websockets.serve(ws_handler, "localhost", WS_PORT):
        await asyncio.Future()

if __name__ == "__main__":
    asyncio.run(main())
