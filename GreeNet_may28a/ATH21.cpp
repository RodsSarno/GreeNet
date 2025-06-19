#include "ATH21.h"

#define ATH21_address 0x38

void ATH21_trigger(){
  //trigger
  Wire.beginTransmission(ATH21_address);
  Wire.write(0xAC);
  Wire.write(0x33);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(85);
}

void ATH21_read(float* tempC, float* rh){
  // Data aquisition
  ATH21_trigger();
  Wire.requestFrom(ATH21_address,7);
  
  uint8_t data[7];
  for(int i = 0; i < 7; i++) {
    data[i] = Wire.read();
  }

  // Verificação básica de CRC (bit mais significativo)
  if((data[0] & 0x80) || (data[3] & 0x80)) {
    *tempC = NAN;
    *rh = NAN;
    return;
  }

  uint32_t raw_hum = ((data[1] << 12) | (data[2] << 4) | (data[3] >> 4));
  uint32_t raw_temp = (((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5]);

  *rh = ((float)raw_hum / 1048576.0) * 100.0;
  *tempC = ((float)raw_temp / 1048576.0) * 200.0 - 50.0;
}