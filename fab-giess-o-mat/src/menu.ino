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

#include "configuration.h"

extern bool print_sensorvalues;

void print_mainmenu() {
  Serial.println();
  Serial.print("vorletzte Pump-Zeit: "); show_lasttime_pump_on(1);
  Serial.print("letzte    Pump-Zeit: "); show_lasttime_pump_on(0);
  Serial.println();
  Serial.println("* fab-giess-o-mat Hauptmenue *");
  Serial.println(" u - Uhr stellen");
  Serial.print  (" e - maximale Einschaltzeit [sek]: "); Serial.println(configuration.seconds_on);
  Serial.print  (" a - minimale Ausschaltzeit [std:min]: ");
  char txtbuf[10];
  sprintf(txtbuf, "%02d:%02d", configuration.minutes_off / 60, configuration.minutes_off % 60);
  Serial.println(txtbuf);
  Serial.print  (" t - Schaltschwelle trocken: "); Serial.println(configuration.threashold_dry);
  Serial.print  (" n - Schaltschwelle nass: "); Serial.println(configuration.threashold_wet);
  Serial.println(" s - Sensor lesen");
  Serial.println(" i - Pumpe ein");
  Serial.println(" o - Pumpe aus");
  Serial.println();

  while(Serial.available() > 0)
    Serial.read();
}

void loop_mainmenu() {
  if(Serial.available() > 0) {
      print_sensorvalues = false;
      switch(Serial.read()) {
        case 'u':
          set_time();
          break;

        case 'e': {
          Serial.print("\r\nEinschaltzeit in Sekunden: ");
          configuration.seconds_on = Serial_readNumber();
          save_configuration();
          break;
        }

        case 'a': {
          Serial.print("\r\nminimale Ausschaltzeit Stunden: ");
          int hours_off = Serial_readNumber();
          Serial.print("\r\nMinuten: ");
          int minutes_off = Serial_readNumber();
          configuration.minutes_off = hours_off * 60 + minutes_off;
          save_configuration();
          break;
        }

        case 'i': {
          pump_on();
          break;
        }

        case 'o': {
          pump_off();
          break;
        }

        case 't': {
          Serial.print("\r\nSchaltschwelle trocken: ");
          configuration.threashold_dry = Serial_readNumber();
          save_configuration();
          break;
        }

        case 'n': {
          Serial.print("\r\nSchaltschwelle nass: ");
          configuration.threashold_wet = Serial_readNumber();
          save_configuration();
          break;
        }

        case 's':
          Serial.println("Simple,Analog, FreqCount");
          print_sensorvalues = true;
          break;
      }
      print_mainmenu();
  }
}


int Serial_readNumber() {
  int number = 0;
  while(1) {
    if(Serial.available() > 0) {
      int num = Serial.read();
      if(num == '\n' || num == '\r')
        return number;

      if(num >= '0' && num <= '9') {
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
      if(c == '\n') {
        buf[l] = 0;
        return l-1;
      }
      if(c > 31)
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

void wait_press_anykey() {
  while(Serial.available() > 0)
    Serial.read();

  while(Serial.available() == 0);
}
