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
  pinMode (PIN_SPI_MISO, OUTPUT);
  pinMode (PIN_SPI_MOSI, INPUT);
  pinMode (PIN_SPI_SCK,  INPUT);
  pinMode (PIN_SPI_SS,   INPUT);

  // now turn on interrupts
  SPI.attachInterrupt();
}

volatile uint16_t sensorval = 0;
volatile uint8_t command = 0;
volatile uint8_t nxtbyte = 0;

// SPI interrupt routine
ISR (SPI_STC_vect)
{
	uint8_t bufptr = 0;
	uint8_t i = SPDR;

  switch(i) {
    case 's': {
      command = i;
      sensorval = get_sensorvalue();
      break;
    }
    case 'c': {
      command = i;
      break;
    }

    default: {
    	if(i >= 0 && i <= 9)
    		bufptr = i;
    	break;
    }
  }

  switch(command) {
    case 's': {
      if(bufptr < sizeof(sensorval)) {
        uint8_t *pSens = (uint8_t*)&sensorval;
        SPDR = pSens[bufptr];
      }
      break;
    }
    case 'c': {
      if(bufptr < sizeof(configuration)) {
        byte *pConfig = (byte*)&configuration;
        SPDR = pConfig[bufptr];
      }
      break;
    }
    default:
    	SPDR = i;
    	break;
  }
}
