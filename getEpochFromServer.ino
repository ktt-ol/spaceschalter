void getEpochFromServer (){

 while (!client.connected()) {
     client.connect(serverName,80);
    }
    String req = "GET /time.php?format=UnixEpochUnsignedInt HTTP/1.0\nHost: " + String(serverName)+"" ;

    Serial.println("About to send request for Time");
    Serial.println(req);
    client.println(req+"\n");
    
    
    while(!client.available()){
      delay(1000);
      String dbg = "Trying to get time from Server since " + String(now()) + " Seconds";
      Serial.println(dbg);
    }
    
    Serial.println("Skipping HTTP Headers");
    skipHttpHeaders(); //&client
    Serial.println("Skipped Headers");

  byte buf;
  unsigned long epoch;
  
  /*
        Serial.write( buf = client.read() );
        epoch = (unsigned long)buf << 24 ;
                                  
        Serial.write( buf = client.read() );
        epoch = (unsigned long)buf << 16 | epoch;
                 
        Serial.write( buf = client.read() );
        epoch = (unsigned long)buf <<  8 | epoch;
                 
        Serial.write( buf = client.read() );
        epoch = (unsigned long)buf | epoch;
*/

    epoch = client.parseInt();

    const unsigned long UTCcorrection = 0;//0;//60 * 60 * 1; // Deutschland UTC +1

    Serial.println(epoch, DEC);

    time_t t = epoch + UTCcorrection;
    
    setTime(t);    // sets arduino internal clock
   Serial.println("Time Set");
   Serial.println(now());
   //Serial.println(getDateString());
   

  Serial.print(year());
  Serial.print("-");
  Serial.print(month());
  Serial.print("-");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(hour());
  Serial.print(":");
  Serial.println(minute());  
    
    if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    delay(100);
    }
    
}




const String getDateString(unsigned long t) {
  String s = String();
  
  int UTCcorrection = 60*60; //Timezone Berlin
 
  if (IsDST(day(t),month(t),weekday(t)) == true)
  {
  UTCcorrection += 60*60;
  }
    
  t = t + UTCcorrection;
  
  s = s + day(t) + ".";
  s = s + month(t) + ".";
  s = s + year(t) + " ";

  if (hour(t) < 10){
  s = s + "0";
  }
  s = s + hour(t) + ":";
  if (minute(t) < 10){
  s = s + "0" ;
  }
  s = s + minute(t) ;
  
  //s = s + ":";
  //  s = s + second(t);
  
  return(s);
}

boolean IsDST(int day, int month, int dow) {
        //January, february, and december are out.
        if (month < 3 || month > 11) { return false; }
        //April to October are in
        if (month > 3 && month < 11) { return true; }
        int previousSunday = day - dow;
        //In march, we are DST if our previous sunday was on or after the 8th.
        if (month == 3) { return previousSunday >= 8; }
        //In november we must be before the first sunday to be dst.
        //That means the previous sunday must be before the 1st.
        return previousSunday <= 0;
    }
