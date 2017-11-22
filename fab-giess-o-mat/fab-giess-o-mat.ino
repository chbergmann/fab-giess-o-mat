/*
 *  This file is part of fab-giess-o-mat.
 *  
 *  fab-giess-o-mat is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include <EEPROM.h>
#include <TimeLib.h>
#include "configuration.h"

struct config_item configuration; 
time_t lasttime_pump_on[2]; 

void setup() {
  // put your setup code here, to run once:
  EEPROM.get(EEPROM_ADDRESS_CONFIG, configuration);
  if(configuration.auto_mode > AUTO_MODE_TIMER) {
    // Standardwerte
    configuration.auto_mode = AUTO_MODE_TIMER;
    configuration.threashold = 250;
    configuration.seconds_on = 5;
    configuration.minutes_off = 24 * 60;
    configuration.sensor_cntval = 62;
  }
 
  lasttime_pump_on[0] = 0;
  lasttime_pump_on[1] = 0;
  
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  start_read_sensors();
  
  Serial.begin(115200);
  print_mainmenu();
}

unsigned long last_millis = 0;
void loop() {
  // put your main code here, to run repeatedly:
  loop_sensors();
  loop_mainmenu();
  
  if(millis() - last_millis >= 1000) {
    last_millis += 1000;
    show_time_sensor();
    loop_giessomat();
  } 
}

void pump_on() {
  if(digitalRead(PUMP_PIN) == LOW) {
    digitalWrite(PUMP_PIN, HIGH);
    lasttime_pump_on[1] = lasttime_pump_on[0];
    lasttime_pump_on[0] = now();
  }
}

void pump_off() {
  digitalWrite(PUMP_PIN, LOW);
}

void show_time(time_t t) {
  char clock[32];
  sprintf(clock, "%d.%d.%d %02d:%02d:%02d", day(t), month(t), year(t), hour(t), minute(t), second(t));
  Serial.print(clock);
}

void show_time_sensor() {
  Serial.print('\r');
  show_time(now());
  Serial.print(" Sensor: ");
  Serial.print(get_sensorvalue());
}

void show_lasttime_pump_on(uint8_t last) {
  show_time(lasttime_pump_on[last]);
  Serial.println();
}

void loop_giessomat() {
  if(configuration.auto_mode != MANUAL_MODE) {
    time_t difftime = now() - lasttime_pump_on[0];
    uint16_t seconds_on = minute(difftime) * 60 + second(difftime);
    uint16_t minutes_off = hour(difftime) * 60 + minute(difftime);

    if(seconds_on >= configuration.seconds_on) {
      pump_off();
    }
    
    if(minutes_off >= configuration.minutes_off) {
      if((configuration.auto_mode == AUTO_MODE_HIGHER && get_sensorvalue() > configuration.threashold) ||
         (configuration.auto_mode == AUTO_MODE_LOWER  && get_sensorvalue() < configuration.threashold) ||
         (configuration.auto_mode == AUTO_MODE_TIMER)) {
            pump_on();
         }
    } 
  }
}

void save_configuration() {
  EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);
}

