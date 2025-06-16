#ifndef ATH21_H
#define ATH21_H

#include <Arduino.h>
#include <Wire.h>
#include <stdlib.h>
#include <stdio.h>

void ATH21_trigger();
void ATH21_read(float* tempC, float* rh);

#endif /* ATH21_H */