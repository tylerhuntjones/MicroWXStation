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

// LCD (4x20) Configuration
LiquidCrystal lcd(33, 31, 29, 27 ,25, 23, 32, 30, 28, 26);

double MinTemperature = 99;
double MaxTemperature = 0;
float MinPressure = 1100;
float MaxPressure = 900;
int MinHumidity = 100;
int MaxHumidity = 0;
double MinDewPoint = 120;
double MaxDewPoint = 0;
long id = 1;

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
  
int humidity;       // % RH
float pressure;     // In millibars
double temperature; // In F or C dependng on SW_UNITS current state
double dewpoint;    // Celcius
float altitude;     // Altitude
String UnitAbbr = " C";

// Button Pins
#define BTN_INC 24  // Increment
#define BTN_DEC 22  // Decrement
#define BTN_MENU 34
#define BTN_SELECT 36
#define SW_UNITS 38
#define BTN_PRESSHOLD_DUR 1250

// LED Pins
#define LED_STS_OK 8 //Small green led
//#define LED_STS_ERR  //Small red led
#define LED_UPDATE_INTERVAL 9
#define LED_RGB_RED 10
#define LED_RGB_GRN 11
#define LED_RGB_BLUE 12
#define LED_LOG_STS_RED 2
#define LED_LOG_STS_GRN 3

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
  AllWeatherData,
  MinMaxRecords,
  AboutScreen
};

static Temperature T;
CurrentLCDView LCD_curView = AllWeatherData;
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
 
// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
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
  pinMode(53, OUTPUT);
  
  // Setup LED pins
  pinMode(LED_STS_OK, OUTPUT);
  //pinMode(LED_STS_ERR, OUTPUT);
  pinMode(LED_UPDATE_INTERVAL, OUTPUT);
  pinMode(LED_RGB_RED, OUTPUT);
  pinMode(LED_RGB_GRN, OUTPUT);
  pinMode(LED_RGB_BLUE, OUTPUT);
  pinMode(LED_LOG_STS_RED, OUTPUT);
  pinMode(LED_LOG_STS_GRN, OUTPUT);
    
    // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if(SDENABLE) {
    if (!card.init(SPI_QUARTER_SPEED, chipSelect)) {
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
  }
  
  // Setup 4x20 LCD
  lcd.begin(20, 4);
  lcd.clear();
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
  lcdprint("MicroWXStation", 0);
  lcdprint("v0.2.1 (HW rev 3)", 1);
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
  digitalWrite(LED_STS_OK, ON);
  lcd.setCursor(j,3); j++;
  lcd.print(".");  
  digitalWrite(LED_RGB_RED, OFF);
  delay(BOOT_DELAY_INTERVAL);
  lcd.setCursor(j,3); j++;
  lcd.print(".");  
  delay(BOOT_DELAY_INTERVAL); 
  digitalWrite(LED_RGB_RED, ON);
  digitalWrite(LED_RGB_GRN, OFF);
  lcd.setCursor(j,3); j++;
  lcd.print(".");  
  delay(BOOT_DELAY_INTERVAL); 
  lcd.setCursor(j,3); j++;
  lcd.print(".");  
  delay(BOOT_DELAY_INTERVAL); 
  digitalWrite(LED_RGB_GRN, ON);
  digitalWrite(LED_RGB_BLUE, OFF);
  lcd.setCursor(j,3); j++;
  lcd.print(".");  
  delay(BOOT_DELAY_INTERVAL); 
  digitalWrite(LED_RGB_BLUE, ON);
  lcd.setCursor(j,3); j++;
  lcd.print(".");  
  delay(BOOT_DELAY_INTERVAL / 2);
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
 
  if(!DisableDHT22) {
    UINT_DHT++;
    if(UINT_DHT > UTHOLD_DHT) {
      UINT_DHT= 0;
      int chk = DHT.read22(DHT22_PIN);
      int DHT22ERR_COUNTER = 0;
      switch (chk)
      {
        case DHTLIB_OK:  
          //Serial.println("DHT22 - OK,\t");
          break;
        case DHTLIB_ERROR_CHECKSUM:
          Serial.println("DHT22 Checksum error!");
          lcdprint("DHT22 CKSUM FAIL!", -1);
          lcdprint("Hold MENU to disable", 1);
          for(int i=0;i<800;i++) {
            DHT22ERR_COUNTER = 0;
            while(digitalRead(BTN_MENU) == HIGH && !DisableDHT22) {
              DHT22ERR_COUNTER++;
              if(DHT22ERR_COUNTER == 300) {
                 DisableDHT22 = true;
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
          break;
        case DHTLIB_ERROR_TIMEOUT:
          Serial.println("DHT22 Time out error!");
          lcdprint("DHT22 TIMEOUT ERR!", -1);
          for(int i=0;i<800;i++) {
            DHT22ERR_COUNTER = 0;
            while(digitalRead(BTN_MENU) == HIGH && !DisableDHT22) {
              DHT22ERR_COUNTER++;
              if(DHT22ERR_COUNTER == 300) {
                 DisableDHT22 = true;
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
          break;
        default:
          Serial.println("DHT22 Unknown error!");
          lcdprint("DHT22 UNKNOWN ERR!", -1);
          for(int i=0;i<800;i++) {
            DHT22ERR_COUNTER = 0;
            while(digitalRead(BTN_MENU) == HIGH && !DisableDHT22) {
              DHT22ERR_COUNTER++;
              if(DHT22ERR_COUNTER == 300) {
                 DisableDHT22 = true;
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
          break;
      }
    }
  }
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
  temperature = (double)T.dht_c;
  pressure = (float)bmp.readPressure() / 100;
  altitude = (float)bmp.readAltitude();
  dewpoint = dewPointFast(T.dht_c, humidity);
  if (digitalRead(SW_UNITS) == ON) { dewpoint = Fahrenheit(dewpoint); }
  if (digitalRead(SW_UNITS) == ON) { temperature = (double)T.dht_f; }
  UnitAbbr = (digitalRead(SW_UNITS) == ON) ? "F" : "C"; 
  
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
    MinDewPoint = dewpoint;
  }
  if(dewpoint > MaxDewPoint) {
    MaxDewPoint = dewpoint;
  }
  /*
  Serial.println("---> BMP085 Data <---");
  Serial.print("Temperature: ");
  Serial.print((double)(T.bmp_c));
  Serial.print("C / ");
  Serial.print(T.bmp_f);
  Serial.println("F");
  Serial.print("Pressure: ");
  Serial.print((double)pressure);
  Serial.println("mb");
  Serial.print("Altitude: ");
  Serial.print(bmp.readAltitude());
  Serial.println("m");
  Serial.println("---> DHT22 Data <---");
  Serial.print("Temperature (DHT22): ");
  Serial.print((double)T.dht_c);
  Serial.print("C / ");
  Serial.print(T.dht_f);
  Serial.println("F");
  Serial.print("Relative Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  Serial.print("Dew Point: ");
  Serial.print(dewpoint);
  Serial.print("C / ");
  Serial.print(Fahrenheit(dewpoint));
  Serial.println("F");
  */
  int btnMenu_S = digitalRead(BTN_MENU);
  int btnSelect_S = digitalRead(BTN_SELECT);
  if(btnMenu_S == LOW || btnSelect_S == LOW) { 
    Buttons_Pressed_Time = 0; 
  }
  if(btnMenu_S == HIGH && btnSelect_S == HIGH) { 
    if(Buttons_Pressed_Time == 0) { 
      Buttons_Pressed_Time = millis(); 
    }
    if(millis() - Buttons_Pressed_Time > BTN_PRESSHOLD_DUR) {
      lcdprint("Simultaneus Btn", -1);
      lcdprint("Press!!", 1);
      delay(2000);
    }
  } else { 
    if(btnSelect_S != btnSelect_LS) {
      if(btnSelect_S == HIGH) {
        Serial.println("Select Button PRESSED!");
        LCD_curView = AllWeatherData;
      } else {
        Serial.println("Select Button RELEASED!");
      }
    }
    if(btnMenu_S != btnMenu_LS) {
      if(btnMenu_S == HIGH) {
        Serial.println("Menu Button PRESSED!");
       } else {
        Serial.println("Menu Button RELEASED!");
       }
      }
    btnMenu_LS = btnMenu_S;
    btnSelect_LS = btnSelect_S;
  }
  
  // LCD Buttons Handling
  int btnDown_S = digitalRead(BTN_DEC);
  int btnUp_S = digitalRead(BTN_INC);
  if(btnDown_S == LOW || btnUp_S == LOW) { 
    Buttons_Pressed_Time = 0; 
  }
  if(btnDown_S == HIGH || btnUp_S == HIGH) { 
    if(btnDown_S != btnDown_LS) {
      if(btnDown_S == HIGH) {
        Serial.println("Down Button PRESSED!");
      } else {
        Serial.println("Down Button RELEASED!");
      }
    }
    if(btnUp_S != btnUp_LS) {
      if(btnUp_S == HIGH) {
        Serial.println("Up Button PRESSED!");
       } else {
        Serial.println("Up Button RELEASED!");
       }
      }
    btnUp_LS = btnUp_S;
    btnDown_LS = btnDown_S;
  }

  //Wait until LCD update interval to update the LCD
  UINT_LCD++;
  if(UINT_LCD > UTHOLD_LCD) {
    UINT_LCD = 0;
      switch(LCD_curView) {
      case AllWeatherData:
        LCD_showAllData();
        break;
      case MinMaxRecords:
        LCD_MinMaxRecords();
        break;
      case AboutScreen:
        LCD_showAboutScreen();
        break;
      default:
        LCD_showAllData();
        break;
    }
  }
  
  if(SDENABLE) {
    UINT_SD++;
     if(UINT_SD > UTHOLD_SD) {
      //Create Data string for storing to SD card
      //We will use CSV Format  
      String dataString = String(id) + ", " + String((int)(T.dht_c*10)) + ", " + String((int)humidity) + ", " + String((long)(pressure)) + ", " + String((int)dewPoint(T.dht_c, humidity)); 
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
      //Increment ID number
      id++; 
     }
  }
  delay(100);
}

void LCD_showAboutScreen() {
  
}

void LCD_MinMaxRecords() {
  
}

void LCD_showAllData() { // Show all current weather data on the infomation LCD (4x20)
  lcdprint("Temp: ", 0);
  lcd.print(temperature);
  lcd.write(0b11011111);
  lcd.print(UnitAbbr);
  lcdprint("Pressure: ", 1);
  lcd.print((double)pressure);
  lcd.print("mb");
  lcdprint("Humidity: ", 2);
  if(DisableDHT22) {lcd.print("DISABLED"); } else { lcd.print(humidity); lcd.print("%"); }
  lcdprint("DewPoint: ", 3);
  if(DisableDHT22) {lcd.print("DISABLED"); } else { lcd.print(dewpoint); lcd.write(0b11011111); lcd.print(UnitAbbr); }
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
   case 0:
    lcd.setCursor(0,0);
    break;
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
