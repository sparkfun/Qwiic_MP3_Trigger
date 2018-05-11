/*
  Controlling the Qwiic MP3 Trigger with I2C Commands
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 23rd, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example shows how to play all the MP3s on the SD card. Press any key in the terminal
  window to play the next song.

  Once mp3PlayNext() has gotten to the end of the songs it will loop to the beginning and continue
  playing.

  Hardware Connections:
  Plug in headphones
  Make sure the SD card is in the socket
  Don't have a USB microB cable connected right now
  If needed, attach a Qwiic Shield to your Arduino/Photon/ESP32 or other
  Plug the Qwiic device onto an available Qwiic port
  Open the serial monitor at 9600 baud
*/

#include <Wire.h>

byte mp3Address = 0x37; //Unshifted 7-bit default address for Qwiic MP3

void setup()
{
  Serial.begin(9600);

  Wire.begin();

  //Check to see if MP3 is present
  if (mp3IsPresent() == false)
  {
    Serial.println("Qwiic MP3 failed to respond. Please check wiring and possibly the I2C address. Freezing...");
    while (1);
  }

  mp3ChangeVolume(10); //Volume can be 0 (off) to 31 (max)

  Serial.print("Song count: ");
  Serial.println(mp3SongCount());

  Serial.println("Press any key to play next song");
}

void loop()
{
  while (Serial.available() == false)
    delay(10); //Do nothing until user sends a character

  while (Serial.available()) Serial.read(); //Throw away any incoming characters

  Serial.println("Playing next song");

  mp3PlayNext();
}
