#ifndef _FRAM_I2C_H_
#define _FRAM_I2C_H_

#if ARDUINO >= 100
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif

#include <Wire.h>

#define MB85RC_DEFAULT_ADDRESS        (0x50) /* 1010 + A2 + A1 + A0 = 0x50 default */
#define MB85RC_SLAVE_ID       (0xF8)

class FRAM_I2C {
 public:
  FRAM_I2C(void);
  
  boolean  begin(uint8_t addr = MB85RC_DEFAULT_ADDRESS);
  void     write8 (uint16_t framAddr, uint8_t value);
  uint8_t  read8  (uint16_t framAddr);
  void     write32(uint16_t framAddr, uint32_t value);
  uint32_t read32 (uint16_t framAddr);
  void     getDeviceID(uint16_t *manufacturerID, uint16_t *productID);

 private:
  uint8_t i2c_addr;
  boolean _framInitialised;
};

#endif

