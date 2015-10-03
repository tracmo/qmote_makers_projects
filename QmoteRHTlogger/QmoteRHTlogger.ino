/*
  Qmote Humidity and Temperature Logger with Maker's Module
  For more infotmation about this project, please refer to http://qblinks.com/devkit/makers/project-3-ambient-temperature-logger-on-dropbox
  
  Get the humidity and temperature reading and log them on the cloud storage will never be easier with Qmote Maker's Module
  For more information, please refer to http://qblinks.com/devkit
  For Sensirion SHT3x Arduino library, please refer to https://github.com/winkj/arduino-sht
  Fot the information of AltSoftSerial, please refer to https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html
  
  Author: Samson Chen, Qblinks Inc.
  Date: Aug 27th, 2015
*/

#include <AltSoftSerial.h>
#include <Wire.h>
#include <IRdaikin.h>
#include "sht3x.h"

// Using Sensirion SHT3x for the accurate measurements
SHT3X sht3x;

// for AltSoftSerial, TX/RX pins are hard coded: RX = digital pin 8, TX = digital pin 9 on Arduino Uno
AltSoftSerial portOne;

// Daikin Control
IRdaikin irdaikin;
int isOn = 0;

void setup() {
  // Setup SHT3x
  sht3x.setAddress(SHT3X::I2C_ADDRESS_44);
  sht3x.setAccuracy(SHT3X::ACCURACY_MEDIUM);
  Wire.begin();

  // Start the software serial port
  portOne.begin(38400);

  // let serial console settle
  delay(1000);
}

void loop() {
  String qmoteCmd;

  // read SHT3x
  sht3x.readSample();

  // let SHT3x read
  delay(1000);

  // get Qmote command for plaintext
  qmoteCmd = "ATBN=0x0A,\"RH=";      // using click combination code 0x0A
  qmoteCmd += sht3x.getHumidity();
  qmoteCmd += ", Temp=";
  qmoteCmd += sht3x.getTemperature();
  qmoteCmd += "\"\r\n";

  // output to Maker's module
  portOne.write(qmoteCmd.c_str());

  // delay before next command
  delay(2000);
  
  // get Qmote command for ThingSpeak.com with Humidity
  qmoteCmd = "ATBN=0x09,\"";        // using click combination code 0x09
  qmoteCmd += sht3x.getHumidity();  // To save UartMsg size, leave "field1=" to IFTTT
  qmoteCmd += "&field2=";
  qmoteCmd += sht3x.getTemperature();
  qmoteCmd += "\"\r\n";

  // output to Maker's module
  portOne.write(qmoteCmd.c_str());
  
  // delay a minutes until next reading
  delay(60000);
}
