const char serverName[] = "status.kreativitaet-trifft-technik.de";

  int restTimeUntilSend = 0;  // The Switch will wait this amount of Seconds before Sending
                              // the current State to the server. The delay on the server                              // is unaffected by this value
  int restTimeUntilSendUntil = 10;

String serverNameString = String(serverName);

/*
enum ServerReturnState{
  SwitchOK = 0,
  SwitchAlreadyOK = 3,
  Failed = 5
};
*/

//SPI is needed for ethernet
#include <SPI.h>

#define LCD_ENABLED 1
#define Relais_ENABLED 1

#ifdef LCD_ENABLED
#include <SoftwareSerial.h>
#endif LCD_ENABLED

#include <Ethernet.h>/*belegt pins 10,11,12,13*/
#include <Time.h>
//#include <time.h>

#include "MemoryFree.h"

//DateStrings.cpp patchen mit const. für arduino 1.0

/* Rotary encoder*/
// define Encoder to enable Encoder-Code
// #define ENCODER
#ifdef ENCODER
// #define ENC_A 8
// #define ENC_B 9
// #define ENC_PORT PINB
#endif ENCODER

#ifdef LCD_ENABLED
//SoftwareSerial LCD(23,22); // serial LCD rx,tx
// To verify/compile the Sketch successfully you need to have 
// Arduino Mega selected under Tools->Board
// otherwise the compiler will state that SerialX is not defined.

HardwareSerial LCD = Serial1;
//Serial1 has pins 19 (RX) and 18 (TX)
#endif LCD_ENABLED

// Using one serial output for each of the two relais
HardwareSerial RelaisOne = Serial2; // Serial2 has pins 17 (RX) 16 (TX)
HardwareSerial RelaisTwo = Serial3; // Serial3 has pins ?? (RX) ?? (TX)

// Define Commands for Relais
// Main cmd will look like: 4 Bytes: (address|cmd=(set,ask)|err|errorcode)
const byte ADDRONE = 0x23; // First relais
const byte ADDRTWO = 0x42; // Second relais
const byte SET = 0x12; //Command to set the relais
const byte ASK = 0x21; //Command to ask the state of the relais
const byte ERROR = 0x33; //error Code
const byte EWRONGADD = 0x02;
const byte ESHORTFB = 0x03;
const byte EWRONGFB = 0x04;
const byte EWRONGCOM = 0x05;
const byte COMMANDLENGTH = 4;
//Serial LCD()


String L1;
String L2;

//Pins Normaler Arduino
//int stateOnPin = 6;
//int stateOffPin = 7;

// Mega Pins
int stateOnPin = 8;
int stateOffPin = 9;


int ampelGreen = A0;
int ampelYellow = A1;
int ampelRed = A2;

// Vom Ethernet-Shield belegte Pins: 
/*
    D2 - Ethernet interrupt (optional with solder bridge "INT")
    D4 - SD SPI CS
    D10 - Ethernet SPI CS
    D11 - Not connected (but should be SPI MOSI)
    D12 - Not connected (but should be SPI MISO)
    D13 - SPI SCK
    A0 - SD Write Protect
    A1 - SD Detect
*/

//ntp ist nicht dringend erforderlicht
//lieber serverseitig per get request die unix epoch abholen...

// http://marco.guardigli.it/2011/11/tomware-arduino-ethernet-datalogger.html
// http://interactive-matter.eu/how-to/arduino-http-client-library/
// https://github.com/interactive-matter/HTTPClient

// HTTP-Entity Body beginnt nach CRLF allein auf Zeile
// ergo headersCRLFCRLFbody 13101310body 0x0d0x0a0x0d0x0a

    int untilSetManuallyAtLeastOnce = 0;
    int lastDisplayType = 0;
    unsigned long timeOfLastSwitch = 0;
    int sendStateNeeded = 0;
    String stateString = String("unknown");
	String relaisState = String("unknown");
    
    
    unsigned long timeOfLastUntilChange = 0;
    int sendUntilNeeded = 0;
    unsigned long until = 0;
    int untilHasChanged = false;

    int oldStateOn = 0;
    int oldStateOff = 0;

    unsigned long timeOfLastHeartbeat=0;

    time_t timeOfLastPrintStuff = 0;
    unsigned long timeOfLastUntilDisplay = 0;

	unsigned long answer = 0x00000001; // Answer from Relais
	
byte mac[] = {
  0x90, 0xA2, 0xda, 0x00, 0x1c, 0xe6 }; // Meine MAC von meinem Ethernetshield
 //My IP address: 10.18.10.250.

String serverDir = ""; // no trailing slash

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):

EthernetClient client;



void setup() {
  
  //Since Arduino 1.01 INPUT_PULLUP
  pinMode(stateOnPin,INPUT_PULLUP);
  pinMode(stateOffPin,INPUT_PULLUP);

//This was used before arduino 1.01 to set the internal pullups  
/*
  pinMode(stateOnPin,INPUT);
  pinMode(stateOffPin,INPUT);
  digitalWrite(stateOnPin,HIGH);
  digitalWrite(stateOffPin,HIGH);
*/

  pinMode(ampelGreen,OUTPUT);
  pinMode(ampelYellow,OUTPUT);
  pinMode(ampelRed,OUTPUT);
  
  
  /* Setup encoder pins as inputs */
#ifdef ENCODER
  pinMode(ENC_A, INPUT);
  digitalWrite(ENC_A, HIGH);
  pinMode(ENC_B, INPUT);
  digitalWrite(ENC_B, HIGH);
#endif ENCODER  
  
  // start the serial library:
  Serial.begin(9600);
  Serial.println("Mainframe-Switch starting up.");

  // start relais serial
  Serial2.begin(1200);
  
  #ifdef LCD_ENABLED
	  // Start LCD Output
	  //pinMode(LCDpin, OUTPUT);
	  //digitalWrite(LCDpin,HIGH);
	  LCD.begin(9600);
	  backlightOn();
		clearLCD(); 
	  //selectLineOne();
	  goTo(0);
	  LCD.print("Mainframe");  
	  goTo(16);
	  //selectLineTwo();
	  LCD.print("  ...starting up");  
  #endif LCD_ENABLED
  
  // start the Ethernet connection:
  Serial.println("About to get IP-Address");
  int dhcpSuccess = Ethernet.begin(mac);
  while (dhcpSuccess == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    #ifdef LCD_ENABLED
		showCantConnect();
    #endif LCD_ENABLED
    dhcpSuccess = Ethernet.begin(mac);
    //Serial.println("Failed");
    delay(1000);
  }
  
  
    // give the Ethernet shield a second to initialize:
  delay(1000);
  
  // print your local IP address to DBG Serial:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
  
  //LCD-Print the IP-Adress from DHCP
  #ifdef LCD_ENABLED
	showLocalIP();
  #endif LCD_ENABLED
  
   delay(1000);
   getEpochFromServer();
   Serial.println("DBG");
   until=now();
   
   #ifdef LCD_ENABLED
     clearLCD();
     selectLineOne();
     LCD.print("Got Time");
     selectLineTwo();
     LCD.print("Ready...");
   #endif LCD_ENABLED
}


    
void loop() {

  //static int loopCounter = 0;  
  //Serial.println("LoopStart " + String(loopCounter));
  Serial.println("LoopStart");

  
#ifdef ENCODER
  //TODO check refactored function
  checkEncoder()
#endif ENCODER


// Debug-Serial updaten
    if ( now() - timeOfLastPrintStuff > 5){
      Serial.print("OnState : " + String(!digitalRead(stateOnPin )) ); //inverted for Display purposes;
      Serial.print(" ");
      Serial.println("OffState: " + String(!digitalRead(stateOffPin)) ); //inverted for Display purposes;
  
      //Serial.print("freeMemory()=");
      //Serial.println(freeMemory()); 
      //Serial.println(timeSinceLastSwitch,DEC);
      
      /*
       * Statusdaten auf LCD ausgeben.
       */
        Serial.println("beforeInfo");
      #ifdef LCD_ENABLED  
		lcdInfoPanels();
      #endif LCD_ENABLED
        Serial.println("afterInfo");      
      
	  timeOfLastPrintStuff = now();
      Serial.println(now(),DEC);
    }
    
    
    // digitalRead the pins once and use these values throughout so that
    // there won't be a race condition when the switch is operated very fast
    
    int stateOfTheOnPin = digitalRead(stateOnPin);
    int stateOfTheOffPin = digitalRead(stateOffPin);
    
    if( (oldStateOn != stateOfTheOnPin || oldStateOff != stateOfTheOffPin ) && now() - timeOfLastSwitch >= restTimeUntilSend){

      Serial.println("State changed");
      
      // internal pullups !Logic
      if (stateOfTheOnPin == LOW &&  stateOfTheOffPin == HIGH) {
        stateString = "on";
        sendStateNeeded = 1;
        timeOfLastSwitch = now();
        
        digitalWrite(ampelGreen ,HIGH);
        digitalWrite(ampelYellow,LOW );        
        digitalWrite(ampelRed   ,LOW );

		// TODO: some Realis on 
		#ifdef Relais_ENABLED
		//answer = 0x23234242;
		//sendToRelais(Relais, SET);
		// Reset timer!
		#endif Relais_ENABLED
      }
	  // Off (red)
      if (stateOfTheOnPin == HIGH &&  stateOfTheOffPin == LOW) {
        stateString = "off";
        sendStateNeeded = 1;
        timeOfLastSwitch = now();        
       
        digitalWrite(ampelGreen ,LOW );
        digitalWrite(ampelYellow,LOW );        
        digitalWrite(ampelRed   ,HIGH);

		// TODO: some Realis off after delay of 30 min
		
		#ifdef Relais_ENABLED
		//unsigned long timer = millis();
		//if(millis()-timer > 300000 &&)
		//sendToRelais(Relais, SET);
		answer = 0x00000001;
		#endif Relais_ENABLED
      }
	  // Yellow
      if (stateOfTheOnPin == HIGH &&  stateOfTheOffPin == HIGH) {
        stateString = "unk";
        digitalWrite(ampelGreen ,LOW );
        digitalWrite(ampelYellow,HIGH);        
        digitalWrite(ampelRed   ,LOW );
        sendStateNeeded = 0;        
      }
        Serial.println("NewState:" + stateString);      

#ifdef LCD_ENABLED
        clearLCD();
        selectLineOne();
        LCD.print("New State:");
        selectLineTwo();
        LCD.print("     " + stateString);
#endif LCD_ENABLED
       
       oldStateOn  = stateOfTheOnPin ;
       oldStateOff = stateOfTheOffPin;
      
    }
  
    if (sendStateNeeded > 0) {
        
        Serial.println("Sending State...");        
        sendStateNeeded = sendPost( serverName, serverDir, "switch", "state="+stateString );
        
        if (sendStateNeeded == 3){ // Schalter steht bereits auf Eingabe.
          sendStateNeeded = 0;
        }
        
        if (sendStateNeeded == 0){
          #ifdef LCD_ENABLED
          clearLCD();
          selectLineOne();
          LCD.print("State Sent:");
          selectLineTwo();
          LCD.print("     " + stateString);
          #endif LCD_ENABLED
        }
        
        
        Serial.print("sendStateNeeded: ");
        Serial.println(sendStateNeeded);
    }

/*
    if ( now() - timeOfLastHeartbeat > 60 ){
      Serial.println("Sending Heartbeat");
      sendPost( serverName, serverDir, "heartbeat.php", "time=" + String( now() ) );
      timeOfLastHeartbeat = now();
    }
 */
    //delay(1000);

	// TODO: Check if Realais are still alive every 5s
	if(millis() % 5000 == 0) {
		long answerOne = sendToRelais(RelaisOne, ASK);
		long answerTwo = sendToRelais(RelaisTwo, ASK);

		// Relais online
		if(answer == 0x23232323) {
			relaisState = "     online";
		} 
		// Relais offline
		else {
			Serial.println("Err: Relais dead!");	
			relaisState =  "     offline";
		}
	}

     //Serial.println("Loop End " + String(loopCounter));
	 //loopCounter++;
}
