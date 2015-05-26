/*
 * 
 * MicroWXStation for Arduino Mega 2560 r3 - Version 0.3.0 
 * Copyright (C) 2014, Tyler H. Jones (me@tylerjones.me)
 * http://tylerjones.me/
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Filename: LCD.ino
 * 
 * Description: LCD functions for viewing data and navigating menus.
 *
 */
  
void showAboutInfo() {
  lcdprint("MicroWXStation rev4", 0);
  lcdprint("By: Tyler H. Jones", 2);
  lcdprint("Software Ver 0.2.6", 1);
  lcdprint("Blog: tylerjones.me", 3);
}

void showMinMaxValues() {
  if(MinMaxToggle == 0) {
    if(MinMax_ListShift == 0) {
      lcdprint("Max Temp: ", 0);
      lcd.print((digitalRead(SW_UNITS) == HIGH) ? Fahrenheit(MinMax.MaxTemperature) : MinMax.MaxTemperature);
      lcd.print(TempUnitChar);
      lcd.print("   ");
    }
    lcdprint("Max Pres: ", 1 - MinMax_ListShift);
    lcd.print((double)MinMax.MaxPressure);
    lcd.print("mb ");
    lcdprint("Max Hgm: ", 2 - MinMax_ListShift);
    lcd.print(MinMax.MaxHumidity); 
    lcd.print("%   ");
    lcdprint("Max DP: ", 3 - MinMax_ListShift);
    lcd.print((digitalRead(SW_UNITS) == HIGH) ? Fahrenheit(MinMax.MaxDewPoint) : MinMax.MaxDewPoint); 
    lcd.write(TempUnitChar); 
    lcd.print("  ");    
    if(MinMax_ListShift > 0) {
      lcd.setCursor(19,0);
      lcd.write(CHAR_UPARROW);
      lcd.setCursor(19,3);
      lcd.print(" ");
      lcdprint("Max Alt: ", 4 - MinMax_ListShift);
      lcd.print((digitalRead(SW_UNITS) == HIGH) ? (MinMax.MaxAltitude*3.28084) : MinMax.MaxAltitude);
      lcd.print(AltUnitAbbr);
      lcd.print("  ");
    } else {
      lcd.setCursor(19,3);
      lcd.write(CHAR_DOWNARROW);
    }
    
  } else {
    if(MinMax_ListShift == 0) {
      lcdprint("Min Temp: ", 0);
      lcd.print((digitalRead(SW_UNITS) == HIGH) ? Fahrenheit(MinMax.MinTemperature) : MinMax.MinTemperature);
      lcd.write(TempUnitChar);
      lcd.print("   ");
    }
    lcdprint("Min Pres: ", 1 - MinMax_ListShift);
    lcd.print((double)MinMax.MinPressure);
    lcd.print("mb ");
    lcdprint("Min Hgm: ", 2 - MinMax_ListShift);
    lcd.print(MinMax.MinHumidity); 
    lcd.print("%   ");
    lcdprint("Min DP: ", 3 - MinMax_ListShift);
    lcd.print((digitalRead(SW_UNITS) == HIGH) ? Fahrenheit(MinMax.MinDewPoint) : MinMax.MinDewPoint); 
    lcd.write(TempUnitChar); 
    lcd.print("   ");
    if(MinMax_ListShift > 0) {
      lcd.setCursor(19,0);
      lcd.write(CHAR_UPARROW);
      lcd.setCursor(19,3);
      lcd.print(" ");
      lcdprint("Min Alt: ", 4 - MinMax_ListShift);
      lcd.print((digitalRead(SW_UNITS) == HIGH) ? (MinMax.MinAltitude*3.28084) : MinMax.MinAltitude);
      lcd.print(AltUnitAbbr);
      lcd.print("  ");
    } else {
      lcd.setCursor(19,3);
      lcd.write(CHAR_DOWNARROW);
    }
  }
}

void showMainMenu() {
   lcdprint("  Current WX Data", 0);
   lcdprint("  Show non-WX Data", 1);
   lcdprint("  Min/Max WX Values", 2);
   lcdprint("  About This Device", 3);
   lcd.setCursor(0, MainMenu_CursorPos);
   lcd.print("> ");
 }

void showCurrentWXData() { // Show all current weather data on the infomation LCD (4x20)
  lcdprint("Temp: ", 0);
  lcd.print(temperature);
  lcd.write(TempUnitChar); // Degree symbol 
  lcd.print("  ");
  lcdprint("Pressure: ", 1);
  lcd.print((double)pressure);
  lcd.print("mb");
  lcdprint("Humidity: ", 2);
  if(DisableDHT22) {lcd.print("DISABLED"); } else { lcd.print(humidity); lcd.print("%"); }
  lcdprint("DewPoint: ", 3);
  if(DisableDHT22) {lcd.print("DISABLED"); } else { lcd.print(dewpoint); lcd.write(TempUnitChar); lcd.print("  "); }
  if(FrostWarnLevel == 1) {
    lcd.setCursor(19,3);
    lcd.print("!"); 
  } else if(FrostWarnLevel > 1) {
    lcd.setCursor(18,3);
    lcd.print("!!"); 
  }
}

void showNonWXData() {
  lcdprint("Altitude: ", 0);
  lcd.print((double)altitude);
  lcd.print(AltUnitAbbr);
  lcd.print("  ");
  lcdprint("DHT Temp: ", 1);
  lcd.print((digitalRead(SW_UNITS) == HIGH) ? (double)T.dht_f : (double)T.dht_c);
  lcd.write(TempUnitChar);
  lcd.print("  ");
  lcdprint("BMP Temp: ", 2);
  lcd.print((digitalRead(SW_UNITS) == HIGH) ? (double)T.bmp_f : (double)T.bmp_c);
  lcd.write(TempUnitChar);
  lcd.print("  ");
  lcdprint("Uptime: ", 3);
  float uptime = 0;
  String timeunit = "";
  if((millis()/1000/60) < 1) { uptime = (millis()/1000 ); timeunit = "sec"; }
  else if((millis()/1000/60/60) < 1) { uptime = (float)(millis()/1000/60); timeunit = "min   "; }
  else if((millis()/1000/60/60/24) < 1) { uptime = (float)(millis()/1000/60/60); timeunit = "hrs  "; }
  else if((millis()/1000/60/60/24) >= 1) { uptime = (float)(millis()/1000/60/60/24); timeunit = "days  "; }
  lcd.print((double)uptime);
  lcd.print(timeunit);
}

// Print string to LCD on a specified line.
// USAGE: lcdprint("Line text goes here", 0) 
// Line number is between 0 and 3 or set to -1 to clear the LCD before printing
// NOTE: The first line is line 0! Line 1 is the second line!
void lcdprint(String msg) {
  lcdprint(msg, 0);  
}

void lcdprint(String msg, int line) {
  if(line > 3) {
    Serial.println("LCD line number is greater than the total available lines! Using line '0' instead...");
    line = 0;
  }
  if(line < 0) {
    lcd.clear();
    line = 0;
  } 
  switch(line) {
   case 1:
    lcd.setCursor(0,1);
    break;
   case 2:
    lcd.setCursor(0,2);
    break;   
   case 3:
    lcd.setCursor(0,3);
    break;
   default:
    lcd.setCursor(0,0);
    break;
  }
  lcd.print(msg);
}
