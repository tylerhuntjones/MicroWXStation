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
 * Filename: MicroWXStation_v2.ino
 * 
 * Description: The primary source file for the main controller board for
 * MicroWXStation_v2. This contains all the working code for interfacing
 * with the sensors via serial from the SparkfunWeatherShield and communicate
 * over the network with the logging/database/website server.
 * 
 */
 
#include "MicroWXStation_v2.h"
#include <UTFT.h>
#include <ITDB02_Touch.h>
#include <UTFT_Buttons_ITDB.h>

// Declare which fonts we will be using
extern uint8_t SmallFont[];
extern uint8_t BigFont[];

// Setup the TFT scree
UTFT myGLCD(SSD1289,38,39,40,41);

// Setup the touch screen
ITDB02_Touch myTouch(6,5,4,3,2);

// Set up UTFT_Buttons
UTFT_Buttons  myButtons(&myGLCD, &myTouch);

// Pressure typedef (mb)
typedef struct {
  float bmp;     // BMP085 Temperature value (in Celcius)
  float ms;     // DHT22 Temperature value (in Celcius)
} Pressure;

// set up variables using the SD utility library functions:

static Temperature T;
static Pressure P;

static int curView; // Current view on the screen
static bool redraw;     // Redraw the screen on next loop (data changed/button pressed)
static int btnMenu, btnHome, btnMinMax, btnAll, btnWind, pressed_button;

float humidity;     // % RH
float pressure;     // In millibars
double dewpoint;    // Celcius
float heatindex;    // Heat index


void setup() {
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myGLCD.setFont(SmallFont);

  myTouch.InitTouch(1);
  myTouch.setPrecision(PREC_MEDIUM);
  
  myButtons.setTextFont(BigFont);
  
  myGLCD.print("MicroWXStation v2 - Booting...", 0, 5);
  delay(1500);
  myGLCD.print("Complete!", 0, 25);
  delay(1000);
  myGLCD.clrScr();
  redraw = true; 
}

void loop() {
  char status;
  double Temp,Pres,p0,a;
  bool touch_wait = true;
  int touch_wait_count = 0;
  
  if(redraw) {
    myGLCD.clrScr();
    switch(curView) {
      case VIEW_HOME:
        draw_Home();
        break;
      case VIEW_MENU:
        draw_Menu();
        break;
      case VIEW_ALL:
        draw_All();
        break;
      case VIEW_WIND:
        draw_Wind();
        break;
      case VIEW_MINMAX:
        draw_MinMax();
        break;
      default:
        draw_Home();
        break;
    }
    redraw = false;
  }
  
  touch_wait_count = 0;
  touch_wait = true;
  
  while(touch_wait) {
    if (myTouch.dataAvailable() == true) {
      pressed_button = myButtons.checkButtons();

      if(pressed_button == btnMenu) {
          curView = VIEW_MENU;
          redraw = true;
          touch_wait = false;
      }
      if(pressed_button == btnHome) {
        curView = VIEW_HOME;
        redraw = true;
        touch_wait = false;
      }
      if(pressed_button == btnAll) {
        curView = VIEW_ALL;
        redraw = true;
        touch_wait = false;
      }
      if(pressed_button == btnWind) {
        curView = VIEW_WIND;
        redraw = true;
        touch_wait = false;
      }
      if(pressed_button == btnMinMax) {
        curView = VIEW_MINMAX;
        redraw = true;
        touch_wait = false;
      }
          
    }
    delay(1);
    touch_wait_count++;
    if(touch_wait_count > 2000) { // Exit the while loop to parse data, refresh screen, etc...
        touch_wait = false;
    }
  }
}

void draw_Home() {
  // myButtons.addButton(X_POS, Y_POS, WIDTH, HEIGHT)
  btnMenu = myButtons.addButton(0, 199, 100,  40, "Menu");
  myButtons.drawButtons();
  myGLCD.print("MicroWXStation v2", 110, 212);
  
}

void draw_MinMax() {
  // myButtons.addButton(X_POS, Y_POS, WIDTH, HEIGHT)
  btnHome = myButtons.addButton( 10,  20, 300,  30, "Home - Overview");
  btnMinMax = myButtons.addButton( 10,  60, 300,  30, "Min/Max Values");
  btnAll = myButtons.addButton( 10, 100, 300,  30, "All Data (Raw)");
  btnWind = myButtons.addButton( 10, 140, 300,  30, "Wind View");
  myButtons.drawButtons();
  myGLCD.print("MENU - Make a selection!", 0, 200);
}

void draw_Menu() {
  btnMenu = myButtons.addButton(0, 199, 100,  40, "Menu");
  myButtons.drawButtons();
  myGLCD.print("Min/Max Values", 110, 212);
}

void draw_Wind() {
  btnMenu = myButtons.addButton(0, 199, 100,  40, "Menu");
  myButtons.drawButtons();
  myGLCD.print("Wind View", 110, 212);
}

void draw_All() {
  btnMenu = myButtons.addButton(0, 199, 100,  40, "Menu");
  myButtons.drawButtons();
  myGLCD.print("All Data", 110, 212);
}
