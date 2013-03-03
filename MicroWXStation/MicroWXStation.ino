#include <string.h>
#include <Wire.h>
#include <dht.h>
#include <Adafruit_BMP085.h>
#include <LiquidCrystal.h>
#include <SD.h>

//General Definitions
#define ON HIGH
#define OFF LOW
#define SDENABLE false
#define BOOT_DELAY_INTERVAL 400
#define CHAR_DEGF 4
#define CHAR_DEGC 3
#define CHAR_UPARROW 1
#define CHAR_DOWNARROW 2
#define CHAR_MICRO 5
#define RGB_OFF 0
#define RGB_RED 1
#define RGB_BLUE 2
#define RGB_GRN 3

// LCD (4x20) Configuration
LiquidCrystal lcd(33, 31, 29, 27 ,25, 23, 32, 30, 28, 26);

static double MinTemperature = 1000;
static double MaxTemperature = -1000;
static float MinPressure = 1100;
static float MaxPressure = 900;
static int MinHumidity = 100;
static int MaxHumidity = 0;
static double MinDewPoint = 120;
static double MaxDewPoint = -100;
static float MinAltitude = 10000;
static float MaxAltitude = -1000;
static int MinMaxToggle = 0;

//long id = 1;
static int loopCount = 0;

// DHT22 Config
#define DHT22_PIN 7
dht DHT;

// BMP085 COnfiguration
Adafruit_BMP085 bmp;

// Temperature typedef (Celcius and Fahrenheit)
typedef struct {
  float bmp_c;     // BMP085 Temperature value (in Celcius)
  float dht_c;     // DHT22 Temperature value (in Celcius)
  double bmp_f;    // BMP085 Temperature value (in Fahrenheit)
  double dht_f;    // DHT22 Temperature value (in Fahrenheit)
} Temperature;

// Initialize the main WX data variables
int humidity;       // % RH
float pressure;     // In millibars
double temperature; // In F or C dependng on SW_UNITS current state
double dewpoint;    // Celcius
float altitude;     // Altitude
String TempUnitAbbr = " C";
int TempUnitChar = CHAR_DEGC;
String AltUnitAbbr = " ft";

// Button Pins
#define BTN_INC 24  // Increment
#define BTN_DEC 22  // Decrement
#define BTN_MENU 34
#define BTN_SELECT 36
#define SW_UNITS 38
#define BTN_PRESSHOLD_DUR 1250

// LED Pins
#define LED_STATUS_RED 6 // Small red LED
#define LED_STATUS_GRN 13  // Small green LED
#define LED_LOG_RED 8 // Bi-color LED - Red
#define LED_LOG_GRN 9  // Bi-color LED - Green
#define LED_RGB_RED 10
#define LED_RGB_GRN 11
#define LED_RGB_BLUE 12

//Error handling variables
boolean DisableDHT22 = false;

// Button debounce variables
int btnMenu_LS = 0;
int btnSelect_LS = 0;
int btnUp_LS = 0;
int btnDown_LS = 0;
int Buttons_Pressed_Time = 0; //Time in milliseconds

//Enum for holding the current view for the GLCD
enum CurrentLCDView {
  CurrentWXData,
  NonWXData,
  MinMaxValues,
  AboutInfo,
  MainMenu
};
static int MainMenu_CursorPos = 0; // The current position of the cursor in the Main Menu
static int MinMax_ListShift = 0; // The current position of the list of values used to determine what value is listed first. 0 = Top of list

static Temperature T;
CurrentLCDView CurrentView = CurrentWXData;
int UINT_LCD = 0;
static const int UTHOLD_LCD = 5;
int UINT_DHT = 0;
static const int UTHOLD_DHT = 5;
int UINT_SD = 0;
static const int UTHOLD_SD = 60;

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = 53;   

// Setup NES Controller
#define NES_LATCH_PIN 41
#define NES_CLK_PIN 40
#define NES_SER_PIN 42
// NES contoller buttons
const byte NES_UP = B11110111;
const byte NES_DOWN = B11111011;
const byte NES_LEFT = B11111101;
const byte NES_RIGHT = B11111110;
const byte NES_SELECT = B11011111;
const byte NES_START = B11101111;
const byte NES_A = B01111111;
const byte NES_B = B10111111;
static byte Last_NESData = 0;

// LCD custom character definitions
byte CharUpArrow[8] = { B00000, B00100, B01110, B11111, B00100, B00100, B00100, B00000 };
byte CharDownArrow[8] = { B00000, B00100, B00100, B00100, B11111, B01110, B00100, B00000 };
byte CharDegreeC[8] = { B01000, B10100, B01000, B00011, B00100, B00100, B00011, B00000 };
byte CharDegreeF[8] = { B01000, B10100, B01000, B00011, B00100, B00111, B00100, B00000 };
byte CharMicro[8] = { B00000, B00000, B00000, B10010, B10010, B10010, B11100, B10000 };

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
  
    // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
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
  lcdprint("v0.2.4 (HW rev 4)", 1);
  lcdprint("By: Tyler H. Jones", 2);
  lcdprint("Loading", 3);
  // THe RGB LED is activated when LOW rather than HIGH
  digitalWrite(LED_RGB_RED, ON);
  digitalWrite(LED_RGB_GRN, ON);
  digitalWrite(LED_RGB_BLUE, ON);
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

  if(loopCount == 2 || loopCount == 4) { // Perform sensor updates every other loop
    if(!DisableDHT22) {
      DHT22Operations();
      T.dht_c = (float)DHT.temperature;
      T.dht_f = Fahrenheit((double)DHT.temperature);
      humidity = DHT.humidity;
    } else {
      T.dht_c = -99;
      T.dht_f = -99;
      humidity = -99;
    }
    T.bmp_c = (float)bmp.readTemperature();
    T.bmp_f = Fahrenheit((double)bmp.readTemperature());
    pressure = (float)bmp.readPressure() / 100;
    altitude = (digitalRead(SW_UNITS) == ON) ? (float)bmp.readAltitude()*3.28084 : (float)bmp.readAltitude();
    dewpoint = (digitalRead(SW_UNITS) == ON) ? dewPoint(T.bmp_f, humidity) : dewPoint(T.bmp_c, humidity);
    temperature = (digitalRead(SW_UNITS) == ON) ? (double)T.bmp_f : (double)T.bmp_c;
    TempUnitAbbr = (digitalRead(SW_UNITS) == ON) ? "F" : "C"; 
    TempUnitChar = (digitalRead(SW_UNITS) == ON) ? CHAR_DEGF : CHAR_DEGC; 
    AltUnitAbbr = (digitalRead(SW_UNITS) == ON) ? "ft" : "m";
    
    if(T.bmp_c > 0 && T.bmp_f < 100 && pressure >= 980) {
       RGBLEDState(RGB_OFF);
    } else if(pressure < 970) {
      RGBLEDState(RGB_RED);
    } else if(T.bmp_c <= 0) {
       RGBLEDState(RGB_BLUE);
    } else if(T.bmp_f >= 100) {
       RGBLEDState(RGB_RED); 
    } else if(pressure < 980) {
      RGBLEDState(RGB_GRN); 
    }
    
    if(T.bmp_f < MinTemperature) {
      MinTemperature = (digitalRead(SW_UNITS) == ON) ? (double)T.bmp_f : (double)T.bmp_c;
    }
    if(T.bmp_f > MaxTemperature) {
      MaxTemperature = (digitalRead(SW_UNITS) == ON) ? (double)T.bmp_f : (double)T.bmp_c;
    }
    if(pressure < MinPressure) {
      MinPressure = pressure;
    }
    if(pressure > MaxPressure) {
      MaxPressure = pressure;
    }
    if(humidity < MinHumidity) {
      MinHumidity = humidity;
    }
    if(humidity > MaxHumidity) {
      MaxHumidity = humidity;
    }
    if(dewpoint < MinDewPoint) {
      MinDewPoint = dewPointFast(T.bmp_c, humidity);
    }
    if(dewpoint > MaxDewPoint) {
      MaxDewPoint = dewPointFast(T.bmp_c, humidity);
    }
    if(altitude < MinAltitude) {
      MinAltitude = (float)bmp.readAltitude();
    }
    if(altitude > MaxAltitude) {
      MaxAltitude = (float)bmp.readAltitude();
    }
  }
  
  // Get data from the NES controller and trigger the cooresponding handler function
  // Also debounce the buttons as they are read in the same manner as a hardware button would be
  byte NESData = GetNESData();
  if(NESData == NES_UP && Last_NESData != NES_UP) { // UP button
    Serial.println("NES UP button pressed");
    UpBtnHandler();
  } else if(NESData == NES_DOWN && Last_NESData != NES_DOWN) { // DOWN button
    Serial.println("NES DOWN button pressed");
    DownBtnHandler();
  } else if(NESData == NES_LEFT && Last_NESData != NES_LEFT) { // LEFT button
    Serial.println("NES LEFT button pressed");
  } else if(NESData == NES_RIGHT && Last_NESData != NES_RIGHT) { // RIGHT button   
    Serial.println("NES RIGHT button pressed");
  } else if(NESData == NES_SELECT && Last_NESData != NES_SELECT) { // SELECT button
    Serial.println("NES SELECT button pressed");
    SelectBtnHandler();
  } else if(NESData == NES_START && Last_NESData != NES_START) { // START button
    Serial.println("NES START button pressed");
    MenuBtnHandler();
  } else if(NESData == NES_A && Last_NESData != NES_A) { // A button
    Serial.println("NES A button pressed");
  } else if(NESData == NES_B && Last_NESData != NES_B) { // B button
    Serial.println("NES B button pressed");
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
      Serial.println("Select Button PRESSED!");
      SelectBtnHandler();
    }
    if(digitalRead(BTN_MENU) == HIGH && digitalRead(BTN_MENU) != btnMenu_LS) {
      MenuBtnHandler();
      Serial.println("Menu Button PRESSED!");
    }
    btnMenu_LS = digitalRead(BTN_MENU);
    btnSelect_LS = digitalRead(BTN_SELECT);
  }
  if(digitalRead(BTN_DEC) == LOW && digitalRead(BTN_INC) == LOW) { 
    Buttons_Pressed_Time = 0; 
  }
  if(digitalRead(BTN_DEC) == HIGH && digitalRead(BTN_DEC) != btnDown_LS) {
    Serial.println("Down Button PRESSED!");
    DownBtnHandler();
  }
  if((digitalRead(BTN_INC) == HIGH && digitalRead(BTN_INC) != btnUp_LS)) {
    Serial.println("Up Button PRESSED!");
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
  
  if(loopCount == 4) {
    // Send the current WX data to the serial port
    Serial.println("----- BMP085 Data -----");
    Serial.print("Temperature: ");
    Serial.print((double)(T.bmp_c));
    Serial.print("*C / ");
    Serial.print(T.bmp_f);
    Serial.println("*F");
    Serial.print("Pressure: ");
    Serial.print((double)pressure);
    Serial.println("mb");
    Serial.print("Altitude: ");
    Serial.print(bmp.readAltitude());
    Serial.println("m / ");
    Serial.print(bmp.readAltitude()*3.28084);
    Serial.println("ft");    
    Serial.println("----- DHT22 Data -----");
    Serial.print("Temperature (DHT22): ");
    Serial.print((double)T.dht_c);
    Serial.print("*C / ");
    Serial.print(T.dht_f);
    Serial.println("*F");
    Serial.print("Relative Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
    Serial.print("Dew Point: ");
    Serial.print(dewPointFast(T.bmp_c, humidity));
    Serial.print("*C / ");
    Serial.print(dewPointFast(T.bmp_f, humidity));
    Serial.println("*F");
    // Clear the loop counter
    loopCount = 0; 
  }
  
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


void showAboutInfo() {
  lcdprint("MicroWXStation rev4", 0);
  lcdprint("By: Tyler H. Jones", 2);
  lcdprint("Software Ver 0.2.4", 1);
  lcdprint("Blog: tylerjones.me", 3);
}

void showMinMaxValues() {
  if(MinMaxToggle == 0) {
    if(MinMax_ListShift == 0) {
      lcdprint("Max Temp: ", 0);
      lcd.print(MaxTemperature);
      lcd.write(0b11011111); // Degree symbol
      lcd.print(TempUnitAbbr);
      lcd.print("   ");
    }
    lcdprint("Max Pres: ", 1 - MinMax_ListShift);
    lcd.print((double)MaxPressure);
    lcd.print("mb ");
    lcdprint("Max Hgm: ", 2 - MinMax_ListShift);
    lcd.print(MaxHumidity); 
    lcd.print("%   ");
    lcdprint("Max DP: ", 3 - MinMax_ListShift);
    lcd.print((digitalRead(SW_UNITS) == ON) ? Fahrenheit(MaxDewPoint) : MaxDewPoint); 
    lcd.write(0b11011111); // Degree symbol 
    lcd.print(TempUnitAbbr); 
    lcd.print("  ");    
    if(MinMax_ListShift > 0) {
      lcd.setCursor(19,0);
      lcd.write(0b00011001);
      lcd.setCursor(19,3);
      lcd.print(" ");
      lcdprint("Max Alt: ", 4 - MinMax_ListShift);
      lcd.print((digitalRead(SW_UNITS) == ON) ? (MaxAltitude*3.28084) : MaxAltitude);
      lcd.print(AltUnitAbbr);
      lcd.print("  ");
    } else {
      lcd.setCursor(19,3);
      lcd.write(0b00011000);
    }
    
  } else {
    if(MinMax_ListShift == 0) {
      lcdprint("Min Temp: ", 0);
      lcd.print(MinTemperature);
      lcd.write(0b11011111); // Degree symbol
      lcd.print(TempUnitAbbr);
      lcd.print("   ");
    }
    lcdprint("Min Pres: ", 1 - MinMax_ListShift);
    lcd.print((double)MinPressure);
    lcd.print("mb ");
    lcdprint("Min Hgm: ", 2 - MinMax_ListShift);
    lcd.print(MinHumidity); 
    lcd.print("%   ");
    lcdprint("Min DP: ", 3 - MinMax_ListShift);
    lcd.print((digitalRead(SW_UNITS) == ON) ? Fahrenheit(MinDewPoint) : MinDewPoint); 
    lcd.write(TempUnitChar); 
    lcd.print("   ");
    if(MinMax_ListShift > 0) {
      lcd.setCursor(19,0);
      lcd.write(CHAR_UPARROW);
      lcd.setCursor(19,3);
      lcd.print(" ");
      lcdprint("Min Alt: ", 4 - MinMax_ListShift);
      lcd.print((digitalRead(SW_UNITS) == ON) ? (MinAltitude*3.28084) : MinAltitude);
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
}

void showNonWXData() {
  lcdprint("Altitude: ", 0);
  lcd.print((double)altitude);
  lcd.print(AltUnitAbbr);
  lcd.print("  ");
  lcdprint("DHT Temp: ", 1);
  lcd.print((digitalRead(SW_UNITS) == ON) ? (double)T.dht_f : (double)T.dht_c);
  lcd.write(TempUnitChar);
  lcd.print("  ");
  lcdprint("BMP Temp: ", 2);
  lcd.print((digitalRead(SW_UNITS) == ON) ? (double)T.bmp_f : (double)T.bmp_c);
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
// ------------------------------------------------------------------------------
// Metorlogical Calculation Functions

//Celsius to Fahrenheit conversion
double Fahrenheit(double celsius)
{
        return 1.8 * celsius + 32;
}

// dewPoint function NOAA
double dewPoint(double celsius, double humidity)
{
        if(humidity < 0) { return -99; }
        double A0= 373.15/(273.15 + celsius);
        double SUM = -7.90298 * (A0-1);
        SUM += 5.02808 * log10(A0);
        SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/A0)))-1) ;
        SUM += 8.1328e-3 * (pow(10,(-3.49149*(A0-1)))-1) ;
        SUM += log10(1013.246);
        double VP = pow(10, SUM-3) * humidity;
        double T = log(VP/0.61078);   // temp var
        return (241.88 * T) / (17.558-T);
}

// delta max = 0.6544 wrt dewPoint()
// 5x faster than dewPoint()
double dewPointFast(double celsius, double humidity)
{
        if(humidity < 0) { return -99; }
        double a = 17.271;
        double b = 237.7;
        double temp = (a * celsius) / (b + celsius) + log(humidity/100);
        double Td = (b * temp) / (a - temp);
        return Td;
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

void MenuBtnHandler() {
  if(CurrentView != MainMenu) {
    CurrentView = MainMenu;
    lcd.clear();
    showMainMenu();
  }
}

void SelectBtnHandler() {
  if(CurrentView == MainMenu) {
    switch(MainMenu_CursorPos) {
      case 0:
        CurrentView = CurrentWXData;
        lcd.clear();
        showCurrentWXData();
        break;
      case 1:
        CurrentView = NonWXData;
        lcd.clear();
        showNonWXData();
        break;
      case 2:
        CurrentView = MinMaxValues;
        lcd.clear();
        showMinMaxValues();
        break;
      case 3:
        CurrentView = AboutInfo;
        lcd.clear();
        showAboutInfo();
        break;
    }
  }
  if(CurrentView == MinMaxValues) {
    if(MinMaxToggle == 0) {
      MinMaxToggle = 1;
    } else { 
      MinMaxToggle = 0;
    } 
  }
}

void UpBtnHandler() {
  if(CurrentView == MainMenu) {
    if(MainMenu_CursorPos == 0) {
      MainMenu_CursorPos = 3;
    } else {
       MainMenu_CursorPos--;
    }
  }
  if(CurrentView == MinMaxValues) {
    if(MinMax_ListShift == 1) MinMax_ListShift--;
  }
}

void DownBtnHandler() {
  if(CurrentView == MainMenu) {
    if(MainMenu_CursorPos == 3) {
      MainMenu_CursorPos = 0;
    } else {
       MainMenu_CursorPos++;
    }
  }
  if(CurrentView == MinMaxValues) {
    if(MinMax_ListShift < 1) MinMax_ListShift++;
  }
}

// Read data from NES controller
byte GetNESData() {
  byte data = 0;
  digitalWrite(NES_LATCH_PIN, LOW);
  digitalWrite(NES_CLK_PIN, LOW);

  digitalWrite(NES_LATCH_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(NES_LATCH_PIN, LOW);

  data = digitalRead(NES_SER_PIN);

  for (int i=1;i<=7;i++) {
    digitalWrite(NES_CLK_PIN, HIGH);
    delayMicroseconds(2);
    data = data << 1;
    data = data + digitalRead(NES_SER_PIN) ;
    delayMicroseconds(4);
    digitalWrite(NES_CLK_PIN, LOW);
  }
  return data;
}

void SetStatusLED(int stat) {
  switch(stat) {
    case 1: // Status = OK
      digitalWrite(LED_STATUS_RED, LOW);
      digitalWrite(LED_STATUS_GRN, HIGH);
      break;
    case -1: // Stauts = ERROR
      digitalWrite(LED_STATUS_RED, HIGH);
      digitalWrite(LED_STATUS_GRN, LOW);
      break;
    default:
      digitalWrite(LED_STATUS_RED, HIGH);
      digitalWrite(LED_STATUS_GRN, HIGH);
      break;
  } 
}

void SetLogLED(int stat) {
  switch(stat) {
    case 1: // Status = OK
      digitalWrite(LED_LOG_RED, LOW);
      digitalWrite(LED_LOG_GRN, HIGH);
      break;
    case -1: // Stauts = ERROR
      digitalWrite(LED_LOG_RED, HIGH);
      digitalWrite(LED_LOG_GRN, LOW);
      break;
    default:
      digitalWrite(LED_LOG_RED, LOW);
      digitalWrite(LED_LOG_GRN, LOW);
      break;
  } 
}

void DHT22Operations() {
  if(DisableDHT22) return;
  UINT_DHT++;
  if(UINT_DHT > UTHOLD_DHT) {
    UINT_DHT= 0;
    int chk = DHT.read22(DHT22_PIN);
    switch (chk)
    {
      case DHTLIB_OK:
        SetStatusLED(1); 
        //Serial.println("DHT22 - OK,\t");
        break;
      case DHTLIB_ERROR_CHECKSUM:
        SetStatusLED(-1);
        Serial.println("DHT22 Checksum error!");
        lcdprint("DHT22 CKSUM FAIL!", -1);
        ShowDHT22Error();
        break;
      case DHTLIB_ERROR_TIMEOUT:
        SetStatusLED(-1);
        Serial.println("DHT22 Time out error!");
        lcdprint("DHT22 TIMEOUT ERR!", -1);
        ShowDHT22Error();
        break;
      default:
        SetStatusLED(-1);
        Serial.println("DHT22 Unknown error!");
        lcdprint("DHT22 UNKNOWN ERR!", -1);
        ShowDHT22Error();
        break;
    }
  }
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

void RGBLEDState(int color) {
  switch(color) {
    case RGB_OFF:
      digitalWrite(LED_RGB_BLUE, HIGH);
      digitalWrite(LED_RGB_GRN, HIGH);
      digitalWrite(LED_RGB_RED, HIGH);
      break;
    case RGB_RED:
      digitalWrite(LED_RGB_BLUE, HIGH);
      digitalWrite(LED_RGB_GRN, HIGH);
      digitalWrite(LED_RGB_RED, LOW);
      break;
    case RGB_BLUE:
      digitalWrite(LED_RGB_BLUE, LOW);
      digitalWrite(LED_RGB_GRN, HIGH);
      digitalWrite(LED_RGB_RED, HIGH);
      break;
    case RGB_GRN:
      digitalWrite(LED_RGB_BLUE, HIGH);
      digitalWrite(LED_RGB_GRN, LOW);
      digitalWrite(LED_RGB_RED, HIGH);
      break;      
  } 
}
