#ifndef GET_WEATHER_H
#define GET_WEATHER_H

#include "bme280.h"

uint8_t get_temperature(char temp_format);
uint16_t get_pressure();
uint8_t get_humidity();

#endif // GET_WEATHER_H