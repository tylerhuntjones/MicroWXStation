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
 
#include <Wire.h>
#include <SFE_BMP180.h>
#include <MS5611.h>
#include <DHT.h>
#include <SD.h>

#define DHTPIN 13     // AM2302/1 Pin
#define DHTTYPE DHT21   // DHT 22  (AM2302/1)
#define SDENABLE true

// Typedef declarations
// Temperature typedef (Celcius and Fahrenheit)
typedef struct {
  float bmp_c;     // BMP085 Temperature value (in Celcius)
  float dht_c;     // DHT22 Temperature value (in Celcius)
  double bmp_f;    // BMP085 Temperature value (in Fahrenheit)
  double dht_f;    // DHT22 Temperature value (in Fahrenheit)
  float ms_c;     // MS5611 Temperature value (in Celcius)
  double ms_f;    // MS5611 Temperature value (in Fahrenheit)
} Temperature;

// Pressure typedef (mb)
typedef struct {
  float bmp;     // BMP085 Temperature value (in Celcius)
  float ms;     // DHT22 Temperature value (in Celcius)
} Pressure;

DHT dht(DHTPIN, DHTTYPE);
SFE_BMP180 bmp;
MS5611 ms5611;

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = 53;   

static Temperature T;
static Pressure P;

float humidity;     // % RH
float pressure;     // In millibars
double dewpoint;    // Celcius
float heatindex;    // Heat index


void setup() {
  Serial.begin(9600); 
  Serial.println("MicroWXStation Test!!");
  // put your setup code here, to run once:
  if (bmp.begin())
    Serial.println("BMP180 init success");
  else
  {
    Serial.println("BMP180 init fail\n\n");
    //while(1); // Pause forever.
  }
  
  while(!ms5611.begin())
  {
    Serial.println("Could not find a valid MS5611 sensor, check wiring!");
    delay(500);
  }
  dht.begin();
  
  // SD Card pin
  pinMode(53, OUTPUT);
  
  // SD Card setup
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
  } else {
    Serial.println("SD card fail!"); 
  }
}

void loop() {
  char status;
  double Temp,Pres,p0,a;
    
  status = bmp.startTemperature();
  if (status != 0)
  {
    delay(status);

    status = bmp.getTemperature(Temp);
    if (status != 0)
    {
      T.bmp_c = (float)Temp;
      T.bmp_f = Fahrenheit((float)Temp);

      status = bmp.startPressure(3);
      if (status != 0)
      {
        delay(status);

        status = bmp.getPressure(Pres,Temp);
        if (status != 0)
        {
          P.bmp = (float)Pres / 100;
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");

  // MS5611 Readings
  T.ms_c = ms5611.readTemperature();
  T.ms_f = Fahrenheit(T.ms_c);
  P.ms = ms5611.readPressure();
  
  delay(10);
  
    // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  humidity = dht.readHumidity();
  // Read temperature as Celsius
  T.dht_c = (float)dht.readTemperature();
  // Read temperature as Fahrenheit
  T.dht_f = (float)dht.readTemperature(true);
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(T.dht_c) || isnan(T.dht_f)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
  }

  // Compute heat index
  // Must send in temp in Fahrenheit!
  heatindex = dht.computeHeatIndex(T.dht_f, humidity);
  
  delay(1000);
  String dataString = String(String((int)(T.dht_c*10)) + ", " + String((int)humidity) + ", " + String((int)heatindex) + ", " + String((long)P.bmp) + ", " + String((long)P.ms) + ", " + String((int)dewPoint(T.dht_c, humidity))); 
  Serial.println(dataString);
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

// Celsius to Fahrenheit conversion
double Fahrenheit(double celsius)
{
        return 1.8 * celsius + 32.0;
}

