#ifndef ENS160_H
#define ENS160_H

#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>

void ENS160_init();
void ENS160_write_temp_in(float temp_in_celcius);
void ENS160_write_rh_in(float relative_humidity);
float ENS160_read_temp_in();
float ENS160_read_rh_in();
uint8_t ENS160_read_AQI();
uint16_t ENS160_read_TVOC();
uint16_t ENS160_read_ECO2();

#endif