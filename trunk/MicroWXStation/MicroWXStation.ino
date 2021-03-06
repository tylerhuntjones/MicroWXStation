/*
 * 
 * MicroWXStation for Arduino Mega 2560 r3 - Version 0.3.0 
 * Copyright (C) 2016, Tyler H. Jones (me@tylerjones.me)
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
 * Filename: MicroWXStation.ino
 * 
 * Description: Main Arduino IDE sketch file. Contains the loop() and setup()
 * functions that the Arduino code is based around. Everything starts from here...
 *
 */

#include "MicroWXStation.h"
#include <dht.h>
#include <Adafruit_BMP085.h>
#include <string.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <SD.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"

// LCD (4x20) Configuration
LiquidCrystal lcd(33, 31, 29, 27 ,25, 23, 32, 30, 28, 26); //RS, EN, D0, D1, D2, D3, D4, D5, D6, D7 (R/W -> Ground)

static int loopCount = 10;
static int loopCountSerial = 0; // Keep track of when to update the serial data.
// DHT22 Config
dht DHT;
static boolean DisableDHT22 = false;

// BMP085 COnfiguration
Adafruit_BMP085 bmp;

// Initialize the WX data variables
int humidity;       // % RH
float pressure;     // In millibars
double temperature; // In F or C dependng on SW_UNITS current state
double dewpoint;    // Celcius
float altitude;     // Altitude
String TempUnitAbbr = "C";
int TempUnitChar = CHAR_DEGC;
String AltUnitAbbr = "ft";
static Temperature T;
static int MinMaxToggle = 0;
// Hazardous weather warnings
static int FrostWarnLevel = 0;

// Button debounce variables
int btnMenu_LS = 0; // LS = "Last State"
int btnSelect_LS = 0;
int btnUp_LS = 0;
int btnDown_LS = 0;
int Buttons_Pressed_Time = 0; //Time in milliseconds

static int MainMenu_CursorPos = 0; // The current position of the cursor in the Main Menu
static int MinMax_ListShift = 0; // The current position of the list of values used to determine what value is listed first. 0 = Top of list

CurrentLCDView CurrentView = CurrentWXData;
int UINT_LCD = 20;
static const int UTHOLD_LCD = 20;
int UINT_DHT = 15;
static const int UTHOLD_DHT = 15;
int UINT_SD = 0;
static const int UTHOLD_SD = 60;

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = 53;   

// ------------------------------------------------------------------------------
// setup() Core Function

void setup(void)
{
  Serial.begin(9600);
  
  // Declare input/output pins
  pinMode(BTN_INC, INPUT);
  pinMode(BTN_DEC, INPUT);
  pinMode(BTN_MENU, INPUT);
  pinMode(BTN_SELECT, INPUT);
  pinMode(SW_UNITS, INPUT);
  pinMode(NES_SER_PIN, INPUT);
  
  // Setup LED pins
  pinMode(LED_STATUS_RED, OUTPUT);
  pinMode(LED_STATUS_GRN, OUTPUT);
  pinMode(LED_LOG_RED, OUTPUT);
  pinMode(LED_LOG_GRN, OUTPUT);
  pinMode(LED_RGB_RED, OUTPUT);
  pinMode(LED_RGB_GRN, OUTPUT);
  pinMode(LED_RGB_BLUE, OUTPUT);
  // Setup NES Pins
  pinMode(NES_LATCH_PIN, OUTPUT);
  pinMode(NES_CLK_PIN, OUTPUT);
  // SD Card pin
  pinMode(53, OUTPUT);

  // Initialize NES Controller
  digitalWrite(NES_LATCH_PIN, HIGH);
  digitalWrite(NES_CLK_PIN, HIGH);
  
  // Read Min/Max values from EEPROM
  EEPROM_readAnything(0, MinMax);
  
  // SD Card setup
  if(SDENABLE) {
    if (!card.init(SPI_QUARTER_SPEED, chipSelect)) {
      SetStatusLED(-1);
      Serial.println("initialization failed. Things to check:");
      Serial.println("* is a card is inserted?");
      Serial.println("* Is your wiring correct?");
      Serial.println("* did you change the chipSelect pin to match your shield or module?");
      return;
    } else {
     Serial.println("Wiring is correct and a card is present.");
    }
   
    // print the type of card
    Serial.print("\nCard type: ");
    switch(card.type()) {
      case SD_CARD_TYPE_SD1:
        Serial.println("SD1");
        break;
      case SD_CARD_TYPE_SD2:
        Serial.println("SD2");
        break;
      case SD_CARD_TYPE_SDHC:
        Serial.println("SDHC");
        break;
      default:
        Serial.println("Unknown");
    }
   
    // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
    if (!volume.init(card)) {
      Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
      return;
    }
   
    // print the type and size of the first FAT-type volume
    long volumesize;
    Serial.print("\nVolume type is FAT");
    Serial.println(volume.fatType(), DEC);
    Serial.println();
   
    volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
    volumesize *= volume.clusterCount();       // we'll have a lot of clusters
    volumesize *= 512;                            // SD card blocks are always 512 bytes
    Serial.print("Volume size (bytes): ");
    Serial.println(volumesize);
    Serial.print("Volume size (Kbytes): ");
    volumesize /= 1024;
    Serial.println(volumesize);
    Serial.print("Volume size (Mbytes): ");
    volumesize /= 1024;
    Serial.println(volumesize);
   
    Serial.println("\nFiles found on the card (name, date and size in bytes): ");
    root.openRoot(volume);
   
    // list all files in the card with date and size
    root.ls(LS_R | LS_DATE | LS_SIZE);
  } else {
    SetLogLED(0); 
  }
  
  // Setup 4x20 LCD
  lcd.begin(20, 4);
  lcd.clear();
  lcd.createChar(CHAR_UPARROW, CharUpArrow);
  lcd.createChar(CHAR_DOWNARROW, CharDownArrow);
  lcd.createChar(CHAR_DEGC, CharDegreeC);
  lcd.createChar(CHAR_DEGF, CharDegreeF);
  lcd.createChar(CHAR_MICRO, CharMicro);
  if (!bmp.begin()) { // Setup BMP085 Barometer
        lcdprint("Barometer FAIL!", -1);
	Serial.println("*** No BMP085 barometer sensor found! Check wiring and try again! ***");
        lcdprint("Sensor not found!", 1);
        lcdprint("****FATAL ERROR*****", 2);
        lcdprint("Syetem halted!",3);
	while (1) { delay(10); }
  }
       
  //------------------------------------------
  int j = 7;
  lcdprint("MicroWXStation ", 0);
  lcdprint("v0.2.6 (HW rev 4)", 1);
  lcdprint("By: Tyler H. Jones", 2);
  lcdprint("Loading", 3);
  // THe RGB LED is activated when LOW rather than HIGH
  digitalWrite(LED_RGB_RED, HIGH);
  digitalWrite(LED_RGB_GRN, HIGH);
  digitalWrite(LED_RGB_BLUE, HIGH);
  lcd.setCursor(j,3); j++;
  lcd.print(".");  
  delay(BOOT_DELAY_INTERVAL / 2); 
  lcd.setCursor(j,3); j++;
  lcd.print("."); 
  //------------------------------------------
  // Test LEDs
  SetStatusLED(-1);
  SetLogLED(-1);
  lcd.setCursor(j,3); j++;
  lcd.print(".");  
  delay(BOOT_DELAY_INTERVAL);
  SetStatusLED(0);
  SetLogLED(1);
  lcd.setCursor(j,3); j++;
  lcd.print(".");  
  delay(BOOT_DELAY_INTERVAL); 
  SetLogLED(0);
  RGBLEDState(RGB_GRN);
  lcd.setCursor(j,3); j++;
  lcd.print(".");  
  delay(BOOT_DELAY_INTERVAL); 
  lcd.setCursor(j,3); j++;
  lcd.print(".");  
  delay(BOOT_DELAY_INTERVAL); 
  RGBLEDState(RGB_BLUE);
  lcd.setCursor(j,3); j++;
  lcd.print(".");  
  delay(BOOT_DELAY_INTERVAL); 
  RGBLEDState(RGB_RED);
  lcd.setCursor(j,3); j++;
  lcd.print(".");  
  delay(BOOT_DELAY_INTERVAL);
  RGBLEDState(RGB_OFF);
  for(int i=j;i<20;i++) {
    lcd.setCursor(i,3);
    lcd.print(".");  
    delay(BOOT_DELAY_INTERVAL / 2); 
  }
  delay(BOOT_DELAY_INTERVAL);
  lcd.clear();
}
// ------------------------------------------------------------------------------
// loop() Core Function

void loop(void)
{
  /*
   * The delay for the loop funtion is set very low to allow for speedy button responses.
   * However the delay is too short for the LCD to update properly and for the sensors to
   * read data correctly. So to make everything work, there is a loopCounter that only lets 
   * certain operations contained in loop() to ber performed after X number of loops. Thus
   * effectivley lengthening the time between LCD and Data updates without comprimising 
   * program speed and responsivness. 
  */
  loopCount++; // Increment aforementioned loopCounter

  if(loopCount >= 5) { // Perform sensor updates every other loop
    DHT22Operations();
    if(DisableDHT22) {
      T.dht_c = -99;
      T.dht_f = -99;
      humidity = -99;
    } else {
      T.dht_c = (float)DHT.temperature;
      T.dht_f = Fahrenheit((double)DHT.temperature);
      humidity = DHT.humidity;
    }
    T.bmp_c = (float)bmp.readTemperature();
    T.bmp_f = Fahrenheit((double)bmp.readTemperature());
    pressure = (float)bmp.readPressure() / 100;
    altitude = (digitalRead(SW_UNITS) == HIGH) ? (float)bmp.readAltitude()*3.28084 : (float)bmp.readAltitude();
    if(DisableDHT22) {
       dewpoint = -99;
    } else {
       dewpoint = (digitalRead(SW_UNITS) == HIGH) ? dewPoint(T.bmp_f, humidity) : dewPoint(T.bmp_c, humidity);
    }
    temperature = (digitalRead(SW_UNITS) == HIGH) ? (double)T.bmp_f : (double)T.bmp_c;
    TempUnitAbbr = (digitalRead(SW_UNITS) == HIGH) ? "F" : "C"; 
    TempUnitChar = (digitalRead(SW_UNITS) == HIGH) ? CHAR_DEGF : CHAR_DEGC; 
    AltUnitAbbr = (digitalRead(SW_UNITS) == HIGH) ? "ft" : "m";
    
    //Turn on the RGB_BLUE LED if there is evidence of frost
    if(!DisableDHT22) {
      if(dewPoint(T.bmp_c, humidity) < 0 && T.bmp_c < 0) {
         if(dewPoint(T.bmp_c, humidity) >= T.bmp_c) {
           FrostWarnLevel = 2;
           RGBLEDState(RGB_BLUE);
         } else {
           FrostWarnLevel = 1;  
           digitalWrite(LED_RGB_BLUE, HIGH);
         }
      } else { 
        digitalWrite(LED_RGB_BLUE, HIGH);
        FrostWarnLevel = 0;  
      }
    }
    
    if(T.bmp_c > 0) {
      digitalWrite(LED_RGB_BLUE, HIGH); 
      digitalWrite(LED_RGB_GRN, HIGH);       
    } else if(T.bmp_f < 100 || pressure >= 980) {
      digitalWrite(LED_RGB_RED, HIGH);
    } else if(pressure < 970) {
      digitalWrite(LED_RGB_GRN, HIGH); 
      digitalWrite(LED_RGB_RED, LOW); 
    } else if(T.bmp_c <= 0) {
      digitalWrite(LED_RGB_BLUE, LOW); 
      digitalWrite(LED_RGB_GRN, LOW); 
    } else if(T.bmp_f >= 100) {
      digitalWrite(LED_RGB_RED, LOW); 
    } else if(pressure < 980) {
      digitalWrite(LED_RGB_GRN, LOW); 
    }
    
    if(T.bmp_c < MinMax.MinTemperature) {
      MinMax.MinTemperature = (double)T.bmp_c;
    }
    if(T.bmp_c > MinMax.MaxTemperature) {
      MinMax.MaxTemperature = (double)T.bmp_c;
    }
    if(pressure < MinMax.MinPressure) {
      MinMax.MinPressure = pressure;
    }
    if(pressure > MinMax.MaxPressure) {
      MinMax.MaxPressure = pressure;
    }
    if(humidity < MinMax.MinHumidity) {
      MinMax.MinHumidity = humidity;
    }
    if(humidity > MinMax.MaxHumidity) {
      MinMax.MaxHumidity = humidity;
    }
    if(dewpoint < MinMax.MinDewPoint) {
      MinMax.MinDewPoint = dewPointFast(T.bmp_c, humidity);
    }
    if(dewpoint > MinMax.MaxDewPoint) {
      MinMax.MaxDewPoint = dewPointFast(T.bmp_c, humidity);
    }
    if(altitude < MinMax.MinAltitude) {
      MinMax.MinAltitude = (float)bmp.readAltitude();
    }
    if(altitude > MinMax.MaxAltitude) {
      MinMax.MaxAltitude = (float)bmp.readAltitude();
    }
    loopCount = 0;
  }
  
  // Get data from the NES controller and trigger the cooresponding handler function
  // Also debounce the buttons as they are read in the same manner as a hardware button would be
  byte NESData = GetNESData();
  if(NESData == NES_UP && Last_NESData != NES_UP) { // UP button
    //Serial.println("NES UP button pressed");
    UpBtnHandler();
  } else if(NESData == NES_DOWN && Last_NESData != NES_DOWN) { // DOWN button
    //Serial.println("NES DOWN button pressed");
    DownBtnHandler();
  } else if(NESData == NES_LEFT && Last_NESData != NES_LEFT) { // LEFT button
    //Serial.println("NES LEFT button pressed");
  } else if(NESData == NES_RIGHT && Last_NESData != NES_RIGHT) { // RIGHT button   
    //Serial.println("NES RIGHT button pressed");
  } else if(NESData == NES_SELECT && Last_NESData != NES_SELECT) { // SELECT button
    //Serial.println("NES SELECT button pressed");
    SelectBtnHandler();
  } else if(NESData == NES_START && Last_NESData != NES_START) { // START button
    //Serial.println("NES START button pressed");
    MenuBtnHandler();
  } else if(NESData == NES_A && Last_NESData != NES_A) { // A button
    //Serial.println("NES A button pressed");
  } else if(NESData == NES_B && Last_NESData != NES_B) { // B button
    //Serial.println("NES B button pressed");
  }
  Last_NESData = GetNESData();
 
  // Check for hardware button state changes and trigger the cooresponding handler function
  // Debouce the button presses before triggering an action.
  if(digitalRead(BTN_MENU) == LOW || digitalRead(BTN_SELECT) == LOW) { 
    Buttons_Pressed_Time = 0; 
  }
  if(digitalRead(BTN_MENU) == HIGH && digitalRead(BTN_SELECT) == HIGH) { 
    if(Buttons_Pressed_Time == 0) { 
      Buttons_Pressed_Time = millis(); 
    }
    if(millis() - Buttons_Pressed_Time > BTN_PRESSHOLD_DUR) {
      lcdprint("Simultaneous", -1);
      lcdprint("Button Press!", 1);
      lcdprint("Menu + Select", 3);
      delay(2000);
    }
  } else { 
    if(digitalRead(BTN_SELECT) == HIGH && digitalRead(BTN_SELECT) != btnSelect_LS) {
      SelectBtnHandler();
    }
    if(digitalRead(BTN_MENU) == HIGH && digitalRead(BTN_MENU) != btnMenu_LS) {
      MenuBtnHandler();
    }
    btnMenu_LS = digitalRead(BTN_MENU);
    btnSelect_LS = digitalRead(BTN_SELECT);
  }
  if(digitalRead(BTN_DEC) == LOW && digitalRead(BTN_INC) == LOW) { 
    Buttons_Pressed_Time = 0; 
  }
  if(digitalRead(BTN_DEC) == HIGH && digitalRead(BTN_DEC) != btnDown_LS) {
    DownBtnHandler();
  }
  if((digitalRead(BTN_INC) == HIGH && digitalRead(BTN_INC) != btnUp_LS)) {
    UpBtnHandler();
  }
  btnUp_LS = digitalRead(BTN_INC);
  btnDown_LS = digitalRead(BTN_DEC);

  //Wait until LCD update interval to update the LCD
    UINT_LCD++;
    if(UINT_LCD > UTHOLD_LCD) {
      UINT_LCD = 0;
        switch(CurrentView) {
        case CurrentWXData:
          showCurrentWXData();
          break;
        case MinMaxValues:
          showMinMaxValues();
          break;
        case MainMenu:
          showMainMenu();
          break;
        case AboutInfo:
          showAboutInfo();
          break;
        case NonWXData:
          showNonWXData();
          break;
      }
    } 
  
  if(loopCountSerial == 100) {
    // Send the current WX data to the serial port
    Serial.print("[");
    delay(2);
    Serial.print((double)(T.dht_c));
    delay(2);
    Serial.print("|");
    delay(2);
    Serial.print((double)(T.bmp_c));
    delay(2);
    Serial.print("|");
    delay(2);
    Serial.print(humidity);
    delay(2);
    Serial.print("|");
    delay(2);  
    Serial.print((double)pressure);
    delay(2);
    Serial.print("|");
    delay(2);  
    Serial.print(dewPoint(T.bmp_c, humidity));
    delay(5);
    Serial.print("|");
    delay(2);
    Serial.print((double)altitude);
    delay(2);
    Serial.print("]\n\r");
    delay(2);   
    loopCountSerial = 0;
  }
  loopCountSerial++;
  
  if(SDENABLE) {
    UINT_SD++;
    if(UINT_SD > UTHOLD_SD) {
      //Create Data string for storing to SD card
      //We will use CSV Format  
      String dataString = String(millis()) + ", " + String((int)(T.dht_c*10)) + ", " + String((int)humidity) + ", " + String((long)(pressure)) + ", " + String((int)dewPoint(T.dht_c, humidity)); 
      Serial.println(dataString);
      //Open a file to write to
      //Only one file can be open at a time
      File logFile = SD.open("LOG.CSV", FILE_WRITE);
      if (logFile)
      {
        logFile.println(dataString);
        logFile.close();
        Serial.println(dataString);
      } else {
        Serial.println("Couldn't open log file");
      }
    }
  }
  delay(50);
}


void ShowDHT22Error() {
  int DHT22ERR_COUNTER = 0;
  lcdprint("Hold MENU to disable", 1);
  for(int i=0;i<800;i++) {
    DHT22ERR_COUNTER = 0;
    while(digitalRead(BTN_MENU) == HIGH && !DisableDHT22) {
      DHT22ERR_COUNTER++;
      if(DHT22ERR_COUNTER == 300) {
        DisableDHT22 = true;
        SetStatusLED(0);
      }
      delay(10);
    }
    if(DisableDHT22) {
      i = 800;
    } 
    delay(10);
  }
  DHT22ERR_COUNTER = 0;
  lcd.clear();
  if(DisableDHT22) {
    lcdprint("DHT22 Disabled!", -1);
    lcdprint("Humidity/ExtTemp are", 1);
    lcdprint("no longer available!", 2);
  } else {
    lcdprint("DHT22 Error Ignored!", -1);
    lcdprint("Error will reoccur", 1); 
    lcdprint("unless sensor fixed", 2); 
    lcdprint("or disabled!", 3);                                     
  }
  delay(5000);
  lcd.clear();
}

void DHT22Operations() {
  //if(DisableDHT22) return;
  UINT_DHT++;
  if(UINT_DHT > UTHOLD_DHT) {
    UINT_DHT= 0;
    int chk = DHT.read22(DHT22_PIN);
    switch (chk)
    {
      case DHTLIB_OK:
        SetStatusLED(1);
        DisableDHT22 = false; 
        //Serial.println("DHT22 - OK,\t");
        break;
      case DHTLIB_ERROR_CHECKSUM:
        SetStatusLED(-1);
        //Serial.println("DHT22 Checksum error!");
        DisableDHT22 = true; 
        //lcdprint("DHT22 CKSUM FAIL!", -1);
        //ShowDHT22Error();
        break;
      case DHTLIB_ERROR_TIMEOUT:
        SetStatusLED(-1);
        //Serial.println("DHT22 Time out error!");
        DisableDHT22 = true; 
        //lcdprint("DHT22 TIMEOUT ERR!", -1);
        //ShowDHT22Error();
        break;
      default:
        SetStatusLED(-1);
        //Serial.println("DHT22 Unknown error!");
        DisableDHT22 = true; 
        //lcdprint("DHT22 UNKNOWN ERR!", -1);
        //ShowDHT22Error();
        break;
    }
  }
}
