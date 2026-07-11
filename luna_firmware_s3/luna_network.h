#ifndef LUNA_NETWORK_H
#define LUNA_NETWORK_H

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
class LunaBLE;
extern LunaBLE ble;
class LunaAudio;
extern LunaAudio audio;

class LunaNetwork {
private:
  WiFiUDP udp;
  websockets::WebsocketsServer wsServer;
  websockets::WebsocketsClient activeClient;
  WebServer localServer;
  bool wifiConnected;
  bool clientConnected;
  
  unsigned long lastWifiRetry;
  
  String savedSSID;
  String savedPass;
  String macStr;
  
public:
  LunaNetwork() 
    : localServer(8000),
      wifiConnected(false), 
      clientConnected(false),
      lastWifiRetry(0) {}
                    
  void init() {
    macStr = WiFi.macAddress();
    // Keep MAC format consistent: colons removed for simple URL query params
    macStr.replace(":", ""); 
    
    // Load persisted settings from NVS Preferences
    Preferences prefs;
    prefs.begin("luna", true); // Read-only
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
    
    // Configure local web server routes on port 8000
    localServer.on("/api/robots", HTTP_GET, [this]() {
      localServer.sendHeader("Access-Control-Allow-Origin", "*");
      String variantStr = negativeDisplay ? "ms_luna" : "mr_luna";
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
    clientConnected = false;
    lastWifiRetry = millis();
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
        wsServer.listen(8001); // Start WebSocket Server on port 8001!
        Serial.println("[Network] Local WebServer started on port 8000");
        Serial.println("[Network] WebSocket Server started on port 8001");
        ble.sendLog("Wi-Fi Connected! IP: " + WiFi.localIP().toString());
      }
      
      // Handle client requests on the web server
      localServer.handleClient();
      
      // Accept incoming WebSocket connections from phone
      if (wsServer.available()) {
        if (clientConnected) {
          activeClient.close();
        }
        activeClient = wsServer.accept();
        clientConnected = true;
        Serial.println("[Network] Phone connected directly!");
        ble.sendLog("Phone Connected Directly!");

        activeClient.onMessage([this](websockets::WebsocketsMessage message) {
          if (message.isBinary()) {
            const std::string& raw = message.rawData();
            audio.writeTxStream((const uint8_t*)raw.data(), raw.length());
          } else {
            Serial.print("[Network] WS Message: ");
            Serial.println(message.data());
            handleRobotCommand(message.data());
          }
        });

        activeClient.onEvent([this](websockets::WebsocketsEvent event, String data) {
          if (event == websockets::WebsocketsEvent::ConnectionClosed) {
            Serial.println("[Network] Phone disconnected.");
            clientConnected = false;
            ble.sendLog("Phone Disconnected.");
          }
        });
      }

      // Handle active client communication
      if (clientConnected && activeClient.available()) {
        activeClient.poll();
        
        // If microphone streaming is active, fetch data from rxRingBuffer and send via WebSocket
        if (audio.micStreaming) {
          uint8_t buffer[256];
          size_t size = 0;
          while (audio.getRxItem(buffer, &size)) {
            activeClient.sendBinary((const char*)buffer, size);
          }
        }
      } else if (clientConnected) {
        clientConnected = false;
      }
      
      // Listen for UDP discovery pings from the phone
      int packetSize = udp.parsePacket();
      if (packetSize) {
        char packetBuffer[128];
        int len = udp.read(packetBuffer, 127);
        if (len > 0) {
          packetBuffer[len] = 0;
        }
        String msg = String(packetBuffer);
        if (msg == "MR_LUNA_DISCOVER" && udp.remoteIP() != WiFi.localIP()) {
          // Reply to the phone that we are the active endpoint (direct control mode)
          udp.beginPacket(udp.remoteIP(), udp.remotePort());
          udp.print("MR_LUNA_SERVER_HERE");
          udp.endPacket();
          Serial.print("[Network] Replied to phone UDP discovery from IP: ");
          Serial.println(udp.remoteIP());
        }
      }
      
    } else {
      if (wifiConnected) {
        wifiConnected = false;
        clientConnected = false;
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
    }
  }
  
  bool isWifiConnected() const { return wifiConnected; }
  bool isCloudConnected() const { return clientConnected; }
  String getSSID() const { return savedSSID; }
  String getMAC() const { return macStr; }
};

#endif // LUNA_NETWORK_H
