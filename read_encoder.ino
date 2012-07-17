#ifdef ENCODER

/* returns change in encoder state (-1,0,1) */
int8_t read_encoder()
{
  static int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  static uint8_t old_AB = 0;
  /**/
  old_AB <<= 2;                   //remember previous state
  old_AB |= ( ENC_PORT & 0x03 );  //add current state
  return ( enc_states[( old_AB & 0x0f )]);
}


void checkEncoder(){

   /*encoder*/
   do {
   static uint8_t counter = 0;      //this variable will be changed by encoder input
   int8_t tmpdata;
      tmpdata = read_encoder();
      if( tmpdata ) {
          int minutesOff;
        if (minutesOff = now()/60%60 %15 != 0){ //minutes not divisible by 15
          until = now() + 60 * ( 15 - minutesOff) ; // round up to nearest 15 min.
        }
        
            until += 60 * 60 * tmpdata / 2 /2/2 ; // durch 4 ist drehgeberspezifisch???.
            if (until < now()){ until=now(); }
            
            Serial.println("tmpdata:" + String(tmpdata) );
            Serial.print("New Until: ");
            Serial.println(getDateString(until));
            
            untilSetManuallyAtLeastOnce = 1;
            
            timeOfLastUntilChange = millis();
            untilHasChanged = true;
      }


    // LCD-Display updaten
    if (untilHasChanged && millis() - timeOfLastUntilChange > 100 && millis() - timeOfLastUntilDisplay > 1000){
      timeOfLastUntilDisplay = millis();
      clearLCD();
      selectLineOne();
      LCD.print("Until: ");
      selectLineTwo();
      LCD.print(""  + getDateString(until));
      Serial.print("Until: "+getDateString(until)+"\n");
    }
   } while (millis() - timeOfLastUntilChange < 100);
  
    if ( untilHasChanged && millis() - timeOfLastUntilChange > restTimeUntilSendUntil * 1000 ){
      sendUntilNeeded = true;
      untilHasChanged = false;
    }


    if (sendUntilNeeded == true){
        Serial.println("Sending Until...");        
        sendUntilNeeded = sendPost( serverName, serverDir, "until.php", "until=" + String(until) );
        
        if (sendUntilNeeded == 3){ // Keine Veränderung.
          sendUntilNeeded = 0;
          untilHasChanged = false;
        }
        
        Serial.print("sendUntilNeeded: ");
        Serial.println(sendUntilNeeded);

    }


}

#endif ENCODER
