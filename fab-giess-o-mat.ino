/* fab-giess-o-mat */

#include <EEPROM.h>
#include <TimeLib.h>
#include "configuration.h"

struct config_item configuration[MAX_NR_CONFIGS]; 
time_t lasttime_pump_on[MAX_NR_CONFIGS]; 
extern int sensorvalues[MAX_NR_CONFIGS];

void setup() {
  // put your setup code here, to run once:
  EEPROM.get(EEPROM_ADDRESS_CONFIG, configuration);
  init_pins();
  for(uint8_t i=0; i<MAX_NR_CONFIGS; i++) {
    lasttime_pump_on[i] = now();
  }
  start_read_sensors();
  Serial.begin(115200);
  mainmenu();
}

void init_pins() {
  for(uint8_t i=0; i<MAX_NR_CONFIGS; i++) {
    if(config_valid(i)) {
      if(configuration[i].pump_pin >= 0)
        pinMode(configuration[i].pump_pin, OUTPUT);
      if(configuration[i].sensor_pin >= 0)
        pinMode(A0 + configuration[i].sensor_pin, INPUT);
    }
  }
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


bool config_valid(uint8_t cfg_index) {
  if(cfg_index < 0 || cfg_index >= MAX_NR_CONFIGS)
    return false;
  if(configuration[cfg_index].name[0] == 0xFF)
    return false;
  if(strlen(configuration[cfg_index].name) >= 16)
    return false;
  return true;
}


void show_time() {
  char clock[16];
  sprintf(clock, "\r%d.%d.%d %02d:%02d:%02d", day(), month(), year(), hour(), minute(), second());
  Serial.print(clock);
}

void process_giessomat() {
  loop_sensors();
    
  for(uint8_t i=0; i<MAX_NR_CONFIGS; i++) {
    if(config_valid(i)) {
      time_t difftime = now() - lasttime_pump_on[i];
      int seconds_on = minute(difftime) * 60 + second(difftime);
      
      if(hour(difftime) == 0 && seconds_on > configuration[i].seconds_on) {
        digitalWrite(configuration[i].pump_pin, LOW);
      }
      
      if(hour(difftime) >= configuration[i].hours_off) {
        if((configuration[i].auto_mode == AUTO_MODE_HIGHER && sensorvalues[i] > configuration[i].threashold) ||
           (configuration[i].auto_mode == AUTO_MODE_LOWER  && sensorvalues[i] < configuration[i].threashold) ||
           (configuration[i].auto_mode == AUTO_MODE_TIMER)) {
              digitalWrite(configuration[i].pump_pin, HIGH);
              lasttime_pump_on[i] = now();
           }
      }
      
    }
  }
}

