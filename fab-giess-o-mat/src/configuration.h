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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Arduino.h>
#include <avr/io.h>

#define SENSOR_PIN A1
#define PUMP_PIN   2
#define LED_PIN    5
#define BUTTON_START_PIN A0
#define BUTTON_STOP_PIN A3

#define BUTTON_PRESSED  LOW
#define PUMP_ON_VAL     HIGH
#define RGB_BRIGHTNESS  64

struct config_item
{
  uint16_t threashold_dry;
  uint16_t threashold_wet;
  uint16_t seconds_on;
  uint16_t minutes_off;
};

const int EEPROM_ADDRESS_CONFIG = 0;
extern struct config_item configuration;

#endif // CONFIGURATION_H
