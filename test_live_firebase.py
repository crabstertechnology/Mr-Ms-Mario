import urllib.request
import urllib.error
import json
import sys

DATABASE_URL = "https://mrluna-66b68-default-rtdb.firebaseio.com"

def main():
    print("==================================================")
    print("      LUNA LIVE FIREBASE REALTIME DATABASE TEST   ")
    print("==================================================")
    print(f"Target Database URL: {DATABASE_URL}")
    print("[*] Sending ping to check database status...")

    try:
        url = f"{DATABASE_URL}/.json"
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode('utf-8'))
            print("[+] Connection successful! Firebase Realtime Database is online and accessible.")
            print(f"[+] Current Root Data: {data}")
    except urllib.error.HTTPError as e:
        print(f"[!] Access Refused (HTTP {e.code}): {e.reason}")
        if e.code == 401 or e.code == 403:
            print("\n>>> INSTRUCTIONS TO FIX SECURITY RULES:")
            print("1. In your Firebase Console, click 'Realtime Database' (under the 'Build' category on the left).")
            print("2. If not created yet, click 'Create Database' and choose a region.")
            print("3. Switch to the 'Rules' tab.")
            print("4. Edit the rules to allow public read/write access for testing:")
            print("   {")
            print('     "rules": {')
            print('       ".read": true,')
            print('       ".write": true')
            print("     }")
            print("   }")
            print("5. Click 'Publish'.")
        else:
            print(f"[!] Unknown HTTP Error: {e}")
    except urllib.error.URLError as e:
        print(f"[!] URLError: {e.reason}")
        print("\n>>> INSTRUCTIONS:")
        print("1. In your Firebase Console, make sure you have clicked 'Realtime Database' and completed the 'Create Database' wizard.")
        print("2. Verify that your database domain matches 'mrluna-66b68-default-rtdb.firebaseio.com'. If your database has a different region, it might have a URL ending in '.firebasedatabase.app'. Update the URL in `firebase_service.dart` accordingly.")
    print("==================================================")

if __name__ == "__main__":
    main()
