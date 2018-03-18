#include <Arduino.h>
#include <SPIMaster.h>
#include "giessomat.h"
#include "WifiManager.h"

#ifdef ESP8266
#include <EWiFi.h>
#elif defined ESP32
#include <WiFi.h>
#include "SPIFFS.h"
#endif

#include <ESP8266FtpServer.h>
FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

extern void setup_webserver();
extern void loop_webserver();

SPIMaster SpiMaster;
WifiManager wifiManager;
byte command = 0;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("\r\n* Giess-o-mat Wifi Module *");

  /////FTP Setup, ensure SPIFFS is started before ftp;  /////////
#ifdef ESP32       //esp32 we send true to format spiffs if cannot mount
  if (SPIFFS.begin(true)) {
#elif defined ESP8266
  if (SPIFFS.begin()) {
#endif
    wifiManager.setup();
    setup_webserver();
    ftpSrv.begin("esp8266","esp8266");    //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)
  }

  command = 's';
  SpiMaster.setup();
  SpiMaster.start_transfer(command, sizeof(uint16_t));
}

int last_sec = 0;

void loop() {
  uint16_t sensorval = 0;
  struct config_item configuration;

  wifiManager.poll();
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
  loop_webserver();

  int sec = millis() / 1000;
  if(SpiMaster.poll() && last_sec != sec)
  {
	  last_sec = sec;

	  if(command == 's') {
			if(SpiMaster.transfer_ready((byte*)&sensorval, sizeof(sensorval))) {
				Serial.print("Sensor: "); Serial.println(sensorval);
				command = 'c';
				SpiMaster.start_transfer(command, sizeof(configuration));
			}
	  }
	  else if(command == 'c') {
			if(SpiMaster.transfer_ready((uint8_t*)&configuration, sizeof(configuration))) {
				Serial.println("Configuration:");
				Serial.print("version \t"); Serial.println((configuration.version));
				Serial.print("threashold_dry \t"); Serial.println((configuration.threashold_dry));
				Serial.print("threashold_wet \t"); Serial.println((configuration.threashold_wet));
				Serial.print("seconds_on \t"); Serial.println((configuration.seconds_on));
				Serial.print("minutes_off \t"); Serial.println((configuration.minutes_off));
				command = 's';
				SpiMaster.start_transfer(command, sizeof(sensorval));
			}
	  }
  }
}
