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
    addToQue(COMMAND_STOP); //We have to add commands to the que because in the interrupt, we cannot do software serial reception
  }
  else if (incoming == COMMAND_PLAY_TRACK)
  {
    if (Wire.available())
    {
      byte trackNumber = Wire.read();
      addToQue(COMMAND_PLAY_TRACK, trackNumber);
    }
  }
  else if (incoming == COMMAND_PLAY_FILENUMBER)
  {
    if (Wire.available())
    {
      byte fileNumber = Wire.read();
      addToQue(COMMAND_PLAY_FILENUMBER, fileNumber); //For example 3 will play F003xxx.mp3
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

      addToQue(COMMAND_SET_VOLUME, settingVolume); //Go to this volume
    }
  }
  else if (incoming == COMMAND_PAUSE)
  {
    addToQue(COMMAND_PAUSE);
  }
  else if (incoming == COMMAND_PLAY_NEXT)
  {
    addToQue(COMMAND_PLAY_NEXT);
  }
  else if (incoming == COMMAND_PLAY_PREVIOUS)
  {
    addToQue(COMMAND_PLAY_PREVIOUS);
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

      addToQue(COMMAND_SET_EQ); //Go to this equalizer setting
    }
  }
  else if (incoming == COMMAND_GET_SONG_COUNT)
  {
    //addToQue(COMMAND_GET_SONG_COUNT);

    responseType = RESPONSE_TYPE_SONG_COUNT; //Count is loaded at setup()
  }
  else if (incoming == COMMAND_GET_SONG_NAME)
  {
    addToQue(COMMAND_GET_SONG_NAME);
  }
  else if (incoming == COMMAND_GET_PLAY_STATUS)
  {
    playStatus = 0x00; //Stopped
    if (digitalRead(playing) == HIGH) //Song is playing
      playStatus = 0x01;

    responseType = RESPONSE_TYPE_PLAY_STATUS; //Change response type
  }
  else if (incoming == COMMAND_GET_CARD_STATUS)
  {
    addToQue(COMMAND_GET_CARD_STATUS);
  }
  else if (incoming == COMMAND_GET_VERSION)
  {
    responseType = RESPONSE_TYPE_FIRMWARE_VERSION;
  }
  else if (incoming == COMMAND_CLEAR_INTERRUPTS)
  {
    //If we are in the Interrupt state, then turn off interrupt
    if (interruptState == STATE_INT)
    {
      //This will set the int pin to high impedance (aka pulled high by external resistor)
      digitalWrite(interruptOutput, LOW); //Push pin to disable internal pull-ups
      pinMode(interruptOutput, INPUT); //Go to high impedance

      interruptState = STATE_INT_CLEARED; //Go to next state
    }
  }
  else if (incoming == COMMAND_GET_VOLUME)
  {
    responseType = RESPONSE_TYPE_GET_VOLUME;
  }
  else if (incoming == COMMAND_GET_EQ)
  {
    responseType = RESPONSE_TYPE_GET_EQ;
  }
  else if (incoming == COMMAND_GET_ID)
  {
    responseType = RESPONSE_TYPE_GET_ID;
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
  else if (responseType == RESPONSE_TYPE_GET_VOLUME)
  {
    Wire.write(settingVolume);
  }
  else if (responseType == RESPONSE_TYPE_GET_EQ)
  {
    Wire.write(settingEQ);
  }
  else if (responseType == RESPONSE_TYPE_GET_ID)
  {
    Wire.write(DEVICE_ID);
  }
  else //By default we respond with the result from the last operation
  {
    Wire.write(systemStatus);
  }

  responseType = RESPONSE_TYPE_SYSTEM_STATUS; //Reset response type
}
