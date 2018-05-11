/*
  Controlling the Qwiic MP3 Trigger with I2C Commands
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 23rd, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example demonstrates how to pause or stop an MP3. If a track is playing then pausing
  will pause at that spot. Pressing pause again will continuing playing from that spot. If
  no track is playing and pause is pressed then the last played track will be started from
  the beginning.

  Note: For this example you will need to load an MP3 onto the device and name the file F003.mp3. You can
  use a microB cable to load a file directly onto the SD card.

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
  if(mp3IsPresent() == false)
  {
    Serial.println("Qwiic MP3 failed to respond. Please check wiring and possibly the I2C address. Freezing...");
    while(1);
  }

  mp3ChangeVolume(10); //Volume can be 0 (off) to 31 (max)

  Serial.print("Song count: ");
  Serial.println(mp3SongCount());

  Serial.println("Press S to stop or P to pause.");

  mp3PlayFile(3); //Play file F003.mp3
}

void loop()
{
  while(Serial.available() == false) delay(10); //Wait for user to send a character

  byte incoming = Serial.read();

  switch(incoming)
  {
    case 'S':
    case 's':
      mp3Stop();
      break;
    case 'P':
    case 'p':
      mp3Pause(); //Pause, or play from pause, the current track
      break;
    default:
      Serial.print("Unknown character: ");
      Serial.write(incoming);
      Serial.println();
      break;
  }
}
