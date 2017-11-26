#include <Arduino.h>
#include <SPI.h>

#define SPI_SS_PIN  16
void setup() {
  // put your setup code here, to run once:

  // have to send on master in, *slave out*
  pinMode (MISO, OUTPUT);
  pinMode (SPI_SS_PIN, OUTPUT);
  SPI.begin ();
  // Slow down the master a bit
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  Serial.begin(115200);
  Serial.println("* Giess-o-mat Wifi Module *");
}

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(SPI_SS_PIN, LOW);
  uint16_t sensorval = 0;
  sensorval = SPI.transfer(0) + (SPI.transfer(1) << 8);
  Serial.println(sensorval);
  digitalWrite(SPI_SS_PIN, HIGH);
  delay(1000);
}
