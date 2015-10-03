int leftMotorEnable   = D1;
int rightMotorEnable  = A7;
int leftMotorDir    = D3;
int rightMotorDir   = D4;

int status = 0; // 1:FORWARD 2:BACK 3:RIGHT 4:LEFT

void setup()
{
  //Register Spark function
  Spark.function("rccar", rcCarControl);

  pinMode(leftMotorDir, OUTPUT);
  pinMode(leftMotorEnable, OUTPUT);
  pinMode(rightMotorDir, OUTPUT);
  pinMode(rightMotorEnable, OUTPUT);

  pinMode(D7,OUTPUT);
}

void loop()
{
  if(status == 1 || status == 2)
  {
      delay(4000);
      rcCarStop();
      status = 0;
  }
    if(status == 3 || status == 4)
  {
      delay(500);
      rcCarStop();
      status = 0;
  }
  
}

void rcCarStop()
{
    digitalWrite(leftMotorEnable,LOW);
    digitalWrite(rightMotorEnable,LOW);

    digitalWrite(leftMotorDir,LOW);
    digitalWrite(rightMotorDir,LOW);
}

/*******************************************************************************
 * Function Name  : rcCarControl
 * Description    : Parses the incoming API commands and sets the motor control
          pins accordingly
 * Input          : RC Car commands
          e.g.: rc,FORWARD
            rc,BACK
 * Output         : Motor signals
 * Return         : 1 on success and -1 on fail
 *******************************************************************************/
int rcCarControl(String command)
{
  if(command.substring(3,7) == "STOP")
  {
    digitalWrite(leftMotorEnable,LOW);
    digitalWrite(rightMotorEnable,LOW);

    digitalWrite(leftMotorDir,LOW);
    digitalWrite(rightMotorDir,LOW);

    return 1;
  }

  if(command.substring(3,7) == "BACK")
  {
    digitalWrite(leftMotorDir,LOW);
    digitalWrite(rightMotorDir,HIGH);

    digitalWrite(leftMotorEnable,HIGH);
    digitalWrite(rightMotorEnable,HIGH);

    status = 2;
    return 1;
  }

  if(command.substring(3,10) == "FORWARD")
  {
    digitalWrite(leftMotorDir,HIGH);
    digitalWrite(rightMotorDir,LOW);

    digitalWrite(leftMotorEnable,HIGH);
    digitalWrite(rightMotorEnable,HIGH);
    
    status = 1;
    return 1;
  }

  if(command.substring(3,8) == "RIGHT")
  {
    digitalWrite(leftMotorDir,HIGH);
    digitalWrite(rightMotorDir,HIGH);

    digitalWrite(leftMotorEnable,HIGH);
    digitalWrite(rightMotorEnable,HIGH);

status = 3;
    return 1;
  }

  if(command.substring(3,7) == "LEFT")
  {
    digitalWrite(leftMotorDir,LOW);
    digitalWrite(rightMotorDir,LOW);

    digitalWrite(leftMotorEnable,HIGH);
    digitalWrite(rightMotorEnable,HIGH);
status = 4;
    return 1;
  }

  // If none of the commands were executed, return false
  return -1;
}