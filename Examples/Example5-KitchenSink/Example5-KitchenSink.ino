/*
  Controlling the Qwiic MP3 Trigger with I2C Commands
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 23rd, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example uses a serial menu system to demonstrate all the various functions of the 
  Qwiic MP3 Trigger.

  Use the +/- commands to change the global number and press an option to use that global number.
  For example, hit + three times to increase the number to four, then press '2' to play file four.

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

  Serial.print("Song count: ");
  Serial.println(mp3SongCount());

  Serial.print("Firmware version: ");
  Serial.println(mp3GetVersion());
}

void loop()
{
  Serial.println();
  Serial.println("Qwiic MP3 Menu");
  Serial.println("Press +/- to adjust the number");
  Serial.println("This number is then used for any sub menu, like track # or volume level");
  Serial.print("Number: ");
  Serial.println(adjustableNumber);
  Serial.println();

  if(mp3IsPlaying() == true)
  {
    String songName = mp3SongName();
    Serial.print("Now playing: ");
    Serial.println(songName);
  }
  
  Serial.println("1) Play track");
  Serial.println("2) Play file");
  Serial.println("3) Play next");
  Serial.println("4) Play previous");
  Serial.println("5) Stop");
  Serial.println("6) Set volume");
  Serial.println("7) Set EQ");
  Serial.println("8) Play silence");
  Serial.println("P) Pause/Play from Pause");

  while(Serial.available() == false) delay(10);

  byte incoming = Serial.read();

  switch(incoming)
  {
    case '1':
      mp3PlayTrack(adjustableNumber);
      break;
    case '2':
      mp3PlayFile(adjustableNumber);
      break;
    case '3':
      mp3PlayNext();
      break;
    case '4':
      mp3PlayPrevious();
      break;
    case '5':
      mp3Stop();
      break;
    case '6':
      mp3ChangeVolume(adjustableNumber); //Volume can be 0 (off) to 31 (max)
      break;
    case '7':
      mp3ChangeEQ(adjustableNumber); //EQ is 0-normal, 1-pop, 2-rock, 3-jazz, 4-classical, 5-bass
      break;
    case '8':
      mp3PlayFile(1); //Play F001.mp3, a 1 hour silence track to reduce external speaker buzzing
      break;
    case 'P':
      mp3Pause(); //Pause, or play from pause, the current track
      break;
    case '+':
      adjustableNumber++;
      break;
    case '-':
      adjustableNumber--;
      break;
    default:
      Serial.print("Unknown: ");
      Serial.write(incoming);
      Serial.println();
      break;
  }
}
