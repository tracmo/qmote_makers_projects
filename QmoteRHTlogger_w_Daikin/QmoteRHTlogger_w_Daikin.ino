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
int resend;

// Daikin Control
IRdaikin irdaikin;
int isOn = 0;

//  ambient environmental parameters
#define TEMP_SETTING          24          // cooling setting to this temperature
#define HEAT_INDEX_HIGH       28          // restart cooling if temperature reaches this measurement
#define HEAT_INDEX_TOO_HIGH   30          // use Heat Index to check if the ambient condition needs to be changed, if so, KEEP_OFF_TIMER will be ignored
#define TEMP_HIGH             26.5        // Highheat index may be caused by Rh, set High limitation to determine cooling or dehumidifier
#define TEMP_LOW              25          // stop cooling or switch to humidifier if temerature reaches this measurement
#define TEMP_TOO_LOW          23.5        // if temperature reaches this value means humidifier goes too much, turn off everything
#define RH_HIGH               69          // restart humidifier if Rh reaches to this measurement
#define RH_LOW                60          // stop humidifier if Rh reaches to this measurement

// avoid frequent ON-OFF mode switch
#define KEEP_ONOFF_TIMER      45
#define RESEND_ATTEMPS        3

#define MODE_OFF              0
#define MODE_COOLING          1
#define MODE_DEHUMIDIFIER     2

// Elapsed Time Control
elapsedMillis timeElapsed;

/**
 *  Setup
 */
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

  // needs no UART echo
  String qmoteCmd = "ATE0\r\n";
  portOne.write(qmoteCmd.c_str());
  delay(3000);

  // skip the KEEP_ONOFF_TIMER to initiate the first mode check
  timeElapsed = (long) KEEP_ONOFF_TIMER * (long) 60000;

  // send the first one
  qmoteCmd = "ATBN=0x0A,\"RhT Starts\"\r\n";      // using click combination code 0x0A
  portOne.write(qmoteCmd.c_str());                // output to Maker's module

  // delay extra 10 seconds so the first RhT information can be sent to Qmote
  delay(10000);
}

/**
 *  Flash Qmote LED
 */
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

/**
 *  WeMo ON
 */
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

/**
 *  WeMo OFF
 */
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

/**
 *  Switch to cooling mode
 */
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

  // report this state change
  qmoteCmd = "ATBN=0x0A,\"cooling ON\"\r\n";      // using click combination code 0x0A, --..
  portOne.write(qmoteCmd.c_str());                // output to Maker's module

  // flash Qmote LED as indicator
  flash_LED();

  // turn off FAN through WeMo Switch
  wemo_fan_off();
}

/**
 *  Switch to dehumidifier mode
 */
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

  // report this state change
  qmoteCmd = "ATBN=0x0A,\"dehumidifier ON\"\r\n"; // using click combination code 0x0A, --..
  portOne.write(qmoteCmd.c_str());                // output to Maker's module

  // flash Qmote LED as indicator
  flash_LED();

  // turn off FAN through WeMo Switch
  wemo_fan_off();
}

/**
 *  Air conditioning OFF
 */
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

  // report this state change
  qmoteCmd = "ATBN=0x0A,\"air cond OFF\"\r\n";    // using click combination code 0x0A, --..
  portOne.write(qmoteCmd.c_str());                // output to Maker's module

  // flash Qmote LED as indicator
  flash_LED();

  // turn on FAN through WeMo Switch
  wemo_fan_on();
}

/**
 * Heat Index Calculator
 * Code based on Robtillaart's post on http://forum.arduino.cc/index.php?topic=107569.0
 */
float heatIndex(double tempC, double humidity)
{
 double c1 = -42.38, c2 = 2.049, c3 = 10.14, c4 = -0.2248, c5= -6.838e-3, c6=-5.482e-2, c7=1.228e-3, c8=8.528e-4, c9=-1.99e-6;
 double T = (tempC * ((double) 9 / (double) 5)) + (double) 32;
 double R = humidity;

 double A = (( c5 * T) + c2) * T + c1;
 double B = ((c7 * T) + c4) * T + c3;
 double C = ((c9 * T) + c8) * T + c6;

 double rv = (C * R + B) * R + A;
 double rvC = (rv - (double) 32) * ((double) 5 / (double) 9);
 
 return ((float) rvC);
}

/**
 *  Main loop
 */
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

  // ---------------------------------------------------------------------------------------------------------------------------------
  // Sending reports
  // ---------------------------------------------------------------------------------------------------------------------------------

  // filter out incorrect SHT3x reading
  if(currentTemp > -100 && currentRh >= 0)
  {
    // get Qmote command for plaintext output
    unsigned int currentRhInt = (unsigned int) currentRh;
    float currentHeatIndex = heatIndex((double) currentTemp, (double) currentRh);

    /*
    //debug output
    unsigned int teim = timeElapsed / (long) 60000;
    qmoteCmd = "ATBN=0x0A,\"";        // using click combination code 0x0A, --..
    qmoteCmd += isOn;
    qmoteCmd += ",";
    qmoteCmd += teim;
    qmoteCmd += "\"\r\n";
    portOne.write(qmoteCmd.c_str());
    delay(3000);
    */

    // RhT logging in text file
    qmoteCmd = "ATBN=0x0A,\"RH";      // using click combination code 0x0A, --..
    qmoteCmd += currentRhInt;
    qmoteCmd += ",T";
    qmoteCmd += currentTemp;
    qmoteCmd += ",HI";
    qmoteCmd += currentHeatIndex;
    qmoteCmd += "\"\r\n";
    portOne.write(qmoteCmd.c_str());    // output to Maker's module
    delay(2000);                        // delay before next command

    // get Qmote command for ThingSpeak output
    qmoteCmd = "ATBN=0x09,\"";        // using click combination code 0x09, -...
    qmoteCmd += currentRh;            // To save UartMsg size, leave "field1=" to IFTTT
    qmoteCmd += "&field2=";
    qmoteCmd += currentTemp;
    qmoteCmd += "\"\r\n";
  
    // output to Maker's module
    portOne.write(qmoteCmd.c_str());
  }

  // ---------------------------------------------------------------------------------------------------------------------------------
  // Processing incoming commands
  // ---------------------------------------------------------------------------------------------------------------------------------

  // check if Qmote has incoming notifications or commands
  while (portOne.available() > 0)
  {
    char c = portOne.read();
    
    if(c == 0x0D)
    {
      // null terminated
      recvLine += 0x00;

      // reserved for the future command processing use
      
      // clear the line
      recvLine = "";
    }
    else if(c != 0x0A)
    {
      recvLine += c;
    }
    // ignore 0x0A
  }

  // =================================================================================================================================
  // PROCEED THE ENVIRONMENT CONTROL
  // =================================================================================================================================

  // delay before next command and sample the data again
  sht3x.readSample();
  delay(2000);
  
  // read SHT3x again
  currentTemp = sht3x.getTemperature();
  currentRh = sht3x.getHumidity();

  // filter out incorrect SHT3x reading
  if(currentTemp > -100 && currentRh >= 0)
  {
    // Calculate Heat Index
    float curHeatIndex = heatIndex((double) currentTemp, (double) currentRh);
    
    // the following conditions will ignore the ON-OFF timer
    if(
       (isOn == MODE_OFF && ((float) curHeatIndex) >= ((float) HEAT_INDEX_TOO_HIGH)) ||                   // heat index too high
       (isOn != MODE_OFF && ((float) currentTemp) <= ((float) TEMP_TOO_LOW)) ||                           // temperature too low
       (isOn == MODE_COOLING && ((float) currentTemp) < ((float) TEMP_LOW) && currentRh > RH_LOW) ||      // cooling -> dehumidifier
       (isOn == MODE_DEHUMIDIFIER &&                                                                      // dehumidifier -> cooling
        ((((float) currentTemp) > ((float) TEMP_LOW) && currentRh < RH_LOW) ||
         ((float) curHeatIndex >= (float) HEAT_INDEX_HIGH)))
      )
    {
      // ambient condition exceeds the limit, skip the KEEP_ONOFF_TIMER
      timeElapsed = (long) KEEP_ONOFF_TIMER * (long) 60000;
    }

    // once the air conditioning mode is switched, keep the state for a certain amount of time
    // otherwise, it will be annoyed
    unsigned int timeElaspedInMinutes = timeElapsed / (long) 60000;
    if(timeElaspedInMinutes >= KEEP_ONOFF_TIMER)
    {
      // Since there is no beep now (I took off the buzzer on the air conditioning set), resend the same command every KEEP_ONOFF_TIMER in case the previous IR is missed
      resend = 0;
      
      // determine the air conditioning state
      if(isOn == MODE_OFF)
      {
        // assume current air conditioning is OFF
        // heat index (temperature) priority
        if((float) curHeatIndex >= (float) HEAT_INDEX_HIGH)
        {
          // I guess this is a Winter Mode
          if(((float) currentTemp) > ((float) TEMP_HIGH))
          {
            daikin_cooling_on();
            timeElapsed = 0;  // reset mode-switch timer
          }
          else
          {
            daikin_dehumidifier_on();
            timeElapsed = 0;  // reset mode-switch timer
          }
        }
      }
      else if(isOn == MODE_COOLING)
      {
        // assume current air condition in running cooling
        if(((float) currentTemp) < ((float) TEMP_LOW) && currentRh <= RH_LOW)
        {
          daikin_all_off();
          timeElapsed = 0;  // reset mode-switch timer
        }
        else if(((float) currentTemp) < ((float) TEMP_LOW) && currentRh > RH_LOW)
        {
          daikin_dehumidifier_on();
          timeElapsed = 0;  // reset mode-switch timer
        }
      }
      else // MODE_DEHUMIDIFIER
      {
        // assume current air conditioning is running dehumidifier
        if((((float) currentTemp) <= ((float) TEMP_LOW) && currentRh < RH_LOW) ||       // meet both criterias
           (((float) currentTemp) <= ((float) TEMP_TOO_LOW)))                           // humidifier goes too far, stop the air conditioning temperarily
        {
          daikin_all_off();
          timeElapsed = 0;  // reset mode-switch timer
        }
        else if((((float) currentTemp) > ((float) TEMP_LOW) && currentRh < RH_LOW) ||   // humidity reaches low but temperature is higher than low
                ((float) currentTemp > (float) TEMP_HIGH))                              // if high temp reached, switch to cooling
        {
          daikin_cooling_on();
          timeElapsed = 0;  // reset mode-switch timer
        }
      }
    } // end if(timeElaspedInMinutes >= KEEP_ONOFF_TIMER)
  } // end if(currentTemp > -100 ...)

  // =================================================================================================================================

  // delay a minutes until next reading
  delay(60000);

  // Since IR has no feedback, multiple sends to ensure the command was received
  if(resend < RESEND_ATTEMPS)
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

