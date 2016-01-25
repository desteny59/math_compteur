#include <Wire.h>
#include "I2CFRAM.h"
#include <SPI.h>
#include <Ethernet.h>
#include <TimerOne.h>
#include <EnableInterrupt.h>


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
uint16_t    conso_moy[8];
uint32_t conso_moy_temp[8];
uint32_t compteur_time[8];
uint32_t compteur_time2[8];
uint8_t  PinState[8] = {1,1,1,1,1,1,1,1};
uint32_t mincounter=0;
boolean  change;
boolean  changedebug;


// Parametrage carte ethernet.
// pas d'adressage IP car DHCP.
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // adresse MAC
EthernetServer server(80);  // Port d'écoute du serveur web

// NTPSERVER gestion du temps.
//EthernetClient client;
//uint32_t unixTime=0;
//uint32_t LastNetUpdated=0;
//boolean ErrorNetTimeUpdate=false;
//#define MaxTimeNoNetUpdated 86400 //temps en seconde avant nouvelle update sur le net.

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
  //erase_FRAM();
  Compteur_Init();
  //unixTime = webUnixTime(client);
  //LastNetUpdated=unixTime;
  Timer1.attachInterrupt(Update_Time);
  Timer1.initialize(1000000);
  
  //Serial.print("timestamp = ");
  //Serial.println(unixTime);
  
  }

void loop()
{

  while(1)
  {
  if (change)
    {
    Compteur_Fram_Update();
    change=0;
    }
    if (changedebug)
    {
    Serial.println(conso_moy[0]);
    changedebug=0;
    }
  XML_Routine();  // gestion du serveur web XML
  }
}

void calcmoy()
{
  int i;
  for(int i=0;i<8;i++)
  {
    conso_moy[i]=conso_moy_temp[i]*60;
    conso_moy_temp[i]=0;
  }
}


void Update_Time()
{
  mincounter++;
  if(mincounter==60)
  {
    mincounter=0;
    calcmoy();
    changedebug=true;
  }
}

void Compteur_Init()
{
  for(int i=0;i<8;i++)
  {
    compteur[i]=FRAM.read32(i*4);
    conso_moy_temp[i]=0;
  }
  pinMode(cpt0_pin,INPUT_PULLUP);
  pinMode(cpt1_pin,INPUT_PULLUP);
  pinMode(cpt2_pin,INPUT_PULLUP);
  pinMode(cpt3_pin,INPUT_PULLUP);
  pinMode(cpt4_pin,INPUT_PULLUP);
  pinMode(cpt5_pin,INPUT_PULLUP);
  pinMode(cpt6_pin,INPUT_PULLUP);
  pinMode(cpt7_pin,INPUT_PULLUP);
  enableInterrupt(cpt0_pin, compteur0, CHANGE);
  enableInterrupt(cpt1_pin, compteur1, CHANGE);
  enableInterrupt(cpt2_pin, compteur2, CHANGE);
  enableInterrupt(cpt3_pin, compteur3, CHANGE);
  enableInterrupt(cpt4_pin, compteur4, CHANGE);
  enableInterrupt(cpt5_pin, compteur5, CHANGE);
  enableInterrupt(cpt6_pin, compteur6, CHANGE);
  enableInterrupt(cpt7_pin, compteur7, CHANGE);

  
}

void Compteur_Fram_Update()
{
  for(int i=0;i<8;i++)
  {
    FRAM.write32(i*4,compteur[i]);
  }
  
}

void compteur0()
{
  uint32_t time=millis();
  int pinread=digitalRead(cpt0_pin);
  if( pinread != (PinState[0]) )
  {
    if (pinread ==0 )
    { // detection du début de la pulse
      PinState[0] = 0;
      compteur_time[0]=time;
    }
    else
    { // fin de la pulse
      PinState[0] = 1;
      if( (time-compteur_time[0]) > min_pulse_duration) //test anti rebond
      {
        compteur[0]++;
        compteur_since[0]++;
        conso_moy_temp[0]++;
        if(compteur_time2[0] != 0)
            conso_inst[0]=(float)3600000/(time-compteur_time2[0]);
        compteur_time2[0]=time;
        change = true;
      }
    }
  }
}

void compteur1()
{
  uint32_t time=millis();
  int pinread=digitalRead(cpt1_pin);
  if( pinread != (PinState[1]) )
  {
    if (pinread ==0 )
    { // detection du début de la pulse
      PinState[1] = 0;
      compteur_time[1]=time;
    }
    else
    { // fin de la pulse
      PinState[1] = 1;
      if( (time-compteur_time[1]) > min_pulse_duration) //test anti rebond
      {
        compteur[1]++;
        compteur_since[1]++;
        conso_moy_temp[1]++;
        if(compteur_time2[1] != 0)
            conso_inst[1]=(float)3600000/(time-compteur_time2[1]);
        compteur_time2[1]=time;
        change = true;
      }
    }
  }
}

void compteur2()
{
  uint32_t time=millis();
  int pinread=digitalRead(cpt2_pin);
  if( pinread != (PinState[2]) )
  {
    if (pinread ==0 )
    { // detection du début de la pulse
      PinState[2] = 0;
      compteur_time[2]=time;
    }
    else
    { // fin de la pulse
      PinState[2] = 1;
      if( (time-compteur_time[2]) > min_pulse_duration) //test anti rebond
      {
        compteur[2]++;
        compteur_since[2]++;
        conso_moy_temp[2]++;
        if(compteur_time2[2] != 0)
            conso_inst[2]=(float)3600000/(time-compteur_time2[2]);
        compteur_time2[2]=time;
        change = true;
      }
    }
  }
}

void compteur3()
{
  uint32_t time=millis();
  int pinread=digitalRead(cpt3_pin);
  if( pinread != (PinState[3]) )
  {
    if (pinread ==0 )
    { // detection du début de la pulse
      PinState[3] = 0;
      compteur_time[3]=time;
    }
    else
    { // fin de la pulse
      PinState[3] = 1;
      if( (time-compteur_time[3]) > min_pulse_duration) //test anti rebond
      {
        compteur[3]++;
        compteur_since[3]++;
        conso_moy_temp[3]++;
        if(compteur_time2[3] != 0)
            conso_inst[3]=(float)3600000/(time-compteur_time2[3]);
        compteur_time2[3]=time;
        change = true;
      }
    }
  }
}

void compteur4()
{
  uint32_t time=millis();
  int pinread=digitalRead(cpt4_pin);
  if( pinread != (PinState[4]) )
  {
    if (pinread ==0 )
    { // detection du début de la pulse
      PinState[4] = 0;
      compteur_time[4]=time;
    }
    else
    { // fin de la pulse
      PinState[4] = 1;
      if( (time-compteur_time[4]) > min_pulse_duration) //test anti rebond
      {
        compteur[4]++;
        compteur_since[4]++;
        conso_moy_temp[4]++;
        if(compteur_time2[4] != 0)
            conso_inst[4]=(float)3600000/(time-compteur_time2[4]);
        compteur_time2[4]=time;
        change = true;
      }
    }
  }
}

void compteur5()
{
  uint32_t time=millis();
  int pinread=digitalRead(cpt5_pin);
  if( pinread != (PinState[5]) )
  {
    if (pinread ==0 )
    { // detection du début de la pulse
      PinState[5] = 0;
      compteur_time[5]=time;
    }
    else
    { // fin de la pulse
      PinState[5] = 1;
      if( (time-compteur_time[5]) > min_pulse_duration) //test anti rebond
      {
        compteur[5]++;
        compteur_since[5]++;
        conso_moy_temp[5]++;
        if(compteur_time2[5] != 0)
            conso_inst[5]=(float)3600000/(time-compteur_time2[5]);
        compteur_time2[5]=time;
        change = true;
      }
    }
  }
}

void compteur6()
{
  uint32_t time=millis();
  int pinread=digitalRead(cpt6_pin);
  if( pinread != (PinState[6]) )
  {
    if (pinread ==0 )
    { // detection du début de la pulse
      PinState[6] = 0;
      compteur_time[6]=time;
    }
    else
    { // fin de la pulse
      PinState[6] = 1;
      if( (time-compteur_time[6]) > min_pulse_duration) //test anti rebond
      {
        compteur[6]++;
        compteur_since[6]++;
        conso_moy_temp[6]++;
        if(compteur_time2[6] != 0)
            conso_inst[6]=(float)3600000/(time-compteur_time2[6]);
        compteur_time2[6]=time;
        change = true;
      }
    }
  }
}

void compteur7()
{
  uint32_t time=millis();
  int pinread=digitalRead(cpt7_pin);
  if( pinread != (PinState[7]) )
  {
    if (pinread ==0 )
    { // detection du début de la pulse
      PinState[7] = 0;
      compteur_time[7]=time;
    }
    else
    { // fin de la pulse
      PinState[7] = 1;
      if( (time-compteur_time[7]) > min_pulse_duration) //test anti rebond
      {
        compteur[7]++;
        compteur_since[7]++;
        conso_moy_temp[7]++;
        if(compteur_time2[7] != 0)
            conso_inst[7]=(float)3600000/(time-compteur_time2[7]);
        compteur_time2[7]=time;
        change = true;
      }
    }
  }
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
        //Serial.write(c);
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

            client.print("<value>");
            client.print(compteur[i]);
            client.print("</value>");

            client.print("<since>");
            client.print(compteur_since[i]);
            compteur_since[i]=0;
            client.print("</since>");

            client.print("<conso>");
            client.print(conso_inst[i]);
            client.print("</conso>");

            client.print("<consomoy>");
            client.print(conso_moy[i]);
            client.print("</consomoy>");

            client.print("</compteur");
            client.print(i);
            client.println(">");
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

void erase_FRAM()
{
  uint16_t i;
  for(i=0;i<0xFFFF;i++)
  {
    FRAM.write8(i,0);
  }
}


