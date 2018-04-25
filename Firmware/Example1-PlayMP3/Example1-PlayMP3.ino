/*
  Testing the interface on the WT2003S
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 23rd, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example shows how to read the PM2.5 and PM10 readings from the sensor
*/

#include <SoftwareSerial.h>

#define COMMAND_PLAY_INDEX_IN_ROOT 0xA2
#define COMMAND_PLAY_FILE_IN_ROOT 0xA3
#define COMMAND_PLAY_INDEX_IN_FOLDER 0xA4
#define COMMAND_PLAY_FILE_IN_FOLDER 0xA5
#define COMMAND_PAUSE 0xAA
#define COMMAND_STOP 0xAB
#define COMMAND_NEXT 0xAC
#define COMMAND_PREVIOUS 0xAD
#define COMMAND_SET_VOLUME 0xAE
#define COMMAND_SET_EQ_MODE 0xB2

#define COMMAND_GET_VOLUME 0xC1
#define COMMAND_GET_CURRENT_STATE 0xC2
#define COMMAND_GET_SONG_COUNT 0xC5
#define COMMAND_GET_SONGS_IN_FOLDER_COUNT 0xC6
#define COMMAND_GET_FILE_PLAYING 0xC9
#define COMMAND_GET_SONG_NAME_PLAYING 0xCB

#define MP3_START_CODE 0x7E
#define MP3_END_CODE 0xEF

SoftwareSerial mp3(6, 7); // RX, TX
//SoftwareSerial mp3(1, 0); // RX, TX on Tiny

byte commandBytes[20];

void setup()
{
  pinMode(7, OUTPUT);

  //Serial.begin(9600);
  //Serial.println("WS2003 Control Example");

  mp3.begin(9600); //WS2003S communicates at 9600bps

  //300ms between commands

  byte response;

  response = stopPlaying();
  Serial.print("Response 0x:");
  Serial.println(response, HEX);

  response = setVolume(15);
  Serial.print("Response 0x:");
  Serial.println(response, HEX);

  response = setEQ(5); //Set to bass EQ
  Serial.print("Response 0x:");
  Serial.println(response, HEX);

  response = playTrackNumber(4);
  Serial.print("Response 0x:");
  Serial.println(response, HEX);
}

void loop()
{
  //if (isPlaying()) Serial.println("Playing...");

  digitalWrite(7, HIGH);
  delay(1000);
  digitalWrite(7, LOW);
  delay(1000);
}

//Play a track # from the root directory
byte playTrackNumber(unsigned int trackNumber)
{
  commandBytes[0] = COMMAND_PLAY_INDEX_IN_ROOT;
  commandBytes[1] = trackNumber >> 8; //MSB
  commandBytes[2] = trackNumber & 0xFF; //LSB
  sendCommand(3);
  return (getResponse());
}

//Set volume. 0 is off. 31 max.
byte setVolume(byte volumeLevel)
{
  if (volumeLevel > 31) volumeLevel = 31; //Error check
  commandBytes[0] = COMMAND_SET_VOLUME;
  commandBytes[1] = volumeLevel;
  sendCommand(2);
  return (getResponse());
}

//Returns the current volume level, 0 to 31
byte getVolume(void)
{
  commandBytes[0] = COMMAND_GET_VOLUME;
  commandBytes[1] = 0xC4;
  sendCommand(2);
  return (getResponse());
}

//Stops playing any current track
byte stopPlaying(void)
{
  commandBytes[0] = COMMAND_STOP;
  sendCommand(1);
  return (getResponse());
}

//Set the equalizer levels to one of 6 levels (normal, pop, rock, jazz, classic, bass)
byte setEQ(byte eqType)
{
  if (eqType > 5) eqType == 0; //Error check. Set to normal by default
  commandBytes[0] = COMMAND_SET_EQ_MODE;
  commandBytes[1] = eqType;
  sendCommand(2);
  return (getResponse());
}

boolean isPlaying(void)
{
  commandBytes[0] = COMMAND_GET_CURRENT_STATE;
  sendCommand(1);

  //01: play
  //02: stop
  //03: pause

  if (getResponse() == 0x01) return (true);
  return (false);
}

//Sends the global command array attaching the start code,
//end code and CRC
void sendCommand(byte commandLength)
{
  mp3.write(MP3_START_CODE);
  mp3.write(commandLength + 2); //Add one byte for 'length', one for CRC

  //Begin sending command bytes while calc'ing CRC
  byte crc = commandLength + 2;
  for (byte x = 0 ; x < commandLength ; x++) //Length + command code + parameter
  {
    mp3.write(commandBytes[x]); //Send this byte
    crc += commandBytes[x]; //Add this byte to the CRC
  }

  mp3.write(crc); //Send CRC
  mp3.write(MP3_END_CODE);
}

byte getResponse(void)
{
  byte counter = 0;
  while (mp3.available() == false)
  {
    delay(1);
    if (counter++ > 200) return (0xFF); //Timeout
  }

  byte response = 0xFF;
  byte i = 0;
  while (mp3.available())
  {
    if (i++ == 0) response = mp3.read();
  }

  return (response);
}

