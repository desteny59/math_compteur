#include <Wire.h>
#include "I2CFRAM.h"
#include <SPI.h>
#include <Ethernet.h>
#include <TimerOne.h>


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
uint32_t compteur_time2[8];
uint8_t  PinState[8] = {1,1,1,1,1,1,1,1};

// Parametrage carte ethernet.
// pas d'adressage IP car DHCP.
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // adresse MAC
EthernetServer server(80);  // Port d'écoute du serveur web

// NTPSERVER gestion du temps.
EthernetClient client;
uint32_t unixTime=0;
uint32_t LastNetUpdated=0;
boolean ErrorNetTimeUpdate=false;
#define MaxTimeNoNetUpdated 86400 //temps en seconde avant nouvelle update sur le net.

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
  unixTime = webUnixTime(client);
  LastNetUpdated=unixTime;
  Timer1.attachInterrupt(Update_Time);
  Timer1.initialize(1000000);
  
  Serial.print("timestamp = ");
  Serial.println(unixTime);
  
  }

void loop()
{
  while(1)
  {
    
  if ( Pin_change() ) //test de detection d'impulsion et comptage si lieu
    Compteur_Fram_Update(); // mise à jour de la fram si impulsion detectée.

  XML_Routine();  // gestion du serveur web XML

  if( (unixTime-LastNetUpdated) > MaxTimeNoNetUpdated) // Mise à jour réguliere de l'horloge.
    {
    if(ErrorNetTimeUpdate != true)
      {
      uint32_t unixTimetemp = webUnixTime(client);
      if(ErrorNetTimeUpdate!= true)
        {
        unixTime=unixTimetemp;
        LastNetUpdated=unixTime;
        }
      }
    }
  }
}

ISR (PCINT0_vect)
{

}


void Update_Time()
{
  unixTime++;
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
  uint32_t time=millis();
  for(i=0;i<8;i++)
 
  {
    int pinread=digitalRead(cpt_pin[i]);
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
          compteur_since[i]++;
          if(compteur_time2[i] != 0)
            conso_inst[i]=(float)3600000/(time-compteur_time2[i]);
          
          Serial.print(compteur_time2[i]);
          Serial.print("=");
          Serial.print(time);
           Serial.print("=");
            Serial.print(time-compteur_time2[i]);
          compteur_time2[i]=time;
          change = true;
          Serial.print("   Pulse Detected Compteur");
          Serial.print(i);
          Serial.print("=");
          Serial.print(compteur[i]);
          Serial.print(" conso_int = ");
          Serial.println(conso_inst[i]);
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

unsigned long webUnixTime (Client &client)
{
  unsigned long time = 0;

  // Just choose any reasonably busy web server, the load is really low
  if (client.connect("www.google.fr", 80))
    {
      // Make an HTTP 1.1 request which is missing a Host: header
      // compliant servers are required to answer with an error that includes
      // a Date: header.
      client.print(F("GET / HTTP/1.1 \r\n\r\n"));

      char buf[5];      // temporary buffer for characters
      client.setTimeout(5000);
      if (client.find((char *)"\r\nDate: ") // look for Date: header
    && client.readBytes(buf, 5) == 5) // discard
  {
    unsigned day = client.parseInt();    // day
    client.readBytes(buf, 1);    // discard
    client.readBytes(buf, 3);    // month
    int year = client.parseInt();    // year
    byte hour = client.parseInt();   // hour
    byte minute = client.parseInt(); // minute
    byte second = client.parseInt(); // second

    int daysInPrevMonths;
    switch (buf[0])
      {
      case 'F': daysInPrevMonths =  31; break; // Feb
      case 'S': daysInPrevMonths = 243; break; // Sep
      case 'O': daysInPrevMonths = 273; break; // Oct
      case 'N': daysInPrevMonths = 304; break; // Nov
      case 'D': daysInPrevMonths = 334; break; // Dec
      default:
        if (buf[0] == 'J' && buf[1] == 'a')
    daysInPrevMonths = 0;   // Jan
        else if (buf[0] == 'A' && buf[1] == 'p')
    daysInPrevMonths = 90;    // Apr
        else switch (buf[2])
         {
         case 'r': daysInPrevMonths =  59; break; // Mar
         case 'y': daysInPrevMonths = 120; break; // May
         case 'n': daysInPrevMonths = 151; break; // Jun
         case 'l': daysInPrevMonths = 181; break; // Jul
         default: // add a default label here to avoid compiler warning
         case 'g': daysInPrevMonths = 212; break; // Aug
         }
      }

    // This code will not work after February 2100
    // because it does not account for 2100 not being a leap year and because
    // we use the day variable as accumulator, which would overflow in 2149
    day += (year - 1970) * 365; // days from 1970 to the whole past year
    day += (year - 1969) >> 2;  // plus one day per leap year 
    day += daysInPrevMonths;  // plus days for previous months this year
    if (daysInPrevMonths >= 59  // if we are past February
        && ((year & 3) == 0)) // and this is a leap year
      day += 1;     // add one day
    // Remove today, add hours, minutes and seconds this month
    time = (((day-1ul) * 24 + hour) * 60 + minute) * 60 + second;
  }
    }
  delay(10);
  client.flush();
  client.stop();
  if (time !=0)
  {
    Serial.print("Time Net Updated. -> ");
    Serial.println(time);
  }
  else
  {
    Serial.println("Error in Net Time Update. NTP Disabled Reboot to restart NTP");
    ErrorNetTimeUpdate=true;
  }
  return time;
}

void erase_FRAM()
{
  uint16_t i;
  for(i=0;i<0xFFFF;i++)
  {
    FRAM.write8(i,0);
  }
}


