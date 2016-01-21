#include <Wire.h>
#include "I2CFRAM.h"
#include <SPI.h>
#include <Ethernet.h>

FRAM_I2C FRAM;

#define cpt0_pin 2
#define cpt1_pin 3
#define cpt2_pin 4
#define cpt3_pin 5
#define cpt4_pin 6
#define cpt5_pin 7
#define cpt6_pin 8
#define cpt7_pin 9

#define min_pulse_duration 20 // temps minimum de la durée d'une pulse en ms (pour eviter les rebonds)


// variable etat compteur
uint8_t  cpt_pin[8] ={cpt0_pin,cpt1_pin,cpt2_pin,cpt3_pin,cpt4_pin,cpt5_pin,cpt6_pin,cpt7_pin};
uint32_t compteur[8];
uint32_t compteur_since[8];
float    conso_inst[8];
uint32_t compteur_time[8];
uint8_t  PinState[8] = {1,1,1,1,1,1,1,1};

// Parametrage carte ethernet.
// pas d'adressage IP car DHCP.
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // adresse MAC
EthernetServer server(80);  // Port d'écoute du serveur web

void setup()
{
  Wire.begin();
  Serial.begin(230400);
  if(!FRAM.begin()){
    Serial.print("ERREUR FRAM");
    while(1);
  }
  Ethernet.begin(mac);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  Compteur_Init();  
  
  }

void loop()
{
  while(1)
  {
    
  if ( Pin_change() ) //test de detection d'impulsion et comptage si lieu
    Compteur_Fram_Update(); // mise à jour de la fram si impulsion detectée.

  XML_Routine();  
 

  }
}

void Compteur_Init()
{
  for(int i=0;i<8;i++)
  {
    compteur[i]=FRAM.read32(i*4);
  }
  pinMode(cpt0_pin,INPUT_PULLUP);
  pinMode(cpt1_pin,INPUT_PULLUP);
  pinMode(cpt2_pin,INPUT_PULLUP);
  pinMode(cpt3_pin,INPUT_PULLUP);
  pinMode(cpt4_pin,INPUT_PULLUP);
  pinMode(cpt5_pin,INPUT_PULLUP);
  pinMode(cpt6_pin,INPUT_PULLUP);
  pinMode(cpt7_pin,INPUT_PULLUP);  
}

void Compteur_Fram_Update()
{
  for(int i=0;i<8;i++)
  {
    FRAM.write32(i*4,compteur[i]);
  }
  
}

boolean Pin_change()
{
  int i;
  boolean change = false;
  for(i=0;i<8;i++)
 
  {
    int pinread=digitalRead(i);
    if( pinread != (PinState[i]) )
    {
      if( pinread == 0 ) 
      { //detection de pulse
        PinState[i] = 0;
        compteur_time[i] = millis();
      }
      else
      { //fin de la pulse
        PinState[i] = 1;
        if( (millis()-compteur_time[i])> min_pulse_duration) //test si pas rebond
        {
          compteur[i]++;
          change = true;
        }
      }
    }  
  }
  return change;  
}

void XML_Routine()
{
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/xml");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<?xml version = \"1.0\" ?>");
          client.println("<compteur>");
          // output the value of each analog input pin
          for (int i = 0; i < 8; i++) {
            client.print("<compteur");
            client.print(i);
            client.print(">");
            client.print(compteur[i]);
            client.print("</compteur");
            client.print(i);
            client.print(">");
          }
          client.println("</compteur>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
    Ethernet.maintain();
  }
  
}

