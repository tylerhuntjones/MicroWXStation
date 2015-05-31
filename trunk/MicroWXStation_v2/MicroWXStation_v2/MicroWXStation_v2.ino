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
#include <SPI.h>
#include <SdFat.h>
#include <UTFT_SdRaw.h>

// Declare which fonts we will be using
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t Dingbats1_XL[];

// Setup the TFT scree
UTFT myGLCD(SSD1289, 38, 39, 40, 41);

// Setup the touch screen
ITDB02_Touch myTouch(6, 5, 4, 3, 2);

// Setup UTFT_Buttons
UTFT_Buttons  myButtons(&myGLCD, &myTouch);

// Setup SD card
UTFT_SdRaw myFiles(&myGLCD);

// file system object
SdFat sd;

static WXData WX;
static MinMax_t MM;

static int curView = 0;     // Current view on the screen
static bool redraw;     // Redraw the screen on next loop (data changed/button pressed)
bool isMenuDisplayed = false;
unsigned long startedMillis;
int TempInF = true; // Display temperature in Farenheit (false if Celcius)

//Wind Directions
char *strWindDirs[16] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SSE", "S", "SSW", "SW", "WSW", "WNW", "W", "WNW", "NW", "NNW"};
float WindDirs[16] =    { 0, 23, 45, 68, 90, 113, 135, 158, 180, 203, 225, 248, 270, 293, 315, 338};

void setup() {
  //Serial.begin(9600);
  Serial1.begin(9600);

  bool mysd = 0;
  // see if the card is present and can be initialized:
  while (!mysd) {
    if (!sd.begin(SD_CHIP_SELECT, SPI_FULL_SPEED)) {
      //Serial.println(F("Card failed, or not present"));
      //Serial.println(F("Retrying...."));
    } else {
      mysd = 1;
      //Serial.println(F("Card initialised."));
    }
  }

  myGLCD.InitLCD();
  myGLCD.clrScr();
  myGLCD.setFont(SmallFont);

  myTouch.InitTouch(1);
  myTouch.setPrecision(PREC_MEDIUM);

  myButtons.setTextFont(BigFont);
  myButtons.setSymbolFont(Dingbats1_XL);

  myGLCD.print(F("MicroWXStation v2 - Booting..."), 0, 5);
  delay(1500);
  myGLCD.print(F("Complete!"), 0, 25);
  delay(1000);
  redraw = true;
}

void loop() {
  String inData, recdString;
  String wxdata_array[15];
  int counter = 0, lastIndex = 0;
  unsigned long lastSerialUpdate = 0;
  char status;
  int btnNext, btnHome, btnMinMax, btnWind, btnAll, btnAbout, btnMenu, pressed_button;
  bool touch_wait = true;

  if(millis() - lastSerialUpdate > 25000) {
    while (Serial1.available() > 0) {
      char recd = Serial1.read();
      if (recd == '\n') {
        //Serial.println(String(inData));
        recdString = inData;
        inData = "";
        lastSerialUpdate = millis();
      }
      inData += String(recd);
    }
  }
  recdString.trim();
  if (recdString.startsWith("$,") && recdString.endsWith(",#")) {
    // Received valid string
    if (recdString.indexOf("$") >= 0) {
      recdString.remove(recdString.indexOf("$"), 1);
    }
    if (recdString.indexOf("#") >= 0) {
      recdString.remove(recdString.indexOf("#"), 1);
    }
    if (recdString.startsWith(",")) {
      recdString.remove(0, 1);
    }
    if (recdString.endsWith(",")) {
      recdString.remove(recdString.length() - 1, 1);
    }
    //Serial.println("Valid string received: " + recdString);
    for (int i = 0; i < recdString.length(); i++) {
      if (recdString.substring(i, i + 1) == ",") {
        wxdata_array[counter] = recdString.substring(lastIndex, i);
        lastIndex = i + 1;
        counter++;
      }
      if (i == recdString.length() - 1) {
        wxdata_array[counter] = recdString.substring(lastIndex, i);
      }
    }
    recdString = "";
    counter = 0;
    lastIndex = 0;

    WX.WindDirection = wxdata_array[0].toInt();
    WX.WindSpeed = StrToFloat(wxdata_array[1], wxdata_array[1].length());
    WX.WindGustSpeed = StrToFloat(wxdata_array[2], wxdata_array[2].length());
    WX.WindGustDir = wxdata_array[3].toInt();
    WX.WindSpeedAvg2M = StrToFloat(wxdata_array[4], wxdata_array[4].length());
    WX.WindDirAvg2M = wxdata_array[5].toInt();
    WX.WindGustSpeed10M = StrToFloat(wxdata_array[6], wxdata_array[6].length());
    WX.WindGustDir10M = wxdata_array[7].toInt();
    WX.Humidity = wxdata_array[8].toInt();
    WX.TempF = StrToFloat(wxdata_array[9], wxdata_array[9].length());
    WX.TempC = Celcius(StrToFloat(wxdata_array[9], wxdata_array[9].length()));
    //WX.Rainfall = StrToDouble(wxdata_array[10], wxdata_array[10].length());
    //WX.DailyRainfall = StrToDouble(wxdata_array[11], wxdata_array[11].length());
    WX.Pressure = StrToFloat(wxdata_array[10], wxdata_array[10].length()) / 100.0;
    WX.PressurePascals = StrToFloat(wxdata_array[10], wxdata_array[10].length());
    //WX.BatteryLevel = StrToDouble(wxdata_array[13], wxdata_array[13].length());
    WX.LightLevel = StrToFloat(wxdata_array[11], wxdata_array[11].length());
  } else {
    recdString = "";
  }

  if (redraw) {
    myGLCD.clrScr();
    if (curView == VIEW_HOME) {
      draw_Home();
    }
    if (curView == VIEW_ALL) {
      draw_All();
    }
    if (curView == VIEW_WIND) {
      draw_Wind();
    }
    if (curView == VIEW_MINMAX) {
      draw_MinMax();
    }
    if (curView == VIEW_MENU) {
      draw_Menu();
    }
    if (curView == VIEW_ABOUT) {
      draw_About();
    }
    redraw = false;
  } else {
    if(curView == VIEW_HOME) {
       draw_Home();
    } 
  }

  myButtons.deleteAllButtons();

  if (curView == VIEW_MENU) {
    // myButtons.addButton(X_POS, Y_POS, WIDTH, HEIGHT)
    btnHome = myButtons.addButton(10, 20, 300, 30, "WX Overview");
    btnMinMax = myButtons.addButton(10, 60, 300, 30, "Min/Max Values");
    btnWind = myButtons.addButton(10, 100, 300, 30, "Wind");
    btnAll = myButtons.addButton(10, 140, 300, 30, "Raw WX Data");
    btnAbout = myButtons.addButton(10, 180, 300, 30, "About");
    myButtons.drawButtons();
  } else {
    btnNext = myButtons.addButton(279, 209, 40, 30, ">");
    btnMenu = myButtons.addButton(234, 209, 40, 30, ";", BUTTON_SYMBOL);
    myButtons.drawButtons();
  }

  touch_wait = true;
  startedMillis = millis();

  while (touch_wait) {
    if (myTouch.dataAvailable() == true) {
      pressed_button = myButtons.checkButtons();

      if (pressed_button == btnNext) {
        if (curView >= 3) {
          curView = 0;
        } else {
          curView++;
        }
        redraw = true;
        touch_wait = false;
      }

      if (pressed_button == btnMenu) {
        curView = VIEW_MENU;
        redraw = true;
        touch_wait = false;
      }
      if (pressed_button == btnAbout) {
        curView = VIEW_ABOUT;
        redraw = true;
        touch_wait = false;
      }
      if (pressed_button == btnAll) {
        curView = VIEW_ALL;
        redraw = true;
        touch_wait = false;
      }
      if (pressed_button == btnMinMax) {
        curView = VIEW_MINMAX;
        redraw = true;
        touch_wait = false;
      }
      if (pressed_button == btnWind) {
        curView = VIEW_WIND;
        redraw = true;
        touch_wait = false;
      }
      if (pressed_button == btnHome) {
        curView = VIEW_HOME;
        redraw = true;
        touch_wait = false;
      }
    }
    if (millis() - startedMillis > 60000) { // Exit the while loop to parse data, refresh screen, etc...
      touch_wait = false;
    }
  }
}

void draw_Menu() {
  myGLCD.print(F("Menu - Make a selection"), VIEW_TITLE_XPOS, VIEW_TITLE_YPOS);
  isMenuDisplayed = true;
}

void draw_Home() {
  int indY, indX;
  float indAngle;
  String temp;
  // Print the view title at the bottom of the screen
  myGLCD.setFont(SmallFont);
  myGLCD.print(F("Weather Overview"), VIEW_TITLE_XPOS, VIEW_TITLE_YPOS);
  myGLCD.setFont(BigFont);
  draw_frame();
  // Draw the divider line (div)
  myGLCD.setColor(FRAME_R_COLOR, FRAME_G_COLOR, FRAME_B_COLOR);
  myGLCD.drawLine(HOME_DIV_X_START, HOME_DIV_Y_START, HOME_DIV_X_END, HOME_DIV_Y_END);

  // Start printing WX data
  temp = TempInF ? String(WX.TempF) : String(WX.TempC);
  temp = String(F("Out Temp: ")) + temp + (TempInF ? "F" : "C");
  myGLCD.print(temp , CENTER, FRAME_Y_START + 5);

  myGLCD.print(F("Pressure") , FRAME_X_START + 12, FRAME_Y_START + 40);
  temp = String(WX.Pressure);
  temp = temp + "mb";
  myGLCD.print(temp , FRAME_X_START + 5, FRAME_Y_START + 65);
  myGLCD.drawLine(10, 100, 150, 100);

  myGLCD.print(F("Humidity") , FRAME_X_START + 12, FRAME_Y_START + 115);
  temp = String(WX.Humidity);
  temp = temp + "%";
  myGLCD.print(temp , FRAME_X_START + 30, FRAME_Y_START + 140);

  myGLCD.print(F("Wind") , FRAME_X_START + 205, FRAME_Y_START + 30);
  temp = String(WX.WindSpeed);
  temp = temp + F("Mph");
  myGLCD.print(temp, FRAME_X_START + 170, FRAME_Y_START + 50);

  // Draw wind direction indicator
  myGLCD.drawCircle(WDIRIND_CENTER_X, WDIRIND_CENTER_Y, WDIRIND_RADIUS);
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(VGA_RED);
  myGLCD.print("N", WDIRIND_CENTER_X - 4, WDIRIND_CENTER_Y - 64);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.print("E", WDIRIND_CENTER_X + 56, WDIRIND_CENTER_Y - 6);
  myGLCD.print("S", WDIRIND_CENTER_X - 3, WDIRIND_CENTER_Y + 52);
  myGLCD.print("W", WDIRIND_CENTER_X - 64, WDIRIND_CENTER_Y - 6);
  getCoorOnCircle(WX.WindDirection, WDIRIND_CENTER_X, WDIRIND_CENTER_Y, WDIRIND_LINE_LENGTH, &indX, &indY);
  myGLCD.drawLine(WDIRIND_CENTER_X, WDIRIND_CENTER_Y, indX, indY);
  isMenuDisplayed = false;
}

void draw_MinMax() {
  myGLCD.print(F("Min/Max Values"), VIEW_TITLE_XPOS, VIEW_TITLE_YPOS);
  isMenuDisplayed = false;
}

void draw_Wind() {
  myGLCD.print(F("Wind Measurements"), VIEW_TITLE_XPOS, VIEW_TITLE_YPOS);
  isMenuDisplayed = false;
}

void draw_All() {
  myGLCD.print(F("Raw Weather Data"), VIEW_TITLE_XPOS, VIEW_TITLE_YPOS);
  draw_frame();
  myGLCD.setFont(SmallFont);
  myGLCD.drawLine(FRAME_X_START, FRAME_Y_START + 18, FRAME_X_END, FRAME_Y_START + 18);
  myGLCD.drawLine(FRAME_X_START, FRAME_Y_START + 36, FRAME_X_END, FRAME_Y_START + 36);
  myGLCD.drawLine(FRAME_X_START, FRAME_Y_START + 54, FRAME_X_END, FRAME_Y_START + 54);
  myGLCD.drawLine(FRAME_X_START, FRAME_Y_START + 72, FRAME_X_END, FRAME_Y_START + 72);
  myGLCD.drawLine(FRAME_X_START, FRAME_Y_START + 90, FRAME_X_END, FRAME_Y_START + 90);
  myGLCD.drawLine(FRAME_X_START, FRAME_Y_START + 108, FRAME_X_END, FRAME_Y_START + 108);
  myGLCD.print(F("Out Temp: "), FRAME_X_START + 3, FRAME_Y_START + 3);
  myGLCD.print(F("Humidity: "), FRAME_X_START + 3, FRAME_Y_START + 21);
  myGLCD.print(F("Pressure: "), FRAME_X_START + 3, FRAME_Y_START + 39);
  myGLCD.print(F("Dew Point: "), FRAME_X_START + 3, FRAME_Y_START + 57);
  myGLCD.print(F("Heat Index: "), FRAME_X_START + 3, FRAME_Y_START + 75);
  myGLCD.print(F("Light Level: "), FRAME_X_START + 3, FRAME_Y_START + 93);
  isMenuDisplayed = false;
}

void draw_About() {
  myGLCD.print(F("About"), VIEW_TITLE_XPOS, VIEW_TITLE_YPOS);
  draw_frame();
  myGLCD.setFont(BigFont);
  myGLCD.print(F("MicroWXStation v2"), CENTER, FRAME_X_START + 5);
  myGLCD.print(F("HW Version: v2"), CENTER, FRAME_X_START + 30);
  myGLCD.print(F("SW Version: 0.1.0"), CENTER, FRAME_X_START + 55);
  myGLCD.setFont(SmallFont);
  myGLCD.print(F("Written By: Tyler H. Jones"), CENTER, FRAME_X_START + 80);
  myGLCD.print(F("(C) 2015 - Apache License 2.0"), CENTER, FRAME_X_START + 95);
  myGLCD.print(F("inquirewue@gmail.com"), CENTER, FRAME_X_START + 110);
  isMenuDisplayed = false;
}

void draw_frame() {
  // myGLCD.fillRect(X_START, Y_START, X_END, Y_END)
  myGLCD.setColor(FRAME_R_COLOR, FRAME_G_COLOR, FRAME_B_COLOR);
  myGLCD.drawRect(FRAME_X_START, FRAME_Y_START, FRAME_X_END, FRAME_Y_END);
}

void getCoorOnCircle(int dir, int centerX, int centerY, int linelen, int *iX, int *iY) {
  float indAngle;
  indAngle = 2.0 * PI * dir / 16.0;
  *iX = centerX + linelen * sin(indAngle);
  *iY = centerY + linelen * cos(indAngle);
}

double StrToDouble(String str, int len) {
  char floatbuf[len];
  str.toCharArray(floatbuf, sizeof(floatbuf));
  return atof(floatbuf);
}

float StrToFloat(String str, int len) {
  char floatbuf[len];
  str.toCharArray(floatbuf, sizeof(floatbuf));
  return atof(floatbuf);
}


