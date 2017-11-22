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

volatile int sensorvalue = 0;
volatile bool mode_charging = true;
volatile bool sensor_ready = false;

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
      Serial.println(sensorvalue);
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
  return sensorvalue;
}

ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{
  if(mode_charging) {
    // charging ready   
    start_discharge();
  }
  else {
    sensorvalue = analogRead(SENSOR_PIN);
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
  Serial.println("Sensor trocken legen !");
  calibrate_sensor(1, 100);
}

void calibrate_sensor(int start, int inc) {  
  for(int t=start; t< start + inc * 10; t = t + inc) {
    calibrate(t);
    if(sensorvalue < 150) {
      if(inc == 1) {
        configuration.sensor_cntval = t - 1;
        save_configuration();
        Serial.print("Ermittelte Entladezeit [us]: ");
        long microseconds = configuration.sensor_cntval * TIMER1_PRESCALE_US;
        Serial.println(microseconds);
      }
      else {
        calibrate_sensor(t - inc, inc / 10);
      }
      return;
    }
  }
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

