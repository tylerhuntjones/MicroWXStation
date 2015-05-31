/*
 * 
 * MicroWXStation_v2 for Arduino Mega 2560 r3 - Version 0.1.0 
 * Copyright (C) 2015, Tyler H. Jones (me@tylerjones.me)
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
 * Filename: MicroWXStation_v2.h
 * 
 * Description: The main header file associated with the Arduino IDE sketch. 
 * Contains variable declarations and vaule definitions along with other
 * static code that does not need to be in MicroWXStation_v2.ino where it might
 * be changed.
 * 
 */
 
#ifndef MicoWXStation_v2_H
#define MicoWXStation_v2_H
 
#include "Arduino.h"

// General Definitions
#define VIEW_HOME 0
#define VIEW_MENU 1
#define VIEW_MINMAX 2
#define VIEW_ALL 3
#define VIEW_WIND 4

// Typedef declarations
// Temperature typedef (Celcius and Fahrenheit)
typedef struct {
  float bmp_c;     // BMP085 Temperature value (in Celcius)
  float dht_c;     // DHT22 Temperature value (in Celcius)
  double bmp_f;    // BMP085 Temperature value (in Fahrenheit)
  double dht_f;    // DHT22 Temperature value (in Fahrenheit)
} Temperature;

// Min/Max values struct for EEPROM storage
struct MinMax_t {
  double MinTemperature;
  double MaxTemperature;
  float MinPressure;
  float MaxPressure;
  int MinHumidity;
  int MaxHumidity;
  double MinDewPoint;
  double MaxDewPoint;
  float MinAltitude;
  float MaxAltitude;
} MinMax;

// Enum for holding the current view for the GLCD
enum CurrentLCDView {
  CurrentWXData,
  NonWXData,
  MinMaxValues,
  AboutInfo,
  MainMenu
};

// ------------------------------------------------------------------------------
// Metorlogical Calculation Functions

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

// Celsius to Fahrenheit conversion
double Fahrenheit(double celsius)
{
        return 1.8 * celsius + 32;
}


#endif /* MicoWXStation_v2_h  */
