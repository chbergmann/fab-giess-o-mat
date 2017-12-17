#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <ArduinoOTA.h>
#include "WifiManager.h"

const char* AP_NAME = "Giess-o-mat-AP";

WifiManager::WifiManager() {
  wifi_ssid[0] = 0;
  wifi_passwd[0] = 0;

}

WifiManager::~WifiManager() {
  disconnect();
}

void WifiManager::disconnect() {
  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true);
}

void WifiManager::setup() {

  //clean FS, for testing
  //SPIFFS.format();

  wifi_ssid[0] = 0;
  wifi_passwd[0] = 0;

  //read configuration from FS json
  if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          if(json["wifi_ssid"]) {
            Serial.println("setting custom ip from config");
            //static_ip = json["ip"];
            strcpy(wifi_ssid, json["wifi_ssid"]);
            strcpy(wifi_passwd, json["wifi_passwd"]);
          }
        } else {
          Serial.println("failed to load json config");
        }
      }
  }

  loop();
}

void WifiManager::loop() {
  if(WiFi.status() == WL_CONNECTED)
    return;

  bool connected = false;
  if(wifi_ssid[0] != 0) {
    WiFi.begin(wifi_ssid, wifi_passwd);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.print("Verbindung konnte nicht aufgebaut werden zu ");
      Serial.println(wifi_ssid);
    }
    else {
      Serial.print("Verbindung aufgebaut zu ");
      Serial.println(wifi_ssid);
      Serial.println(WiFi.localIP());
      WiFi.softAPdisconnect();
      connected = true;
#ifdef ESP8266
      ArduinoOTA.begin();
#endif
    }
  }

  if(!connected) {
    boolean result = WiFi.softAP(AP_NAME);
    if(result == true)
    {
      Serial.print("Access Point ohne Passwort gestartet: ");
      Serial.println(AP_NAME);
      IPAddress myIP = WiFi.softAPIP();
      Serial.print("AP IP Adresse: ");
      Serial.println(myIP);
      while(WiFi.softAPgetStationNum() == 0) {
        delay(500);
      }
      Serial.println("Station verbunden !");
    }
    else
    {
      Serial.println("Access Point konnte nicht erstellt werden !");
    }
  }
}
