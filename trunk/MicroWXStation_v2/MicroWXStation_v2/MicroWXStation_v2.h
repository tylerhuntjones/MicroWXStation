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
#define VIEW_MINMAX 1
#define VIEW_ALL 2
#define VIEW_WIND 3
#define VIEW_MENU 10
#define VIEW_ABOUT 11

#define VIEW_TITLE_XPOS 6
#define VIEW_TITLE_YPOS 220

#define FRAME_R_COLOR 255
#define FRAME_G_COLOR 255
#define FRAME_B_COLOR 255
#define FRAME_X_START 4
#define FRAME_Y_START 5
#define FRAME_X_END 315
#define FRAME_Y_END 205
#define HOME_DIV_X_START 160
#define HOME_DIV_Y_START 35
#define HOME_DIV_X_END 160
#define HOME_DIV_Y_END 197

#define WDIRIND_CENTER_X 240
#define WDIRIND_CENTER_Y 140
#define WDIRIND_RADIUS 45
#define WDIRIND_LINE_LENGTH 50
#define WDIRIND_W 185
#define WDIRIND_NW 190
#define WDIRIND_SW 190
#define WDIRIND_E 295
#define WDIRIND_NE 190
#define WDIRIND_SE 190
#define WDIRIND_N 240
#define WDIRIND_S 240
#define WDIRIND_NNW 190
#define WDIRIND_SSW 190
#define WDIRIND_NNE 190
#define WDIRIND_SSE 190
#define WDIRIND_WSW 190
#define WDIRIND_WNW 190
#define WDIRIND_ESE 190
#define WDIRIND_ENE 190

#define SD_CHIP_SELECT  53  // SD chip select pin

// Typedef declarations
typedef struct {
  float TempF;
  float TempC;
  float Pressure;
  float PressurePascals;
  float Humidity;
  float WindSpeed;
  float WindGustSpeed;
  int WindGustDir;
  float WindSpeedAvg2M;
  float WindSpeedAvg10M;
  float WindGustSpeed10M;
  int WindGustDir10M;
  int WindDirection;
  int WindDirAvg2M;
  float LightLevel;
  double BatteryLevel;
  double Rainfall;
  double DailyRainfall;
} WXData;

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
  double MaxWind;
  double MaxRain;
  double MaxLight;
} MinMax;

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

// Celsius to Fahrenheit conversion
double Celcius(double f)
{
        return (f - 32) / 1.8;
}

#endif /* MicoWXStation_v2_h  */
