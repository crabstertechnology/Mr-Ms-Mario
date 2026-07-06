#ifndef MARIO_NETWORK_H
#define MARIO_NETWORK_H

#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoWebsockets.h>
#include <WebServer.h>
#include <Preferences.h>
#include <esp_wifi.h>
#include "config.h"

// Extern references from main sketch to process commands
extern void handleRobotCommand(String cmd);
extern bool negativeDisplay;
class MarioBLE;
extern MarioBLE ble;

class MarioNetwork {
private:
  WiFiUDP udp;
  websockets::WebsocketsClient wsClient;
  WebServer localServer;
  bool wifiConnected;
  bool wsConnected;
  IPAddress serverIP;
  bool serverDiscovered;
  
  unsigned long lastWifiRetry;
  unsigned long lastDiscoveryAttempt;
  unsigned long lastWsRetry;
  
  String savedSSID;
  String savedPass;
  String macStr;
  
public:
  MarioNetwork() 
    : localServer(8000),
      wifiConnected(false), 
      wsConnected(false), 
      serverDiscovered(false),
      lastWifiRetry(0), 
      lastDiscoveryAttempt(0), 
      lastWsRetry(0) {}
                    
  void init() {
    macStr = WiFi.macAddress();
    // Keep MAC format consistent: colons removed for simple URL query params
    macStr.replace(":", ""); 
    
    // Load persisted settings from NVS Preferences
    Preferences prefs;
    prefs.begin("mario", true); // Read-only
    savedSSID = prefs.getString("wifi_ssid", "");
    savedPass = prefs.getString("wifi_pass", "");
    prefs.end();
    
    savedSSID.trim();
    savedPass.trim();
    
    if (savedSSID.length() > 0) {
      Serial.print("[Network] Saved SSID found: ");
      Serial.println(savedSSID);
      startWifi(savedSSID.c_str(), savedPass.c_str());
    } else {
      Serial.println("[Network] No saved Wi-Fi credentials.");
    }
    
    // Configure WebSocket Client callbacks
    wsClient.onMessage([this](websockets::WebsocketsMessage message) {
      Serial.print("[Network] WS Message: ");
      Serial.println(message.data());
      handleRobotCommand(message.data());
    });
    
    wsClient.onEvent([this](websockets::WebsocketsEvent event, String data) {
      if (event == websockets::WebsocketsEvent::ConnectionOpened) {
        Serial.println("[Network] WebSocket connected!");
        wsConnected = true;
        ble.sendLog("Cloud WebSocket Connected!");
      } else if (event == websockets::WebsocketsEvent::ConnectionClosed) {
        Serial.println("[Network] WebSocket disconnected.");
        wsConnected = false;
        ble.sendLog("Cloud WebSocket Disconnected.");
      }
    });

    // Configure local web server routes on port 8000
    localServer.on("/api/robots", HTTP_GET, [this]() {
      localServer.sendHeader("Access-Control-Allow-Origin", "*");
      String variantStr = negativeDisplay ? "miss_mario" : "mr_mario";
      String json = "[{\"mac\":\"" + macStr + "\",\"variant\":\"" + variantStr + "\",\"status\":\"online\"}]";
      localServer.send(200, "application/json", json);
    });

    localServer.on("/api/trigger", HTTP_POST, [this]() {
      localServer.sendHeader("Access-Control-Allow-Origin", "*");
      if (localServer.hasArg("plain")) {
        String body = localServer.arg("plain");
        String cmd = "";
        
        int cmdKeyIdx = body.indexOf("\"command\"");
        if (cmdKeyIdx != -1) {
          int colonIdx = body.indexOf(":", cmdKeyIdx);
          if (colonIdx != -1) {
            int startQuote = body.indexOf("\"", colonIdx);
            if (startQuote != -1) {
              int endQuote = body.indexOf("\"", startQuote + 1);
              if (endQuote != -1) {
                cmd = body.substring(startQuote + 1, endQuote);
              }
            }
          }
        }
        
        if (cmd.length() > 0) {
          Serial.print("[Network] Direct HTTP Command: ");
          Serial.println(cmd);
          handleRobotCommand(cmd);
          localServer.send(200, "application/json", "{\"success\":true}");
          return;
        }
      }
      localServer.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid command\"}");
    });

    localServer.on("/api/trigger", HTTP_OPTIONS, [this]() {
      localServer.sendHeader("Access-Control-Allow-Origin", "*");
      localServer.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
      localServer.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      localServer.send(200, "text/plain", "");
    });
  }
  
  void startWifi(const char* ssid, const char* pass) {
    savedSSID = String(ssid);
    savedPass = String(pass);
    savedSSID.trim();
    savedPass.trim();
    
    Serial.print("[Network] Connecting to Wi-Fi SSID: ");
    Serial.println(savedSSID);
    ble.sendLog("Wi-Fi credentials received. Connecting to " + savedSSID + "...");
    
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
    esp_wifi_set_ps(WIFI_PS_NONE); // Disable Wi-Fi power save to prevent connection dropouts
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());
    
    wifiConnected = false;
    serverDiscovered = false;
    wsConnected = false;
    lastWifiRetry = millis();
  }
  
  void discoverServer() {
    if (!wifiConnected) return;
    
    Serial.println("[Network] Sending UDP server discovery broadcast on port 8002...");
    
    // Send broadcast packet to subnet
    IPAddress broadcastIP = WiFi.localIP();
    broadcastIP[3] = 255; // Subnet broadcast address
    
    udp.beginPacket(broadcastIP, 8002);
    udp.print("MR_MARIO_DISCOVER");
    udp.endPacket();
    
    // Await response with timeout
    unsigned long startWait = millis();
    while (millis() - startWait < 1200) {
      int packetSize = udp.parsePacket();
      if (packetSize) {
        char packetBuffer[128];
        int len = udp.read(packetBuffer, 127);
        if (len > 0) {
          packetBuffer[len] = 0;
        }
        String reply = String(packetBuffer);
        if (reply == "MR_MARIO_SERVER_HERE") {
          serverIP = udp.remoteIP();
          serverDiscovered = true;
          Serial.print("[Network] Server discovered at IP: ");
          Serial.println(serverIP);
          ble.sendLog("Server discovered at IP: " + serverIP.toString());
          break;
        }
      }
      delay(10);
    }
  }
  
  void connectWebSocket() {
    if (!wifiConnected || !serverDiscovered) return;
    
    String url = "ws://" + serverIP.toString() + ":8001/ws?mac=" + macStr + "&variant=" + (negativeDisplay ? "miss_mario" : "mr_mario");
    Serial.print("[Network] Connecting WebSocket client to: ");
    Serial.println(url);
    ble.sendLog("Connecting to Cloud WebSocket: " + serverIP.toString() + ":8001...");
    
    bool ok = wsClient.connect(url);
    if (!ok) {
      Serial.println("[Network] WebSocket connection failed.");
      ble.sendLog("Cloud WebSocket Connection Failed!");
    }
  }
  
  void update() {
    unsigned long now = millis();
    
    // 1. Monitor Wi-Fi state
    if (WiFi.status() == WL_CONNECTED) {
      if (!wifiConnected) {
        wifiConnected = true;
        Serial.print("[Network] Wi-Fi connected. IP: ");
        Serial.println(WiFi.localIP());
        udp.begin(8002); // Bind UDP to port 8002
        localServer.begin(); // Start HTTP Server only after we have an IP!
        Serial.println("[Network] Local WebServer started on port 8000");
        ble.sendLog("Wi-Fi Connected! IP: " + WiFi.localIP().toString());
        discoverServer();
      }
      
      // Handle client requests on the web server
      localServer.handleClient();
      
      // Listen for UDP discovery pings from the phone
      int packetSize = udp.parsePacket();
      if (packetSize) {
        char packetBuffer[128];
        int len = udp.read(packetBuffer, 127);
        if (len > 0) {
          packetBuffer[len] = 0;
        }
        String msg = String(packetBuffer);
        if (msg == "MR_MARIO_DISCOVER" && udp.remoteIP() != WiFi.localIP()) {
          // Reply to the phone that we are the active endpoint (direct control mode)
          udp.beginPacket(udp.remoteIP(), udp.remotePort());
          udp.print("MR_MARIO_SERVER_HERE");
          udp.endPacket();
          Serial.print("[Network] Replied to phone UDP discovery from IP: ");
          Serial.println(udp.remoteIP());
        }
      }
      
    } else {
      if (wifiConnected) {
        wifiConnected = false;
        wsConnected = false;
        serverDiscovered = false;
        localServer.close(); // Close HTTP Server safely
        udp.stop(); // Close UDP socket
        Serial.println("[Network] Wi-Fi disconnected!");
        ble.sendLog("Wi-Fi Disconnected!");
      }
      
      // Periodically attempt reconnection
      if (savedSSID.length() > 0 && (now - lastWifiRetry > 20000)) {
        lastWifiRetry = now;
        Serial.println("[Network] Retrying Wi-Fi association...");
        ble.sendLog("Retrying Wi-Fi association with " + savedSSID + "...");
        WiFi.disconnect();
        delay(100);
        WiFi.begin(savedSSID.c_str(), savedPass.c_str());
      }
      return;
    }
    
    // 2. Discover server if IP unknown
    if (wifiConnected && !serverDiscovered) {
      if (now - lastDiscoveryAttempt > 15000) {
        lastDiscoveryAttempt = now;
        discoverServer();
      }
      return;
    }
    
    // 3. Connect/maintain WebSocket connection
    if (wifiConnected && serverDiscovered) {
      if (wsConnected) {
        wsClient.poll();
      } else {
        if (now - lastWsRetry > 20000) {
          lastWsRetry = now;
          connectWebSocket();
        }
      }
    }
  }
  
  bool isWifiConnected() const { return wifiConnected; }
  bool isCloudConnected() const { return wsConnected; }
  String getSSID() const { return savedSSID; }
  String getMAC() const { return macStr; }
};

#endif // MARIO_NETWORK_H
