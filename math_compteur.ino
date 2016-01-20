#include <Wire.h>
#include "I2CFRAM.h"
#include <SPI.h>
#include <Ethernet.h>

FRAM_I2C FRAM;

byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetServer server(80);

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  if(!FRAM.begin()){
    Serial.print("ERREUR FRAM");
    while(1);
  }
  Ethernet.begin(mac);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());  
  
  }

void loop()
{
     uint32_t data=65500;
  //   uint8_t data=0;
  while(1){
    FRAM.write32(50,data);
    
    Serial.print(FRAM.read8(50),HEX);
    Serial.print(" ");
    Serial.print(FRAM.read8(51),HEX);
    Serial.print(" ");
    Serial.print(FRAM.read8(52),HEX);
    Serial.print(" ");
    Serial.print(FRAM.read8(53),HEX);
    Serial.println(" ");
    Serial.println(FRAM.read32(50));
   // Serial.println(" ");
    data++;

  }
}
