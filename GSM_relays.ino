#include <gprs.h>
#include <SoftwareSerial.h>
 
#define TIMEOUT    5000
const int relay1 = 7;
const int relay2 = 10;
bool relay1status; 
bool relay2status;

GPRS gprs;
 
void setup()
{
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  
  Serial.begin(19200);
  
  while(!Serial);
 
  Serial.println("Starting SIM800 SMS Command Processor");
  gprs.preInit();
  delay(1000);
 
  while(0 != gprs.init())
  {
      delay(1000);
      Serial.print("init error\r\n");
  } 
 
  //Set SMS mode to ASCII
  if(0 != gprs.sendCmdAndWaitForResp("AT+CMGF=1\r\n", "OK", TIMEOUT)) {
    ERROR("ERROR:CNMI");
    return;
  }
   
  //Start listening to New SMS Message Indications
  if(0 != gprs.sendCmdAndWaitForResp("AT+CNMI=1,2,0,0,0\r\n", "OK", TIMEOUT)) {
    ERROR("ERROR:CNMI");
    return;
  }
 
  Serial.println("Init success");
}
 
//Variable to hold last line of serial output from SIM800
char currentLine[500] = "";
int currentLineIndex = 0;
 
//Boolean to be set to true if message notificaion was found and next
//line of serial output is the actual SMS message content
bool nextLineIsMessage = false;
 
void loop()
{
  //Write current status to LED pin
  digitalWrite(relay1, relay1status);
  digitalWrite(relay2, relay2status);
   
  //If there is serial output from SIM800
  if(gprs.serialSIM800.available())
  {
    char lastCharRead = gprs.serialSIM800.read();
    //Read each character from serial output until \r or \n is reached (which denotes end of line)
    if(lastCharRead == '\r' || lastCharRead == '\n')
    {
        String lastLine = String(currentLine);
         
        //If last line read +CMT, New SMS Message Indications was received.
        //Hence, next line is the message content.
        if(lastLine.startsWith("+CMT:"))
        {
          Serial.println(lastLine);
          nextLineIsMessage = true;
           
        }
        else if (lastLine.length() > 0)
        {
           
          if(nextLineIsMessage)
          {
            Serial.println(lastLine);
             
            //Read message content and set status according to SMS content
            if(lastLine.indexOf("Vklop dva") >= 0)
            {
              relay1status = 1;
            }
            else if(lastLine.indexOf("Izklop dva") >= 0)
            {
              relay1status = 0;
            }
            else if(lastLine.indexOf("Vklop ena") >= 0)
            {
              relay2status = 1;
            }
            else if(lastLine.indexOf("Izklop ena") >= 0)
            {
              relay2status = 0;
            }
            nextLineIsMessage = false;
          }
           
        }
         
        //Clear char array for next line of read
        for( int i = 0; i < sizeof(currentLine);  ++i )
        {
         currentLine[i] = (char)0;
        }
        currentLineIndex = 0;
    }
    else
    {
      currentLine[currentLineIndex++] = lastCharRead;
    }
  }
}
