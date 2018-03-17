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

#include <SPI.h>
#include "configuration.h"

void setup_spi (void)
{
  // turn on SPI in slave mode
  SPCR |= bit (SPE);

  // have to send on master in, *slave out*
  pinMode (MISO, OUTPUT);

  // now turn on interrupts
  SPI.attachInterrupt();
}

volatile uint16_t sensorval = 0;
volatile byte command = 0;
volatile uint8_t bufptr = 0;

// SPI interrupt routine
ISR (SPI_STC_vect)
{
  byte i = SPDR;
  byte o = 0;

  switch(i) {
    case 's': {
      command = i;
      sensorval = get_sensorvalue();
      bufptr = 0;
      break;
    }
    case 'c': {
      command = i;
      bufptr = 0;
      break;
    }
  }

  switch(command) {
    case 's': {
      if(bufptr < sizeof(sensorval)) {
        uint8_t *pSens = (uint8_t*)&sensorval;
        o = pSens[bufptr];
      }
      break;
    }
    case 'c': {
      if(bufptr < sizeof(configuration)) {
        byte *pConfig = (byte*)&configuration;
        o = pConfig[bufptr];
      }
      break;
    }
  }
  SPDR = o;
  bufptr++;
}
