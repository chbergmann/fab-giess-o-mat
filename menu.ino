/* fab-giess-o-mat */

#include <EEPROM.h>
#include "configuration.h"
extern int16_t sensor_cntval;

void mainmenu() {
  Serial.println("\r\n* Giess-o-mat Hauptmenue *");
  Serial.println(" k - Konfiguration anzeigen");
  Serial.println(" u - Uhr stellen");
  Serial.print(" c - Sensor timer value [us]: ");
  Serial.println(sensor_cntval * 16);
  Serial.println(" s - Sensoren lesen");
  
  for(uint8_t i=0; i<MAX_NR_CONFIGS; i++) {
    if(config_valid(i))
    {
      Serial.print(' ');
      Serial.print(i+1);
      Serial.print(" - ");
      Serial.println(configuration[i].name);
    }
  }
  Serial.println(" n - neue Pflanze\r\n");
  
  while(1) {
    loop(); 
    if(Serial.available() > 0) {
      int menu = Serial.read();
      int cfg_index = menu - '1';
      if(config_valid(cfg_index)) {
          configmenu(cfg_index);
      }
      else switch(menu) {
        case 'k': print_configuration(); break;
        case 'n': new_plant(); break;
        case 'u': set_time(); break;
        case 'c': {
          Serial.print("\r\nnew timer value: ");
          int us = Serial_readNumber();
          sensor_cntval = us / 16;
          break;
        }
        case 's': loop_read_sensors(); break;
      }
      mainmenu();
      return;
    }
  }
}

void print_configuration() {
  Serial.println("\r\nPflanzen-Name    Pumpe  Sensor Wert");
  int count = 0;
  for(uint8_t i=0; i<MAX_NR_CONFIGS; i++) {
    if(config_valid(i))
    {
      Serial.print(configuration[i].name);
      for(int space=strlen(configuration[i].name); space < 17; space++)
        Serial.print(' ');

      if(configuration[i].pump_pin < 0) {
        Serial.print("keine");
      }
      else {
        Serial.print("Pin:D");
        Serial.print(configuration[i].pump_pin);
      }
      
      if(configuration[i].sensor_pin < 0) {
        Serial.print(" keiner ");
      }
      else {
        Serial.print(" Pin:A");
        Serial.print(configuration[i].sensor_pin); 
        Serial.print(" ");
        Serial.println(analogRead(configuration[i].sensor_pin));
      }
      
      count++;
    }
  }
  if(count == 0)
    Serial.println("keine Pflanze konfiguriert.");
}


const String SWITCH_PUMP_IF = "Pumpe einschalten, wenn ";
const String AUTO_MODE_HIGHER_STR = "Sensorwert ueberschritten wird";
const String AUTO_MODE_LOWER_STR = "Sensorwert unterschritten wird";
const String AUTO_MODE_TIMER_STR = "die minimale Ausschaltzeit abgelaufen ist (Sensor ignorieren)";

void configmenu(int cfg_index) {
  Serial.println("\r\nKonfiguration");
  Serial.print(" n - Name: ");
  Serial.println(configuration[cfg_index].name);
  
  Serial.print(" p - Pumpe ");
  if(configuration[cfg_index].pump_pin < 0) {
    Serial.println("anschliessen");
  }
  else {
    Serial.print("Pin: D");
    Serial.println(configuration[cfg_index].pump_pin);
    Serial.print("     ");
    Serial.print(SWITCH_PUMP_IF);
    switch(configuration[cfg_index].auto_mode) {
      case AUTO_MODE_HIGHER: Serial.println(AUTO_MODE_HIGHER_STR); break;
      case AUTO_MODE_LOWER:  Serial.println(AUTO_MODE_LOWER_STR); break;
      case AUTO_MODE_TIMER:  Serial.println(AUTO_MODE_TIMER_STR); break;
    }
    Serial.print(" e - Einschaltzeit: ");
    Serial.print(configuration[cfg_index].seconds_on);
    Serial.println(" sek");
    Serial.print(" a - minimale Ausschaltzeit: ");
    Serial.print(configuration[cfg_index].hours_off);
    Serial.println(" std");
    Serial.println(" i - Pumpe ein");
    Serial.println(" o - Pumpe aus");
  }
  
  Serial.print(" s - Sensor ");
  if(configuration[cfg_index].sensor_pin < 0) {
    Serial.println("anschliessen");
  }
  else {
    Serial.print("Pin: A");
    Serial.println(configuration[cfg_index].sensor_pin);
    Serial.print(" x - Schaltschwelle: ");
    Serial.println(configuration[cfg_index].threashold);
  }
  
  Serial.println(" l - loeschen");
  Serial.println(" z - Hauptmenue");

  int lastmilli = 0;
  while(1) {
    
    if(Serial.available() > 0) {
      int menu = Serial.read();
      switch(menu) {
        case 'n': {
          Serial.print("\r\nneuer Name: ");
          Serial_readCharArray(configuration[cfg_index].name, sizeof(configuration[cfg_index].name));
          EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);
          break;
        }
        
        case 'p': {
          Serial.print("\r\nPumpe angeschlossen an Pin D");
          int pump_pin = Serial_readNumber();
          Serial.println();
          if(!pin_already_used(pump_pin, false)) {
            configuration[cfg_index].pump_pin = pump_pin;
            pump_mode(cfg_index);
            pump_seconds_on(cfg_index);
            pump_hours_off(cfg_index);
            init_pins();
            EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);
          }
          break;
        }
        
        case 's': {
          Serial.print("\r\nSensor angeschlossen an Pin A");
          int sensor_pin = Serial_readNumber();
          if(!pin_already_used(sensor_pin, true)) {              
            configuration[cfg_index].sensor_pin = sensor_pin;
            init_pins();
            EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);
          }
          break;
        }
        
        case 'l': {
          memset(&configuration[cfg_index], 0xFF, sizeof(configuration[cfg_index]));
          EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);
          print_configuration();
          return;
        }

        case 'e': {
          pump_seconds_on(cfg_index);
          EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);
          break;
        }

        case 'a': {
          pump_hours_off(cfg_index);
          EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);
          break;
        }
        
        case 'i': {
          if(configuration[cfg_index].pump_pin >= 0)
            digitalWrite(configuration[cfg_index].pump_pin, HIGH);
          break;  
        }
        
        case 'o': {
          if(configuration[cfg_index].pump_pin >= 0)
            digitalWrite(configuration[cfg_index].pump_pin, LOW);
          break;  
        }        

        case 'x': {
          Serial.print("\r\nSchaltschwelle: ");
          configuration[cfg_index].threashold = Serial_readNumber();
          EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);
          break;
        }
        
        default: {
          // back to mainmenu
          print_configuration();
          return;
        }
      }
      configmenu(cfg_index);
      return;
    }
  }
}


void pump_mode(int cfg_index) {
  Serial.println(SWITCH_PUMP_IF);
  Serial.print(" 1 - ");
  Serial.println(AUTO_MODE_HIGHER_STR);
  Serial.print(" 2 - ");
  Serial.println(AUTO_MODE_LOWER_STR);
  Serial.print(" 3 - ");
  Serial.println(AUTO_MODE_TIMER_STR);
  while(!Serial.available());
  switch(Serial.read()) {
    case '1': configuration[cfg_index].auto_mode = AUTO_MODE_HIGHER; break;
    case '2': configuration[cfg_index].auto_mode = AUTO_MODE_LOWER; break;
    case '3': configuration[cfg_index].auto_mode = AUTO_MODE_TIMER; break;
  }
}

void pump_seconds_on(int cfg_index) {
  Serial.print("\r\nEinschaltzeit in Sekunden: ");
  configuration[cfg_index].seconds_on = Serial_readNumber();
}

void pump_hours_off(int cfg_index) {
  Serial.print("\r\nminimale Ausschaltzeit in Stunden: ");
  configuration[cfg_index].hours_off = Serial_readNumber();
}

bool pin_already_used(int pinnr, bool analog) {
  for(uint8_t i=0; i<MAX_NR_CONFIGS; i++) {
    if(config_valid(i) && 
      ((analog && configuration[i].sensor_pin == pinnr) || 
      (!analog && configuration[i].pump_pin == pinnr))) {
        Serial.print("\r\nDieser Pin wird schon benutzt von ");
        Serial.println(configuration[i].name);
        return true;              
    }
  }
  return false;
}

int Serial_readNumber() {
  int number = 0;
  while(1) {
    if(Serial.available() > 0) {
      int num = Serial.read();
      if(num == '\r' || num == '\n')
        return number;

      if(num >= '0' || num <= '9') {
        number = number * 10 + num - '0';
        Serial.write(num);
      }
    }
  }
}

int Serial_readCharArray(char* buf, int len) {
  for(int l=0; l<len; l++) {
      while(Serial.available() == 0);
      char c = Serial.read();
      Serial.write(c);
      if(c == '\r' || c == '\n') {
        buf[l] = 0;
        return l-1;
      }
      buf[l] = c;
  }
  buf[len-1] = 0;
  return len-1;
}

void new_plant() {
  for(uint8_t i=0; i<MAX_NR_CONFIGS; i++) {
    if(!config_valid(i))
    {
      sprintf(configuration[i].name, "Pflanze %d", i+1);
      configuration[i].sensor_pin = -1;
      configuration[i].pump_pin = -1;
      configuration[i].auto_mode = AUTO_MODE_TIMER;
      configuration[i].threashold = 0;
      configuration[i].seconds_on = 1;
      configuration[i].hours_off = 24;
      configmenu(i);
      return;
    }
  }
  Serial.println("mehr geht nicht !");
}

void set_time() {
  Serial.print("\r\nStunde: ");
  int hr = Serial_readNumber();
  Serial.print("\r\nMinute: ");
  int mn = Serial_readNumber();
  Serial.print("\r\nSekunde: ");
  int sec = Serial_readNumber();
  Serial.print("\r\nTag: ");
  int dy = Serial_readNumber();
  Serial.print("\r\nMonat: ");
  int mnth = Serial_readNumber();
  Serial.print("\r\nJahr: ");
  int yr = Serial_readNumber();
  setTime(hr, mn, sec, dy, mnth, yr);
}

