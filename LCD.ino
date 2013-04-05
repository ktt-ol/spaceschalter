#ifdef LCD_ENABLED

/*Infopanels*/

void lcdInfoPanels(){
      lastDisplayType = lastDisplayType % (5);
	  switch (lastDisplayType) {
		case 0:
			clearLCD();
			selectLineOne();
			LCD.print("Mainframe is: ");
			selectLineTwo();
			LCD.print("      "  + stateString);
			break;
		case 1:
			clearLCD();
			selectLineOne();
			LCD.print("Current Time: ");
			selectLineTwo();
			//Serial.println(""  + getDateString(now()));
			LCD.print(""  + getDateString(now()));
			//tzset();
			//strftime(buf,"",ctime(now()))
			break;
		case 2:
			#ifdef ENCODER
			clearLCD();
			selectLineOne();
			LCD.print("Open until: ");
			selectLineTwo();
			if(untilSetManuallyAtLeastOnce){
			  LCD.print(""  + getDateString(until));
			} else {
			  LCD.print("  Not available  ");
			}  
		  #endif ENCODER
		  break;
        case 3:
			clearLCD();
			selectLineOne();
			LCD.print("IP: ");
			selectLineTwo();
			for (byte thisByte = 0; thisByte < 4; thisByte++) {
			  // print the value of each byte of the IP address:
			  LCD.print(Ethernet.localIP()[thisByte], DEC);
			  LCD.print(".");           
			}
			break;
		case 4:
			#ifdef Relais_ENABLED 
			clearLCD();
			selectLineOne();
			LCD.print("Relais are ");
			selectLineTwo();
			LCD.print(relaisState);
			#endif Relais_ENABLED
		default:
			break;
      }
      lastDisplayType++;
}


/*Display Can't reach Server*/
void showCantConnect(){
   clearLCD();
   selectLineOne();
   delay(10);
   LCD.print("Error: ");
   selectLineTwo();
   LCD.print("Can't Connect to Server");
}


  //LCD-Print IP
void showLocalIP(){
  clearLCD();
  selectLineOne();
  LCD.print("Got IP: ");
  selectLineTwo();
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    LCD.print(Ethernet.localIP()[thisByte], DEC);
    LCD.print("."); 
  }
}



/*LCD Convenience Functions*/

void selectLineOne(){  //puts the cursor at line 0 char 0.
   LCD.write(0xFE);   //command flag
   LCD.write(128);    //position
   delay(10);
}
void selectLineTwo(){  //puts the cursor at line 0 char 0.
   LCD.write(0xFE);   //command flag
   LCD.write(192);    //position
   delay(10);
}
void goTo(int position) { //position = line 1: 0-15, line 2: 16-31, 31+ defaults back to 0
	if (position<16){ LCD.write(0xFE);   //command flag
				  LCD.write((position+128));    //position
	} else if (position<32){LCD.write(0xFE);   //command flag
				  LCD.write((position+48+128));    //position 
	} else { goTo(0); }
	   delay(10);
}

void clearLCD(){
   LCD.write(0xFE);   //command flag
   LCD.write(0x01);   //clear command.
   delay(10);
}
void backlightOn(){  //turns on the backlight
    LCD.write(0x7C);   //command flag for backlight stuff
    LCD.write(157);    //light level.
   delay(10);
}
void backlightOff(){  //turns off the backlight
    LCD.write(0x7C);   //command flag for backlight stuff
    LCD.write(128);     //light level for off.
   delay(10);
}
void serCommand(){   //a general function to call the command flag for issuing all other commands   
  LCD.write(0xFE);
}


#endif LCD_ENABLED
