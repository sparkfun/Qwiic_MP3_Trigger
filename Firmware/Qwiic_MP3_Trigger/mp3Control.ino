//These are the commands that are sent over serial to the WT2003S
#define MP3_COMMAND_PLAY_INDEX_IN_ROOT 0xA2
#define MP3_COMMAND_PLAY_FILE_IN_ROOT 0xA3
#define MP3_COMMAND_PLAY_INDEX_IN_FOLDER 0xA4
#define MP3_COMMAND_PLAY_FILE_IN_FOLDER 0xA5
#define MP3_COMMAND_PAUSE 0xAA
#define MP3_COMMAND_STOP 0xAB
#define MP3_COMMAND_NEXT 0xAC
#define MP3_COMMAND_PREVIOUS 0xAD
#define MP3_COMMAND_SET_VOLUME 0xAE //Can take more than 150ms to complete
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

//Query the total number of music files on the SD card (including sub directories)
//NOTE: This will cause any current playing song to stop
unsigned int getSongCount()
{
  commandBytes[0] = MP3_COMMAND_GET_SONG_COUNT;
  sendCommand(1);

  if (responseAvailable() == false) return (0); //Timeout

  //Get three byte response. Timeout after 250ms
  byte i = 0;
  while (mp3.available() < 3)
  {
    noIntDelay(1);
    if (i++ > 250) return (0); //Error
  }

  unsigned int count = 0xFFFF;
  i = 0;
  while (mp3.available())
  {
    byte incoming = mp3.read();
    if (i == 0) ; //This is throw away value 0xC5
    else if (i == 1) count = (incoming << 8); //MSB
    else if (i == 2) count |= incoming; //LSB

    i++;
  }

  return (count);
}

//Query the song name of the current play
//Caller must provide an array of 9 characters
//We can't do any serial receiving during an interrupt
void getSongName()
{
  commandBytes[0] = MP3_COMMAND_GET_SONG_NAME_PLAYING;
  sendCommand(1);

  strcpy(songName, (char *)"Error\0");
  if (responseAvailable() == false) return;

  //Wait for 9 byte response. Timeout after 250ms
  byte i = 0;
  while (mp3.available() < 9)
  {
    noIntDelay(1);
    if (i++ > 250) return; //Return with Error in songName
  }

  //Parse the response
  i = 0;
  while (mp3.available())
  {
    byte incoming = mp3.read();
    if (i == 0) ; //This is throw away value 0xCB
    else if (i < 9) songName[i - 1] = incoming;

    i++;
  }
  songName[8] = '\0'; //Terminate this string
}

//Play a given track number. This is a number of listed MP3s in the root directory.
//User can arrange the MP3s however. Not necessarily in alpha-order.
//Does nothing if file is not available
byte playTrackNumber(byte trackNumber)
{
  commandBytes[0] = MP3_COMMAND_PLAY_INDEX_IN_ROOT;
  commandBytes[1] = trackNumber >> 8; //MSB
  commandBytes[2] = trackNumber & 0xFF; //LSB
  sendCommand(3);
  byte response = getResponse();

  if (trackPlaying == false) setMP3Volume(settingVolume); //Go to the volume stored in system settings. This can take >150ms
  trackPlaying = true;

  return (response);
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
  byte response = getResponse();

  if (trackPlaying == false) setMP3Volume(settingVolume); //Go to the volume stored in system settings. This can take >150ms
  trackPlaying = true;

  return (response);
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
  byte response = getResponse();

  if (trackPlaying == false) setMP3Volume(settingVolume); //Go to the volume stored in system settings. This can take >150ms
  trackPlaying = true;

  return (response);
}

//Set the internal variable but don't change MP3 IC level
byte setInternalVolume(byte volumeLevel)
{
  if (volumeLevel > 31) volumeLevel = 31; //Error check
  settingVolume = volumeLevel; //Update internal value

#if defined(__AVR_ATmega328P__)
  Serial.print("Set Internal volume to: ");
  Serial.println(settingVolume);
#endif

  if (trackPlaying == true) setMP3Volume(settingVolume);
  return (0);
}

//Set volume. 0 is off. 31 max.
byte setMP3Volume(byte volumeLevel)
{
  if (volumeLevel > 31) volumeLevel = 31; //Error check
  //
  //  settingVolume = volumeLevel; //Update internal value
  //
  //if(trackPlaying == false) return(settingVolume); //Autoloop work around: If we're ignoring the auto-loop playing, don't turn up volume.

  //If we are actively playing, go ahead and set volume
  commandBytes[0] = MP3_COMMAND_SET_VOLUME;
  commandBytes[1] = volumeLevel;
  sendCommand(2);
  byte response = getResponse();

#if defined(__AVR_ATmega328P__)
  Serial.print("SetMP3volume to: ");
  Serial.println(volumeLevel);
#endif

  return (response);
}

//Returns the current volume level, 0 to 31
byte getVolume(void)
{
  return (settingVolume); //Rather than poll IC, just return local var
  /*commandBytes[0] = MP3_COMMAND_GET_VOLUME;
    sendCommand(1);

    //Get two byte response
    unsigned int volLevel = getTwoByteResponse();

    //First byte is 0xC1, second byte is volume level
    return (volLevel & 0xFF);*/
}

//Set the equalizer levels to one of 6 levels (normal, pop, rock, jazz, classic, bass)
byte setEQ(byte eqType)
{
  if (eqType > 5) eqType = 0; //Error check. Set to normal by default
  commandBytes[0] = MP3_COMMAND_SET_EQ_MODE;
  commandBytes[1] = eqType;
  sendCommand(2);
  return (getResponse());
}

//Returns the current EQ setting: one of 6 levels (normal, pop, rock, jazz, classic, bass)
byte getEQ()
{
  return (settingEQ);
}

//Checks status. Returns true if a song is playing (status 0x01)
//01: play, 02: stop, 03: pause
boolean isPlaying(void)
{
  if (digitalRead(playing) == HIGH) return (true); //Song is playing
  return (false);

  //if(getPlayStatus() == 0x01) return (true);
  //return (false);
}

//Returns the current play/stop/pause status
//01: play, 02: stop, 03: pause
byte getPlayStatus(void)
{
  //  commandBytes[0] = MP3_COMMAND_GET_CURRENT_STATE;
  //  sendCommand(1);
  //  return (getTwoByteResponse() & 0xFF);

  if (trackPlaying == true) return (1); //Playing
  else return (2); //Stop
}

//Pause: will pause currently playing track, or starting playing if paused
byte pause(void)
{
  //  commandBytes[0] = MP3_COMMAND_PAUSE;
  //  sendCommand(1);
  //  return (response);

  //Because of the loop issue, start playing doesn't really work, but we can turn the volume back up...

  //If we are not currently playing a track, turn volume up and remember we're 'playing'
  if (trackPlaying == false)
  {
    setMP3Volume(settingVolume); //Go to the volume stored in system settings. This can take >150ms
    trackPlaying = true;
  }
  //If we *are* currently playing a track, turn off volume and remember we're 'paused'
  else if (trackPlaying == true)
  {
    setMP3Volume(0);
    trackPlaying = false;
  }
}

//Play next track
byte playNext(void)
{
  commandBytes[0] = MP3_COMMAND_NEXT;
  sendCommand(1);
  byte response = getResponse();

  if (trackPlaying == false) setMP3Volume(settingVolume); //Go to the volume stored in system settings. This can take >150ms
  trackPlaying = true;

  return (response);
}

//Play previous track
byte playPrevious(void)
{
  commandBytes[0] = MP3_COMMAND_PREVIOUS;
  sendCommand(1);
  byte response = getResponse();

  if (trackPlaying == false) setMP3Volume(settingVolume); //Go to the volume stored in system settings. This can take >150ms
  trackPlaying = true;

  return (response);
}

//Stops playing any current track
byte stopPlaying(void)
{
  //  commandBytes[0] = MP3_COMMAND_STOP;
  //  sendCommand(1);
  //  return (getResponse());

  if (trackPlaying == true) setMP3Volume(0); //Mute volume
  trackPlaying = false;
  return (true);
}

//Sends the global command array attaching the start code,
//end code and CRC
void sendCommand(byte commandLength)
{
  clearBuffer(); //Clear anything in the buffer

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

//Wait for a serial response from the MP3 IC
//Time out if the MP3 IC fails to respond
byte getResponse(void)
{
  if (responseAvailable() == false)
    return (0xFF); //Timeout

  byte response = 0xFE;
  byte i = 0;
  while (mp3.available())
  {
    byte incoming = mp3.read();
    if (i++ == 0) response = incoming;
  }

  return (response);
}

unsigned int getTwoByteResponse(void)
{
  if (responseAvailable() == false) return (0xFF); //Timeout

  unsigned int response = 0xFFFF;
  byte i = 0;
  while (mp3.available())
  {
    byte incoming = mp3.read();

    if (i == 0) response = incoming << 8; //MSB
    else if (i == 1) response |= incoming; //LSB

    i++;
    noIntDelay(1); //At 9600bps 1 byte takes 0.8ms
  }

  return (response);
}

//Returns true if serial data is available within an alloted amount of time
boolean responseAvailable()
{
  byte counter = 0;
  while (mp3.available() == false)
  {
    noIntDelay(1); //No delays in interrupts

    if (counter++ > 250) return (false); //Timeout
  }
  return (true);
}

//Clear anything sitting in the incoming buffer
void clearBuffer()
{
  while (mp3.available())
  {
    mp3.read();
    noIntDelay(1); //1 byte at 9600bps should take 1ms
  }
}
