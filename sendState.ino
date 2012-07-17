int sendPost( const char serverName[], String serverDir, String urlString, String queryString ){
  char tmp;
  int returnValue = 5;
  String buf = String("          ");
    //Serial.println(strlen(serverName));
    time_t start = now();
    // Try to connect
    Serial.println("Trying to connect...");
    client.connect(serverName,80);
    
    while (!client.connected()) {
     if (now() - start > 5) break;
     Serial.println(start);
     client.stop();
     client.connect(serverName,80);
     delay(500);
    }
  
  
  //Make Post Request
  if ( client.connected() ) { 
    Serial.println("connected");
    // Make a HTTP request:
    //Serial.println(queryString);    
    //String body = queryString; //"state="+state;

     
    client.print("POST " + serverDir + "/"+ urlString+" ");
          Serial.print("freeMemory()=");
      Serial.println(freeMemory()); 
    client.print("HTTP/1.1\nHost: "+serverNameString);
          Serial.print("freeMemory()=");
      Serial.println(freeMemory()); 
    client.print("\nContent-Type: application/x-www-form-urlencoded\nContent-Length:");
          Serial.print("freeMemory()=");
      Serial.println(freeMemory()); 
    client.print(String(queryString.length(),DEC) +"\r\n\r\n");
          Serial.print("freeMemory()=");
      Serial.println(freeMemory()); 
    client.print(queryString +"\r\n");
  
      Serial.print("freeMemory()=");
      Serial.println(freeMemory()); 

  
    Serial.println("Request:");
    Serial.print("POST " + serverDir + "/"+ urlString);
    Serial.print(" HTTP/1.1\nHost: "+serverNameString);
    Serial.print("\nContent-Type: application/x-www-form-urlencoded\nContent-Length:");
    Serial.print(String(queryString.length(),DEC) +"\r\n\r\n");
    Serial.print(queryString +"\r\n");
    
        
    time_t start = now();
  while (!client.available()) {
    Serial.println(now()-start);
    if (now() - start > 5) {
      Serial.println("connection failed");
      client.stop();
      return 5;
      //break;
    }
    delay(100);
    //Serial.print("x");
  }


  if(client.available()){
    
    skipHttpHeaders();
  }

  //Serial.println("DBG");

  Serial.println("Server Response Body:");  
  while (client.available()) {
    tmp = client.read();
    Serial.print( String(tmp) );
    if (buf.length() <= 10){
      buf += String(tmp);
    }
  }
  
  
  returnValue = buf.toInt();
  //Since this assumption is too simple especially since buf could be
  //empty and thusly even erroneous server-output would be interpreted
  // as all went well we should check if the buffer actually contains
  // an ascii 0
  if (buf.length()>0){
    if(buf.charAt(0) != String(returnValue).charAt(0))
    {
      returnValue=3;
    }
  }
  else{
    //buffer is empty thus try again
  return 3;
  }
  buf = String("");
  
  if (client.connected()) {
      Serial.println("disconnecting.");
      client.stop();
    }
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
    returnValue = 5;
  }
 return returnValue;
}
