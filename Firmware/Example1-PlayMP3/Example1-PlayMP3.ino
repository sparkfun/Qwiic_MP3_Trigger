/*
  Testing the interface on the WT2003S
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 23rd, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example shows how to read the PM2.5 and PM10 readings from the sensor

  I2C Commands to respect:
  
  Stop
  Play file named F003.mp3 - limited to 255
  Play file # by index - limited to 255
  Change volume - limited to 31
  Play next
  Play previous

  Return status of last command:
  0x00 = OK
  0x01 = Fail, command error, no execution
  0x02 = No such file
  0x05 = SD error

  Play file named T09 from triggers.

*/

#include <Wire.h>

byte mp3Address = 0x37; //Unshifted 7-bit default address for Qwiic MP3

//These are the commands we can send
#define COMMAND_STOP 0x00
#define COMMAND_PLAY_TRACK 0x01 //Play a given track number like on a CD: regardless of file names this plays 2nd file in dir.
#define COMMAND_PLAY_FILENUMBER 0x02 //Play a file # from the root directory: 3 will play F003xxx.mp3
#define COMMAND_CHANGE_VOLUME 0x03
#define COMMAND_PLAY_NEXT 0x04
#define COMMAND_PLAY_PREVIOUS 0x05
#define COMMAND_CHANGE_EQ 0x06
#define COMMAND_CHANGE_ADDRESS 0xC7

void setup()
{
  Serial.begin(9600);
  Serial.println("Play track 1");

  Wire.begin();

  //mp3Command(COMMAND_CHANGE_ADDRESS, 20); //Change address to 20
  mp3Address = 20;

  mp3Command(COMMAND_PLAY_TRACK, 1); //Play track
  mp3Command(COMMAND_CHANGE_EQ, 5); //Change equalizer to bass
  
  //mp3Command(COMMAND_CHANGE_VOLUME, 15); //Change volume
  //mp3Command(COMMAND_PLAY_FILENUMBER, 6); //Play file F006xxx.mp3

  //delay(3000);
  mp3Command(COMMAND_PLAY_NEXT);
  //delay(4000);
  mp3Command(COMMAND_PLAY_NEXT);

  /*byte status = mp3Status();
  if(status == 0x00)
  {
    Serial.println("QMP3 OK");
  }
  else if(status == 0x01)
  {
    Serial.println("Command failed");
  }
  else
  {
    Serial.print("Status: ");
    Serial.println(status);
  }*/
}

void loop()
{
}

//Send command to Qwiic MP3
boolean mp3Command(byte command, byte option)
{
  Wire.beginTransmission(mp3Address);
  Wire.write(command);
  Wire.write(option);
  if (Wire.endTransmission() != 0)
    return(false); //Sensor did not ACK
  return(true);
}

//Send command to Qwiic MP3
boolean mp3Command(byte command)
{
//  return(mp3Command(command, 0));
  Wire.beginTransmission(mp3Address);
  Wire.write(command);
  if (Wire.endTransmission() != 0)
    return(false); //Sensor did not ACK
  return(true);
}

//Get the current status of the Qwiic MP3
byte mp3Status()
{
  Wire.requestFrom(mp3Address, 1);

  if (Wire.available())
    return (Wire.read());

  Serial.println("Error: Sensor did not respond");
  return(0);
}


