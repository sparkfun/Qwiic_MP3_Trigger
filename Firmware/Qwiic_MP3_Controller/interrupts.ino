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
    }
  }
  else if (incoming == COMMAND_PLAY_FILENUMBER)
  {
    if (Wire.available())
    {
      byte fileNumber = Wire.read();
      systemStatus = playFileName(fileNumber); //For example 3 will play F003xxx.mp3
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
  }
  else if (incoming == COMMAND_PLAY_PREVIOUS)
  {
    systemStatus = playPrevious();
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
  else if (incoming == COMMAND_CLEAR_INTERRUPTS)
  {
    //If we are in the Interrupt state, then turn off interrupt
    if(interruptState == STATE_INT)
    {
      //This will set the int pin to high impedance (aka pulled high by external resistor)
      digitalWrite(interruptOutput, LOW); //Push pin to disable internal pull-ups
      pinMode(interruptOutput, INPUT); //Go to high impedance
      
      interruptState = STATE_INT_CLEARED; //Go to next state
    }
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
}
