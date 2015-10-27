/*
  Qmote Humidity and Temperature Logger with Maker's Module
  
  Get the humidity and temperature reading and log them on the cloud storage will never be easier with Qmote Maker's Module
  For more information, please refer to http://qblinks.com/devkit
  For the information of AltSoftSerial, please refer to https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html
  For Dallas Temperature Sensor, please refer to https://github.com/milesburton/Arduino-Temperature-Control-Library/blob/master/DallasTemperature.h
  For DHT11 Sensor, please refer to https://github.com/adafruit/DHT-sensor-library
  
  Author: Samson Chen, Qblinks Inc.
  Date: Oct 27th, 2015
*/

#include <AltSoftSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <elapsedMillis.h>

// for AltSoftSerial, TX/RX pins are hard coded: RX = digital pin 8, TX = digital pin 9 on Arduino Uno
AltSoftSerial portOne;

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 3

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// DHT11
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// serial number for tracking
unsigned long seq_no;

// Elapsed Time
elapsedMillis timeElapsed;

void setup(void)
{
  // start serial port
  Serial.begin(9600);

  // Start the software serial port
  portOne.begin(38400);
  
  // Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement

  // send the first request
  sensors.requestTemperatures();

  // reset seqeuence number
  seq_no = 0;
}


void loop(void)
{ 
  String qmoteCmd;
  float tempC;
  float rhT;

  // DHT11 humidity reading
  rhT = dht.readHumidity();
  
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures
  tempC = sensors.getTempCByIndex(0);

  Serial.println(tempC);
  Serial.println(rhT);

  // ****************************************
  
  // get Qmote command for plaintext
  qmoteCmd = "ATBN=0x07,\"TempC=";      // using click combination code 0x07
  qmoteCmd += tempC;
  qmoteCmd += "\"\r\n";

  // output to Maker's module
  portOne.write(qmoteCmd.c_str());

  // delay before next command
  delay(5000);
  
  // ****************************************
  
  // get Qmote command for plaintext
  qmoteCmd = "ATBN=0x08,\"RhT=";      // using click combination code 0x08
  qmoteCmd += rhT;
  qmoteCmd += "\"\r\n";

  // output to Maker's module
  portOne.write(qmoteCmd.c_str());

  // delay before next command
  delay(5000);
  
  // ****************************************
  
  // get Qmote command for ThingSpeak.com
  qmoteCmd = "ATBN=0x09,\"";        // using click combination code 0x09
  qmoteCmd += "field1=";
  qmoteCmd += tempC;
  qmoteCmd += "\"\r\n";

  // output to Maker's module
  portOne.write(qmoteCmd.c_str());
  
  // delay before next command
  delay(5000);
  
  // ****************************************
  
  // get Qmote command for ThingSpeak.com
  qmoteCmd = "ATBN=0x0A,\"";        // using click combination code 0x0A
  qmoteCmd += "field2=";
  qmoteCmd += rhT;
  qmoteCmd += "\"\r\n";

  // output to Maker's module
  portOne.write(qmoteCmd.c_str());

  // delay before next command
  delay(5000);  

  // ****************************************
  
  // get Qmote command for sequence test
  qmoteCmd = "ATBN=0x01,\"";
  qmoteCmd += "seq=";
  qmoteCmd += seq_no++;
  qmoteCmd += "\"\r\n";

  // output to Maker's module
  portOne.write(qmoteCmd.c_str());

  // delay before next command
  delay(2000);

  // ****************************************
  
  // get Qmote command for elapsed time
  qmoteCmd = "ATBN=0x02,\"";
  qmoteCmd += "et=";
  qmoteCmd += timeElapsed;
  qmoteCmd += "\"\r\n";

  // output to Maker's module
  portOne.write(qmoteCmd.c_str());

  // delay before next command
  delay(2000);

  // ****************************************
  
  // get Qmote command for general test
  qmoteCmd = "ATBN=0x03\r\n";

  // output to Maker's module
  portOne.write(qmoteCmd.c_str());

  // delay before next command
  delay(2000);

  // ****************************************
  
  // get Qmote command for general test
  qmoteCmd = "ATBN=0x04\r\n";

  // output to Maker's module
  portOne.write(qmoteCmd.c_str());

  // delay before next command
  delay(2000);

  // ****************************************
  
  // get Qmote command for general test
  qmoteCmd = "ATBN=0x05\r\n";

  // output to Maker's module
  portOne.write(qmoteCmd.c_str());

  // delay before next command
  delay(2000);

  // ****************************************
  
  // get Qmote command for general test
  qmoteCmd = "ATBN=0x06\r\n";

  // output to Maker's module
  portOne.write(qmoteCmd.c_str());

  // delay before next command
  delay(2000);
}


