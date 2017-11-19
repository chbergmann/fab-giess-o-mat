/* fab-giess-o-mat */

#include <EEPROM.h>
#include "configuration.h"
extern int16_t sensor_cntval;

const String SWITCH_PUMP_IF = "Pumpe einschalten, wenn ";
const String AUTO_MODE_HIGHER_STR = "Sensorwert ueberschritten wird";
const String AUTO_MODE_LOWER_STR = "Sensorwert unterschritten wird";
const String AUTO_MODE_TIMER_STR = "die minimale Ausschaltzeit abgelaufen ist (Sensor ignorieren)";
const String MANUAL_MODE_STR = "i oder o gedrueckt wird";

void mainmenu() {
  Serial.println("\r\n* Giess-o-mat Hauptmenue *");
  Serial.print("Pumpe an Pin D");
  Serial.println(PUMP_PIN);
  Serial.print("Sensor an Pin A");
  Serial.println(SENSOR_PIN - A0);
  Serial.println(" u - Uhr stellen");
  Serial.print(" c - Sensor timer value [us]: ");
  Serial.println(configuration.sensor_cntval * 16);
  Serial.print(" p - ");
  Serial.print(SWITCH_PUMP_IF);
  switch(configuration.auto_mode) {
    case AUTO_MODE_HIGHER: Serial.println(AUTO_MODE_HIGHER_STR); break;
    case AUTO_MODE_LOWER:  Serial.println(AUTO_MODE_LOWER_STR); break;
    case AUTO_MODE_TIMER:  Serial.println(AUTO_MODE_TIMER_STR); break;
    case MANUAL_MODE:     Serial.println(MANUAL_MODE_STR); break;
  }
  Serial.print(" e - Einschaltzeit: ");
  Serial.print(configuration.seconds_on);
  Serial.println(" sek");
  Serial.print(" a - minimale Ausschaltzeit: ");
  Serial.print(configuration.hours_off);
  Serial.println(" std");
  Serial.println(" i - Pumpe ein");
  Serial.println(" o - Pumpe aus");
  
  Serial.print(" x - Schaltschwelle: ");
  Serial.println(configuration.threashold);
  Serial.println(" s - Sensoren lesen");
  Serial.println();
  
  while(1) {
    loop(); 
    if(Serial.available() > 0) {
      switch(Serial.read()) {
        case 'u': 
          set_time(); 
          break;
          
        case 'c': {
          Serial.print("\r\nnew timer value: ");
          int us = Serial_readNumber();
          configuration.sensor_cntval = us / 16;
          break;
        }        
                
        case 'p': {
          Serial.println();
          Serial.println(SWITCH_PUMP_IF);
          Serial.print(" 1 - ");
          Serial.println(AUTO_MODE_HIGHER_STR);
          Serial.print(" 2 - ");
          Serial.println(AUTO_MODE_LOWER_STR);
          Serial.print(" 3 - ");
          Serial.println(AUTO_MODE_TIMER_STR);
          Serial.print(" 4 - ");
          Serial.println(MANUAL_MODE_STR);
          while(!Serial.available());
          switch(Serial.read()) {
            case '1': configuration.auto_mode = AUTO_MODE_HIGHER; break;
            case '2': configuration.auto_mode = AUTO_MODE_LOWER; break;
            case '3': configuration.auto_mode = AUTO_MODE_TIMER; break;
            case '4': configuration.auto_mode = MANUAL_MODE; break;
          }
          EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);          
          break;
        }

        case 'e': {
          Serial.print("\r\nEinschaltzeit in Sekunden: ");
          configuration.seconds_on = Serial_readNumber();
          EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);
          break;
        }

        case 'a': {
          Serial.print("\r\nminimale Ausschaltzeit in Stunden: ");
          configuration.hours_off = Serial_readNumber();
          EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);
          break;
        }
        
        case 'i': {
          configuration.auto_mode = MANUAL_MODE;
          digitalWrite(PUMP_PIN, HIGH);
          break;  
        }
        
        case 'o': {
          configuration.auto_mode = MANUAL_MODE;
          digitalWrite(PUMP_PIN, LOW);
          break;  
        }        

        case 'x': {
          Serial.print("\r\nSchaltschwelle: ");
          configuration.threashold = Serial_readNumber();
          EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);
          break;
        }
        
        case 's': 
          loop_print_sensors(); 
          break;
      }
      mainmenu();
      return;
    }
  }
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

