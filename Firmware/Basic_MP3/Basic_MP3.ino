/*
  Testing the interface on the WT2003S
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 23rd, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

*/

#include <SoftwareSerial.h>

SoftwareSerial mp3(6, 7); //RX, TX on dev hardware. 6 to ylw, 7 to blu
//SoftwareSerial mp3(1, 0); // RX, TX on Tiny

void setup()
{
  Serial.begin(9600);
  Serial.println("Qwiic MP3 Controller");

  mp3.begin(9600); //WS2003S communicates at 9600bps

  //byte systemStatus = playTrackNumber(1);
  byte systemStatus = playFileName(6);

  if(systemStatus == 0x00) Serial.println("QMP3 online");
  else Serial.println("No MP3 found: ");
  Serial.println(systemStatus, HEX);
}

void loop()
{

}

