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

// Setup NES Controller
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



#endif /* MicoWXStation_h  */
