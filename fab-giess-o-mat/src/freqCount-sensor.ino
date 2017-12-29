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
#include <FreqCount.h>

unsigned long freqCountSensor_value = 0;

void freqCountSensor_init() {
  FreqCount.begin(1000);
}

void freqCountSensor_loop() {
  if (FreqCount.available()) {
    freqCountSensor_value = FreqCount.read();
  }
}

int freqCountSensor_get_value() {
  return freqCountSensor_value / 100;
}
