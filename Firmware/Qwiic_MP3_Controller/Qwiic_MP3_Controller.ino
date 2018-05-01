/*
  Testing the interface on the WT2003S
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 23rd, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example shows how to read the PM2.5 and PM10 readings from the sensor

  I2C Commands to respect:

  Stop
  0x00

  Play file named F003.mp3 - limited to 255
  0x01 + 0x03

  Play file # by index - limited to 255
  0x02 + 0x24

  Change volume - limited to 31
  0x03 + 15

  Play next
  0x04

  Play previous
  0x05

  Change I2C address
  0xC7

  Return status of last command:
  0x00 = OK
  0x01 = Fail, command error, no execution
  0x02 = No such file
  0x05 = SD error

  Play file named T09 from triggers.

  TODO:
  Clock stretches until MP3 responds with result. Maybe 10ms?
  track system Volume to NVM
  add file count and query
  maybe add debounce to trigger function
*/

#include <SoftwareSerial.h>

SoftwareSerial mp3(6, 7); //RX, TX on dev hardware. 6 to ylw, 7 to blu
//SoftwareSerial mp3(1, 0); //RX, TX on Tiny

#include <Wire.h>

#include <EEPROM.h>

#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers

//Location in EEPROM where various settings will be stored
#define LOCATION_I2C_ADDRESS 0x01
#define LOCATION_VOLUME 0x02
#define LOCATION_EQ 0x03

//There is an ADR jumpber on this board. When closed, forces I2C address to a given address.
#define I2C_ADDRESS_DEFAULT 0x37
#define I2C_ADDRESS_JUMPER_CLOSED 0x38

//These are the commands we understand and may respond to
#define COMMAND_STOP 0x00
#define COMMAND_PLAY_TRACK 0x01 //Play a given track number like on a CD: regardless of file names plays 2nd file in dir.
#define COMMAND_PLAY_FILENUMBER 0x02 //Play a file # from the root directory: 3 will play F003xxx.mp3
#define COMMAND_CHANGE_VOLUME 0x03
#define COMMAND_PLAY_NEXT 0x04
#define COMMAND_PLAY_PREVIOUS 0x05
#define COMMAND_CHANGE_EQ 0x06
#define COMMAND_CHANGE_ADDRESS 0xC7

#define SYSTEM_STATUS_OK 0x00
#define SYSTEM_STATUS_FAIL 0x01
#define SYSTEM_STATUS_NO_SUCH_FILE 0x02
#define SYSTEM_STATUS_SD_ERROR 0x05

#define RESPONSE_TYPE_SYSTEM_STATUS 0x00
#define RESPONSE_TYPE_FILE_COUNT 0x01


const byte adr = 9; //Address select jumper is on pin 9
const byte trigger1 = 5; //There are four 'trigger' pins used to trigger the playing of a track
const byte trigger2 = 10;
const byte trigger3 = 8;
const byte trigger4 = 3;

//Variables used in the I2C interrupt so we use volatile
volatile byte systemStatus = SYSTEM_STATUS_OK; //Tracks the response from the MP3 IC
volatile byte responseType = RESPONSE_TYPE_SYSTEM_STATUS; //Tracks how to respond based on incoming requests
volatile byte fileCount = 0;

volatile byte settingAddress = I2C_ADDRESS_DEFAULT; //The 7-bit I2C address of this QMP3
volatile byte settingVolume = 0;
volatile byte settingEQ = 0;

byte oldTriggerNumber = 0; //Tracks trigger state changes

void setup()
{
  Serial.begin(9600);
  Serial.println("Qwiic MP3 Controller");

  pinMode(adr, INPUT_PULLUP);

  pinMode(trigger1, INPUT_PULLUP);
  pinMode(trigger2, INPUT_PULLUP);
  pinMode(trigger3, INPUT_PULLUP);
  pinMode(trigger4, INPUT_PULLUP);

  readSystemSettings(); //Load all system settings from EEPROM

  mp3.begin(9600); //WS2003S communicates at 9600bps

  systemStatus = stopPlaying(); //On power on, stop any playing MP3
  systemStatus = setVolume(settingVolume); //Go to the volume stored in system settings
  systemStatus = setEQ(settingEQ); //Set to last EQ setting

  //Begin listening on I2C only after we've setup all our config and opened any files
  startI2C(); //Determine the I2C address we should be using and begin listening on I2C bus

  //300ms between commands
  Serial.print("QMP3 Address: 0x");
  Serial.println(settingAddress, HEX);

  if (systemStatus == 0x00)
    Serial.println("QMP3 online");
  else
  {
    Serial.print("No MP3 found: 0x");
    Serial.println(systemStatus, HEX);
  }
}

void loop()
{
  //Check trigger pins and act accordingly
  if (digitalRead(trigger1) == LOW ||
      digitalRead(trigger2) == LOW ||
      digitalRead(trigger3) == LOW ||
      digitalRead(trigger4) == LOW)
  {
    byte fileNumber = 0;
    if (digitalRead(trigger1) == LOW) fileNumber += 1;
    if (digitalRead(trigger2) == LOW) fileNumber += 2;
    if (digitalRead(trigger3) == LOW) fileNumber += 3;
    if (digitalRead(trigger4) == LOW) fileNumber += 4;

    //If no song is playing then allow re-triggering by holding down button or trigger
    //This will cause songs to repeat as long as trigger is activated
    if (isPlaying() == false) oldTriggerNumber = 0;

    if (fileNumber != oldTriggerNumber)
    {
      //We have a new trigger!
      if (isPlaying()) stopPlaying(); //Stop any currently playing track
      playTriggerFile(fileNumber); //Ex: 2 will play T002xxx.mp3

      oldTriggerNumber = fileNumber; //Update the state so we don't play this again
    }

  }
  else
  {
    oldTriggerNumber = 0;
  }
}

//When Qwiic MP3 receives data bytes, this function is called as an interrupt
//We act immediately on the incoming command so that QMP3 will stretch the clock
//while it is completing the requested operation
void receiveEvent(int numberOfBytesReceived)
{
  //Record bytes to local array
  byte incoming = Wire.read();

  if (incoming == COMMAND_CHANGE_ADDRESS) //Set new I2C address
  {
    if (Wire.available())
    {
      settingAddress = Wire.read();

      //Error check
      if (settingAddress < 0x08 || settingAddress > 0x77)
        return; //Command failed. This address is out of bounds.

      EEPROM.write(LOCATION_I2C_ADDRESS, settingAddress);

      //Our I2C address may have changed because of user's command
      startI2C(); //Determine the I2C address we should be using and begin listening on I2C bus
    }
  }
  else if (incoming == COMMAND_STOP)
  {
    systemStatus = stopPlaying();
  }
  else if (incoming == COMMAND_PLAY_TRACK)
  {
    Serial.print("T");
    if (Wire.available())
    {
      byte trackNumber = Wire.read();
      systemStatus = playTrackNumber(trackNumber);
    }
  }
  else if (incoming == COMMAND_PLAY_FILENUMBER)
  {
    Serial.print("N");
    if (Wire.available())
    {
      byte fileNumber = Wire.read();
      systemStatus = playFileName(fileNumber); //For example 3 will play F003xxx.mp3
    }
  }
  else if (incoming == COMMAND_CHANGE_VOLUME)
  {
    if (Wire.available())
    {
      settingVolume = Wire.read();

      //Error check
      if (settingVolume > 31)
        return; //Command failed. This volume is out of bounds.

      EEPROM.write(LOCATION_VOLUME, settingVolume);

      systemStatus = setVolume(settingVolume); //Go to this volume
    }
  }
  else if (incoming == COMMAND_PLAY_NEXT)
  {
    systemStatus = playNext();
  }
  else if (incoming == COMMAND_PLAY_PREVIOUS)
  {
    systemStatus = playPrevious();
  }
  else if (incoming == COMMAND_CHANGE_EQ)
  {
    if (Wire.available())
    {
      settingEQ = Wire.read();

      //Error check
      if (settingEQ > 5)
        return; //Command failed. This EQ setting is out of bounds.

      EEPROM.write(LOCATION_EQ, settingEQ);

      systemStatus = setEQ(settingEQ); //Go to this equalizer setting
    }
  }

}

//Send back a number of bytes, max 32 bytes
//When QMP3 gets a request for data from the user, this function is called as an interrupt
//The interrupt will respond with different types of data depending on what response state we are in
//The response type is set during the receiveEvent interrupt
void requestEvent()
{
  if (responseType == RESPONSE_TYPE_FILE_COUNT)
  {
    Wire.write(fileCount);
  }
  else //By default we respond with the last event from the MP3
  {
    Wire.write(systemStatus);
  }
}

//Reads the current system settings from EEPROM
//If anything looks weird, reset setting to default value
void readSystemSettings(void)
{
  //Read what I2C address we should use
  settingAddress = EEPROM.read(LOCATION_I2C_ADDRESS);
  if (settingAddress == 255)
  {
    settingAddress = I2C_ADDRESS_DEFAULT; //By default, we listen for I2C_ADDRESS_DEFAULT
    EEPROM.write(LOCATION_I2C_ADDRESS, settingAddress);
  }

  //Read what volume we should be at
  settingVolume = EEPROM.read(LOCATION_VOLUME);
  if (settingVolume > 31)
  {
    settingVolume = 15; //By default, go to 50% volume
    EEPROM.write(LOCATION_VOLUME, settingVolume);
  }

  //Read what eqaulizer setting we should be at
  settingEQ = EEPROM.read(LOCATION_EQ);
  if (settingEQ > 5)
  {
    settingEQ = 0; //By default, go normal EQ setting
    EEPROM.write(LOCATION_EQ, settingEQ);
  }
}

//Begin listening on I2C bus as I2C slave using the global variable setting_i2c_address
void startI2C()
{
  Wire.end(); //Before we can change addresses we need to stop

  if (digitalRead(adr) == HIGH) //Default is HIGH.
    Wire.begin(settingAddress); //Start I2C and answer calls using address from EEPROM
  else //User has closed jumper with solder to GND
    Wire.begin(I2C_ADDRESS_JUMPER_CLOSED); //Force address to I2C_ADDRESS_NO_JUMPER if user has opened the solder jumper

  //The connections to the interrupts are severed when a Wire.begin occurs. So re-declare them.
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}
