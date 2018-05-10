/*
  Testing the interface on the WT2003S
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 23rd, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example shows how to read the PM2.5 and PM10 readings from the sensor

  Available I2C Commands:
  0x00 : Stop
  0x01 then 0x03 : Play file named F003.mp3 - limited to 255
  0x02 then 0x24 : Play file # 36, by index - limited to 255
  0x03 then 15 : Change volume to 15 (half) - limited to 31
  0x04 : Play next
  0x05 : Play previous
  0x06 then 5 : Change Equalizer Setting to bass - 0-normal, 1-pop, 2-rock, 3-jazz, 4-classical, 5-bass
  0x07 : Get available song count within the root folder
  0xC7 : Change I2C address

  Return status of last command:
  0x00 = OK
  0x01 = Fail, command error, no execution
  0x02 = No such file
  0x05 = SD error

  Play file named T09 from triggers.


  Does playNext auto wrap?
*/

#include <Wire.h>

byte mp3Address = 0x37; //Unshifted 7-bit default address for Qwiic MP3

void setup()
{
  Serial.begin(9600);
  Serial.println("Play track 1");

  Wire.begin();

  //Optional configuration of player
  mp3ChangeEQ(0); //Set equalizer to normal
  mp3ChangeVolume(15); //Volume can be 0 (off) to 31 (max)
  
  //mp3Command(COMMAND_CHANGE_ADDRESS, 20); //Change address to 20
  //mp3Address = 20;

  Serial.print("Song count: ");
  Serial.println(mp3SongCount());

  Serial.print("Firmware version: ");
  Serial.println(mp3GetVersion());

  delay(500);
  
  mp3PlayTrack(1); //Play track number 1
  //delay(20);

  delay(3000);
  mp3Stop();
  while(1);

  String songName = mp3SongName();
  Serial.print("Now playing: ");
  Serial.println(songName);
  
  //mp3Command(COMMAND_PLAY_FILENUMBER, 6); //Play file F006xxx.mp3

  delay(3000);
  Serial.println("Playing next");
  mp3PlayNext();
}

void loop()
{
  //Check to see when track is done playing
  if(mp3IsPlaying())
  {
    String songName = mp3SongName();
    Serial.print("Now playing: ");
    Serial.println(songName);
  }
  else Serial.println("No song");
  delay(1000);
}



