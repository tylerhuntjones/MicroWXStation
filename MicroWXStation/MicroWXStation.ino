#include <string.h>
#include <Wire.h>
#include <dht.h>
//#include <PCD8544.h>
#include <Adafruit_BMP085.h>
#include <LiquidCrystal.h>

//General Definitions
#define ON HIGH
#define OFF LOW

// GLCD Configuration Definitons
//PCD8544 glcd(9, 10, 11, 12, 13); //LCD Pins: (SCLK, SDIN, DC, RESET, SCE)                 
#define LCD_WIDTH 84
#define LCD_HEIGHT 48

// LCD (4x20) Configuration
LiquidCrystal lcd(33, 31, 29, 27 ,25, 23, 32, 30, 28, 26);

// The number of lines for the temperature chart...
#define CHART_HEIGHT 4
#define CHART_MIN_TEMP 15



#define CHART_MAX_TEMP 80
#define TEMPCHART_UPDATE_THRESHOLD 6
// A custom "degrees" symbol...
//#define DEGREES_CHAR 1
//static const byte degrees_glyph[] = { 0x00, 0x07, 0x05, 0x07, 0x00 };
// A bitmap graphic (10x2) of a thermometer...
//#define THERMO_WIDTH 10
//#define THERMO_HEIGHT 2
//static const byte thermometer[] = { 0x00, 0x00, 0x48, 0xfe, 0x01, 0xfe, 0x00, 0x02, 0x05, 0x02, 0x00, 0x00, 0x62, 0xff, 0xfe, 0xff, 0x60, 0x00, 0x00, 0x00};
long LastTempChartUpdate = 1;
int TempChartUpdateCounter = 0;
double MinTemperature = 99;
double MaxTemperature = 0;

// Barometric pressure chart
#define CHART_MIN_PRES 950
#define CHART_MAX_PRES 1050
#define PRESCHART_UPDATE_THRESHOLD 6
long LastPresChartUpdate = 1;
int PresChartUpdateCounter = 0;
float MinPressure = 1100;
float MaxPressure = 900;

// Humidity chart
#define CHART_MIN_HUMIDITY 0
#define CHART_MAX_HUMIDITY 100
#define HGMCHART_UPDATE_THRESHOLD 6
long LastHgmChartUpdate = 1;
int HgmChartUpdateCounter = 0;
int MinHumidity = 100;
int MaxHumidity = 0;

// Dew Point chart
#define CHART_MIN_DEWPOINT 20
#define CHART_MAX_DEWPOINT 90
#define DEWCHART_UPDATE_THRESHOLD 6
long LastDewChartUpdate = 1;
int DewChartUpdateCounter = 0;
double MinDewPoint = 120;
double MaxDewPoint = 0;

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
double temperature; // In F or C dependng on SWITCH_UNITS current state
double dewpoint;    // Celcius
float altitude;     // Altitude

// Button Pins
#define BTN_CHART_NEXT 16
#define BTN_VIEW_DATA 17
#define BTN_INC 24  // Increment
#define BTN_DEC 22  // Decrement
#define BTN_MENU 34
#define BTN_SELECT 36
#define SWITCH_UNITS 38
#define BTN_PRESSHOLD_DUR 1250

// Button debounce variables
int lastbtnChartNext_state = 0;
int lastbtnViewAllData_state = 0;
int lastbtnMenu_state = 0;
int lastbtnSelect_state = 0;
int lastbtnUp_state = 0;
int lastbtnDown_state = 0;
int Buttons_Pressed_Time = 0; //Time in milliseconds


//Enum for holding the current view for the GLCD
enum CurrentChartView {
  AllData,
  TempChart,
  HgmChart,
  PresChart,
  DewChart
};

//Enum for holding the current view for the GLCD
enum CurrentLCDView {
  AllWeatherData,
  MinMaxRecords,
  AboutScreen
};

static Temperature T;
CurrentChartView GLCD_curView = AllData;
CurrentLCDView LCD_curView = AllWeatherData;

// ------------------------------------------------------------------------------
// setup() Core Function

void setup(void)
{
  Serial.begin(9600);
  
  // Declare input/output pins
  pinMode(BTN_CHART_NEXT, INPUT);
  pinMode(BTN_VIEW_DATA, INPUT);

  // Setup the graphic LCD
  //glcd.begin(LCD_WIDTH, LCD_HEIGHT);
  //glcd.createChar(DEGREES_CHAR, degrees_glyph);
  //glcd.clear();
  // Setup 4x20 LCD
  lcd.begin(20, 4);
  lcd.clear();
  lcd.print("Starting up");
  delay(200);
  lcd.setCursor(0,2);
  lcd.print(".");  
  delay(110);
  if (!bmp.begin()) { // Setup BMP085 Barometer
	Serial.println("*** No BMP085 barometer sensor found! Check wiring and try again! ***");
	while (1) {}
  }
  lcd.setCursor(1,2);
  lcd.print(".");  
  delay(110);
  
  // Display Boot Splash Text on GLCD
  //glcd.setCursor(0,0);
  //glcd.print("MicroWXStation");
  //glcd.setCursor(0,1);
  //glcd.print("v0.1 (HWrev 1)");
  //glcd.setCursor(0,2);
  //glcd.print("http://thj.me");
  //glcd.setCursor(0,5);
  //glcd.print("Booting");
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
  // Start "Booting." animation
  //glcd.setCursor(42,5);
  //glcd.print("."); // "Booting."
  //delay(350);
  //glcd.setCursor(48,5);
  //glcd.print("."); // "Booting.."
  //delay(350);
  //glcd.setCursor(54,5);
  //glcd.print("."); // "Booting..."
  //delay(350);
  //glcd.setCursor(60,5);
  //glcd.print("."); // "Booting...."
  //delay(350);
  //glcd.setCursor(66,5);
  //glcd.print("."); // "Booting....."
  //delay(350);
  //glcd.setCursor(72,5);
  //glcd.print("OK"); // "Booting.....OK"
  //delay(2000);
  //glcd.clear();
}

// ------------------------------------------------------------------------------
// loop() Core Function

void loop(void)
{
  Serial.println();
  Serial.println("--------------------");
  Serial.println();
  int chk = DHT.read22(DHT22_PIN);
  switch (chk)
  {
    case DHTLIB_OK:  
      Serial.println("DHT22 - OK,\t");
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.println("DHT22 - Checksum error,\t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.println("DHT22 - Time out error,\t");
      break;
    default:
      Serial.println("DHT22 - Unknown error,\t");
      break;
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
  if (digitalRead(SWITCH_UNITS) == ON) { dewpoint = Fahrenheit(dewpoint); }
  if (digitalRead(SWITCH_UNITS) == ON) { temperature = (double)T.dht_f; }
  delay(350);
  
  if(T.bmp_f < MinTemperature) {
    MinTemperature = (digitalRead(SWITCH_UNITS) == ON) ? T.bmp_f : (double)T.bmp_c;
  }
  if(T.bmp_f > MaxTemperature) {
    MaxTemperature = (digitalRead(SWITCH_UNITS) == ON) ? T.bmp_f : (double)T.bmp_c;
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
  
  int btnChartNext_state = digitalRead(BTN_CHART_NEXT);
  int btnViewAllData_state = digitalRead(BTN_VIEW_DATA);
  if(btnChartNext_state == LOW || btnViewAllData_state == LOW) { 
    Buttons_Pressed_Time = 0; 
  }
  if(btnChartNext_state == HIGH && btnViewAllData_state == HIGH) { 
    if(Buttons_Pressed_Time == 0) { 
      Buttons_Pressed_Time = millis(); 
    }
    if(millis() - Buttons_Pressed_Time > BTN_PRESSHOLD_DUR) {
      //glcd.clear();
      //glcd.print("Simul. press clear!");
      lcd.clear();
      lcd.print("Simul. press clear!");
      delay(1000);
    }
  } else { 
    if(btnViewAllData_state != lastbtnViewAllData_state) {
      if(btnViewAllData_state == HIGH) {
        Serial.println("View ALl Data Button PRESSED!");
        GLCD_curView = AllData;
        //glcd.clear();
      } else {
        Serial.println("View ALl Data Button RELEASED!");
      }
    }
    if(btnChartNext_state != lastbtnChartNext_state) {
      if(btnChartNext_state == HIGH) {
        Serial.println("Next Chart Button PRESSED!");
        if(GLCD_curView == AllData || GLCD_curView == DewChart) {
          GLCD_curView = TempChart; 
          //glcd.clear();
          //glcd.setCursor(0,0);
          //glcd.print("Temperature");
          //delay(750);
          //glcd.clear();
        } else {
           if(GLCD_curView == PresChart) {
             GLCD_curView = DewChart;
             //glcd.clear();
             //glcd.setCursor(0,0);
             //glcd.print("Dew Point (\001F)");
             //delay(750);
             //glcd.clear();
           }
           if(GLCD_curView == HgmChart) {
             GLCD_curView = PresChart; 
             //glcd.clear();
             //glcd.setCursor(0,0);
             //glcd.print("Pressure (mb)");
             //delay(750);
             //glcd.clear();
           }
           if(GLCD_curView == TempChart) {
             GLCD_curView = HgmChart;
             //glcd.clear();
             //glcd.setCursor(0,0);
             //glcd.print("Humidity");
             //delay(750);
             //glcd.clear();
           } 
        }
       } else {
        Serial.println("Next Chart Button RELEASED!");
       }
      }
    lastbtnChartNext_state = btnChartNext_state;
    lastbtnViewAllData_state = btnViewAllData_state;
  }
  
  // LCD Buttons Handling
  int btnMenu_state = digitalRead(BTN_MENU);
  int btnSelect_state = digitalRead(BTN_SELECT);
  if(btnSelect_state == LOW || btnMenu_state == LOW) { 
    Buttons_Pressed_Time = 0; 
  }
  if(btnSelect_state == HIGH && btnMenu_state == HIGH) { 
    if(Buttons_Pressed_Time == 0) { 
      Buttons_Pressed_Time = millis(); 
    }
    if(millis() - Buttons_Pressed_Time > BTN_PRESSHOLD_DUR) {
      lcd.clear();
      lcd.print("Simul. press clear!");
      delay(1000);
    }
  } else { 
    if(btnMenu_state != lastbtnMenu_state) {
      if(btnMenu_state == HIGH) {
        Serial.println("Menu Button PRESSED!");
      } else {
        Serial.println("Menu Button RELEASED!");
      }
    }
    if(btnSelect_state != lastbtnSelect_state) {
      if(btnSelect_state == HIGH) {
        Serial.println("Select Button PRESSED!");
       } else {
        Serial.println("Select Button RELEASED!");
       }
      }
    lastbtnSelect_state = btnSelect_state;
    lastbtnMenu_state = btnMenu_state;
  }
    // LCD Buttons Handling
  int btnDown_state = digitalRead(BTN_DEC);
  int btnUp_state = digitalRead(BTN_INC);
  if(btnDown_state == LOW || btnUp_state == LOW) { 
    Buttons_Pressed_Time = 0; 
  }
  if(btnDown_state == HIGH || btnUp_state == HIGH) { 
    if(btnDown_state != lastbtnDown_state) {
      if(btnDown_state == HIGH) {
        Serial.println("Down Button PRESSED!");
      } else {
        Serial.println("Down Button RELEASED!");
      }
    }
    if(btnUp_state != lastbtnUp_state) {
      if(btnUp_state == HIGH) {
        Serial.println("Up Button PRESSED!");
       } else {
        Serial.println("Up Button RELEASED!");
       }
      }
    lastbtnUp_state = btnUp_state;
    lastbtnDown_state = btnDown_state;
  }

  switch(GLCD_curView) {
    case AllData:
      GLCD_showAllData();
      break;
    case TempChart:
      showTempChart();
      break;
    case HgmChart:
      showHgmChart();
      break;
    case PresChart:
      showPresChart();
      break;
    case DewChart:
      showDewChart();
      break;
    default:
      GLCD_showAllData();
      break;
  }
  
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

void LCD_showAboutScreen() {
  
}

void LCD_MinMaxRecords() {
  
}

void showDewChart() {
  static byte xChart = LCD_WIDTH;
  
  //glcd.setCursor(24, 0);
  //glcd.print(dewpoint, 1);
  //glcd.print(" \001F");
  
  // Update the humdity chart...
  if(millis() - LastDewChartUpdate > 10000) {
    DewChartUpdateCounter++;
    if(DewChartUpdateCounter >= DEWCHART_UPDATE_THRESHOLD) {
      // Wrap the chart's current position...
      if (xChart >= LCD_WIDTH) {
        xChart = 1;
      }
      //glcd.setCursor(xChart, 2);
      //glcd.drawColumn(CHART_HEIGHT, map(dewpoint, CHART_MIN_DEWPOINT, CHART_MAX_DEWPOINT, 0, CHART_HEIGHT*8));  // ...clipped to the 0-45C range.
      //glcd.drawColumn(CHART_HEIGHT, 0);         // ...with a clear marker to see the current chart position.
      xChart++;
      DewChartUpdateCounter = 0;
    }
    LastDewChartUpdate = millis();
    if(DewChartUpdateCounter == 1 || DewChartUpdateCounter == 3 || DewChartUpdateCounter == 5) {
      //glcd.setCursor(0, 1);
      //glcd.print("Range 20-90 \001F");
    } else {
      //glcd.setCursor(0, 1);
      //glcd.print("               ");
      //glcd.setCursor(0, 1);
      //glcd.print("Min ");
      //glcd.print(MinDewPoint);
      //glcd.print("/Max ");
      //glcd.print(MaxDewPoint);     
    }
  }
}

void showHgmChart() {
  static byte xChart = LCD_WIDTH;
  
  //glcd.setCursor(30, 0);
  //glcd.print(humidity, 1);
  //glcd.print("%");
  
  // Update the humdity chart...
  if(millis() - LastHgmChartUpdate > 10000) {
    HgmChartUpdateCounter++;
    if(HgmChartUpdateCounter >= HGMCHART_UPDATE_THRESHOLD) {
      // Wrap the chart's current position...
      if (xChart >= LCD_WIDTH) {
        xChart = 1;
      }
      //glcd.setCursor(xChart, 2);
      //glcd.drawColumn(CHART_HEIGHT, map(humidity, CHART_MIN_HUMIDITY, CHART_MAX_HUMIDITY, 0, CHART_HEIGHT*8));  // ...clipped to the 0-45C range.
      //glcd.drawColumn(CHART_HEIGHT, 0);         // ...with a clear marker to see the current chart position.
      xChart++;
      HgmChartUpdateCounter = 0;
    }
    LastHgmChartUpdate = millis();
    if(HgmChartUpdateCounter == 1 || HgmChartUpdateCounter == 3 || HgmChartUpdateCounter == 5) {
      //glcd.setCursor(0, 1);
      //glcd.print("Range 0-100%");
    } else {
      //glcd.setCursor(0, 1);
      //glcd.print("               ");
      //glcd.setCursor(0, 1);
      //glcd.print("Min ");
      //glcd.print(MinHumidity);
      //glcd.print("/Max ");
      //glcd.print(MaxHumidity);     
    }
  }
}

void showPresChart() {
  static byte xChart = LCD_WIDTH;
  
  //glcd.setCursor(12, 0);
  //glcd.print(pressure, 1);
  //glcd.print("mb");
  
  // Update the temperature chart...
  if(millis() - LastPresChartUpdate > 10000) {
    PresChartUpdateCounter++;
    if(PresChartUpdateCounter >= PRESCHART_UPDATE_THRESHOLD) {
      // Wrap the chart's current position...
      if (xChart >= LCD_WIDTH) {
        xChart = 1;
      }
      //glcd.setCursor(xChart, 2);
      //glcd.drawColumn(CHART_HEIGHT, map(pressure, CHART_MIN_PRES, CHART_MAX_PRES, 0, CHART_HEIGHT*8));  // ...clipped to the 0-45C range.
      //glcd.drawColumn(CHART_HEIGHT, 0);         // ...with a clear marker to see the current chart position.
      xChart++;
      PresChartUpdateCounter = 0;
    }
    LastPresChartUpdate = millis();
    if(PresChartUpdateCounter == 1 || PresChartUpdateCounter == 3 || PresChartUpdateCounter == 5) {
      //glcd.setCursor(0, 1);
      //glcd.print("Rng 950-1050mb");
    } else {
      //glcd.setCursor(0, 1);
      //glcd.print("               ");
      //glcd.setCursor(0, 1);
      //glcd.print("Min ");
      //glcd.print((int)MinPressure);
      //glcd.print("/Max ");
      //glcd.print((int)MaxPressure);     
    }
  }
}

void showTempChart() {
  static byte xChart = LCD_WIDTH;
  
  //glcd.setCursor(24, 0);
  //glcd.print(T.dht_f, 1);
  //glcd.print("\001F");
  //glcd.setCursor(0, LCD_HEIGHT/8 - THERMO_HEIGHT);
  //glcd.drawBitmap(thermometer, THERMO_WIDTH, THERMO_HEIGHT);

  // Update the temperature chart...
  if(millis() - LastTempChartUpdate > 10000) {
    TempChartUpdateCounter++;
    if(TempChartUpdateCounter >= TEMPCHART_UPDATE_THRESHOLD) {
      // Wrap the chart's current position...
      if (xChart >= LCD_WIDTH) {
        //xChart = THERMO_WIDTH + 2;
      }
      //glcd.setCursor(xChart, 2);
      //glcd.drawColumn(CHART_HEIGHT, map(T.bmp_f, CHART_MIN_TEMP, CHART_MAX_TEMP, 0, CHART_HEIGHT*8));  // ...clipped to the 0-45C range.
      //glcd.drawColumn(CHART_HEIGHT, 0);         // ...with a clear marker to see the current chart position.
      xChart++;
      TempChartUpdateCounter = 0;
    }
    LastTempChartUpdate = millis();
    if(TempChartUpdateCounter == 1 || TempChartUpdateCounter == 3 || TempChartUpdateCounter == 5) {
      //glcd.setCursor(0, 1);
      //glcd.print("Range 15-80 \001F");
    } else {
      //glcd.setCursor(0, 1);
      //glcd.print("               ");
      //glcd.setCursor(0, 1);
      //glcd.print("Min ");
      //glcd.print((int)MinTemperature);
      //glcd.print("/Max ");
      //glcd.print((int)MaxTemperature);     
    }
  }
}

void GLCD_showAllData() {
  //glcd.setCursor(0,0);
  //glcd.print("Temp:        ");
  //glcd.setCursor(36,0);  
  //glcd.print(T.dht_f);
  //glcd.print(" \001F");
  //glcd.setCursor(0,1);
  //glcd.print("Pres:         ");
  //glcd.setCursor(36,1);  
  //glcd.print((double)pressure);
  //glcd.setCursor(0,2);
  //glcd.print("Humid:         ");
  //glcd.setCursor(42,2);  
  //glcd.print(humidity);
  //glcd.print("%");
  //glcd.setCursor(0,3);
  //glcd.print("Dew:           ");
  //glcd.setCursor(30,3);  
  //glcd.print((double)dewpoint);
  //glcd.print(" \001F");
  //glcd.setCursor(0,4);
  //glcd.print("Alt:           ");
  //glcd.setCursor(30,4);  
  //glcd.print((double)altitude);
  //glcd.print("m");
}

void LCD_showAllData() { // Show all current weather data on the infomation LCD (4x20)
  lcd.setCursor(0,0);
  lcd.print("Temp:               ");
  lcd.setCursor(6,0);  
  lcd.print(temperature);
  lcd.print(" *F");
  lcd.setCursor(0,2);
  lcd.print("Pressure:           ");
  lcd.setCursor(10,2);  
  lcd.print((double)pressure);
  lcd.setCursor(0,1);
  lcd.print("Humidity:           ");
  lcd.setCursor(10,1);  
  lcd.print(humidity);
  lcd.print("%");
  lcd.setCursor(0,3);
  lcd.print("DewPoint:           ");
  lcd.setCursor(10,3);  
  lcd.print(dewpoint);
  lcd.print(" F");
}
// ------------------------------------------------------------------------------
// Metorlogical Calculations Functions

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
