#ifndef LUNA_NETWORK_H
#define LUNA_NETWORK_H

#include "config.h"

class LunaNetwork {
public:
  LunaNetwork() {}
  void init() {}
  void startWifi(const char* ssid, const char* pass) {}
  void discoverServer() {}
  void connectWebSocket() {}
  void update() {}
  bool isWifiConnected() const { return false; }
  bool isCloudConnected() const { return false; }
  String getSSID() const { return ""; }
  String getMAC() const { return ""; }
};

#endif // LUNA_NETWORK_H
