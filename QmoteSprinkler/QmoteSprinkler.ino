/*
  Qmote Sprinkler

  A water sprinkler control with a web server on the Arduino Ethernet Shield.
  This sprinkler control module can be controlled by IFTTT working with Qmote by Qblinks

  For more information about this project, please refer to http://qblinks.com/devkit/makers/project-2-the-weather-controlled-sprinkler
  
  Command To Trigger this action looks like:
  http://mysprinkler.qblinks.com/?pw=mypassword&water=10&weather={{CurrentCondition}}
  
  Where:
  mypassword is the password hard coded in this code
  10 is the time in minutes to turn on the spinkler
  {{TodaysCondition}} is the ingredient used in IFTTT for the Weather channel
  Condition inclides: Sunny, Mostly Sunny, Partly Cloudy, Partly Cloudy, Light Rain, Rain, Showers, PM Showers with or without "/Wind"
  
  Choose the conditions you want to turn on. In this example, only Sunny, Mostly Sunny will turn on the sprinkler
  If water= parameter is missing, it will return only the current status without changing the state of the relay
  If water=0, the sprinkler will be turned off immediately
  If water= and weather= are used together, weather condition must meet the hard coded condition to turn on the sprinkler based on water= timer
  
  Note: parameters are case-sensitive
  
  Author: Samson Chen, Qblinks Inc.
  Date: Feb 15th, 2015
*/

#include <SPI.h>
#include <Ethernet.h>

// MAKE YOUR PARAMETERS
// ====================================================================================================================================================================
#define  MANUAL_RUNNING_TIME     600           // running time for manually on
#define  MAX_RUNNING_TIME        3600          // max time the sprinkler can be turned on in seconds
#define  OK_WEATHER              "Sunny"       // If weather parameter exists, what condition is OK to turn on the sprinkler
                                               // Note: in this example, "Sunny" also matches to "Mostly Sunny" or "PM Sunny" since partial match is used in the code
#define  OK_WEATHER2             "Cloudy"      // Another weather condition that is OK. This includes "Partly Cloudy". Change it to "Sunny" to avoid second condition
#define  MY_PASSWORD             "password"    // password for the sprinkler control

// Enter a MAC address for your Ethernet Shield.
// Make sure it is unique. You can use MAC address generator to get one: http://www.miniwebtool.com/mac-address-generator/
byte mac[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

// Make your IP address. The IP address will be dependent on your local network:
IPAddress ip(192, 168, 1, 2);

// Initialize the Ethernet server library with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
// ====================================================================================================================================================================

int BUTTONpin = 2;
int LEDpin = 3;
int RELAYpin = 5;

#define  HEADER_LINE_SIZE        255

bool stat = false;
int lastButtonState = 0;
bool relay_state = false;
unsigned long last_trigger_time;
unsigned long relay_on_until;
int last_relay_timer;
unsigned long last_printout_timer = 0;
unsigned long last_sprinkler_on_time = 0;

char header_line[HEADER_LINE_SIZE+1];
int header_index = 0;

String pwLine = "pw=";

void setup() 
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) 
  {
    ; // do nothing, just wait for serial port to connect. Needed for Leonardo only
  }

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  
  // switch off relay and LED indicator
  pinMode(RELAYpin, OUTPUT);
  digitalWrite(RELAYpin, LOW);

  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, LOW);
  
  // get the manual button ready
  pinMode(BUTTONpin, INPUT);
  
  // get the password checking line
  pwLine.concat(MY_PASSWORD);
}


void loop() 
{
  int buttonState;
  unsigned long currentTime;
  
  // get the button status
  buttonState = digitalRead(BUTTONpin);
  
  // manual button control
  if(lastButtonState != buttonState)
  {
    // button will be handled after the board has been initialized for more than 3 seconds to avoid button-configuration casued trigger
    if(millis() > 3000)
    {
      // state change when "push"
      if(buttonState != 0)
      {
        // check previous trigger time to prevent button debouncing
        currentTime = millis();
        if((currentTime - last_trigger_time) > 200)
        {
          // switch relay state
          if(relay_state)
          {
            turn_relay_off();
          }
          else
          {
            turn_relay_on(MANUAL_RUNNING_TIME);
          }
          
          // track the debouncing time
          last_trigger_time = currentTime;
        }
      }
    }
    
    lastButtonState = buttonState;    
  }
  
  // ------------------------------------------------------------
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) 
  {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean authenticationOK = false;
    boolean checkWeather = false;
    boolean weatherOK = false;
    int waterTime = -1;
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) 
        {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");

          if(authenticationOK)
          {
            // perform actions only if the authentication passed
            if(waterTime > 0)
            {
              // if weatherParameter is used, the weather condition must meet the criteria, or no weather parameter is used to skip the weather condition check
              if((checkWeather && weatherOK) || !checkWeather)
              {
                int waterTimeSec = waterTime * 60;
                if(waterTimeSec <= MAX_RUNNING_TIME)
                {
                  turn_relay_on(waterTimeSec);
                  
                  client.print("Turn on the sprinkler for ");
                  client.print(waterTimeSec);
                  client.println(" seconds<br>");
                }
                else
                {
                  client.println("Water timer exceeds the MAX_RUNNING_TIME<br>");
                }
              }
              else
              {
                client.print("Weather condition does not meet the criteria to turn on the sprinkler<br>");
                turn_relay_off();
              }
            }
            else if(waterTime == 0)
            {
              turn_relay_off();
            }
            
            // Output the new status
            if(relay_state)
            {
              client.println("Current Status: ON<br>");
              client.print("Remaining Time: ");
              client.print((relay_on_until - millis()) / 1000);
              client.println(" seconds<br>");
            }
            else
            {
              int last_on_min = (millis() - last_sprinkler_on_time) / 1000 / 60;
              int last_on_hour = 0;
              
              while(last_on_min >= 60)
              {
                last_on_hour++;
                last_on_min -= 60;
              }
              
              client.println("Current Status: OFF<br>");

              client.print("Sprinkler was ON at ");
              client.print(last_on_hour);
              client.print(" hour(s) and ");
              client.print(last_on_min);
              client.println(" minute(s) ago.<br>");
            }
          }
          else
          {
            client.println("Bye");
          }
          
          client.println("</html>");
          break;
        }
        
        // newline received
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
          
          // make it null-terminated
          header_line[header_index] = 0;
          
          // analyze the line
          String hdr = header_line;
          // ----------------------

          // GET/HEAD method
          String methodGet = hdr.substring(0, 3);
          String methodHead = hdr.substring(0, 4);
          
          // Process only the line with Get or HEAD method
          if(methodGet.equalsIgnoreCase("GET") || methodHead.equalsIgnoreCase("HEAD"))
          {
            // check password
            int pwOK = hdr.indexOf(pwLine);
            if(pwOK > 0)
            {
              authenticationOK = true;
              
              // check water parameter
              int waterParam = hdr.indexOf("water=");
              if(waterParam > 0)
              {
                String waterTimeStr = hdr.substring(waterParam + 6);    // 6 = length of "water="
                waterTime = waterTimeStr.toInt();
              }
              
              // check weather parameter
              int weatherParam = hdr.indexOf("weather=");
              if(weatherParam > 0)
              {
                String weatherCondStr = hdr.substring(weatherParam + 8);  // 8 = length of "weather="
                int weatherCond = weatherCondStr.indexOf(OK_WEATHER);
                int weatherCond2 = weatherCondStr.indexOf(OK_WEATHER2);
                
                // weather parameter is used
                checkWeather = true;
                
                if(weatherCond >= 0 || weatherCond2 >=0)
                {
                  weatherOK = true;
                }
              }
            }

            // output to console
            Serial.println(hdr);
          }
          
          // reset the header collector
          header_index = 0;          
        }
        else if (c != '\r') 
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
          
          if(header_index < HEADER_LINE_SIZE)
          {
            header_line[header_index] = c;
            header_index++;
          }
        }
      }
    }
    
    // give the web browser time to receive the data
    delay(1);
    
    // close the connection:
    client.stop();
    
    Serial.println("client disconnected");
  }
  
  // peridically check the timer
  check_relay_timer();
}


/*
 * turn on the relay with a timer
 *
 * @param onTimer time in seconds to turn the relay on. If the relay has been on, the only change the timer
 */
void turn_relay_on(int onTimer)
{
  // check the limitation
  if(onTimer > MAX_RUNNING_TIME)
  {
    onTimer = MAX_RUNNING_TIME;
  }
  
  digitalWrite(RELAYpin, HIGH);
  digitalWrite(LEDpin, HIGH);
  relay_state = true;

  Serial.print("turn on for ");
  Serial.print(onTimer);
  Serial.println(" seconds");
  
  last_relay_timer = onTimer;
  relay_on_until = millis();

  relay_on_until += ((unsigned long) onTimer) * 1000;

  // check the previous on time
  Serial.print("Sprinkler was on at ");
  Serial.print((millis() - last_sprinkler_on_time) / 1000 / 60);
  Serial.println(" minutes ago");
  last_sprinkler_on_time = millis();
}


/*
 * Simply turn off the relay
 */
void turn_relay_off(void)
{
  digitalWrite(RELAYpin, LOW);
  digitalWrite(LEDpin, LOW);
  relay_state = false;

  Serial.println("turn off");
}


/**
 * Make sure the relay will be turned off when time is up
 */
void check_relay_timer(void)
{
  if(relay_state)
  {
    unsigned long currentTime = millis();
    
    // time up check
    if(currentTime >= relay_on_until)
    {
      turn_relay_off();
    }
    // overflow check
    else if((relay_on_until - currentTime) > (((unsigned long) last_relay_timer) * 1000))
    {
      // system clock overflow, shut the relay off
      turn_relay_off();
    }
    
    if((currentTime - last_printout_timer) > 10000)
    {
      Serial.print(currentTime);
      Serial.print(" --> ");
      Serial.println(relay_on_until);
      
      last_printout_timer = currentTime;
    }
  }
}


