/*
  Testing the interface on the WT2003S
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 23rd, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This code is used to simply and immediately play the first track on an SD card. Useful
  when testing production units when you just want the unit under test to play after 
  Uno is reset.
*/

#include <Wire.h>

byte mp3Address = 0x37; //Unshifted 7-bit default address for Qwiic MP3

byte adjustableNumber = 1;

void setup()
{
  Serial.begin(9600);

  Wire.begin();

  //Check to see if MP3 is present
  if(mp3IsPresent() == false)
  {
    Serial.println("Qwiic MP3 failed to respond. Please check wiring and possibly the I2C address. Freezing...");
    while(1);
  }

  if(mp3HasCard() == false)
  {
    Serial.println("SD card missing. Freezing...");
    while(1);
  }

  mp3ChangeVolume(10); //Volume can be 0 (off) to 31 (max)

  //Get the number of songs on the SD card
  byte songCount = mp3SongCount();
  
  Serial.print("Song count: ");
  Serial.println(mp3SongCount());

  Serial.print("Firmware version: ");
  Serial.println(mp3GetVersion());

  mp3PlayTrack(1);
}

void loop()
{
  
}
