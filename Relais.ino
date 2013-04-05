#ifdef Relais_ENABLED

// Send command to relais and verify recievement
long sendToRelais(HardwareSerial Relais, long cmd) {
	char cmdBuffer[COMMANDLENGTH];
	
	// Clear buffer
	while(Relais.available()) {
		Relais.read();
	}

	// Send command
	Relais.print(cmd);

	// Wait until buffer is filled
	unsigned long time = millis(); // Currend ms of ardruino lifetime
	while(Relais.available() < COMMANDLENGTH && millis()-time < 1000);

	// Timeout?
	if(Relais.available() >= COMMANDLENGTH) {
		Relais.readBytes(cmdBuffer, COMMANDLENGTH);
		
		// Convert chararray to int
		long receivedCmd = bufferToLong(cmdBuffer);
		
		// Check if cmd has arrived correct
		if(cmd == receivedCmd) {
			Serial.println("Relais: Command " + String(cmd) + " received correctly");
			// Invert second byte and send cmd back (self designed protocol)
			cmdBuffer[1] = ~cmdBuffer[1];
			cmdBuffer[2] = ~cmdBuffer[2];
			cmdBuffer[3] = ~cmdBuffer[3];
			cmdBuffer[4] = ~cmdBuffer[4];
			
			// Clear Serial Buffer
			while(Relais.available()) {
				Relais.read();
			}
			Relais.print(cmdBuffer);
	
			// if status is requested 
			if(cmd == ASK) {
				// Wait until buffer is filled
				time = millis();
				while(Relais.available() < COMMANDLENGTH && millis()-time < 1000);

				// Return answer
				if(Relais.available() >= COMMANDLENGTH) {
					char answerBuffer[COMMANDLENGTH];
					Relais.readBytes(answerBuffer, COMMANDLENGTH);
					long answer = bufferToLong(answerBuffer);
					Serial.println("Relais answer is: " + String(answer));
					return answer;
				}
				// Timeout
				else {
					Serial.println("Relais: Timeout (ASK-request)!");
					return 10;
				}
			}
		}
		// Error handling (command failed)
		else {
			switch(cmdBuffer[2]) {
				case EWRONGADD:
					Serial.println("Relais: Errorcode: 2 (wrong adress)");
					return EWRONGADD;
				case ESHORTFB:
					Serial.println("Relais: Errorcode: 3 (short ramebuffer)");
					return ESHORTFB;
				case EWRONGFB:
					Serial.println("Relais: Errorcode: 4 (wrong framebuffer)");
					return EWRONGFB;
				case EWRONGCOM:
					Serial.println("Relais: Errorcode: 5 (wrong communication)");
					return EWRONGCOM;
				default:
					Serial.println("Relais: Errorcode: unknown");
					return 1;
			}
		}
	}
	// Timeout
	else {
		Serial.println("Relais: Timeout (ack cmd)!");
		return 11;
	}
	Serial.println("Relais: Command success!");
	return 0; // command success
}

// Change endianess of 32 bit integer
long changeEndian(long val) {
    return (val << 24) | ((val <<  8) & 0x00ff0000) | ((val >>  8) & 0x0000ff00) | ((val >> 24) & 0x000000ff);
}

// Convert char[] to long
long bufferToLong(char cmdBuffer[]) {
	return changeEndian(*((long*) cmdBuffer));
}

#endif Relais_ENABLED
