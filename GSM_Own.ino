#include <avr/pgmspace.h>
#include "Arduino.h"
#include <SoftwareSerial.h> // Software serial library
#include "TimerOne.h"   // Used for timer interrupts

String PhoneNumber = "";

String _buffer;
uint8_t timeout;
String text, SendBuffer = "";
String GetNumber;
uint8_t SMSIndex = 0;

int BaudRate = 9600;
byte Relay1 = 10;     // Relay 1 is connected to pin 10
byte Relay2 = 7;      // Relay 1 is connected to pin 7
byte PWRKey = 6;
String RL1_State = "Off";   // State of relay 1
String RL2_State = "Off";   // State of relay 2
volatile int RL1_Time = 0, RL2_Time = 0;

SoftwareSerial GSMBoard(8,9); // RX, TX
/*
 * ADC pin init
 */
#define ADC_15V    A0
#define ADC_30V    A1
#define ADC_TEMP    A2

float fData15V = 0;
float fData30V = 0;
float Temperature = 0;

#define ZERO15V 0
#define SPAN15V 1024

#define ZERO30V 0
#define SPAN30V 1024

#define ZEROTEMP 0
#define SPANTEMP 1024

// Interrupt variables
int RL1_ForcedOff, RL2_ForcedOff;
void setup()
{
  Serial.begin(BaudRate);
  // Initialize Timer
  Timer1Initialize();
  RelayInitialize();
  // Initialize the GSM
  InitializeGSM();    // Initialize GSM
  SendSMS("Hello World",PhoneNumber); // Send Test SMS
  DeleteAllSMS();       // Delete all previous SMS
}

void loop()
{
  fncReadTempV();
  text = ReadSMSContent(1);     // Read SMS text from index 1
  GetNumber = ReadSMSNumber(1);
  if(GetNumber.length() > 5)
  {
    PhoneNumber = GetNumber; // Reads the phone number
  }
  if (text.indexOf("One on")!=-1)   // if Relay 1 is on
  {
    DeleteAllSMS();
    RL1_State = "On";
    Serial.println("Relay 1 Turned On");
    SendSMS("Relay 1 Turned On", PhoneNumber);
    digitalWrite(Relay1,HIGH);
    // 7-10 characters are for time
    if(RL1_Time == 0)
    {
      RL1_Time = text.substring(7,10).toInt();
      RL1_Time *= 60;   // Convert to seconds
      if(RL1_Time > 0)
      {
        RL1_ForcedOff = 1;
        SendBuffer = "Relay 1 will remain ON for " + (String)RL1_Time + " seconds";
        SendSMS(SendBuffer, PhoneNumber);
      }
    }
  }
  else if (text.indexOf("One off")!=-1)   // if Relay 1 is off
  {
    DeleteAllSMS();
    RL1_State = "Off";
    Serial.println("Relay 1 Turned Off");
    SendSMS("Relay 1 Turned Off", PhoneNumber);
    digitalWrite(Relay1,LOW);
  }
  else if (text.indexOf("Two on")!=-1)   // if Relay 2 is on
  {
    DeleteAllSMS();
    RL2_State = "On";
    Serial.println("Relay 2 Turned On");
    SendSMS("Relay 2 Turned On", PhoneNumber);
    digitalWrite(Relay2,HIGH);
    // 7-10 characters are for time
    if(RL2_Time == 0)
    {
      RL2_Time = text.substring(7,10).toInt();
      RL2_Time *= 60;   // Convert to seconds
      if(RL2_Time > 0)
      {
        RL2_ForcedOff = 1;
        SendBuffer = "Relay 2 will remain ON for " + (String)RL2_Time + " seconds";
        SendSMS(SendBuffer, PhoneNumber);
      }
    }
  }
  else if (text.indexOf("Two off")!=-1)   // if Relay 2 is off
  {
    DeleteAllSMS();
    RL2_State = "Off";
    Serial.println("Relay 2 Turned Off");
    SendSMS("Relay 2 Turned Off", PhoneNumber);
    digitalWrite(Relay2,LOW);
  }
  else if(text.indexOf("Status") != -1)   // if Status is asked
  {
    DeleteAllSMS();
    SendBuffer = "Relay 1 is " + RL1_State + "\rRelay 2 is " + RL2_State + "\rTemperature is " + Temperature;
    SendSMS(SendBuffer, PhoneNumber);
    SendBuffer = "";
  }
  else if(text.length() > 0 && GetNumber.length() != 0)
  {
    SendSMS("Wrong Command! Please Enter Correct Command", PhoneNumber);
    DeleteAllSMS();
  }
  if(RL1_Time == 0 && RL1_ForcedOff == 1)
  {
    RL1_ForcedOff = 0;
    RL1_State = "Off";
    Serial.println("Relay 1 Turned Off");
    SendSMS("Relay 1 Turned Off", PhoneNumber);
    digitalWrite(Relay1,LOW);
  }
  if(RL2_Time == 0 && RL2_ForcedOff == 1)
  {
    RL2_ForcedOff = 0;
    RL2_State = "Off";
    Serial.println("Relay 2 Turned Off");
    SendSMS("Relay 2 Turned Off", PhoneNumber);
    digitalWrite(Relay2,LOW);
  }
}
void InitializeGSM(void)
{
  _buffer.reserve(255); // Declare a buffer of 255 bytes
  GSMBoard.begin(BaudRate);
  pinMode(PWRKey, OUTPUT);
  digitalWrite(PWRKey, LOW);  // Initially low (The transistor is off)
  ResetGSM();
  GSMBoard.print("ATE0\r");   // Echo off
  GSMBoard.print("AT");
  GSMBoard.print("\r");
  if(ReadGSMSerial(1000).indexOf("OK"))
    Serial.println("GSM Initialized");
  else
  {
    Serial.println("GSM Initialization Error!");
    Serial.println("Check connections and baudrate of GSM");
  }
}
void ResetGSM(void)
{
  digitalWrite(PWRKey, HIGH); 
  delay(1200);
  digitalWrite(PWRKey, LOW);  //
  delay(10000);
}
String ReadGSMSerial(int Delay)
{
  timeout = 0;
  while(!GSMBoard.available() && timeout < Delay) 
  {
    delay(13);
    timeout++;
  }
  if(timeout >= Delay)
    setup();
  if (GSMBoard.available()) {
   return GSMBoard.readString();
  }
}
bool SendSMS(String text, String number)
{
  _buffer ="";
  GSMBoard.print("AT+CMGF=1\r"); //set sms to text mode  
  _buffer=ReadGSMSerial(500);
  GSMBoard.print ("AT+CMGS=\"");  // command to send sms
  GSMBoard.print (number);           
  GSMBoard.print("\"\r");   
  _buffer=ReadGSMSerial(500);
  GSMBoard.print (text);
  //GSMBoard.print ("\r"); 
  //change delay 100 to readserial
  GSMBoard.print((char)26);
  _buffer =""; 
  _buffer=ReadGSMSerial(2000);
  //expect CMGS:xxx   , where xxx is a number,for the sending sms.
  if (((_buffer.indexOf("CMGS") ) != -1 ) )
  {
    Serial.println("SMS Sent");
    return true;
  }
  else
  {
    Serial.println("SMS Sending Error");
    return false;
  }
}
String ReadSMS(uint8_t index)
{
  _buffer = "";
  GSMBoard.print("AT+CMGF=1\r");
  _buffer=ReadGSMSerial(500);
  GSMBoard.print("AT+CMGR=");
  GSMBoard.print(index);
  GSMBoard.print("\r");
  _buffer = "";
  _buffer=ReadGSMSerial(3000);
  if (_buffer.indexOf("CMGR:")!=-1)
  {
    return _buffer;
  }
  else 
    return "";    
}
String ReadSMSContent(uint8_t index)
{
  _buffer = "";
  GSMBoard.print("AT+CMGF=1\r");
  _buffer=ReadGSMSerial(500);
  GSMBoard.print("AT+CMGR=");
  GSMBoard.print(index);
  GSMBoard.print("\r");
  _buffer = "";
  _buffer=ReadGSMSerial(2000);
  uint8_t _idx1=_buffer.indexOf("+",30); // Starting from index number 30
  return _buffer.substring(_idx1+6,_buffer.indexOf("OK",_idx1)-4);  // +00" Text SMS OK
}
String ReadSMSNumber(uint8_t index)
{
  _buffer=ReadSMS(index);
  if (_buffer.length() > 10) //avoid empty sms
  {
    uint8_t _idx1=_buffer.indexOf("+",5); // Starting from index number 2
    return _buffer.substring(_idx1,_buffer.indexOf("\",\"",_idx1+10));
  }
  else
    return "";
}
bool DeleteAllSMS()
{ 
  _buffer = "";
  GSMBoard.print("AT+CMGDA=\"DEL ALL\"\r");
  _buffer=ReadGSMSerial(2000);
  if(_buffer.indexOf("OK")!=-1)
    return true;
  else
    return false;
}
// ADC Functions
void fncADCRead(){
    fncRead15V();
    fncRead30V();
    fncReadTempV();
 }

 void fncRead15V()
 {
    float fDat = analogRead(ADC_15V);

    fData15V = fDat - ZERO15V;
    fData15V *= 15.00f;
    fData15V /= (SPAN15V - ZERO15V);
 }

 void fncRead30V()
 {
    float fDat = analogRead(ADC_30V);

    fData30V = fDat - ZERO30V;
    fData30V *= 30.00f;
    fData30V /= (SPAN30V - ZERO30V);
 }

 void fncReadTempV()
 {
  float temp = analogRead(ADC_TEMP)*5/1024.0;
  temp = temp - 0.5;
  temp = temp / 0.01;
  Temperature = temp;
 }
 // Timer Functions
 void Timer1Initialize(void)
{
  Timer1.initialize(1000000);         // initialize timer1, and set a 1 second period
  Timer1.attachInterrupt(Timer1_Handler);  // attaches callback() as a timer overflow interrupt
}
void Timer1_Handler(void)
{
  if(RL1_Time > 0)
  {
    RL1_Time--;
  }
  if(RL2_Time > 0)
  {
    RL2_Time--;
  }
}
void RelayInitialize(void)
{
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  digitalWrite(Relay1, LOW);
  digitalWrite(Relay2, LOW);
}
