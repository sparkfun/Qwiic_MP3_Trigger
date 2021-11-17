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
  0x0D : Clear interrupts
  0x0E : Get Volume
  0x0F : Get EQ
  0x10 : Get ID
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

  Change long:
  v1.1 - Added clear interrupt command. Created Interrupt State Machine.
  v1.2 - Supplier delivered WT2003S ICs that seem to be incorrectly programmed.
         The device re-plays the last played tracked in an endless loop. To
         circumvent the problem we will mute volume once track has played once.
         This IC error also prevents the pause and stop functions from working.
         Stop function will be replaced with mute. Pause/play cannot be fixed.
         IsPlaying command will be replaced with internal state rather than checking
         what the IC reports (always playing).
*/

#include <SoftwareSerial.h>

#if defined(__AVR_ATmega328P__)
SoftwareSerial mp3(3, 2); //RX, TX on dev hardware
#else
SoftwareSerial mp3(1, 0); //RX, TX on production hardware
#endif

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

#define DEVICE_ID 0x39 //This is a hard coded number we test against to verify the device type.

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
#define COMMAND_CLEAR_INTERRUPTS 0x0D
#define COMMAND_GET_VOLUME 0x0E
#define COMMAND_GET_EQ 0x0F
#define COMMAND_GET_ID 0x10
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
#define RESPONSE_TYPE_GET_VOLUME 0x06
#define RESPONSE_TYPE_GET_EQ 0x07
#define RESPONSE_TYPE_GET_ID 0x08

#define TASK_NONE 0x00
#define TASK_GET_SONG_NAME 0x01
#define TASK_GET_SONG_COUNT 0x02
#define TASK_GET_PLAY_STATUS 0x03

//Firmware version. This is sent when requested. Helpful for tech support.
const byte firmwareVersionMajor = 1;
const byte firmwareVersionMinor = 2;

//Hardware pins
#if defined(__AVR_ATmega328P__)
//Test hardware
const byte adr = 9; //Address select jumper is on pin 9
const byte trigger1 = 6; //There are four 'trigger' pins used to trigger the playing of a track
const byte trigger2 = 10;
const byte trigger3 = 8;
const byte trigger4 = 11;
const byte interruptOutput = 7;
const byte playing = 5;
#else
//Production hardware
const byte adr = 9; //Address select jumper is on pin 9
const byte trigger1 = 5; //There are four 'trigger' pins used to trigger the playing of a track
const byte trigger2 = 10;
const byte trigger3 = 8;
const byte trigger4 = 3;
const byte interruptOutput = 7;
const byte playing = 2;
#endif
//Variables used in the I2C interrupt so we use volatile
volatile byte systemStatus = SYSTEM_STATUS_OK; //Tracks the response from the MP3 IC

//The MP3 IC can take up to 250ms to respond to a command. We don't want to clock stretch
//after every command so instead, we use a que of commands.
const byte MAX_QUE_SIZE = 10;
volatile byte commandQue[MAX_QUE_SIZE][2];
volatile byte queHead = 0;
volatile byte queTail = 0;

volatile byte responseType = RESPONSE_TYPE_SYSTEM_STATUS; //Tracks how to respond based on incoming requests
volatile char songName[9]; //Loaded with the track name upon request
volatile byte playStatus = 0; //Tracks the play response. 1=play, 2=stop
volatile byte cardStatus = 0; //Tracks if a card is detected or not

volatile byte settingAddress = I2C_ADDRESS_DEFAULT; //The 7-bit I2C address of this QMP3
volatile byte settingVolume = 0;
volatile byte settingEQ = 0;

//Interrupt turns on when track has completed,
//turns off when interrupts are cleared by command
//And goes to No Int if a song has started playing
enum State {
  STATE_INT = 0,
  STATE_INT_CLEARED,
  STATE_NO_INT
};
volatile byte interruptState = STATE_NO_INT;

byte songCount = 0; //Tracks how many songs are available to play
byte oldTriggerNumber = 0; //Tracks trigger state changes
unsigned long lastCheck = 0; //Tracks the last time we checked if song is playing
bool trackPlaying = false; //Workaround for loop issue: Goes true when play starts, goes false as soon as playing pin goes low

void setup()
{
#if defined(__AVR_ATmega328P__)
  Serial.begin(115200);
  Serial.println("Qwiic MP3 Controller");
#endif

  pinMode(adr, INPUT_PULLUP);
  pinMode(playing, INPUT_PULLUP);

  pinMode(trigger1, INPUT_PULLUP);
  pinMode(trigger2, INPUT_PULLUP);
  pinMode(trigger3, INPUT_PULLUP);
  pinMode(trigger4, INPUT_PULLUP);

  //This will set the int pin to high impedance (aka pulled high by external resistor)
  digitalWrite(interruptOutput, LOW); //Push pin to disable internal pull-ups
  pinMode(interruptOutput, INPUT); //Go to high impedance

  readSystemSettings(); //Load all system settings from EEPROM

  mp3.begin(9600); //WS2003S communicates at 9600bps

  //The MP3 IC takes ~1520ms to boot, and won't respond to commands during this time
  //Verify connection to MP3 IC. But give up after 2500ms
  byte counter = 0;
  while (counter++ < 10)
  {
    clearBuffer(); //Read any stray serial bytes being sent from MP3 ICs
    systemStatus = stopPlaying(); //On power on, stop any playing MP3. Times out after 250ms.
    if (systemStatus == 0x00) break; //IC responded OK
  }

  setMP3Volume(0); //Mute volume
  //setMP3Volume(settingVolume); //Go to the volume stored in system settings. This can take >150ms
  systemStatus = setEQ(settingEQ); //Set to last EQ setting
  songCount = getSongCount(); //Preload this before we begin

#if defined(__AVR_ATmega328P__)
  Serial.print("QMP3 Address: 0x");
  Serial.println(settingAddress, HEX);

  if (systemStatus == 0x00)
    Serial.println("QMP3 online");
  else
  {
    Serial.print("No MP3 found: 0x");
    Serial.println(systemStatus, HEX);
  }
#endif

  //Begin listening on I2C only after we've setup all our config and opened any files
  startI2C(); //Determine the I2C address we should be using and begin listening on I2C bus
}

void loop()
{
  //Mute volume once a song has completed
  if (isPlaying() == false && trackPlaying == true)
  {
    trackPlaying = false; //Update internal state
    setMP3Volume(0); //Mute because track will auto-loop
  }

  //Check trigger pins and act accordingly
  if (digitalRead(trigger1) == LOW ||
      digitalRead(trigger2) == LOW ||
      digitalRead(trigger3) == LOW ||
      digitalRead(trigger4) == LOW)
  {
    delay(100); //Debounce/Wait a bit in case user is pulling other pins low

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
        Serial.print("result: 0x");
        Serial.println(result, HEX);
#endif

        oldTriggerNumber = fileNumber; //Update the state so we don't play this again
      }//End new trigger number
    }//End fileNumber > 0
  } //End trigger is low
  else
  {
    oldTriggerNumber = 0;
  }


  //Interrupt pin state machine
  //There are three state: No int, Int, and Int Cleared
  //The third state is set in the I2C interrupt when Clear Ints command is received.
  if (interruptState == STATE_NO_INT)
  {
    //Every 100ms check to see if we're done playing the song
    if (millis() - lastCheck > 100)
    {
      lastCheck = millis();

      if (isPlaying() == false)
      {
        //If so, set the interrupt pin low to indicate interrupt
        pinMode(interruptOutput, OUTPUT);
        digitalWrite(interruptOutput, LOW);

        interruptState = STATE_INT; //Go to next state
      }
    }
  }
  else if (interruptState == STATE_INT_CLEARED)
  {
    if (isPlaying() == true)
    {
      interruptState = STATE_NO_INT; //Go to next state
    }
  }

  //Process any commands sitting in the que
  if (queTail != queHead)
  {
    queTail++;
    if (queTail == MAX_QUE_SIZE) queTail = 0;

    if (commandQue[queTail][0] == COMMAND_STOP)
      systemStatus = stopPlaying();
    else if (commandQue[queTail][0] == COMMAND_PLAY_TRACK)
      systemStatus = playTrackNumber(commandQue[queTail][1]);
    else if (commandQue[queTail][0] == COMMAND_PLAY_FILENUMBER)
      systemStatus = playFileName(commandQue[queTail][1]);
    else if (commandQue[queTail][0] == COMMAND_SET_VOLUME)
      systemStatus = setInternalVolume(commandQue[queTail][1]);
    else if (commandQue[queTail][0] == COMMAND_PAUSE)
      systemStatus = pause();
    else if (commandQue[queTail][0] == COMMAND_PLAY_NEXT)
      systemStatus = playNext();
    else if (commandQue[queTail][0] == COMMAND_PLAY_PREVIOUS)
      systemStatus = playPrevious();
    else if (commandQue[queTail][0] == COMMAND_SET_EQ)
      systemStatus = setEQ(settingEQ);
    else if (commandQue[queTail][0] == COMMAND_GET_SONG_NAME)
    {
      getSongName(); //Load current song name into global array
      responseType = RESPONSE_TYPE_SONG_NAME;
    }
    else if (commandQue[queTail][0] == COMMAND_GET_CARD_STATUS)
    {
      cardStatus = 0x01; //Card is good
      if (setEQ(settingEQ) == 0x05) //0x05 is the no card or corrupt card error
        cardStatus = 0x00; //No card

      responseType = RESPONSE_TYPE_CARD_STATUS; //Change response type
    }
  }
}

//Begin listening on I2C bus as I2C slave using the global variable settingAddress
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

//Software delay. Does not rely on internal timers.
void noIntDelay(byte amount)
{
  for (volatile byte y = 0 ; y < amount ; y++)
  {
#if defined(__AVR_ATmega328P__)
    for (volatile unsigned int x = 0 ; x < 3000 ; x++) //1ms at 16MHz. Validated with analyzer
#else
    //ATtiny84 at 8MHz
    for (volatile unsigned int x = 0 ; x < 1500 ; x++) //1ms at 8MHz
#endif
    {
      __asm__("nop\n\t");
    }
  }
}

//Adds a command to the que of commands we need to enact
void addToQue(byte command, byte value)
{
  queHead++;
  if (queHead == MAX_QUE_SIZE) queHead = 0;

  commandQue[queHead][0] = command;
  commandQue[queHead][1] = value;
}

void addToQue(byte command)
{
  addToQue(command, 0);
}
