#include "configuration.h"

int sensorvalues[MAX_NR_CONFIGS];
bool mode_charging = true;
bool sensor_ready = false;
int16_t sensor_cntval = 62;

void start_read_sensors() {
  sensor_ready = false;
  
  // charge condensators
  for(uint8_t i=0; i<MAX_NR_CONFIGS; i++) {
    if(configuration[i].sensor_pin >= 0) {
      pinMode(A0 + configuration[i].sensor_pin, OUTPUT);
      digitalWrite(A0 + configuration[i].sensor_pin, HIGH);
    }
  }
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

void loop_read_sensors() {
  Serial.println();
  for(uint8_t i=0; i<MAX_NR_CONFIGS; i++) {
    if(configuration[i].sensor_pin >= 0) {
      Serial.print(" A");
      Serial.print(configuration[i].sensor_pin);
    }
  }
  Serial.println();
  start_read_sensors();
  
  while(Serial.available() == 0) {
    if(sensor_ready) {
      for(uint8_t i=0; i<MAX_NR_CONFIGS; i++) {
        if(configuration[i].sensor_pin >= 0) {
          Serial.print(sensorvalues[i]);
          Serial.print(" ");
        }          
      }  
      Serial.println();
      start_read_sensors();
    }
  }
}

void start_discharge() {
  for(uint8_t i=0; i<MAX_NR_CONFIGS; i++) {
    if(configuration[i].sensor_pin >= 0) {
      pinMode(A0 + configuration[i].sensor_pin, INPUT);
      digitalWrite(A0 + configuration[i].sensor_pin, LOW);
      mode_charging = false;
    }
  }
  
  // initialize timer1 to call ISR in xxx milliseconds
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = sensor_cntval;    // compare match register 16MHz/256/?Hz
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();   
}


ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{
  if(mode_charging) {
    // charging ready   
    start_discharge();
  }
  else {
    for(uint8_t i=0; i<MAX_NR_CONFIGS; i++) {
      if(configuration[i].sensor_pin >= 0) {
        sensorvalues[i] = analogRead(configuration[i].sensor_pin);
      }
    }
    sensor_ready = true;
    noInterrupts();           // disable all interrups
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    interrupts();   
  }
}

