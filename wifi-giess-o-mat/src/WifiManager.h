#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>

class WifiManager {
	bool AP_open;
public:
  char wifi_ssid[32];
  char wifi_passwd[32];

  WifiManager();
  virtual ~WifiManager();

  void setup();
  void saveConfig();
  void loop();
  void disconnect();
  void disconnectAP();
  void poll();
  bool connect();
  bool connect(String ssid, String passwd);
};

#endif // WIFIMANAGER_H
