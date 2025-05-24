#include "get_weather.h"

uint8_t get_temperature(char temp_format) {
    // uint8_t temperature = (uint8_t) bme280_readTemperature(1);
    uint8_t temperature = 20;
    if (temp_format == 'F') {
        temperature = temperature * 9 / 5 + 32;
    }
    return temperature;
}

uint16_t get_pressure() {
    // uint8_t pressure = (uint16_t) bme280_readPressure(1);
    uint16_t pressure = 1021;
    return pressure;
}

uint8_t get_humidity() {
    // uint8_t humidity = (uint8_t) bme280_readHumidity(1);
    uint8_t humidity = 40;
    return humidity;
}
