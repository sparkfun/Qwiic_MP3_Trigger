//These are the commands we can send
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

//Checks the status of the player to see if MP3 is playing
//Returns true if song is playing
boolean mp3IsPlaying()
{
  mp3Command(COMMAND_GET_PLAY_STATUS);

  delay(20); //Give the QMP3 time to get the status byte from MP3 IC before we ask for it

  //01: play, 02: stop, 03: pause
  byte playStatus = mp3GetResponse();
  if(playStatus == 0x01) return(true);
  return(false);
}

//Plays a given track number
//Think of this like a CD. The user can arrange the order of MP3s
//however. playTrack(4) will play whatever is in the 4th file.
void mp3PlayTrack(byte trackNumber)
{
  mp3Command(COMMAND_PLAY_TRACK, trackNumber); //Play track  
}

//Plays a file that has been named specifically. 
//For example: passing in 6 will play F006xxx.mp3
void mp3PlayFile(byte fileNumber)
{
  mp3Command(COMMAND_PLAY_FILENUMBER, fileNumber); //Play file number  
}

//Stop playing the current track
void mp3Stop()
{
  mp3Command(COMMAND_STOP);
}

//Change the equalizer to one of 6 types
void mp3ChangeEQ(byte eqType)
{
  //0-normal, 1-pop, 2-rock, 3-jazz, 4-classical, 5-bass
  mp3Command(COMMAND_SET_EQ, eqType); //Change equalizer to bass
}

//Get the current status of the Qwiic MP3
byte mp3Status()
{
  return(mp3GetResponse());
}

//Checks to see if MP3 player has a valid SD card
boolean mp3HasCard()
{
  mp3Command(COMMAND_GET_CARD_STATUS);

  delay(20); //Give the QMP3 time to get the status byte from MP3 IC before we ask for it
  
  return(mp3GetResponse());
}

//Get the 8 characters of the song currently playing
String mp3SongName()
{
  String thisSongName = "";
  mp3Command(COMMAND_GET_SONG_NAME);

  delay(50); //Give the QMP3 time to get the name from MP3 IC before we ask for it

  Wire.requestFrom(mp3Address, 8); //Song names are max 8 chars

  while(Wire.available())
  {
    thisSongName += (char)Wire.read();
  }
  return(thisSongName);
}

//Get the number of songs on the SD card (in root and subfolders)
//Limited to 255
byte mp3SongCount()
{
  mp3Command(COMMAND_GET_SONG_COUNT); //Get current song count

  delay(50); //Give the QMP3 time to get the count from MP3 IC before we ask for it

  return(mp3GetResponse());
}

//Change volume to zero (off) to 31 (max)
void mp3ChangeVolume(byte volumeLevel)
{
  mp3Command(COMMAND_SET_VOLUME, volumeLevel); //Change volume
}

//Play the next track
//Think of this like a CD. The audio files can be in any order. The user
//sets the file order. This plays the next one.
void mp3PlayNext()
{
  mp3Command(COMMAND_PLAY_NEXT);
}

//Play the previous track
//Think of this like a CD. The audio files can be in any order. The user
//sets the file order. This plays the previous one.
void mp3PlayPrevious()
{
  mp3Command(COMMAND_PLAY_PREVIOUS);
}

//Checks to see if Qwiic MP3 is responding over I2C
boolean mp3IsPresent()
{
  Wire.beginTransmission(mp3Address);
  if (Wire.endTransmission() != 0)
    return(false); //Sensor did not ACK
  return(true);
}

//Pause a currently playing song, or begin playing if current track is paused
boolean mp3Pause()
{
  mp3Command(COMMAND_PAUSE);
}

//Change the I2C address
//If you forget what address you've set the QMP3 to then close the
//ADR jumper. This will force the I2C address to 0x36
boolean mp3ChangeAddress(byte address)
{
  mp3Command(COMMAND_SET_ADDRESS, address);
  mp3Address = address; //Change the global variable to match the new address
}

//Send command to Qwiic MP3 with options
boolean mp3Command(byte command, byte option)
{
  Wire.beginTransmission(mp3Address);
  Wire.write(command);
  Wire.write(option);
  if (Wire.endTransmission() != 0)
    return(false); //Sensor did not ACK
  return(true);
}

//Send just a command to Qwiic MP3
boolean mp3Command(byte command)
{
  Wire.beginTransmission(mp3Address);
  Wire.write(command);
  if (Wire.endTransmission() != 0)
    return(false); //Sensor did not ACK
  return(true);
}

//Ask for a byte from Qwiic MP3
//The response depends on what the last command was
//It is often the system status but can be song count or volume level
byte mp3GetResponse()
{
  Wire.requestFrom(mp3Address, 1);

  if (Wire.available())
    return (Wire.read());

  Serial.println("Error: Sensor did not respond");
  return(0);
}

//getFirmwareVersion() returns the firmware version as a float.
float mp3GetVersion()
{
  mp3Command(COMMAND_GET_VERSION);

  Wire.requestFrom(mp3Address, 2); //2 bytes for Version

  if (Wire.available() == 0) return 0;
  float versionNumber = Wire.read();
  versionNumber += (float)Wire.read() / 10.0;

  return (versionNumber);
}

