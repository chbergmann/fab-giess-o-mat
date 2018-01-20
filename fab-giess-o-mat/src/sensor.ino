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
 *  den Kondensator auf. Dann wird der Pin auf ADC-Input umgeschaltet und
 *  gemessen, wie lange es dauert, bis der Kondensator entladen ist.
 *
 */

#include "configuration.h"

#define CHARGE_TIME_MS  250

typedef enum {
  IDLE,
  CHARGE,
  DISCHARGE,
  READY
} SENSOR_STATE;

SENSOR_STATE sensor_state = IDLE;

volatile bool sensor_ready = false;
unsigned long last_time = 0;
volatile int sensorvalue = 7;

#define AVERAGE_COUNT 5
volatile uint16_t average_sum = 0;

void start_read_sensors() {
  sensor_ready = false;

  // charge condensators
  pinMode(SENSOR_PIN, OUTPUT);
  digitalWrite(SENSOR_PIN, HIGH);
  last_time = millis();
}

void loop_sensors() {
  switch(sensor_state) {
  case IDLE:
    start_read_sensors();
    sensor_state = CHARGE;
    break;

  case CHARGE:
    if(millis() - last_time > CHARGE_TIME_MS) {
      start_discharge();
      sensor_state = DISCHARGE;
    }
    break;

  case DISCHARGE:
    break;

  case READY:
    sensor_state = IDLE;
    break;
  }
}

void start_discharge() {

  pinMode(SENSOR_PIN, INPUT);
  digitalWrite(SENSOR_PIN, LOW);
  last_time = micros();
  ADCSRB = 0;
  ACSR =  bit (ACI)     // (Clear) Analog Comparator Interrupt Flag
      | bit (ACIE)    // Analog Comparator Interrupt Enable
      | bit (ACIS1);  // ACIS1, ACIS0: Analog Comparator Interrupt Mode Select (trigger on falling edge)
}

int simple_sensor_get_sensorvalue() {
  return sensorvalue; //average_sum / AVERAGE_COUNT;
}

ISR (ANALOG_COMP_vect)
{
  unsigned long sval = (micros() - last_time) / 10;
  sensorvalue = (int)sval;
  ACSR = bit (ACI);     // (Clear) Analog Comparator Interrupt Flag and disable Analog comparator
  if(average_sum == 0) {
    average_sum = sensorvalue * AVERAGE_COUNT;
  }
  else {
    average_sum = average_sum / AVERAGE_COUNT * (AVERAGE_COUNT - 1);
    average_sum += sensorvalue;
  }
  sensor_state = READY;
}
