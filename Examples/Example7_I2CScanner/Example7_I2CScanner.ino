/*
  i2c_scanner - Outputs the address of any I2C device detected on the bus
  From: https://playground.arduino.cc/Main/I2cScanner

  This sketch tests the standard 7-bit addresses
  Devices with higher bit address might not be seen properly.
*/

#include <Wire.h>

void setup()
{
  Wire.begin();

  Serial.begin(9600);
  Serial.println("I2C Scanner");
}

void loop()
{
  Serial.println("Scanning...");

  byte nDevices = 0;
  for (byte address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    byte result = Wire.endTransmission();

    if (result == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 0x10) Serial.print("0"); // Print the leading zero if necessary
      Serial.println(address, HEX);

      nDevices++;
    }
    else if (result == 4)
    {
      Serial.print("Unknow error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found");
  else
    Serial.println("Done");

  delay(5000); // Wait 5 seconds for next scan
}
