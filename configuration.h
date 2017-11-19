#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <avr/io.h>

#define SENSOR_PIN A0
#define PUMP_PIN   13

struct config_item
{
  unsigned char auto_mode;
  uint16_t threashold;
  uint16_t seconds_on;
  uint16_t hours_off;
  uint16_t sensor_cntval;
};

enum {
  AUTO_MODE_HIGHER = 0,
  AUTO_MODE_LOWER,
  AUTO_MODE_TIMER,
  MANUAL_MODE,
};

const int EEPROM_ADDRESS_CONFIG = 0;
extern struct config_item configuration; 

#endif // CONFIGURATION_H
