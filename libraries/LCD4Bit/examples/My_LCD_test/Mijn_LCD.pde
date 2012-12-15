//example use of LCD4Bit library

#include <LCD4Bit.h> 
//create object to control an LCD.  
//number of lines in display=1
LCD4Bit lcd = LCD4Bit(4); 



void setup() { 
  pinMode(13, OUTPUT);  //we'll use the debug LED to output a heartbeat

  lcd.init();
  //optionally, now set up our application-specific display settings, overriding whatever the lcd did in lcd.init()
  //lcd.commandWrite(0x0F);//cursor on, display on, blink on.  (nasty!)

}

void loop() {  
  digitalWrite(13, HIGH);  //light the debug LED
  lcd.clear();
  lcd.cursorTo(1,0);
  lcd.printIn("Line 1");
  lcd.cursorTo(2,0);
  lcd.printIn("Line 2");
  lcd.cursorTo(3,0); 
  lcd.printIn("Line 3");
  lcd.cursorTo(4,0);
  lcd.printIn("Line 4");
  delay(1000);
  digitalWrite(13, LOW);  //clear the debug LED

 }
