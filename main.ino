/*     
 *    PINOUT: 
 *        _____________________________
 *       |  ARDUINO UNO >>>   SIM800L  |
 *        -----------------------------
 *            GND      >>>   GND
 *        RX  10       >>>   TX    
 *        TX  11       >>>   RX
 *       RESET 2       >>>   RST 
 *                 
 *   POWER SOURCE 4.2V >>> VCC
 *
 *    Created on: April 20, 2016
 *        Author: Cristian Steib
 *        
 *
*/

#include <Sim800l.h>
#include <SoftwareSerial.h> //is necesary for the library!! 

Sim800l Sim800l;  //to declare the library
extern SoftwareSerial SIM;
String textSms,numberSms,SMS;
uint8_t index;
uint8_t RELAY1 = 10, // Relay  pin
	RELAY2 = 7,
	PWR_KEY= 6; // Power Key
	
bool error;



void setup(){
 
pinMode(RELAY1,OUTPUT);
pinMode(RELAY2,OUTPUT);
pinMode(PWR_KEY,OUTPUT);

digitalWrite(RELAY1,LOW);
digitalWrite(RELAY2,LOW);

digitalWrite(PWR_KEY,LOW); //Power Key initialization
_delay_ms(100);
digitalWrite(PWR_KEY,HIGH);
_delay_ms(1250);
digitalWrite(PWR_KEY,LOW);

    Serial.begin(9600); // only for debug the results .
    Sim800l.begin();
    if(Sim800l.delAllSms())
      Serial.println(F("Read Messages are Deleted"));
    else
      Serial.println(F("Error in Deleting Message"));     
}

void loop(){
    while(!index)
    index = (Sim800l.SmsIndication()&0x0F);

    Serial.println(F("New Message Received"));    
    textSms=Sim800l.readSms(index);
    index = 0;
    if (textSms.indexOf("OK")!=-1) //first we need to know if the messege is correct. NOT an ERROR
        {           
        if (textSms.length() > 10)  // optional you can avoid SMS empty
            {                
                textSms.toUpperCase();  // set all char to mayus ;)

                if (textSms.indexOf("ONE ON")!=-1){
                    Serial.println("RELAY ONE TURN ON");
                    digitalWrite(RELAY1,1);
                }
                else if (textSms.indexOf("ONE OFF")!=-1){
                    Serial.println("RELAY ONE TURN OFF");
                    digitalWrite(RELAY1,0);
		}
		else if (textSms.indexOf("TWO ON")!=-1){
                    Serial.println("RELAY TWO TURN ON");
                    digitalWrite(RELAY2,1);
                }
                else if (textSms.indexOf("TWO OFF")!=-1){
                    Serial.println("RELAY ONE TURN OFF");
                    digitalWrite(RELAY2,0);   
                }
		else if (textSms.indexOf("STATUS")!=-1){
		    SMS = F("Relay One : ");
		    if(digitalRead(RELAY1))
		     SMS += F("ON");
		    else
		     SMS += F("OFF");
		    SMS += F("\r\nRelay Two : ");
		    if(digitalRead(RELAY2))
		     SMS += F("ON");
		    else
		     SMS += F("OFF"); 
		    numberSms = Sim800l.getNumberSms(1);
                    Serial.println(SMS); 
		    if(Sim800l.sendSms(numberSms,SMS))
		     Serial.println(F("SMS Sent"));
		    else
		     Serial.println(F("SMS not Sent"));
		}
                else{
                    Serial.println("Invalid command");
                }
            Sim800l.delAllSms(); //do only if the message is not empty,in other case is not necesary
             //delete all sms..so when receive a new sms always will be in first position
            } 
        }
    }
 
