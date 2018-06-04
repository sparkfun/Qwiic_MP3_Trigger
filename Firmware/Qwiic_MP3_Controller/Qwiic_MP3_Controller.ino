/*
  Qwiic MP3 Trigger
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 23rd, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  An ATtiny84 receives I2C commands from a master and sends/receives commands over serial to the
  WT2003S MP3 player IC. Play, stop, next, previous, and volume control are all accessible over
  I2C. The ATtiny also checks four trigger pins and plays the appropriate track based on which
  trigger pins are pulled low.

  The interrupt pin is active low, open drain. The int pin will go low when a track has completed and
  will remain low until either a track is played or the status of the Qwiic MP3 is queried.

  Available I2C Commands:
  0x00 : Stop
  0x01 then 0x03 : Play file named F003.mp3 - limited to 255
  0x02 then 0x24 : Play file # 36, by index - limited to 255
  0x03 then 15 : Change volume to 15 (half) - limited to 31
  0x04 : Play next
  0x05 : Play previous
  0x06 then 5 : Change Equalizer Setting to bass - 0-normal, 1-pop, 2-rock, 3-jazz, 4-classical, 5-bass
  0x07 then 15 : Set volume to 15. 0(off) to 31(max)
  0x08 : Get available song count on entire SD card
  0x09 : Get song name : Returns an array of 8 bytes
  0x0A : Get play status : 1=playing, 2=stopped, 3=paused
  0x0B : Get card status : Returns true if card is present and mounted
  0x0C : Get firmware version : 2 bytes, upper ver and lower ver
  0xC7 : Change I2C address

  Reading data after any command:
  0x00 = OK
  0x01 = Fail, command error, no execution
  0x02 = No such file
  0x05 = SD error

  There are four trigger pins 1, 2, 3, and 4. Pulling pin 2 low will play
  the track named T002.mp3. Pulling pins 1 and 4 low at the same time will
  play track 5. User must name files T001.mp3 to T010.mp3. These are different
  files from the F004.mp3 files that are played using the 'play file name'
  command over I2C.
*/

#include <SoftwareSerial.h>

//#if defined(__AVR_ATmega328P__)
//SoftwareSerial mp3(6, 4); //RX, TX on dev hardware. 6 to ylw, 4 to blu
//#else
SoftwareSerial mp3(1, 0); //RX, TX on Tiny
//#endif

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
#define I2C_ADDRESS_JUMPER_CLOSED 0x36

//These are the commands we understand and may respond to
#define COMMAND_STOP 0x00
#define COMMAND_PLAY_TRACK 0x01 //Play a given track number like on a CD: regardless of file names plays 2nd file in dir.
#define COMMAND_PLAY_FILENUMBER 0x02 //Play a file # from the root directory: 3 will play F003xxx.mp3
#define COMMAND_PAUSE 0x03 //Will pause if playing, or starting playing if paused
#define COMMAND_PLAY_NEXT 0x04
#define COMMAND_PLAY_PREVIOUS 0x05
#define COMMAND_SET_EQ 0x06
#define COMMAND_SET_VOLUME 0x07
#define COMMAND_GET_SONG_COUNT 0x08 //Note: This causes song to stop playing
#define COMMAND_GET_SONG_NAME 0x09 //Fill global array with 8 characters of the song name
#define COMMAND_GET_PLAY_STATUS 0x0A
#define COMMAND_GET_CARD_STATUS 0x0B
#define COMMAND_GET_VERSION 0x0C
#define COMMAND_SET_ADDRESS 0xC7

#define SYSTEM_STATUS_OK 0x00
#define SYSTEM_STATUS_FAIL 0x01
#define SYSTEM_STATUS_NO_SUCH_FILE 0x02
#define SYSTEM_STATUS_SD_ERROR 0x05

#define RESPONSE_TYPE_SYSTEM_STATUS 0x00
#define RESPONSE_TYPE_SONG_COUNT 0x01
#define RESPONSE_TYPE_SONG_NAME 0x02
#define RESPONSE_TYPE_PLAY_STATUS 0x03
#define RESPONSE_TYPE_CARD_STATUS 0x04
#define RESPONSE_TYPE_FIRMWARE_VERSION 0x05

#define TASK_NONE 0x00
#define TASK_GET_SONG_NAME 0x01
#define TASK_GET_SONG_COUNT 0x02
#define TASK_GET_PLAY_STATUS 0x03

//Firmware version. This is sent when requested. Helpful for tech support.
const byte firmwareVersionMajor = 1;
const byte firmwareVersionMinor = 0;

//Hardware pins
const byte adr = 9; //Address select jumper is on pin 9
const byte trigger1 = 5; //There are four 'trigger' pins used to trigger the playing of a track
const byte trigger2 = 10;
const byte trigger3 = 8;
const byte trigger4 = 3;
const byte interruptOutput = 7;
const byte playing = 2;

//Variables used in the I2C interrupt so we use volatile
volatile byte systemStatus = SYSTEM_STATUS_OK; //Tracks the response from the MP3 IC
volatile byte responseType = RESPONSE_TYPE_SYSTEM_STATUS; //Tracks how to respond based on incoming requests
volatile char songName[9]; //Loaded with the track name upon request
volatile byte mainLoopTask = 0; //Tracks a request for tasks that must be done in the main loop
volatile byte interruptOn = true; //Interrupt turns on when track has completed, turns off when status is read
volatile byte playStatus = 0; //Tracks the play response. 1=play, 2=stop, 3=pause
volatile byte cardStatus = 0; //Tracks if a card is detected or not

volatile byte settingAddress = I2C_ADDRESS_DEFAULT; //The 7-bit I2C address of this QMP3
volatile byte settingVolume = 0;
volatile byte settingEQ = 0;

byte songCount = 0; //Tracks how many songs are available to play
byte oldTriggerNumber = 0; //Tracks trigger state changes
unsigned long lastCheck = 0; //Tracks the last time we checked if song is playing

void setup()
{
  pinMode(adr, INPUT_PULLUP);
  pinMode(playing, INPUT_PULLUP);

  pinMode(trigger1, INPUT_PULLUP);
  pinMode(trigger2, INPUT_PULLUP);
  pinMode(trigger3, INPUT_PULLUP);
  pinMode(trigger4, INPUT_PULLUP);

  pinMode(interruptOutput, INPUT); //Go to high impedance, indicating no interrupt

  readSystemSettings(); //Load all system settings from EEPROM

  mp3.begin(9600); //WS2003S communicates at 9600bps

  //Verify connection to MP3 IC. But give up after 1000ms
  byte counter = 0;
  while (counter++ < 100)
  {
    clearBuffer();
    systemStatus = stopPlaying(); //On power on, stop any playing MP3
    if (systemStatus == 0x00) break; //IC responded OK
    noIntDelay(10);
  }

  setVolume(settingVolume); //Go to the volume stored in system settings. This can take >150ms
  systemStatus = setEQ(settingEQ); //Set to last EQ setting

#if defined(__AVR_ATmega328P__)
  /*  Serial.begin(115200);
    Serial.println("Qwiic MP3 Controller");
    Serial.print("QMP3 Address: 0x");
    Serial.println(settingAddress, HEX);

    if (systemStatus == 0x00)
      Serial.println("QMP3 online");
    else
    {
      Serial.print("No MP3 found: 0x");
      Serial.println(systemStatus, HEX);
    }*/
#endif

  //Begin listening on I2C only after we've setup all our config and opened any files
  startI2C(); //Determine the I2C address we should be using and begin listening on I2C bus
}

void loop()
{
  //We can't do serial receive during the I2C interrupt so we do anything that
  //requires a response to a command
  if (mainLoopTask == TASK_GET_SONG_NAME)
  {
    getSongName(); //Load current song name into global array
    mainLoopTask = TASK_NONE; //Reset task to nothing
    responseType = RESPONSE_TYPE_SONG_NAME; //Change response type
  }
  else if (mainLoopTask == TASK_GET_SONG_COUNT)
  {
    songCount = getSongCount();
    mainLoopTask = TASK_NONE; //Reset task to nothing
    responseType = RESPONSE_TYPE_SONG_COUNT; //Change response type
  }

  //Check trigger pins and act accordingly
  if (digitalRead(trigger1) == LOW ||
      digitalRead(trigger2) == LOW ||
      digitalRead(trigger3) == LOW ||
      digitalRead(trigger4) == LOW)
  {
    delay(100); // Debounce/Wait a bit in case user is pulling other pins low

    byte fileNumber = 0;
    if (digitalRead(trigger1) == LOW) fileNumber += 1;
    if (digitalRead(trigger2) == LOW) fileNumber += 2;
    if (digitalRead(trigger3) == LOW) fileNumber += 3;
    if (digitalRead(trigger4) == LOW) fileNumber += 4;

    //Is one of the triggers still activated? More debouncing.
    if (fileNumber > 0)
    {
      //If no song is playing then allow re-triggering by holding down button or trigger
      //This will cause songs to repeat as long as trigger is activated
      if (isPlaying() == false) oldTriggerNumber = 0;

      if (fileNumber != oldTriggerNumber)
      {
        //We have a new trigger!
        if (isPlaying()) stopPlaying(); //Stop any currently playing track
        byte result = playTriggerFile(fileNumber); //Ex: 2 will play T002xxx.mp3

#if defined(__AVR_ATmega328P__)
        //Serial.print("result: 0x");
        //Serial.println(result, HEX);
#endif

        oldTriggerNumber = fileNumber; //Update the state so we don't play this again
      }//End new trigger number
    }//End fileNumber > 0
  } //End trigger is low
  else
  {
    oldTriggerNumber = 0;
  }

  //Every 100ms check to see if we're done playing the song
  //If so, set the interrupt pin low
  if (interruptOn == false)
  {
    if (millis() - lastCheck > 100)
    {
      lastCheck = millis();

      if (isPlaying() == false)
      {
        interruptOn = true;
        pinMode(interruptOutput, OUTPUT);
        digitalWrite(interruptOutput, LOW);
      }
    }
  }
}

//When Qwiic MP3 receives data bytes, this function is called as an interrupt
//We act immediately on the incoming command so that QMP3 will stretch the clock
//while it is completing the requested operation
void receiveEvent(int numberOfBytesReceived)
{
  //Record bytes to local array
  byte incoming = Wire.read();

  if (incoming == COMMAND_SET_ADDRESS) //Set new I2C address
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
    if (Wire.available())
    {
      byte trackNumber = Wire.read();
      systemStatus = playTrackNumber(trackNumber);

      interruptOn = false; //Once a track has started playing, interrupt should turn off
      pinMode(interruptOutput, INPUT); //Go to high impedance
    }
  }
  else if (incoming == COMMAND_PLAY_FILENUMBER)
  {
    if (Wire.available())
    {
      byte fileNumber = Wire.read();
      systemStatus = playFileName(fileNumber); //For example 3 will play F003xxx.mp3

      interruptOn = false; //Once a track has started playing, interrupt should turn off
      pinMode(interruptOutput, INPUT); //Go to high impedance
    }
  }
  else if (incoming == COMMAND_SET_VOLUME)
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
  else if (incoming == COMMAND_PAUSE)
  {
    systemStatus = pause();
  }
  else if (incoming == COMMAND_PLAY_NEXT)
  {
    systemStatus = playNext();

    interruptOn = false; //Once a track has started playing, interrupt should turn off
    pinMode(interruptOutput, INPUT); //Go to high impedance
  }
  else if (incoming == COMMAND_PLAY_PREVIOUS)
  {
    systemStatus = playPrevious();

    interruptOn = false; //Once a track has started playing, interrupt should turn off
    pinMode(interruptOutput, INPUT); //Go to high impedance
  }
  else if (incoming == COMMAND_SET_EQ)
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
  else if (incoming == COMMAND_GET_SONG_COUNT)
  {
    //Count is loaded at setup()
    //responseType = RESPONSE_TYPE_FILE_COUNT; //Change response type
    mainLoopTask = TASK_GET_SONG_COUNT; //The count is requested in the main loop
  }
  else if (incoming == COMMAND_GET_SONG_NAME)
  {
    mainLoopTask = TASK_GET_SONG_NAME; //The name is requested in the main loop
  }
  else if (incoming == COMMAND_GET_PLAY_STATUS)
  {
    playStatus = 0x02; //Stopped
    if (digitalRead(playing) == HIGH) //Song is playing
      playStatus = 0x01;

    responseType = RESPONSE_TYPE_PLAY_STATUS; //Change response type
  }
  else if (incoming == COMMAND_GET_CARD_STATUS)
  {
    cardStatus = 0x01; //Card is good
    if (setEQ(settingEQ) == 0x05) //0x05 is the no card or corrupt card error
      cardStatus = 0x00; //No card

    responseType = RESPONSE_TYPE_CARD_STATUS; //Change response type
  }
  else if (incoming == COMMAND_GET_VERSION)
  {
    responseType = RESPONSE_TYPE_FIRMWARE_VERSION;
  }

}

//Send back a number of bytes, max 32 bytes
//When QMP3 gets a request for data from the user, this function is called as an interrupt
//The interrupt will respond with different types of data depending on what response state we are in
//The response type is set during the receiveEvent interrupt
void requestEvent()
{
  if (responseType == RESPONSE_TYPE_SONG_COUNT)
  {
    Wire.write(songCount);
  }
  else if (responseType == RESPONSE_TYPE_SONG_NAME)
  {
    Wire.write((char *)songName, 8);
  }
  else if (responseType == RESPONSE_TYPE_PLAY_STATUS)
  {
    Wire.write(playStatus);
  }
  else if (responseType == RESPONSE_TYPE_CARD_STATUS)
  {
    Wire.write(cardStatus);
  }
  else if (responseType == RESPONSE_TYPE_FIRMWARE_VERSION)
  {
    Wire.write(firmwareVersionMajor);
    Wire.write(firmwareVersionMinor);
  }
  else //By default we respond with the result from the last operation
  {
    Wire.write(systemStatus);
  }

  responseType = RESPONSE_TYPE_SYSTEM_STATUS; //Reset response type

  //Clear the interrupt pin once user has requested something
  interruptOn = false;
  pinMode(interruptOutput, INPUT); //Go to high impedance
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
