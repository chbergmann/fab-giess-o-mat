#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <ArduinoOTA.h>
#include "WifiManager.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined ESP32
#include <WiFi.h>
#include "SPIFFS.h"
#endif

const char* AP_NAME = "Giess-o-mat-AP";

WifiManager::WifiManager() {
	wifi_ssid[0] = 0;
	wifi_passwd[0] = 0;
	AP_open = false;
	setup();
}

WifiManager::~WifiManager() {
	disconnect();
}

void WifiManager::disconnect() {
	WiFi.softAPdisconnect(true);
	disconnectAP();
}

void WifiManager::disconnectAP()
{
	WiFi.softAPdisconnect(true);
	AP_open = false;
}

void WifiManager::setup() {

	//clean FS, for testing
	//SPIFFS.format();

	wifi_ssid[0] = 0;
	wifi_passwd[0] = 0;

	//read configuration from FS json
	if (SPIFFS.exists("/wificonfig.json")) {
		//file exists, reading and loading
		Serial.println("reading config file");
		File configFile = SPIFFS.open("/wificonfig.json", "r");
		if (configFile) {
			size_t size = configFile.size();

			// Allocate a buffer to store contents of the file.
			std::unique_ptr<char[]> buf(new char[size]);

			configFile.readBytes(buf.get(), size);
			DynamicJsonBuffer jsonBuffer;
			JsonObject& json = jsonBuffer.parseObject(buf.get());
			if (json.success()) {
				const char* c_ssid = json["wifi_ssid"];
				const char* c_pass = json["wifi_passwd"];
				if (c_ssid && c_pass) {
					strncpy(wifi_ssid, c_ssid, sizeof(wifi_ssid));
					strncpy(wifi_passwd, c_pass, sizeof(wifi_passwd));
				}
				else
				{
					Serial.println("wifi_ssid not found in config");
				}
			} else {
				Serial.println("failed to load json config");
			}
			configFile.close();
			//json.printTo(Serial);
		}
	}

	Serial.print("SSID: "); Serial.println(wifi_ssid);
	poll();
}

void WifiManager::saveConfig() {
	File configFile = SPIFFS.open("/wificonfig.json", "w");
	if (configFile) {
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();
		root["wifi_ssid"] = wifi_ssid;
		root["wifi_passwd"] = wifi_passwd;

		root.printTo(configFile);
		configFile.close();
	}
}

void WifiManager::poll() {
	if (WiFi.status() == WL_CONNECTED)
		return;

	bool connected = false;
	if (wifi_ssid[0] != 0) {
		WiFi.begin(wifi_ssid, wifi_passwd);
		if (WiFi.waitForConnectResult() != WL_CONNECTED) {
			Serial.print("Verbindung konnte nicht aufgebaut werden zu ");
			Serial.print(wifi_ssid);
		} else {
			Serial.print("Verbindung aufgebaut zu ");
			Serial.println(wifi_ssid);
			Serial.println(WiFi.localIP());
			connected = true;
#ifdef ESP8266
			ArduinoOTA.begin();
#endif
		}
	}

	if(connected)
		return;

	if(!AP_open)
	{
		boolean result = WiFi.softAP(AP_NAME);
		if (result == true) {
			Serial.print("Access Point ohne Passwort gestartet: ");
			Serial.println(AP_NAME);
			IPAddress myIP = WiFi.softAPIP();
			Serial.print("AP IP Adresse: ");
			Serial.println(myIP);
			AP_open = true;
		} else {
			Serial.println("Access Point konnte nicht erstellt werden !");
		}
	}
}

bool WifiManager::connect(String ssid, String passwd)
{
	strncpy(wifi_ssid, ssid.c_str(), sizeof(wifi_ssid));
	strncpy(wifi_passwd, passwd.c_str(), sizeof(wifi_passwd));

	poll();

	if(WiFi.status() == WL_CONNECTED)
	{
		saveConfig();
		return true;
	}
	return false;
}

