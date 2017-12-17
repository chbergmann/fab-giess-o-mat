#include <Arduino.h>
#include <SPIMaster.h>
#include "giessomat.h"
#include "WifiManager.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined ESP32
#include <WiFi.h>
#include "SPIFFS.h"
#endif

#include <ESP8266FtpServer.h>
FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

#define SPI_SS_PIN  16

#define htons(x)  __builtin_bswap16(x)

extern void setup_wifimanager();
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
  //SpiMaster.setup();
  //SpiMaster.start_transfer(&command, 1, 2);
}

void loop() {
  uint16_t sensorval = 0;
  struct config_item configuration;

  wifiManager.loop();
  SpiMaster.poll();
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
  loop_webserver();
/*
  if(command == 's') {
    if(SpiMaster.transfer_ready((byte*)&sensorval, sizeof(sensorval))) {
      Serial.print("Sensor: \t"); Serial.println(htons(sensorval));
      delay(1000);
      command = 'c';
      SpiMaster.start_transfer(&command, 1, sizeof(configuration));
    }
  }
  else if(command == 'c') {
    if(SpiMaster.transfer_ready((byte*)&configuration, sizeof(configuration))) {
      Serial.println("Configuration:");
      Serial.print("threashold_dry \t"); Serial.println(htons(configuration.threashold_dry));
      Serial.print("threashold_wet \t"); Serial.println(htons(configuration.threashold_wet));
      Serial.print("seconds_on \t"); Serial.println(htons(configuration.seconds_on));
      Serial.print("minutes_off \t"); Serial.println(htons(configuration.minutes_off));
      Serial.print("sensor_cntval \t"); Serial.println(htons(configuration.sensor_cntval));
      delay(1000);
      command = 's';
      SpiMaster.start_transfer(&command, 1, sizeof(sensorval));
    }
  }*/
}
