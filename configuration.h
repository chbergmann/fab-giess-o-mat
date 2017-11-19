#ifndef CONFIGURATION_H
#define CONFIGURATION_H

const int MAX_NR_CONFIGS = 8;
const int EEPROM_ADDRESS_CONFIG = 0;

struct config_item
{
  char name[16];
  int8_t sensor_pin;
  int8_t pump_pin;
  unsigned char auto_mode;
  uint16_t threashold;
  uint16_t seconds_on;
  uint16_t hours_off;
};

enum {
  AUTO_MODE_HIGHER,
  AUTO_MODE_LOWER,
  AUTO_MODE_TIMER
};

extern struct config_item configuration[MAX_NR_CONFIGS]; 

#endif // CONFIGURATION_H
