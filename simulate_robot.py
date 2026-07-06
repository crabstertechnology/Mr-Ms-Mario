import asyncio
import websockets
import sys

async def simulate():
    # Mocking a robot with MAC address A1B2C3D4E5F6 and variant 'mr_mario'
    uri = "ws://localhost:8001/ws?mac=A1B2C3D4E5F6&variant=mr_luna"
    print(f"==================================================")
    print(f"Starting Mr.&Ms Luna Companion Simulator")
    print(f"Connecting to cloud server: {uri}")
    print(f"==================================================")
    
    try:
        async with websockets.connect(uri) as websocket:
            print("[Simulator] Connection established! Awaiting commands...")
            async for message in websocket:
                print(f"\n[Simulator] >>> Received Remote Command: '{message}'")
    except Exception as e:
        print(f"[Simulator] Connection failed or closed: {e}")

if __name__ == "__main__":
    try:
        asyncio.run(simulate())
    except KeyboardInterrupt:
        print("\nSimulator stopped.")
