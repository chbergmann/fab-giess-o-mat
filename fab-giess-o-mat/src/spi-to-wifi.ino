#include <SPI.h>

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

// SPI interrupt routine
ISR (SPI_STC_vect)
{
  byte c = SPDR;

  if(c == 0) {
    sensorval = get_sensorvalue();
    SPDR = (byte)(sensorval >> 8);
  }
  else {
    SPDR = (byte)sensorval;
  }
}  

