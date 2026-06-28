#ifndef MARIO_NETWORK_H
#define MARIO_NETWORK_H

#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoWebsockets.h>
#include <Preferences.h>
#include <esp_wifi.h>
#include "config.h"

// Extern references from main sketch to process commands
extern void handleRobotCommand(String cmd);
extern bool negativeDisplay;

class MarioNetwork {
private:
  WiFiUDP udp;
  websockets::WebsocketsClient wsClient;
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
    : wifiConnected(false), 
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
      } else if (event == websockets::WebsocketsEvent::ConnectionClosed) {
        Serial.println("[Network] WebSocket disconnected.");
        wsConnected = false;
      }
    });
  }
  
  void startWifi(const char* ssid, const char* pass) {
    savedSSID = String(ssid);
    savedPass = String(pass);
    
    Serial.print("[Network] Connecting to Wi-Fi SSID: ");
    Serial.println(ssid);
    
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    esp_wifi_set_ps(WIFI_PS_NONE); // Disable Wi-Fi power save to prevent connection dropouts
    WiFi.begin(ssid, pass);
    
    wifiConnected = false;
    serverDiscovered = false;
    wsConnected = false;
    lastWifiRetry = millis();
  }
  
  void discoverServer() {
    if (!wifiConnected) return;
    
    Serial.println("[Network] Sending UDP server discovery broadcast on port 8002...");
    udp.begin(8002);
    
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
          break;
        }
      }
      delay(10);
    }
    udp.stop();
  }
  
  void connectWebSocket() {
    if (!wifiConnected || !serverDiscovered) return;
    
    String url = "ws://" + serverIP.toString() + ":8001/ws?mac=" + macStr + "&variant=" + (negativeDisplay ? "miss_mario" : "mr_mario");
    Serial.print("[Network] Connecting WebSocket client to: ");
    Serial.println(url);
    
    bool ok = wsClient.connect(url);
    if (!ok) {
      Serial.println("[Network] WebSocket connection failed.");
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
        discoverServer();
      }
    } else {
      if (wifiConnected) {
        wifiConnected = false;
        wsConnected = false;
        serverDiscovered = false;
        Serial.println("[Network] Wi-Fi disconnected!");
      }
      
      // Periodically attempt reconnection
      if (savedSSID.length() > 0 && (now - lastWifiRetry > 30000)) {
        lastWifiRetry = now;
        Serial.println("[Network] Retrying Wi-Fi association...");
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
