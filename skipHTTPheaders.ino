
void skipHttpHeaders(){
  Serial.println("Skipping HTTP Headers");
  int inByte = 0;
  int lastByte = 0;
  while (!(inByte == '\n' && lastByte == '\n'))
    {
      //by that we ignore \r
      if (inByte != '\r')
        {
          lastByte = inByte;
        }
      if(client.available()){
        inByte = client.read();
      }else{
        return;
      }
      Serial.write(inByte);
    }
}
