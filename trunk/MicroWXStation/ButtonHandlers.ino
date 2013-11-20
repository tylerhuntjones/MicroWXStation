/*
 * 
 * MicroWXStation for Arduino Mega 2560 r3 - Version 0.2.6 
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
 * Filename: ButtonHandler.ino
 * 
 * Description: Sketch file for button input handlings
 *
 */

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

