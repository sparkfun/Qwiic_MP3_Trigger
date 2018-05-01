#define MP3_COMMAND_PLAY_INDEX_IN_ROOT 0xA2
#define MP3_COMMAND_PLAY_FILE_IN_ROOT 0xA3
#define MP3_COMMAND_PLAY_INDEX_IN_FOLDER 0xA4
#define MP3_COMMAND_PLAY_FILE_IN_FOLDER 0xA5
#define MP3_COMMAND_PAUSE 0xAA
#define MP3_COMMAND_STOP 0xAB
#define MP3_COMMAND_NEXT 0xAC
#define MP3_COMMAND_PREVIOUS 0xAD
#define MP3_COMMAND_SET_VOLUME 0xAE
#define MP3_COMMAND_SET_EQ_MODE 0xB2

#define MP3_COMMAND_GET_VOLUME 0xC1
#define MP3_COMMAND_GET_CURRENT_STATE 0xC2
#define MP3_COMMAND_GET_SONG_COUNT 0xC5
#define MP3_COMMAND_GET_SONGS_IN_FOLDER_COUNT 0xC6
#define MP3_COMMAND_GET_FILE_PLAYING 0xC9
#define MP3_COMMAND_GET_SONG_NAME_PLAYING 0xCB

#define MP3_START_CODE 0x7E
#define MP3_END_CODE 0xEF

byte commandBytes[11]; //Global array to pass MP3 commands around
//Worst case command may be 0xA3 which could have 8 characters of file name

//Play a given track number. This is a number of listed MP3s in the root directory.
//User can arrange the MP3s however. Not necessarily in alpha-order.
//Does nothing if file is not available
byte playTrackNumber(byte trackNumber)
{
  commandBytes[0] = MP3_COMMAND_PLAY_INDEX_IN_ROOT;
  commandBytes[1] = trackNumber >> 8; //MSB
  commandBytes[2] = trackNumber & 0xFF; //LSB
  sendCommand(3);
  return (getResponse());
}

//Play a file # from the root directory
//For example 3 will play F003xxx.mp3 where x can be anything
//Does nothing if file is not available
byte playFileName(byte fileNumber)
{
  commandBytes[0] = MP3_COMMAND_PLAY_FILE_IN_ROOT;
  commandBytes[1] = 'F';
  commandBytes[2] = '0' + (fileNumber / 100);
  fileNumber %= 100;
  commandBytes[3] = '0' + (fileNumber / 10);
  fileNumber %= 10;
  commandBytes[4] = '0' + fileNumber;
  sendCommand(5);
  return (getResponse());
}

//Play a file # from the root directory
//For example 1 will play T001xxx.mp3 where x can be anything
//Does nothing if file is not available
byte playTriggerFile(byte triggerNumber)
{
  commandBytes[0] = MP3_COMMAND_PLAY_FILE_IN_ROOT;
  commandBytes[1] = 'T';
  commandBytes[2] = '0' + (triggerNumber / 100);
  triggerNumber %= 100;
  commandBytes[3] = '0' + (triggerNumber / 10);
  triggerNumber %= 10;
  commandBytes[4] = '0' + triggerNumber;
  sendCommand(5);
  return (getResponse());
}

//Set volume. 0 is off. 31 max.
byte setVolume(byte volumeLevel)
{
  if (volumeLevel > 31) volumeLevel = 31; //Error check
  commandBytes[0] = MP3_COMMAND_SET_VOLUME;
  commandBytes[1] = volumeLevel;
  sendCommand(2);
  return (getResponse());
}

//Returns the current volume level, 0 to 31
byte getVolume(void)
{
  commandBytes[0] = MP3_COMMAND_GET_VOLUME;
  commandBytes[1] = 0xC4;
  sendCommand(2);
  return (getResponse());
}

//Stops playing any current track
byte stopPlaying(void)
{
  commandBytes[0] = MP3_COMMAND_STOP;
  sendCommand(1);
  return (getResponse());
}

//Set the equalizer levels to one of 6 levels (normal, pop, rock, jazz, classic, bass)
byte setEQ(byte eqType)
{
  if (eqType > 5) eqType == 0; //Error check. Set to normal by default
  commandBytes[0] = MP3_COMMAND_SET_EQ_MODE;
  commandBytes[1] = eqType;
  sendCommand(2);
  return (getResponse());
}

boolean isPlaying(void)
{
  commandBytes[0] = MP3_COMMAND_GET_CURRENT_STATE;
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
