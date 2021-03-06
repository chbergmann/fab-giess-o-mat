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

 #ifndef _SPIMASTER_H_INCLUDED
 #define _SPIMASTER_H_INCLUDED

#include "SPI.h"

enum SPI_STATE {
  SPI_STATE_IDLE,
  SPI_STATE_SEND,
  SPI_STATE_RECV,
  SPI_STATE_READY
};

#define SPI_MAX_BYTES	128

class SPIMaster {
  uint8_t recbuffer[SPI_MAX_BYTES];
  uint8_t buflen;
  uint8_t bufptr;
  SPIClass spi;
public:
  SPIMaster();
  virtual ~SPIMaster();
  void setup();
  bool poll();
  void start_transfer(uint8_t command, int length);
  void stop_transfer();
  int  transfer_ready(uint8_t* data, int length);
private:
  void setDataBits(uint16_t bits);
};

#endif // _SPIMASTER_H_INCLUDED
