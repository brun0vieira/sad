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
  //solarTracker();
  /*json = Serial.readString();
  if(json!=NULL)
  {
  	readJson(json);
  }*/
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

void sendLdrValues(int ldr1, int ldr2) 
{
  Serial.print("{\"C\":8,\"S1\":");
  Serial.print(ldr1);
  Serial.print(",\"S2\":");
  Serial.print(ldr2);
  Serial.print("}");
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
  
}

void solarTracker()
{
  ldr_1_val = analogRead(LDR_1);
  ldr_2_val = analogRead(LDR_2);
  
  if(ldr_1_val - ldr_2_val > limite) 
  {
    do{
      digitalWrite(MOTOR_1, HIGH);
      digitalWrite(MOTOR_2, LOW);
      digitalWrite(LED_1, HIGH);
      digitalWrite(LED_2, LOW);
      delay(1000);
      ldr_1_val = analogRead(LDR_1);
  	  ldr_2_val = analogRead(LDR_2);
      debug(1);
    } while(ldr_1_val - ldr_2_val > limite);
  }
  else if(ldr_2_val - ldr_1_val > limite)
  {
    do{
      digitalWrite(MOTOR_1, LOW);
      digitalWrite(MOTOR_2, HIGH);
      digitalWrite(LED_1, LOW);
      digitalWrite(LED_2, HIGH);
      delay(1000);
      ldr_1_val = analogRead(LDR_1);
  	  ldr_2_val = analogRead(LDR_2);
      debug(2);
    } while(ldr_2_val - ldr_1_val > limite);
  }
  else 
  {
    digitalWrite(MOTOR_1, LOW);
    digitalWrite(MOTOR_2, LOW);
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    delay(1000);
    debug(3);
  }
}
