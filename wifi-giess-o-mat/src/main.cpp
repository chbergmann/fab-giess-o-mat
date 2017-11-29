#include <Arduino.h>
#include <SPIMaster.h>
#include "configuration.h"

#define SPI_SS_PIN  16

#define htons(x)  __builtin_bswap16(x)

SPIMaster SpiMaster;
byte command = 0;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("* Giess-o-mat Wifi Module *");
  command = 's';
  SpiMaster.setup();
  SpiMaster.start_transfer(&command, 1, 2);
}

void loop() {
  uint16_t sensorval = 0;
  struct config_item configuration;
  SpiMaster.poll();

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
  }
}
