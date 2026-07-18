import urllib.request
import urllib.error
import json
import time

DATABASE_URL = "https://mrluna-66b68-default-rtdb.firebaseio.com"

def make_request(url, method="GET", data=None):
    try:
        req_data = json.dumps(data).encode('utf-8') if data is not None else None
        headers = {'Content-Type': 'application/json'} if data is not None else {}
        req = urllib.request.Request(url, data=req_data, headers=headers, method=method)
        with urllib.request.urlopen(req) as response:
            res_content = response.read().decode('utf-8')
            return json.loads(res_content) if res_content and res_content != "null" else None
    except urllib.error.URLError as e:
        print(f"[ERROR] Request to {url} failed: {e}")
        return None

def main():
    print("==================================================")
    print("        LUNA LIVE CLOUD PAIRING SIMULATOR BOT     ")
    print("==================================================")
    
    # 1. Register Alice Owner in the database
    alice_profile = {
        "uid": "user_alicelunagmailcom",
        "email": "alice.luna@gmail.com",
        "displayName": "Alice Owner",
        "photoUrl": "https://api.dicebear.com/7.x/adventurer/png?seed=Alice",
        "robotId": "ROBOT_user_alicelunagmailcom",
        "robotName": "Lumina",
        "robotVariant": "ms_luna",
        "isOnline": True
    }
    
    print("[*] Registering 'Alice Owner' profile in Live database...")
    make_request(f"{DATABASE_URL}/users/user_alicelunagmailcom.json", "PUT", alice_profile)
    print("[+] Alice Owner registered successfully.")
    
    print("\n[*] Bot is now listening for friend requests from 'Sasi Dev'...")
    print(">>> Action needed: In the Flutter app (under Luna Link tab), search for 'Alice' and click 'Add'!")
    
    sasi_uid = "user_sasidevgmailcom"
    alice_uid = "user_alicelunagmailcom"
    
    while True:
        try:
            # 2. Check for incoming friend request from Sasi
            req_data = make_request(f"{DATABASE_URL}/friend_requests/{alice_uid}/{sasi_uid}.json", "GET")
            if req_data:
                print(f"\n[+] Received friend request from '{req_data['displayName']}'!")
                print("[*] Accepting friend request...")
                
                # Delete request
                make_request(f"{DATABASE_URL}/friend_requests/{alice_uid}/{sasi_uid}.json", "DELETE")
                
                # Add to Alice's friends
                alice_friend_data = {
                    "uid": sasi_uid,
                    "email": req_data.get("email", "sasi.dev@gmail.com"),
                    "displayName": req_data.get("displayName", "Sasi Dev"),
                    "photoUrl": req_data.get("photoUrl", ""),
                    "robotId": req_data.get("robotId", ""),
                    "robotName": req_data.get("robotName", "LunaMax"),
                    "robotVariant": req_data.get("robotVariant", "mr_luna"),
                    "isOnline": True
                }
                make_request(f"{DATABASE_URL}/friends/{alice_uid}/{sasi_uid}.json", "PUT", alice_friend_data)
                
                # Add to Sasi's friends
                make_request(f"{DATABASE_URL}/friends/{sasi_uid}/{alice_uid}.json", "PUT", alice_profile)
                
                print("[+] Friend request accepted on Live Database!")
                print(">>> Action needed: Sasi Dev and Alice Owner are now friends! Pair with 'Lumina' in your friends list!")
                break
                
            time.sleep(2)
        except KeyboardInterrupt:
            print("\nBot stopped.")
            return
            
    # 3. Listen to remote triggers
    print("\n[*] Bot is now listening for touch triggers on '/triggers/user_alicelunagmailcom.json'...")
    while True:
        try:
            trigger = make_request(f"{DATABASE_URL}/triggers/{alice_uid}.json", "GET")
            if trigger:
                sender = trigger.get("senderName", "Sasi Dev")
                event = trigger.get("eventType", "TAP")
                expr = trigger.get("exprLabel", "HAPPY")
                sound = trigger.get("soundId", 2)
                
                print(f"\n[Live Cloud Event] >>> Received {event} from {sender} (Expr: {expr}, Sound ID: {sound})!")
                
                # Delete trigger
                make_request(f"{DATABASE_URL}/triggers/{alice_uid}.json", "DELETE")
                
                # React back automatically after 2 seconds
                print("[*] Simulating Lumina's response: Sending back SURPRISED expression and sound sequence...")
                time.sleep(2)
                reaction = {
                    "senderName": "Lumina",
                    "eventType": "REMOTE_TAP",
                    "exprLabel": "SURPRISED",
                    "soundId": 3,
                    "timestamp": int(time.time() * 1000)
                }
                make_request(f"{DATABASE_URL}/triggers/{sasi_uid}.json", "PUT", reaction)
                print("[+] Sent reactive expression to Sasi Dev!")
                
            time.sleep(2)
        except KeyboardInterrupt:
            print("\nBot stopped.")
            break

if __name__ == "__main__":
    main()
