/*
  Testing the interface on the WT2003S
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 23rd, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

*/

#include <SoftwareSerial.h>

SoftwareSerial mp3(6, 4); //RX, TX on dev hardware. 6 to ylw, 4 to blu
//SoftwareSerial mp3(1, 0); // RX, TX on Tiny

void setup()
{
  Serial.begin(115200);
  Serial.println("Qwiic MP3 Controller");

  mp3.begin(9600); //WS2003S communicates at 9600bps

  byte systemStatus = stopPlaying(); //On power on, stop any playing MP3
  delay(300);
  int count = getSongCount(); //Pre-load the number of songs available
  Serial.print("Count: ");
  Serial.println(count);
  delay(300);
  setVolume(15); //Go to the volume stored in system settings
  delay(300);
  setEQ(0); //Set to last EQ setting
  delay(300);

  if(systemStatus == 0x00) Serial.println("QMP3 online");
  else
  {
    Serial.print("No MP3 found: 0x");
    Serial.println(systemStatus, HEX);
  }
  
  //byte result = getVolume();
  //Serial.print("Volume: ");
  //Serial.println(result);

  playTrackNumber(1); //Play a given track number. This is a number of listed MP3s in the root directory.
  //playFileName(6); //Play a file # from the root directory. 3 will play F003xxx.mp3 where x can be anything

  delay(50); //Must give IC time to start song before querying song name. 50ms is good.
  char thisSong[9]; //8 characters + terminator
  getSongName(thisSong);
  Serial.print("Song playing: ");
  Serial.println(thisSong);

  //Check volume
  //Check isPlaying
  //songCount
}

void loop()
{
//  if(isPlaying()) Serial.println("Playing song");
//  else Serial.println("Not currently playing");

  delay(1000);

  //if(millis() > 3000) stopPlaying();
}

