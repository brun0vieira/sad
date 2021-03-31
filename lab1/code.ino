int LED_1 = 10;
int LED_2 = 11;
int LDR_1 = A0; 
int LDR_2 = A1; 
int MOTOR_1 = 6;
int MOTOR_2 = 9;
int ENABLE = 12;

int ldr_1_val;
int ldr_2_val;
int baixa_luminosidade = 20;
int limite = 50;

bool status = true;
bool monitoring = false;
int state_before = 0;

String json;

void setup()
{
  Serial.begin(9600); // Initialize serial with speed = 9600
  pinMode(ENABLE, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(MOTOR_1, OUTPUT);
  pinMode(MOTOR_2, OUTPUT);
  pinMode(LDR_1, INPUT);
  pinMode(LDR_2, INPUT);
  digitalWrite(ENABLE, HIGH);
}

void loop()
{ 
  if(status)
    solarTracker();

  readMessage();
  delay(5000);
  
}

void debug(int if_statement) 
{
  Serial.print("Ciclo ");
  Serial.println(if_statement);
  Serial.println("LDR_1:");
  Serial.println(ldr_1_val);
  Serial.println("LDR_2:");
  Serial.println(ldr_2_val);
}

// Example: for {"C":8,"S1":500,"S2":520} we get the following array: commandsArray = {"C","8","S1","500","S2","520"}
void readJson(String json) 
{
  String commands="";
  int length, no_of_commands=0;
  
  for(int i=0; i<json.length(); i++)
  {
    if(json[i]=='{')
    {
      continue;
    }
    else if(json[i]=='"')
    {
      i++;
      while(json[i]!='"')
      {
        commands+=json[i];
        i++;
      }
      commands+='-';
    }
    else if(json[i]==':')
    {
      i++;
      if(json[i]=='"')
      {
      	i++;
        while(json[i]!='"')
        {
          commands+=json[i];
          i++;
        }
        commands+='-';
        no_of_commands++;
      }
      else
      {
        while(json[i]!=',' && json[i]!='}')
        {
          commands+=json[i];
          i++;
        }
        commands+='-';
        no_of_commands++;
      }
    }
    else if(json[i]==',')
    {
      continue;
    }
    else {
      break;
    }  
  }
  
  String commandsArray[no_of_commands*2];
  String aux = "";
  int j=0;
  
  for(int i=0; i<commands.length(); i++)
  {
  	if(commands[i]!='-')
    {
      aux += commands[i];
    }
    else
    {
      commandsArray[j] = aux;
      j++;
      aux = "";
    }
  }
  
  // Now we can use the commandsArray ...
  int lengthArray = sizeof(commandsArray)/sizeof(commandsArray[0]);
  runJson(lengthArray, commandsArray);
}

void solarTracker()
{
  ldr_1_val = analogRead(LDR_1);
  ldr_2_val = analogRead(LDR_2);
  
  if(ldr_1_val < baixa_luminosidade && ldr_2_val < baixa_luminosidade)
  {
    turnOffMotorAndLeds();
  }
  else if(ldr_1_val - ldr_2_val > limite) 
  {
    do{
      if(status){
        moveLeft();
        delay(1000);
        ldr_1_val = analogRead(LDR_1);
        ldr_2_val = analogRead(LDR_2);
        readMessage();
      }
      state_before=1;
    } while(ldr_1_val - ldr_2_val > limite && status==true);
  }
  else if(ldr_2_val - ldr_1_val > limite)
  {
    do{
      if(status){
        moveRight();
        delay(1000);
        ldr_1_val = analogRead(LDR_1);
        ldr_2_val = analogRead(LDR_2);
        readMessage();
      }
      state_before=2;
    } while(ldr_2_val - ldr_1_val > limite && status==true);
  }
  else 
  {
    turnOffMotorAndLeds();
    delay(1000);
    state_before=3;
  }
}

void runJson(int lengthArray, auto commandsArray)
{
  int switchCase;
  for(int i=0; i<lengthArray; i+=2)
  {
    
    if(commandsArray[i]=="C")
    {
      switchCase = commandsArray[i+1].toInt();
      
      switch(switchCase)
      {
        case 1: 
      		sendLdrValues(ldr_1_val,ldr_2_val);
      		break;
      	case 2:
        	activateMonitoring();
        	acknowledge();
        	break;
      	case 3:
      		deactivateMonitoring();
        	acknowledge();
      		break;
      	case 4:
      		activateNormal();
        	acknowledge();
      		break;
      	case 5:
      		activateStandby();
        	acknowledge();
      		break;
      	case 6:
			if(commandsArray[i+2]=="B")
            {
              setLowLuminosity(commandsArray[i+3].toInt());
              acknowledge();
              i += 2;
            }
      		break;
      	case 7:
      		if(commandsArray[i+2]=="L")
            {
              setDiffLimit(commandsArray[i+3].toInt());
              acknowledge();
              i += 2;
            }
      		break;
      }
    }
  }
}

void sendLdrValues(int ldr1, int ldr2) 
{
  Serial.print("{\"C\":8,\"S1\":");
  Serial.print(ldr1);
  Serial.print(",\"S2\":");
  Serial.print(ldr2);
  Serial.print("}");
  Serial.println("");
}

void setDiffLimit(int limit)
{
  limite = limit;
}

void setLowLuminosity(int luminosity)
{
  baixa_luminosidade = luminosity;
}

void activateStandby()
{
	turnOffMotorAndLeds();
  	status = false;
}

void activateNormal()
{
	status = true;
}

void turnOffMotorAndLeds()
{
	digitalWrite(MOTOR_1, LOW);
    digitalWrite(MOTOR_2, LOW);
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
  	if(monitoring && state_before!=3)
       Serial.println("{\"C\":9,\"M\":\"P\"}");
}

void moveLeft()
{
	digitalWrite(MOTOR_1, HIGH);
    digitalWrite(MOTOR_2, LOW);
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, LOW);
    if(monitoring && state_before!=1)
       Serial.println("{\"C\":9,\"M\":\"E\"}");
}

void moveRight()
{
	digitalWrite(MOTOR_1, LOW);
    digitalWrite(MOTOR_2, HIGH);
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, HIGH);
    if(monitoring && state_before!=2)
      Serial.println("{\"C\":9,\"M\":\"D\"}");
}

void readMessage() 
{
  json = Serial.readString();
  if(json!=NULL)
  {
    readJson(json);
  }
} 

void acknowledge() 
{
  Serial.println("{\"C\":0}");
} 

void activateMonitoring() 
{
  monitoring = true;
}

void deactivateMonitoring() 
{
  monitoring = false;
}