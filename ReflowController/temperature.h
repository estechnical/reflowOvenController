

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <Arduino.h>

typedef struct tcInput {
  double temperature;
  int stat;
  int chipSelect;

};

float mapfloat(float x, long in_min, long in_max, long out_min, long out_max)
{
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

char spi_transfer(volatile char data)
{
  SPDR = data; // Start the transmission
  while (!(SPSR & (1<<SPIF))) // Wait the end of the transmission
  {
  };
  return SPDR; // return the received byte
}

void readThermocouple(struct tcInput* input){
  digitalWrite(input->chipSelect, LOW);

  uint32_t result = 0x0000;
  byte reply = 0;

  char data = 0; // dummy data to write
  
  for(int i = 0; i<4;i++){ // read the 32 data bits from the MAX31855
    reply = spi_transfer(data);
    result = result << 8;
    result |= reply;
  }
  //Serial.print("Read result is: ");
  //Serial.print(result, BIN);
  
  result >>= 18;
  
  uint16_t value = 0xFFF & result; // mask off the sign bit and shit to the correct alignment for the temp data
  
  input->stat = reply & B111;
  
  float temp = value * 0.25;
  
  // Due to Maxim's mis-trimmed MAX31855KASA+ ICs, controllers with 
  // date code 1352A2 253AL or 1352A2 253AK ICs need this correction.  
  input->temperature = mapfloat(temp, 7, 79, 0, 100); 
  
  digitalWrite(input->chipSelect, HIGH);
  
}



#endif

