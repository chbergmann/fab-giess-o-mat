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

#include <Arduino.h>
#include <EEPROM.h>
#include <TimeLib.h>
#include <Adafruit_NeoPixel.h>

#include "configuration.h"

extern void setup_spi (void);

struct config_item configuration;
time_t lasttime_pump_on[2];
Adafruit_NeoPixel ledstrip = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
bool pump_is_on = false;


void setup() {
  // put your setup code here, to run once:
  EEPROM.get(EEPROM_ADDRESS_CONFIG, configuration);
  if(configuration.threashold_dry == 0 || configuration.threashold_dry == 0xffff) {
    // Standardwerte
    configuration.threashold_dry = 10000;
    configuration.threashold_wet = 0;
    configuration.seconds_on = 5;
    configuration.minutes_off = 24 * 60;
  }

  lasttime_pump_on[0] = 0;
  lasttime_pump_on[1] = 0;

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  setup_spi();
  Serial.begin(115200);
  print_mainmenu();

  ledstrip.begin();
}

bool print_sensorvalues = false;
unsigned long last_millis = 0;
void loop() {
  // put your main code here, to run repeatedly:
  loop_sensors();
  loop_mainmenu();
  loop_button();

  if(millis() - last_millis >= 250) {
    last_millis += 250;
    if(print_sensorvalues) {
        uint16_t sval1 = get_sensorvalue();
        Serial.println(sval1);
    }
    else {
      show_time_sensor();
    }
    set_statuscolor_sensor();
    loop_giessomat();
  }
}


int button_last = HIGH;

void loop_button() {
  int button_now = digitalRead(BUTTON_PIN);
  if(button_now == BUTTON_PRESSED && button_last != BUTTON_PRESSED) {
    if(!pump_is_on) {
      //configuration.threashold_dry = get_sensorvalue();
      pump_on();
    }
    else {
      pump_off();
    }
  }

  if(button_now != BUTTON_PRESSED && button_last == BUTTON_PRESSED) {
    if(pump_is_on) {
      //configuration.threashold_wet = get_sensorvalue();
      pump_off();
    }
  }

  button_last = button_now;
}

void pump_on() {
  if(!pump_is_on) {
    digitalWrite(PUMP_PIN, PUMP_ON_VAL);
    lasttime_pump_on[1] = lasttime_pump_on[0];
    lasttime_pump_on[0] = now();
  }
  pump_is_on = true;
}

void pump_off() {
  digitalWrite(PUMP_PIN, !PUMP_ON_VAL);
  pump_is_on = false;
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
  Serial.print("  ");
}

void show_lasttime_pump_on(uint8_t last) {
  show_time(lasttime_pump_on[last]);
  Serial.println();
}

bool is_sensor_config_ok() {
  bool use_sensor = false;
  if(configuration.threashold_dry > 20 && configuration.threashold_wet < configuration.threashold_dry)
    use_sensor = true;

  return use_sensor;
}

void loop_giessomat() {
  int button_now = digitalRead(BUTTON_PIN);
  time_t difftime = now() - lasttime_pump_on[0];
  uint16_t seconds_on = minute(difftime) * 60 + second(difftime);
  uint16_t minutes_off = hour(difftime) * 60 + minute(difftime);

  if(seconds_on >= configuration.seconds_on ||
    (is_sensor_config_ok() && get_sensorvalue() <= configuration.threashold_wet)) {
      if(digitalRead(BUTTON_PIN) != BUTTON_PRESSED)
        pump_off();
  }

  if(minutes_off >= configuration.minutes_off &&
    (!is_sensor_config_ok() || get_sensorvalue() >= configuration.threashold_dry)) {
      pump_on();
  }
}

void save_configuration() {
  EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);
}

void set_color(uint8_t r, uint8_t g, uint8_t b) {
  ledstrip.setPixelColor(0, ledstrip.Color(r, g, b));
  ledstrip.show();
}

void set_statuscolor_sensor() {
  int sensor = get_sensorvalue();
/*
  if(pump_is_on) {
    set_color(0, 0, RGB_BRIGHTNESS);
    return;
  }
*/
  if(!is_sensor_config_ok()) {
    set_color(0, 0, 0);
    return;
  }

  if(sensor <= configuration.threashold_wet) {
    set_color(0, RGB_BRIGHTNESS, 0);
    return;
  }

  if(sensor >= configuration.threashold_dry) {
    set_color(RGB_BRIGHTNESS, 0, 0);
    return;
  }

  int diff = configuration.threashold_dry - configuration.threashold_wet;
  int green = (configuration.threashold_dry - sensor) * RGB_BRIGHTNESS / diff;
  set_color(RGB_BRIGHTNESS - green, green, 0);
}

int get_sensorvalue() {
  return analogRead(SENSOR_PIN);
}
