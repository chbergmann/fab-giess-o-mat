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
#include "SPI.h"
#include "SPIMaster.h"

#define SPI_SS_PIN  16

SPIMaster::SPIMaster() {
  state = SPI_STATE_IDLE;
  buffer = 0;
  buflen = 0;
  bufptr = 0;
}

SPIMaster::~SPIMaster() {
  stop_transfer();
}

void SPIMaster::stop_transfer() {
  if(buffer)
    delete buffer;

  buffer = 0;
  buflen = 0;
  state = SPI_STATE_IDLE;
  digitalWrite(SPI_SS_PIN, HIGH);
}

void SPIMaster::setup() {
  // have to send on master in, *slave out*
  pinMode (MOSI, OUTPUT);
  pinMode (SPI_SS_PIN, OUTPUT);
  state = SPI_STATE_IDLE;
  SPI.begin();
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
}

void SPIMaster::start_transfer(uint8_t* data, int length) {
  start_transfer(data, length, length);
}

void SPIMaster::start_transfer(int recvlength) {
  start_transfer(0, 0, recvlength);
}

void SPIMaster::start_transfer(byte* data, int sendlength, int recvlength) {
  stop_transfer();
  buflen = max(sendlength, recvlength);
  buffer = new byte[buflen];
  bufptr = 0;
  if(data)
    memcpy(buffer, data, sendlength);

  memset(buffer + sendlength, 0, recvlength);
  state = SPI_STATE_SEND;
  digitalWrite(SPI_SS_PIN, LOW);
  setDataBits(8);
  poll();
}

void SPIMaster::poll() {
  if((SPI1CMD & SPIBUSY) == 0) {
    if(state == SPI_STATE_SEND) {
      if(bufptr == buflen) {
        digitalWrite(SPI_SS_PIN, HIGH);
        state = SPI_STATE_READY;
      }
      else {
        SPI1W0 = buffer[bufptr];
        SPI1CMD |= SPIBUSY;
        state = SPI_STATE_RECV;
      }
    }
    else if(state == SPI_STATE_RECV) {
      buffer[bufptr] = SPI1W0 & 0xff;
      bufptr++;
      state = SPI_STATE_SEND;
    }
  }
}

bool SPIMaster::ready() {
  return state == SPI_STATE_IDLE || state == SPI_STATE_READY;
}

int SPIMaster::transfer_ready(uint8_t* data, int length) {
  if(state != SPI_STATE_READY)
    return 0;

  int cpylength = min(length, bufptr);
  if(cpylength > 0)
    memcpy(data, buffer, cpylength);

  return cpylength;
}

void SPIMaster::setDataBits(uint16_t bits) {
    const uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));
    bits--;
    SPI1U1 = ((SPI1U1 & mask) | ((bits << SPILMOSI) | (bits << SPILMISO)));
}
