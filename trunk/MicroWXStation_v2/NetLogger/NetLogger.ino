#include <UIPEthernet.h> // Used for Ethernet


// Typedef declarations
typedef struct {
  float TempF;
  float Pressure;
  float PressurePascals;
  float Humidity;
  float DewPoint;
  float HeatIndex;
  float WindSpeed;
  float WindGustSpeed;
  int WindGustDir;
  float WindSpeedAvg2M;
  float WindSpeedAvg10M;
  float WindGustSpeedAvg10M;
  int WindGustDirAvg10M;
  int WindDirection;
  int WindDirAvg2M;
  float LightLevel;
  //double BatteryLevel;
  //double Rainfall;
  //double DailyRainfall;
} WXData;

static WXData WX;

// **** ETHERNET SETTING ****
// Arduino Uno pins: 10 = CS, 11 = MOSI, 12 = MISO, 13 = SCK
// Ethernet MAC address - must be unique on your network - MAC Reads T4A001 in hex (unique in your network)
byte mac[] = { 0x54, 0x34, 0x41, 0x30, 0x30, 0x33 };                                       
// For the rest we use DHCP (IP address and such)

EthernetClient client;
char server[] = "192.168.1.5"; // IP Adres (or name) of server to dump data to
int  interval = 10000; // Wait between dumps

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Ethernet.begin(mac);

  Serial.print("IP Address        : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet Mask       : ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Default Gateway IP: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS Server IP     : ");
  Serial.println(Ethernet.dnsServerIP());
}

void loop() {
  String inData, recdString;
  String wxdata_array[15];
  int counter, lastIndex;
  
  while (Serial1.available() > 0) {
    char recd = Serial1.read();
    if (recd == '\n') {
      //Serial.println(String(inData));
      recdString = inData;
      inData = "";
    }
    inData += String(recd);
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
    WX.WindGustSpeedAvg10M = StrToFloat(wxdata_array[6], wxdata_array[6].length());
    WX.WindGustDirAvg10M = wxdata_array[7].toInt();
    WX.Humidity = wxdata_array[8].toInt();
    WX.TempF = StrToFloat(wxdata_array[9], wxdata_array[9].length());
    //WX.Rainfall = StrToDouble(wxdata_array[10], wxdata_array[10].length());
    //WX.DailyRainfall = StrToDouble(wxdata_array[11], wxdata_array[11].length());
    WX.Pressure = StrToFloat(wxdata_array[10], wxdata_array[10].length()) / 100.0;
    //WX.PressurePascals = StrToFloat(wxdata_array[10], wxdata_array[10].length());
    //WX.BatteryLevel = StrToDouble(wxdata_array[13], wxdata_array[13].length());
    WX.LightLevel = StrToFloat(wxdata_array[11], wxdata_array[11].length());
  } else {
    recdString = "";
  } 
  
  SendData();
  
  delay(interval);
}

void SendData() {
 // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("-> Connected");
    // Make a HTTP request:
    client.print( "GET /wx/log.php?");
    client.print("key=");
    client.print( "38H4IW2p8nf3DRp1d" );
    
    client.print("&");
    client.print("tf=");
    client.print( WX.TempF );
    
    client.print("&");
    client.print("pres=");
    client.print( WX.Pressure ); 
 
    client.print("&");
    client.print("humid=");
    client.print( WX.Humidity );
 
    client.print("&");
    client.print("dp=");
    client.print( WX.DewPoint );
 
    client.print("&");
    client.print("hi=");
    client.print( WX.HeatIndex );
 
    client.print("&");
    client.print("lt=");
    client.print( WX.LightLevel );
 
    client.print("&");
    client.print("ws=");
    client.print( WX.WindSpeed );
 
    client.print("&");
    client.print("wd=");
    client.print( WX.WindDirection );
 
    client.print("&");
    client.print("wgs=");
    client.print( WX.WindGustSpeed );
 
    client.print("&");
    client.print("wgd=");
    client.print( WX.WindGustDir );
 
    client.print("&");
    client.print("wsavg2m=");
    client.print( WX.WindSpeedAvg2M );
 
    client.print("&");
    client.print("wdavg2m=");
    client.print( WX.WindDirAvg2M );
    
    client.print("&");
    client.print("wgsavg10m=");
    client.print( WX.WindGustSpeedAvg10M );
    
    client.print("&");
    client.print("wgdavg10m=");
    client.print( WX.WindGustDirAvg10M );
    
    client.println( " HTTP/1.1");
    client.print( "Host: " );
    client.println(server);
    client.println( "Connection: close" );
    client.println();
    client.println();
    client.stop();
  }
  else {
    // you didn't get a connection to the server:
    Serial.println("--> connection failed/n");
  } 
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

