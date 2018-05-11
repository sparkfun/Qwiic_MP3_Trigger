/*
  Controlling the Qwiic MP3 Trigger with I2C Commands
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 23rd, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example shows how to change the I2C address of the Qwiic MP3 Trigger. 

  Note: If you accidentally set the device into an unknown you can either use the I2C scanner sketch found online
  or you can close the solder jumper on the Qwiic MP3 Trigger. Closing the jumper will force the device address to 0x36.

  Hardware Connections:
  Plug in headphones
  Make sure the SD card is in the socket
  Don't have a USB microB cable connected right now
  If needed, attach a Qwiic Shield to your Arduino/Photon/ESP32 or other
  Plug the Qwiic device onto an available Qwiic port
  Open the serial monitor at 9600 baud
*/

#include <Wire.h>

byte originalMP3Address = 0x37; //This is the address that the device is currently using
byte newMP3Address = 0x77; //This is the address you want to change the device to

byte mp3Address = originalMP3Address; //This is the variable used in the mp3Control functions

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

  Serial.print("Song count: ");
  Serial.println(mp3SongCount());

  Serial.print("Press a key to change the address to 0x");
  Serial.println(newMP3Address, HEX);

  while(Serial.available() == false)
    delay(100); //Wait for user to send character

  mp3ChangeAddress(newMP3Address); 
  mp3Address = newMP3Address; //We must update our local global variable to this new address so that we can continue to communicate

  Serial.print("Device address should now be changed to 0x");
  Serial.println(newMP3Address, HEX);

  mp3PlayFile(3); //Play file F003.mp3

  Serial.println("The Qwiic MP3 Trigger should be playing a song but from the new I2C address");
}

void loop()
{

}
