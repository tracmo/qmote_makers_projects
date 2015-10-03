/*
  Qmote Humidity and Temperature Logger with Daikin Air Conditioning Automatic Control

  Get the humidity and temperature reading and log them on the cloud storage will never be easier with Qmote Maker's Module
  For more information, please refer to http://qblinks.com/devkit
  For Sensirion SHT3x Arduino library, please refer to https://github.com/winkj/arduino-sht
  For the information of AltSoftSerial, please refer to https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html
  For the information of Daikin IR Control, please refer to https://github.com/danny-source/Arduino_IRremote_Daikin
  
  Author: Samson Chen, Qblinks Inc.
  Date: Sep 20th, 2015
*/

#include <AltSoftSerial.h>
#include <Wire.h>
#include <elapsedMillis.h>
#include <IRdaikin.h>
#include "sht3x.h"

// Using Sensirion SHT3x for the accurate measurements
SHT3X sht3x;

// for AltSoftSerial, TX/RX pins are hard coded: RX = digital pin 8, TX = digital pin 9 on Arduino Uno
AltSoftSerial portOne;
String recvLine = "";
bool qmoteDisconnected = false;
int resend = 0;

// Daikin Control
IRdaikin irdaikin;
int isOn = 0;

//  ambient environmental parameters
#define TEMP_SETTING          24          // cooling setting to this temperature
#define TEMP_HIGH             27.8        // restart cooling if temperature reaches this measurement
#define TEMP_LOW              25          // stop cooling or switch to humidifier if temerature reaches this measurement
#define TEMP_TOO_LOW          23          // if temperature reaches this value means humidifier goes too much, turn off everything
#define TEMP_TOO_HIGH         30          // if temperature reaches this value, KEEP_OFF_TIMER will be ignored
#define RH_TOO_HIGH           80          // if Rh reaches this value, KEEP_OFF_TIMER will be ignored
#define RH_HIGH               68          // restart humidifier if Rh reaches to this measurement
#define RH_LOW                60          // stop humidifier if Rh reaches to this measurement

// once the air conditioning is OFF due to the ambient condition reaches the criteria, 
// do not turn it ON for certain amount of time
#define KEEP_OFF_TIMER        70

// when Qmote is disconnected, a timer counts the time before it is reconnected back
// if the disconnected time is longer than this value, a permanent disconnection is considered and the air conditioning will be off
#define DISCONNECT_OFF_TIMER  20

#define MODE_OFF              0
#define MODE_COOLING          1
#define MODE_DEHUMIDIFIER     2

// Resending Control
unsigned int resend_attempts = 1;

// Flag for startup calibration
bool startup_calibration = true;

// Elapsed Time Control
elapsedMillis timeElapsed;
bool just_off = false;

// Auto Shutoff Control
elapsedMillis disconnectDurationCheck;
bool disconnecting = false;

void setup() {
  // Setup SHT3x
  sht3x.setAddress(SHT3X::I2C_ADDRESS_44);
  sht3x.setAccuracy(SHT3X::ACCURACY_MEDIUM);
  Wire.begin();

  // Start the software serial port
  portOne.begin(38400);

  // let serial console settle
  delay(3000);

  // reset the air conditioning control
  isOn = MODE_OFF;
  disconnecting = false;

  // needs no UART echo
  String qmoteCmd = "ATE0\r\n";
  portOne.write(qmoteCmd.c_str());
  delay(3000);

  // reset timer
  timeElapsed = 0;
  just_off = false;

  // send the first one
  qmoteCmd = "ATBN=0x0A,\"RhT Starts\"\r\n";      // using click combination code 0x0A
  portOne.write(qmoteCmd.c_str());                // output to Maker's module

  // delay extra 10 seconds so the first RhT information can be sent to Qmote
  delay(10000);

  // when startup, issue more IR firing for direction califbration
  startup_calibration = true;
}

void flash_LED()
{
  String qmoteCmd;
  
  // flash Qmote LED as indicator
  delay(2000);
  qmoteCmd = "ATL1=5\r\n";
  portOne.write(qmoteCmd.c_str());

  // off LED
  delay(29000);
  qmoteCmd = "ATL0\r\n";
  portOne.write(qmoteCmd.c_str());  
}

void wemo_fan_on()
{
  String qmoteCmd;
  
  for(int attempt=0; attempt < 2; attempt++)      // let's do it twice to make it goes through
  {
    qmoteCmd = "ATBN=0x07\r\n";                   // using click combination code 0x07, --.
    portOne.write(qmoteCmd.c_str());              // output to Maker's module
    delay(3000);
  }  
}

void wemo_fan_off()
{
  String qmoteCmd;
  
  for(int attempt=0; attempt < 2; attempt++)      // let's do it twice to make it goes through
  {
    qmoteCmd = "ATBN=0x08\r\n";                   // using click combination code 0x08, -..
    portOne.write(qmoteCmd.c_str());              // output to Maker's module
    delay(3000);
  }  
}

void daikin_cooling_on()
{
  String qmoteCmd;
  
  irdaikin.daikin_on();
  irdaikin.daikin_setSwing_off();
  irdaikin.daikin_setMode(1);  // cooling
  irdaikin.daikin_setFan(6);   // FAN speed to night
  irdaikin.daikin_setTemp(TEMP_SETTING);
  irdaikin.daikin_sendCommand();
  isOn = MODE_COOLING;
  resend++;
  just_off = false;

  // report this state change
  qmoteCmd = "ATBN=0x0A,\"cooling ON\"\r\n";      // using click combination code 0x0A, --..
  portOne.write(qmoteCmd.c_str());                // output to Maker's module

  // flash Qmote LED as indicator
  flash_LED();

  // turn off FAN through WeMo Switch
  wemo_fan_off();
}

void daikin_dehumidifier_on()
{
  String qmoteCmd;
  
  irdaikin.daikin_on();
  irdaikin.daikin_setSwing_off();
  irdaikin.daikin_setMode(2);  // dehumidifier
  irdaikin.daikin_setFan(6);   // FAN speed to night
  irdaikin.daikin_setTemp(TEMP_SETTING);  
  irdaikin.daikin_sendCommand();
  isOn = MODE_DEHUMIDIFIER;
  resend++;
  just_off = false;

  // report this state change
  qmoteCmd = "ATBN=0x0A,\"dehumidifier ON\"\r\n"; // using click combination code 0x0A, --..
  portOne.write(qmoteCmd.c_str());                // output to Maker's module

  // flash Qmote LED as indicator
  flash_LED();

  // turn off FAN through WeMo Switch
  wemo_fan_off();
}

void daikin_all_off()
{
  String qmoteCmd;
  
  irdaikin.daikin_off();
  irdaikin.daikin_setSwing_off();
  irdaikin.daikin_setMode(2);  // dehumidifier
  irdaikin.daikin_setFan(6);   // FAN speed to night
  irdaikin.daikin_setTemp(TEMP_SETTING);  
  irdaikin.daikin_sendCommand();
  isOn = MODE_OFF;
  resend++;
  timeElapsed = 0;            // reset keep-off timer
  // don't put just_off here to avoid startup calibration off

  // report this state change
  qmoteCmd = "ATBN=0x0A,\"air cond OFF\"\r\n";    // using click combination code 0x0A, --..
  portOne.write(qmoteCmd.c_str());                // output to Maker's module

  // flash Qmote LED as indicator
  flash_LED();

  if(!qmoteDisconnected)
  {
    // turn on FAN through WeMo Switch
    wemo_fan_on();
  }
  else
  {
    // turn off FAN through WeMo Switch
    // Note: this control is tricky. If Qmote is disconnected, this command never goes to WeMo.
    //       the ultimate solution will be a disconnected trigger of Qmote on IFTTT.
    wemo_fan_off();
  }
}

void loop() {
  String qmoteCmd;
  float currentTemp;
  float currentRh;

  // read SHT3x
  sht3x.readSample();
  delay(2000);  // let SHT3x read

  // read SHT3x
  currentTemp = sht3x.getTemperature();
  currentRh = sht3x.getHumidity();

  // filter out incorrect SHT3x reading
  if(currentTemp > -100 && currentRh >= 0)
  {
    // get Qmote command for plaintext output
unsigned int teim = timeElapsed / (long) 60000;
    qmoteCmd = "ATBN=0x0A,\"";      // using click combination code 0x0A
qmoteCmd += isOn;
qmoteCmd += qmoteDisconnected;
qmoteCmd += just_off;
    qmoteCmd += teim;
    qmoteCmd += ",";
    qmoteCmd += currentRh;
    qmoteCmd += ",";
    qmoteCmd += currentTemp;
    qmoteCmd += "\"\r\n";

//    qmoteCmd = "ATBN=0x0A,\"RH=";      // using click combination code 0x0A, --..
//    qmoteCmd += currentRh;
//    qmoteCmd += ", Temp=";
//    qmoteCmd += currentTemp;
//    qmoteCmd += "\"\r\n";

    // output to Maker's module
    portOne.write(qmoteCmd.c_str());

    // delay before next command
    delay(2000);

    // get Qmote command for ThingSpeak output
    qmoteCmd = "ATBN=0x09,\"";        // using click combination code 0x09, -...
    qmoteCmd += currentRh;            // To save UartMsg size, leave "field1=" to IFTTT
    qmoteCmd += "&field2=";
    qmoteCmd += currentTemp;
    qmoteCmd += "\"\r\n";
  
    // output to Maker's module
    portOne.write(qmoteCmd.c_str());
  }

  // check Qmote Status
  while (portOne.available() > 0)
  {
    char c = portOne.read();
    
    if(c == 0x0D)
    {
      // null terminated
      recvLine += 0x00;
      
      // process this line if Qmote never gets disconnected
      if(!qmoteDisconnected)
      {
        if(recvLine.indexOf("ST: 0") >= 0)
        {
          // received "DISCONNECTED" status message
          
          // set the flag
          disconnecting = true;
          
          // restart the timer the timer
          disconnectDurationCheck = 0;
        }
        
        if(recvLine.indexOf("ST: 1") >= 0)
        {
          // received "CONNECTED" status message

          // uncheck the flag
          disconnecting = false;
        }
      } // end if(!qmoteDisconnected)
      
      // clear the line
      recvLine = "";
    }
    else if(c != 0x0A)
    {
      recvLine += c;
    }
    // ignore 0x0A
  }

  // check disconnecting status
  unsigned int durationCheckInMin = disconnectDurationCheck / (long) 60000;
  if(!qmoteDisconnected && disconnecting && durationCheckInMin > DISCONNECT_OFF_TIMER)
  {
    // status remained on disconnected over 3 minutes
    
    // set the flag, do not operate the air conditioning any more
    qmoteDisconnected = true;
    
    // reset the flag to avoid short delay at the end of loop
    disconnecting = false;

    // turn off air conditioning
    resend = 0;
    resend_attempts = 3;  // perform disconnection IR firing 3 times
    daikin_all_off();  
  }
  
  // this is to prevent unattended air conditioning control
  // once the Qmote is disconnected, no more auto air conditioning control
  // until the next power cycle
  if(!qmoteDisconnected)
  {
    // delay before next command and sample the data again
    sht3x.readSample();
    delay(2000);
    
    // read SHT3x again
    currentTemp = sht3x.getTemperature();
    currentRh = sht3x.getHumidity();

    // filter out incorrect SHT3x reading
    if(currentTemp > -100 && currentRh >= 0)
    {
      // once the air conditioning is OFF, keep the state for a certain amount of time
      // because, IR control is noisy
      unsigned int timeElaspedInMinutes = timeElapsed / (long) 60000;
      if(!just_off || timeElaspedInMinutes > KEEP_OFF_TIMER)
      {
        // determine the air conditioning state
        if(isOn == MODE_OFF)
        {
          // assume current air conditioning is OFF
          // temperature priority
          if((float) currentTemp >= (float) TEMP_HIGH)
          {
            resend = 0;
            resend_attempts = 1;
            daikin_cooling_on();
          }
          else if(currentRh >= RH_HIGH)
          {
            resend = 0;
            resend_attempts = 1;
            daikin_dehumidifier_on();
          }
        }
        else if(isOn == MODE_COOLING)
        {
          // assume current air condition in running cooling
          if(((float) currentTemp) < ((float) TEMP_LOW) && currentRh <= RH_LOW)
          {
            resend = 0;
            resend_attempts = 2;
            daikin_all_off();      
            just_off = true;
          }
          else if(((float) currentTemp) < ((float) TEMP_LOW) && currentRh > RH_LOW)
          {
            resend = 0;
            resend_attempts = 1;
            daikin_dehumidifier_on();
          }
        }
        else // MODE_DEHUMIDIFIER
        {
          // assume current air conditioning is running dehumidifier
          if((((float) currentTemp) <= ((float) TEMP_LOW) && currentRh < RH_LOW) ||       // meet both criterias
             (((float) currentTemp) <= ((float) TEMP_TOO_LOW)))                           // humidifier goes too far, stop the air conditioning temperarily
          {
            resend = 0;
            resend_attempts = 2;
            daikin_all_off();      
            just_off = true;
          }
          else if((((float) currentTemp) > ((float) TEMP_LOW) && currentRh < RH_LOW) ||   // humidity reaches low but temperature is higher than low
                  ((float) currentTemp >= (float) TEMP_HIGH))                             // temperature reaches high no matter how much humidity is
          {
            resend = 0;
            resend_attempts = 1;
            daikin_cooling_on();
          }
        }
      } // end if(!just_off || ...)
      else
      {
        if(((float) currentTemp) >= ((float) TEMP_TOO_HIGH) || currentRh >= RH_TOO_HIGH)
        {
          // ambient condition exceeds the limit, ignore the KEEP_OFF_TIMER
          just_off = false;
        }
      }
    } // end if(currentTemp > -100 ...)
  } // end if(!qmoteDisconnected)

  // delay a minutes until next reading unless the Qmote status in in the disconnecting state
  if(!disconnecting && !startup_calibration)
  {
    delay(60000);
  }
  else
  {
    delay(3000);
  }

  // if this is the first time get here, 
  // issue more IR attempts for the IR direction calibration
  if(startup_calibration)
  {
    resend_attempts = 3;

    // calibration window time is up
    if(resend >= 2)
    {
      startup_calibration = false;
    }
  }

  // Since IR has no feedback, multiple sends to ensure the command was received
  if(resend < resend_attempts)
  {
    switch(isOn)
    {
      case MODE_COOLING:
        daikin_cooling_on();
        break;

      case MODE_DEHUMIDIFIER:
        daikin_dehumidifier_on();
        break;
        
      case MODE_OFF:
      default:
        daikin_all_off();
        break;
    }

    delay(5000);
  }
}
