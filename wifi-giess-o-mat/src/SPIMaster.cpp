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
#include "SPIMaster.h"

#define SPI_SCK_PIN 	14
#define SPI_MISO_PIN	13
#define SPI_MOSI_PIN  	12
#define SPI_SS_PIN 		27
#define ESP32_SPI_BUS	HSPI

#define WAIT_AFTER_EACH_BYTE_MS	5

unsigned long last_millis = 0;

SPIMaster::SPIMaster() {
	bufptr = 0;
	buflen = 0;
	spi = SPIClass(HSPI);
}

SPIMaster::~SPIMaster() {
	stop_transfer();
}

void SPIMaster::stop_transfer() {
	buflen = 0;
}

void SPIMaster::setup() {
	// have to send on master in, *slave out*
	spi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_SS_PIN);
	spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
	digitalWrite(SPI_SS_PIN, HIGH);
}

void SPIMaster::start_transfer(uint8_t command, int recvlength) {
	stop_transfer();
	buflen = recvlength;
	bufptr = 0;
	if (buflen > SPI_MAX_BYTES) {
		Serial.println("SPI_MAX_BYTES too low.");
		return;
	}

	digitalWrite(SPI_SS_PIN, LOW);
	spi.transfer(command);
	digitalWrite(SPI_SS_PIN, HIGH);
	last_millis = millis();
}

bool SPIMaster::poll() {
	if (bufptr == buflen) {
		return true;
	}

	unsigned long now = millis();
	if(now - last_millis >= WAIT_AFTER_EACH_BYTE_MS) {
		last_millis = now;
		digitalWrite(SPI_SS_PIN, LOW);
		recbuffer[bufptr] = spi.transfer(bufptr + 1);
		digitalWrite(SPI_SS_PIN, HIGH);
		bufptr++;
	}
	return false;
}

int SPIMaster::transfer_ready(uint8_t* data, int length) {
	int cpylength = min(length, bufptr);
	if (cpylength > 0)
		memcpy(data, recbuffer, cpylength);

	return cpylength;
}

