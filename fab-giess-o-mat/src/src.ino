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

#define SENSOR_PIN 	PIN_A0
#define PUMP_PIN   	2
#define LED_PIN    	8
#define LED_PIN_5V    	7
#define LED_PIN_GND   	9
#define SENSOR_FLUIDLEVEL_PIN		PIN_A5
#define BUTTON_START_PIN 		3
#define BUTTON_STOP_PIN  		6
#define BUTTON_STOP_PIN_GND 4

extern void setup_spi(void);

struct config_item configuration;
time_t lasttime_pump_on[2];
Adafruit_NeoPixel ledstrip = Adafruit_NeoPixel(1, LED_PIN,
		NEO_GRB + NEO_KHZ800);
bool pump_is_on = false;

void setup() {
	pinMode(PUMP_PIN, OUTPUT);
	digitalWrite(PUMP_PIN, !PUMP_ON_VAL);

	pinMode(BUTTON_START_PIN, INPUT_PULLUP);

	pinMode(BUTTON_STOP_PIN, INPUT_PULLUP);
	pinMode(BUTTON_STOP_PIN_GND, OUTPUT);
	digitalWrite(BUTTON_STOP_PIN_GND, LOW);

	pinMode(SENSOR_PIN, INPUT);
	pinMode(PIN_A2, OUTPUT);
	digitalWrite(PIN_A2, LOW);
	pinMode(PIN_A1, OUTPUT);
	digitalWrite(PIN_A1, HIGH);

	pinMode(SENSOR_FLUIDLEVEL_PIN, INPUT_PULLUP);
	pinMode(PIN_A4, OUTPUT);
	digitalWrite(PIN_A4, LOW);

	pinMode(LED_PIN, OUTPUT);
	pinMode(LED_PIN_5V, OUTPUT);
	digitalWrite(LED_PIN_5V, HIGH);
	pinMode(LED_PIN_GND, OUTPUT);
	digitalWrite(LED_PIN_GND, LOW);

	// Konfiguration aus nicht fl��chtigen Speicher einlesen
	EEPROM.get(EEPROM_ADDRESS_CONFIG, configuration);
	if (configuration.version != VERSION) {
		// Standardwerte
		configuration.version = VERSION;
		configuration.threashold_dry = 120;
		configuration.threashold_wet = 100;
		configuration.seconds_on = 500;
		configuration.minutes_off = 1;
	}

	lasttime_pump_on[0] = 0;
	lasttime_pump_on[1] = 0;

	setup_spi();
	Serial.begin(115200);

	ledstrip.begin();

	Serial.println("\r\n* fab-giess-o-mat *");
	Serial.print("Pumpe  an Pin D"); Serial.println(PUMP_PIN);
	Serial.print("Sensor an Pin A"); Serial.println(SENSOR_PIN - PIN_A0);
	Serial.print("Fuellstandssensor an Pin A"); Serial.println(SENSOR_FLUIDLEVEL_PIN - PIN_A0);
	Serial.print("Taster Start an Pin D"); Serial.println(BUTTON_START_PIN);
	Serial.print("Taster Stop  an Pin D"); Serial.println(BUTTON_STOP_PIN);
	Serial.print("RGB LED an Pin D"); Serial.println(LED_PIN);

	print_mainmenu();
}

bool print_sensorvalues = false;
unsigned long last_millis = 0;

void loop() {
	// put your main code here, to run repeatedly:
	loop_mainmenu();
	loop_buttons();
	// loop_sensors();	// needed for simple sensor

	if (millis() - last_millis >= 250) {
		last_millis += 250;
		if (print_sensorvalues) {
			uint16_t sval1 = get_sensorvalue();
			Serial.println(sval1);
		} else {
			show_time_sensor();
		}
		set_statuscolor_sensor();
		loop_giessomat();
	}
}

unsigned long button_last_start_time = 0;
unsigned long button_last_stop_time = 0;

void loop_buttons() {
	int button_now_start = digitalRead(BUTTON_START_PIN);
	unsigned long time_pressed = millis() - button_last_start_time;
	if (button_now_start != BUTTON_PRESSED) {
		if(time_pressed > 3000) {
			configuration.seconds_on = time_pressed / 1000;
			lasttime_pump_on[0] = now();
			save_configuration();
			pump_off();
		}
		button_last_start_time = millis();
	}
	else {
		if(time_pressed > 20) {
			pump_on();
		}
		if(time_pressed > 1000 && time_pressed < 1100) {
			configuration.threashold_dry = get_sensorvalue();
			save_configuration();
		}
	}

	int button_now_stop = digitalRead(BUTTON_STOP_PIN);
	if (button_now_stop == !BUTTON_PRESSED) {
		button_last_stop_time = millis();
	}
	else {
		unsigned long time_pressed = millis() - button_last_stop_time;
		if(time_pressed > 20) {
			pump_off();
		}
		if(time_pressed > 2000) {
			lasttime_pump_on[0] = now();
			configuration.threashold_wet = get_sensorvalue();
			save_configuration();
		}
	}
}

void pump_on() {
	if (!pump_is_on) {
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
	sprintf(clock, "%d.%d.%d %02d:%02d:%02d", day(t), month(t), year(t), hour(t),
			minute(t), second(t));
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
	if (configuration.threashold_dry > 20
			&& configuration.threashold_wet < configuration.threashold_dry)
		use_sensor = true;

	return use_sensor;
}

void loop_giessomat() {
	if (!is_sensor_config_ok())
		return;

	time_t difftime = now() - lasttime_pump_on[0];
	uint16_t seconds_on = minute(difftime) * 60 + second(difftime);
	uint16_t minutes_off = hour(difftime) * 60 + minute(difftime);

	if (seconds_on >= configuration.seconds_on) {
		pump_off();
	}

	if (minutes_off >= configuration.minutes_off
			&& (get_sensorvalue() >= configuration.threashold_dry)) {
		pump_on();
	}
}

void save_configuration() {
	EEPROM.put(EEPROM_ADDRESS_CONFIG, configuration);
	set_color(RGB_BRIGHTNESS, RGB_BRIGHTNESS, RGB_BRIGHTNESS);
}

void set_color(uint8_t r, uint8_t g, uint8_t b) {
	ledstrip.setPixelColor(0, ledstrip.Color(r, g, b));
	ledstrip.show();
}

uint8_t fluid_level = 0;
uint8_t blink = 0;
void set_statuscolor_sensor() {
	uint16_t sensor = get_sensorvalue();

	if (digitalRead(SENSOR_FLUIDLEVEL_PIN) == 0) {
		fluid_level = 1;
	} else if (fluid_level == 1 && digitalRead(SENSOR_FLUIDLEVEL_PIN) == 1) {
		if (blink) {
			blink = 0;
			set_color(0, 0, RGB_BRIGHTNESS);
			return;
		}
		blink = 1;
	}

	if (!is_sensor_config_ok()) {
		set_color(0, 0, RGB_BRIGHTNESS);
		return;
	}

	if (sensor <= configuration.threashold_wet) {
		set_color(0, RGB_BRIGHTNESS, 0);
		return;
	}

	if (sensor >= configuration.threashold_dry) {
		set_color(RGB_BRIGHTNESS, 0, 0);
		return;
	}

	int diff = configuration.threashold_dry - configuration.threashold_wet;
	int green = (configuration.threashold_dry - sensor) * RGB_BRIGHTNESS / diff;
	set_color(RGB_BRIGHTNESS - green, green, 0);
}

uint16_t get_sensorvalue() {
	//return simple_sensor_get_sensorvalue();
	return analogRead(SENSOR_PIN);
}
