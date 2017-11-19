/* fab-giess-o-mat */

#include <EEPROM.h>
#include <TimeLib.h>
#include "configuration.h"

struct config_item configuration; 
time_t lasttime_pump_on; 

void setup() {
  // put your setup code here, to run once:
  EEPROM.get(EEPROM_ADDRESS_CONFIG, configuration);
  if(configuration.auto_mode > AUTO_MODE_TIMER) {
    // Standardwerte
    configuration.auto_mode = AUTO_MODE_TIMER;
    configuration.threashold = 512;
    configuration.seconds_on = 5;
    configuration.hours_off = 24;
    configuration.sensor_cntval = 62;
  }

  
  lasttime_pump_on = now();
  
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  start_read_sensors();
  
  Serial.begin(115200);
  mainmenu();
}

unsigned long last_millis = 0;
void loop() {
  // put your main code here, to run repeatedly:
  if(millis() - last_millis >= 1000) {
    last_millis += 1000;
    show_time();
    process_giessomat();
  } 
}

void show_time() {
  char clock[16];
  sprintf(clock, "\r%d.%d.%d %02d:%02d:%02d", day(), month(), year(), hour(), minute(), second());
  Serial.print(clock);
  Serial.print(" Sensor: ");
  Serial.print(get_sensorvalue());
}

void process_giessomat() {
  loop_sensors();

  if(configuration.auto_mode != MANUAL_MODE) {
    time_t difftime = now() - lasttime_pump_on;
    int seconds_on = minute(difftime) * 60 + second(difftime);
    
    if(hour(difftime) == 0 && seconds_on > configuration.seconds_on) {
      digitalWrite(PUMP_PIN, LOW);
    }
    
    if(hour(difftime) >= configuration.hours_off) {
      if((configuration.auto_mode == AUTO_MODE_HIGHER && get_sensorvalue() > configuration.threashold) ||
         (configuration.auto_mode == AUTO_MODE_LOWER  && get_sensorvalue() < configuration.threashold) ||
         (configuration.auto_mode == AUTO_MODE_TIMER)) {
            digitalWrite(PUMP_PIN, HIGH);
            lasttime_pump_on = now();
         }
    } 
  }
}

