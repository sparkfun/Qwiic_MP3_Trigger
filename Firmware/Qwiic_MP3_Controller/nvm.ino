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

  //Read what equalizer setting we should be at
  settingEQ = EEPROM.read(LOCATION_EQ);
  if (settingEQ > 5)
  {
    settingEQ = 0; //By default, go normal EQ setting
    EEPROM.write(LOCATION_EQ, settingEQ);
  }
}
