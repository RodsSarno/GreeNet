#include <stdint.h>
#include "ENS160.h"

#define ENS160_address 0x53

void ENS160_init(){
  Wire.beginTransmission(ENS160_address);
  Wire.write(0x10); //Opmode reg
  Wire.write(0x02); //Gas seeing mode
  Wire.endTransmission();
}

void ENS160_write_temp_in(float temp_in_celcius){
  float tempC = temp_in_celcius;
  uint16_t tempIn = ((tempC + 273.15) * 64);

  Wire.beginTransmission(ENS160_address);
  Wire.write(0x13); //TEMP_IN reg
  Wire.write((tempIn & 0x00FF)); //LSB
  Wire.write((tempIn & 0xFF00) >> 8); //MSB
  Wire.endTransmission();
}

void ENS160_write_rh_in(float relative_humidity){
  float rh = relative_humidity;
  uint16_t rhIn = (rh * 512);

  Wire.beginTransmission(ENS160_address);
  Wire.write(0x15);//RH_IN reg
  Wire.write((rhIn & 0x00FF));
  Wire.write((rhIn & 0xFF00) >> 8);
  Wire.endTransmission();
}

float ENS160_read_temp_in(){
  Wire.beginTransmission(ENS160_address);
  Wire.write(0x30);
  Wire.endTransmission();
  Wire.requestFrom(ENS160_address, 2);
  uint8_t b1 = Wire.read();
  uint8_t b2 = Wire.read();

  uint16_t tc = (b1) | (b2 << 8);
  return(((float)tc/64 - 273.15));
}

float ENS160_read_rh_in(){
  Wire.beginTransmission(ENS160_address);
  Wire.write(0x32);
  Wire.endTransmission();
  Wire.requestFrom(ENS160_address, 2);
  uint8_t b1 = Wire.read();
  uint8_t b2 = Wire.read();

  uint16_t rh = (b1) | (b2 << 8);
  return (((float)rh/512));
}

uint8_t ENS160_read_AQI(){
  Wire.beginTransmission(ENS160_address);
  Wire.write(0x21);
  Wire.endTransmission();
  Wire.requestFrom(ENS160_address, 1);
  return Wire.read();
}

uint16_t ENS160_read_TVOC(){
  Wire.beginTransmission(ENS160_address);
  Wire.write(0x22);
  Wire.endTransmission();
  Wire.requestFrom(ENS160_address, 2);
  uint8_t b1 = Wire.read();
  uint8_t b2 = Wire.read();
  return (b1) | (b2 << 8);
}

uint16_t ENS160_read_ECO2(){
  Wire.beginTransmission(ENS160_address);
  Wire.write(0x24);//DATA_ECO2 reg
  Wire.endTransmission();
  Wire.requestFrom(ENS160_address, 2);
  uint8_t b1 = Wire.read();
  uint8_t b2 = Wire.read();
  return (b1) | (b2 << 8);
}