#include <stdlib.h>
#include <math.h>

#include "I2CFRAM.h"

FRAM_I2C::FRAM_I2C(void) 
{
  _framInitialised = false;
}

boolean FRAM_I2C::begin(uint8_t addr) 
{
  i2c_addr = addr;
  Wire.begin();
  
  uint16_t manufID, prodID;
  getDeviceID(&manufID, &prodID);
  if (manufID != 0x00A)
  {
    return false;
  }
  if (prodID != 0x510)
  {
    return false;
  }

  _framInitialised = true;

  return true;
}

void FRAM_I2C::write8 (uint16_t framAddr, uint8_t value)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(framAddr >> 8);
  Wire.write(framAddr & 0xFF);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t FRAM_I2C::read8 (uint16_t framAddr)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(framAddr >> 8);
  Wire.write(framAddr & 0xFF);
  Wire.endTransmission();

  Wire.requestFrom(i2c_addr, (uint8_t)1);
  
  return Wire.read();
}

void FRAM_I2C::write32(uint16_t framAddr, uint32_t value)
{
  write8(framAddr,value >> 24);
  write8(framAddr+1,value >> 16);
  write8(framAddr+2,value >> 8);
  write8(framAddr+3,value);
}


uint32_t FRAM_I2C::read32 (uint16_t framAddr)
{
  uint32_t value=0;
  value = (read8(framAddr) << 8);
  Serial.print(value,HEX);
  Serial.print(" ");
  value += (read8(framAddr+1)) ;
  Serial.print(value,HEX);
  Serial.print(" ");
  value *=65535;
  value += (read8(framAddr+2) << 8) & 0xFF00;
  Serial.print(value,HEX);
  Serial.print(" ");
  value += read8(framAddr+3) & 0xFF;
  Serial.print(value,HEX);
  Serial.println(" ");
  return value;
}

void FRAM_I2C::getDeviceID(uint16_t *manufacturerID, uint16_t *productID)
{
  uint8_t a[3] = { 0, 0, 0 };
  uint8_t results;
  
  Wire.beginTransmission(MB85RC_SLAVE_ID >> 1);
  Wire.write(i2c_addr << 1);
  results = Wire.endTransmission(false);

  Wire.requestFrom(MB85RC_SLAVE_ID >> 1, 3);
  a[0] = Wire.read();
  a[1] = Wire.read();
  a[2] = Wire.read();

  *manufacturerID = (a[0] << 4) + (a[1]  >> 4);
  *productID = ((a[1] & 0x0F) << 8) + a[2];
}

