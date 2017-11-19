#include "configuration.h"

int sensorvalue = 0;
bool mode_charging = true;
bool sensor_ready = false;

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

  OCR1A = 62500;            // compare match register 16MHz/256/1Hz
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
  Serial.println();
  start_read_sensors();
  
  while(Serial.available() == 0) {
    if(sensor_ready) {
      Serial.println(sensorvalue);
      start_read_sensors();
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

