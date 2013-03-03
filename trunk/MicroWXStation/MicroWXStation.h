/*
 * 
 * MicroWXStation for Arduino Mega 2560 r3 - Version 0.2.5 
 * Copyright (C) 2013, Tyler H. Jones (me@tylerjones.me)
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
 * Filename: MicroWXStation.h
 * 
 * Description: The main header file associated with the Arduino IDE sketch. 
 * Contains variable declarations and vaule definitions along with other
 * static code that does not need to be in MicroWXStation.ide where it might
 * be changed.
 *
 */

#ifndef MicoWXStation_H
#define MicoWXStation_H

/*
 * 
 * MicroWXStation for Arduino Mega 2560 r3 - Version 0.2.5 
 * Copyright (C) 2013, Tyler H. Jones (me@tylerjones.me)
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
 * Filename: MicroWXStation.h
 * 
 * Description: The main header file associated with the Arduino IDE sketch. 
 * Contains variable declarations and vaule definitions along with other
 * static code that does not need to be in MicroWXStation.ino where it might
 * be changed.
 *
 */
 
#include "Arduino.h"

//General Definitions
#define SDENABLE false // Enable/disable SD card logging
#define BOOT_DELAY_INTERVAL 400 // Delay (in ms) between boot up checks and hardware tests
// Custom LCD Character code definitions
#define CHAR_DEGF 4
#define CHAR_DEGC 3
#define CHAR_UPARROW 1
#define CHAR_DOWNARROW 2
#define CHAR_MICRO 5
// RGB LED color states
#define RGB_OFF 0
#define RGB_RED 1
#define RGB_BLUE 2
#define RGB_GRN 3
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
// NES Controller pinss
#define NES_LATCH_PIN 41
#define NES_CLK_PIN 40
#define NES_SER_PIN 42
#define DHT22_PIN 7

// Typedef declarations
// Temperature typedef (Celcius and Fahrenheit)
typedef struct {
  float bmp_c;     // BMP085 Temperature value (in Celcius)
  float dht_c;     // DHT22 Temperature value (in Celcius)
  double bmp_f;    // BMP085 Temperature value (in Fahrenheit)
  double dht_f;    // DHT22 Temperature value (in Fahrenheit)
} Temperature;

//Enum for holding the current view for the GLCD
enum CurrentLCDView {
  CurrentWXData,
  NonWXData,
  MinMaxValues,
  AboutInfo,
  MainMenu
};

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

//Celsius to Fahrenheit conversion
double Fahrenheit(double celsius)
{
        return 1.8 * celsius + 32;
}

// ------------------------------------------------------------------------------
// LED control functions 

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

#endif /* MicoWXStation_h  */
