#include <string.h>
#include <Wire.h>
#include <dht.h>
#include <Adafruit_BMP085.h>
#include <LiquidCrystal.h>
#include <SD.h>

//General Definitions
#define ON HIGH
#define OFF LOW

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

// Button debounce variables
int btnMenu_lastS = 0;
int btnSelect_lastS = 0;
int btnUp_lastS = 0;
int btnDown_lastS = 0;
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
  
    // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
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
  
-t5 (

  // Setup 4x20 LCD
  lcd.begin(20, 4);
  lcd.clear();
  if (!bmp.begin()) { // Setup BMP085 Barometer
        lcd.print("Barometer FAIL!");
	Serial.println("*** No BMP085 barometer sensor found! Check wiring and try again! ***");
	while (1) {}
  }
  lcd.setCursor(0,0);
  lcd.print("MicroWXStation");
  lcd.setCursor(0,2);
  lcd.print("v0.1 (HW rev 1)");
  lcd.setCursor(0,1);
  lcd.print("Tyler Jones (thj.me)");
  lcd.setCursor(0,3);
  lcd.print("Loading");  
  for(int i=7;i<20;i++) {
    lcd.setCursor(i,3);
    lcd.print(".");  
    delay(200); 
  }
  delay(2000);
  lcd.clear();
}

// ------------------------------------------------------------------------------
// loop() Core Function

void loop(void)
{
  //Serial.println("--------------------");
  
  UINT_DHT++;
  if(UINT_DHT > UTHOLD_DHT) {
    UINT_DHT= 0;
    int chk = DHT.read22(DHT22_PIN);
    switch (chk)
    {
      case DHTLIB_OK:  
        //Serial.println("DHT22 - OK,\t");
        break;
      case DHTLIB_ERROR_CHECKSUM:
        Serial.println("DHT22 - Checksum error,\t");
        lcd.print("Hgm/Thermo FAIL!");
        delay(2000);
        break;
      case DHTLIB_ERROR_TIMEOUT:
        Serial.println("DHT22 - Time out error,\t");
        lcd.print("Hgm/Thermo FAIL!");
        delay(2000);
        break;
      default:
        Serial.println("DHT22 - Unknown error,\t");
        lcd.print("Hgm/Thermo FAIL!");
        delay(2000);
        break;
    }
  }
  
  T.dht_c = (float)DHT.temperature;
  T.dht_f = Fahrenheit((double)DHT.temperature);
  T.bmp_c = (float)bmp.readTemperature();
  T.bmp_f = Fahrenheit((double)bmp.readTemperature());
  temperature = (double)T.dht_c;
  humidity = DHT.humidity;
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
      lcd.clear();
      lcd.print("Simultaneus Btn");
      lcd.print("Press!!");
      delay(2000);
    }
  } else { 
    if(btnSelect_S != btnSelect_lastS) {
      if(btnSelect_S == HIGH) {
        Serial.println("Select Button PRESSED!");
        LCD_curView = AllWeatherData;
      } else {
        Serial.println("Select Button RELEASED!");
      }
    }
    if(btnMenu_S != btnMenu_lastS) {
      if(btnMenu_S == HIGH) {
        Serial.println("Menu Button PRESSED!");
       } else {
        Serial.println("Menu Button RELEASED!");
       }
      }
    btnMenu_lastS = btnMenu_S;
    btnSelect_lastS = btnSelect_S;
  }
  
  // LCD Buttons Handling
  int btnDown_S = digitalRead(BTN_DEC);
  int btnUp_S = digitalRead(BTN_INC);
  if(btnDown_S == LOW || btnUp_S == LOW) { 
    Buttons_Pressed_Time = 0; 
  }
  if(btnDown_S == HIGH || btnUp_S == HIGH) { 
    if(btnDown_S != btnDown_lastS) {
      if(btnDown_S == HIGH) {
        Serial.println("Down Button PRESSED!");
      } else {
        Serial.println("Down Button RELEASED!");
      }
    }
    if(btnUp_S != btnUp_lastS) {
      if(btnUp_S == HIGH) {
        Serial.println("Up Button PRESSED!");
       } else {
        Serial.println("Up Button RELEASED!");
       }
      }
    btnUp_lastS = btnUp_S;
    btnDown_lastS = btnDown_S;
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
  
  UINT_SD++;
   if(UINT_SD > UTHOLD_SD) {
    //Create Data string for storing to SD card
    //We will use CSV Format  
    String dataString = String(id) + ", " + String(T.dht_c) + ", " + String(humidity) + ", " + String(pressure) + ", " + String(dewPointFast(T.dht_c, humidity)); 
    
    //Open a file to write to
    //Only one file can be open at a time
    File logFile = SD.open("LOG.csv", FILE_WRITE);
    if (logFile)
    {
      logFile.println(dataString);
      logFile.close();
      Serial.println(dataString);
    }
    else
    {
      Serial.println("Couldn't open log file");
    }
    
    //Increment ID number
    id++; 
   }
  delay(50);
}

void LCD_showAboutScreen() {
  
}

void LCD_MinMaxRecords() {
  
}

void LCD_showAllData() { // Show all current weather data on the infomation LCD (4x20)
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.write(0b11011111);
  lcd.print(UnitAbbr);
  lcd.setCursor(0,2);
  lcd.print("Pressure: ");
  lcd.print((double)pressure);
  lcd.print("mb");
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");
  lcd.setCursor(0,3);
  lcd.print("DewPoint: ");
  lcd.print(dewpoint);
  lcd.write(0b11011111);
  lcd.print(UnitAbbr);
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
        double a = 17.271;
        double b = 237.7;
        double temp = (a * celsius) / (b + celsius) + log(humidity/100);
        double Td = (b * temp) / (a - temp);
        return Td;
}
