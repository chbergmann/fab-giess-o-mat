#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>

class WifiManager {
public:
  char wifi_ssid[32];
  char wifi_passwd[32];

  WifiManager();
  virtual ~WifiManager();

  void setup();
  void loop();
  void disconnect();
};

#endif // WIFIMANAGER_H
