import urllib.request
import urllib.error
import json
import time
import subprocess
import os
import sys
import threading
import asyncio
import websockets

PORT = 8000
WS_PORT = 8001

async def ws_client_simulation(stop_event):
    uri = f"ws://localhost:{WS_PORT}/ws?mac=TEST_ROBOT_MAC&variant=mr_luna"
    print(f"[*] Connecting WebSocket client to: {uri}")
    try:
        async with websockets.connect(uri) as websocket:
            print("[+] WebSocket client connected successfully!")
            while not stop_event.is_set():
                try:
                    # Non-blocking wait
                    message = await asyncio.wait_for(websocket.recv(), timeout=0.5)
                    print(f"[WebSocket] Received remote command: '{message}'")
                except asyncio.TimeoutError:
                    continue
    except Exception as e:
        print(f"[!] WebSocket error: {e}")

def run_ws_client(stop_event):
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    loop.run_until_complete(ws_client_simulation(stop_event))

def test_http_endpoints():
    print("[*] Testing HTTP GET /api/robots...")
    try:
        url = f"http://localhost:{PORT}/api/robots"
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode('utf-8'))
            print(f"[+] /api/robots response: {data}")
            return data
    except urllib.error.URLError as e:
        print(f"[!] HTTP request failed: {e}")
        return None

def test_post_trigger():
    print("[*] Sending trigger POST to /api/trigger...")
    try:
        url = f"http://localhost:{PORT}/api/trigger"
        payload = {
            "mac": "TEST_ROBOT_MAC",
            "command": "EXPR:HAPPY"
        }
        data = json.dumps(payload).encode('utf-8')
        req = urllib.request.Request(url, data=data, headers={'Content-Type': 'application/json'})
        with urllib.request.urlopen(req) as response:
            res_data = json.loads(response.read().decode('utf-8'))
            print(f"[+] /api/trigger response: {res_data}")
            return res_data
    except urllib.error.URLError as e:
        print(f"[!] Trigger POST failed: {e}")
        return None

def main():
    print("==================================================")
    print("      LUNA CLOUD & SERVER STACK INTEGRATION TEST  ")
    print("==================================================")

    # Start the ws client thread
    stop_event = threading.Event()
    t = threading.Thread(target=run_ws_client, args=(stop_event,), daemon=True)
    t.start()

    # Wait for websocket connection
    time.sleep(2)

    # Query online robots
    robots = test_http_endpoints()
    
    # Assert robot is online
    if robots:
        found = False
        for robot in robots:
            if robot.get('mac') == 'TEST_ROBOT_MAC':
                print(f"[+] Verified: TEST_ROBOT_MAC is ONLINE with variant {robot.get('variant')}")
                found = True
        if not found:
            print("[!] Fail: TEST_ROBOT_MAC not found in online list")
    
    # Send remote trigger
    test_post_trigger()

    # Let message get printout
    time.sleep(2)

    print("[*] Stopping client...")
    stop_event.set()
    time.sleep(1)
    print("==================================================")
    print("             TEST EXECUTION COMPLETED             ")
    print("==================================================")

if __name__ == "__main__":
    main()
