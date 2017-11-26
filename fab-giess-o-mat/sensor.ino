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
 **
 * 
 *  Einfacher kapazitiver Sensor
 * 
 *  Der Sensor besteht nur aus 2 Kondensatorplatten und einem hochohmigen 
 *  Widerstand. Der Arduino schaltet zuerst einen Ausgang auf high und läd 
 *  den Kondensator auf. Dann wir der Pin auf ADC-Input umgeschaltet und 
 *  nach einer kurzen Zeit gemessen, wieviel Spannung noch anliegt.
 * 
 */
 
#include "configuration.h"

volatile bool mode_charging = true;
volatile bool sensor_ready = false;
volatile int sensorvalue;

#define AVERAGE_COUNT 5
volatile uint16_t average_sum = 0;

void start_read_sensors() {
  sensor_ready = false;
  
  // charge condensators
  pinMode(SENSOR_PIN, OUTPUT);
  digitalWrite(SENSOR_PIN, HIGH);
    
  mode_charging = true;
  
  // initialize timer1 to call ISR in 1 second
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 625;            // compare match register 16MHz/256/100Hz
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();   
}

bool loop_sensors() {
  if(sensor_ready) {
    start_read_sensors();
    return true;
  }
  return false;
}

void loop_print_sensors() {
  unsigned long last_millisec = 0;
  Serial.println();
  start_read_sensors();
  
  while(Serial.available() == 0) {
    loop_giessomat();
    if(sensor_ready) {
      Serial.println(get_sensorvalue());
      start_read_sensors();
      while(millis() - last_millisec < 500);
      last_millisec = millis();
    }
  }
}

void start_discharge() {
  pinMode(SENSOR_PIN, INPUT);
  digitalWrite(SENSOR_PIN, LOW);
  mode_charging = false;
  
  // initialize timer1 to call ISR in xxx milliseconds
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = configuration.sensor_cntval;    // compare match register 16MHz/256/?Hz
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();   
}

int get_sensorvalue() {
  return average_sum / AVERAGE_COUNT;
}

ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{
  if(mode_charging) {
    // charging ready   
    start_discharge();
  }
  else {
    sensorvalue = analogRead(SENSOR_PIN);
    if(average_sum == 0) {
      average_sum = sensorvalue * AVERAGE_COUNT;
    }
    else {
      average_sum = average_sum / AVERAGE_COUNT * (AVERAGE_COUNT - 1);
      average_sum += sensorvalue;
    }
    sensor_ready = true;
    noInterrupts();           // disable all interrups
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    interrupts();   
  }
}

void calibrate_sensor() {
  Serial.println();
  Serial.println("Sensor trocken legen, dann Taste druecken");
  wait_press_anykey();
  if(calibrate_sensor(1, 1000) == false) {
    Serial.println("Der Sensor funktioniert nicht !");
    return;
  }

  configuration.threashold_dry = sensorvalue;
  Serial.println("Jetzt maximale Feuchte, dann Taste druecken");
  wait_press_anykey();
  start_read_sensors();
  while(!loop_sensors());
  configuration.threashold_wet = sensorvalue;
  save_configuration();
}

bool calibrate_sensor(int start, int inc) {  
  for(int t=start; t< start + inc * 10; t = t + inc) {
    calibrate(t);
    if(sensorvalue < 150) {
      if(inc == 1) {
        Serial.print("Ermittelte Entladezeit: ");
        average_sum = 0;
        calibrate(t - 1);
        if(sensorvalue > 20 && sensorvalue < 900) {
          return true;
        }
        else {
          return false;
        }        
      }
      else {
        calibrate_sensor(t - inc, inc / 10);
      }
      return true;
    }
  }
  return false;
}

void calibrate(int timerval) {
  configuration.sensor_cntval = timerval;
  start_read_sensors();
  long microseconds = configuration.sensor_cntval * TIMER1_PRESCALE_US;
  Serial.print(microseconds);
  Serial.print(" us:\t");
  while(!sensor_ready);
  Serial.println(sensorvalue);
  Serial.flush();
}

