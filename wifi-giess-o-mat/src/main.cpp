#include <Arduino.h>
#include <SPI.h>
#include "configuration.h"

#define SPI_SS_PIN  16

#define htons(x)  __builtin_bswap16(x)

void spi_read_configuration() {
  struct config_item configuration;
  byte *pConfig = (byte*)&configuration;

  digitalWrite(SPI_SS_PIN, LOW);
  pConfig[0] = SPI.transfer('c');
  for(int i=1; i<sizeof(struct config_item); i++) {
    pConfig[i] = SPI.transfer(0);
  }
  digitalWrite(SPI_SS_PIN, HIGH);

  Serial.println("Configuration:");
  Serial.print("threashold_dry \t"); Serial.println(htons(configuration.threashold_dry));
  Serial.print("threashold_wet \t"); Serial.println(htons(configuration.threashold_wet));
  Serial.print("seconds_on \t"); Serial.println(htons(configuration.seconds_on));
  Serial.print("minutes_off \t"); Serial.println(htons(configuration.minutes_off));
  Serial.print("sensor_cntval \t"); Serial.println(htons(configuration.sensor_cntval));
}

void spi_read_sensor() {
  uint16_t sensorval = 0;

  digitalWrite(SPI_SS_PIN, LOW);
  int a = SPI.transfer('s');
  int b = SPI.transfer(0);
  sensorval = (a << 8) + b;
  digitalWrite(SPI_SS_PIN, HIGH);

  Serial.print("Sensor: \t"); Serial.println(sensorval);
}

void setup() {
  // put your setup code here, to run once:

  // have to send on master in, *slave out*
  pinMode (MOSI, OUTPUT);
  pinMode (SPI_SS_PIN, OUTPUT);
  SPI.begin();
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));

  Serial.begin(115200);
  Serial.println("* Giess-o-mat Wifi Module *");
}

void loop() {
  // put your main code here, to run repeatedly:
  spi_read_configuration();
  spi_read_sensor();
  delay(1000);
}
