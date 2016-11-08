#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <NtpClientLib.h>

#include <Ticker.h>
#include <String.h>
#include "wifi.h"


#include "ST7565.h"
#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDS3231.h>

#include "wifi.h"

#include "./resources/force_nums.h"
#include "./resources/visitor_14.h"

//******** TIME DRAWING CONSTANTS/VARS *******//
#define TIME_SPACE 6   //not to be confused with the space time continuum
#define TIME_HEIGHT 36
#define TIME_WIDTH 22
#define SCREEN_WIDTH 128
#define TIME_Y_POS 3

#define DATE_Y_POS 50

RtcDS3231 Rtc;

int ledPin =  D7;    // LED connected to digital pin 13

// the LCD backlight is connected up to a pin so you can turn it on & off
#define BACKLIGHT_LED D7

// pin 9 - Serial data out (SID)
// pin 8 - Serial clock out (SCLK)
// pin 7 - Data/Command select (RS or A0)
// pin 6 - LCD reset (RST)
// pin 5 - LCD chip select (CS)
//ST7565 glcd(9, 8, 7, 6, 5);
ST7565 glcd(D6, D5, D4, D3, D0);

//WiFi Defaults
const char *ssid = WIFI_SSID;
const char *password = WIFI_SECRET;

//Time service variables
//IPAddress timeServerIP;
//const char* ntpServerName = "time.nist.gov";
//const char* ntpServerName = "pool.ntp.org";

//const int NTP_PACKET_SIZE = 48;
//unsigned int localPort = 2390;
//const int timeZone = -6;

//byte packetBuffer[ NTP_PACKET_SIZE];
//WiFiUDP Udp;

#include "ntp.cpp"

Ticker timeUpdater;

//webserver
//ESP8266WebServer server ( 80 );

void onSTAGotIP(WiFiEventStationModeGotIP ipInfo) {
  Serial.printf("Got IP: %s\r\n", ipInfo.ip.toString().c_str());
  NTP.begin("pool.ntp.org", -7, true);
  NTP.setInterval(63);
  //digitalWrite(2, LOW);
}

void onSTADisconnected(WiFiEventStationModeDisconnected event_info) {
  Serial.printf("Disconnected from SSID: %s\n", event_info.ssid.c_str());
  Serial.printf("Reason: %d\n", event_info.reason);
  //digitalWrite(2, HIGH);
}

// The setup() method runs once, when the sketch starts
void setup()   {                
  Serial.begin(115200);

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

  initRTC();

  // turn on backlight
  pinMode(BACKLIGHT_LED, OUTPUT);
  
  pinMode(D8, OUTPUT);
  pinMode(D9, OUTPUT);
  
  analogWrite(BACKLIGHT_LED,0);
  analogWrite(D8,255);
  analogWrite(D9,255);

  // initialize and set the contrast to 0x18
  //glcd.begin(0x18);
  glcd.begin(0x1B);
  glcd.clear();

  WiFi.mode(WIFI_STA);
  WiFi.begin ( ssid, password );

  initNTP();
 
  syncTime();
  timeUpdater.attach(30,syncTime);

  Serial.println("Setup complete.");
  
}

void loop()                     
{
  RtcDateTime now = Rtc.GetDateTime();
  drawTime(now); 
  drawDate(now);
  //Serial.println(freeRam());
  delay(1000);
  int sensor = analogRead(A0);
  float voltage = sensor * (5.0/1023.0);
  Serial.println(voltage);
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    //glcd.clear();
    //glcd.fillrect(0,49,128,64,WHITE);
    //glcd.drawstring(0,7, datestring);
    glcd.display();
}

static unsigned char *getForce_BMP(int num){
  switch(num){
    case 0:
      return F0;
      break;
    case 1:
      return F1;
      break;
    case 2:
      return F2;
      break;
    case 3:
      return F3;
      break;
    case 4:
      return F4;
      break;
    case 5:
      return F5;
      break;
    case 6:
      return F6;
      break;
    case 7:
      return F7;
      break;
    case 8:
      return F8;
      break;
    case 9:
      return F9;
      break;
  }
  return F0;
}

void drawTime(const RtcDateTime& dt)
{
  
  int tHour = dt.Hour();
  int tMinute = dt.Minute();

  int tNumTimeChars = 3; //this is the total number of time characters that need drawing

  glcd.fillrect(0,0,128,48,WHITE);
 
  //24 hour time?
  //TODO: support 12h/24h time
  
  //is the hour 1 or two characters?
  if(tHour>9){
    tNumTimeChars = 4; //need an extra character
  }

  int tDrawWidth = (tNumTimeChars * TIME_WIDTH) + ((tNumTimeChars-1) * TIME_SPACE);
  int tDrawPos = (SCREEN_WIDTH - tDrawWidth) / 2;

  int nums[4];

  int i=0;

  if(tHour>9){
    if(tHour>19){
      nums[0] = 2;
      i++;
      tHour = tHour - 20;
    }
    else{
      nums[0] = 1;
      i++;
      tHour = tHour - 10;
    }
  }

  nums[i] = tHour;
  i++;

  int mt = 0;

  while(tMinute>9){
    mt++;
    tMinute = tMinute - 10;
  }

  nums[i] = mt;
  i++;

  nums[i] = tMinute;


  glcd.drawbitmap(tDrawPos, TIME_Y_POS, getForce_BMP(nums[0]), TIME_WIDTH, TIME_HEIGHT, BLACK);
  tDrawPos += TIME_WIDTH + TIME_SPACE;
  
  glcd.drawbitmap(tDrawPos, TIME_Y_POS, getForce_BMP(nums[1]), TIME_WIDTH, TIME_HEIGHT, BLACK);
  tDrawPos += TIME_WIDTH + TIME_SPACE;
  
  glcd.drawbitmap(tDrawPos, TIME_Y_POS, getForce_BMP(nums[2]), TIME_WIDTH, TIME_HEIGHT, BLACK);
  tDrawPos += TIME_WIDTH + TIME_SPACE;
  
  if(tNumTimeChars==4)
    glcd.drawbitmap(tDrawPos, TIME_Y_POS, getForce_BMP(nums[3]), TIME_WIDTH, TIME_HEIGHT, BLACK);

  glcd.display();
  
}

static unsigned char *getV14_BMP(int num){
  switch(num){
    case 0:
      return v14_0;
      break;
    case 1:
      return v14_1;
      break;
    case 2:
      return v14_2;
      break;
    case 3:
      return v14_3;
      break;
    case 4:
      return v14_4;
      break;
    case 5:
      return v14_5;
      break;
    case 6:
      return v14_6;
      break;
    case 7:
      return v14_7;
      break;
    case 8:
      return v14_8;
      break;
    case 9:
      return v14_9;
      break;
  }
  return F0;
}

void drawDate(const RtcDateTime& dt){
  int tMonth = dt.Month();
  int tDay = dt.Day();
  int tYear = dt.Year();

  tYear = tYear - 2000; //FYI, this clock will stop working right after the year 2099 :/

  //break apart the days into tens/ones

  int tDayT = 0;

  while(tDay>9){
    tDayT++;
    tDay-=10;
  }

  //break apart the years into tens/ones

  int tYearT = 0;
  
  while(tYear>9){
    tYearT++;
    tYear-=10;
  }

  int numOnes = 0;
  int spacing[6] = {VISITOR_14_WIDTH,VISITOR_14_WIDTH,VISITOR_14_WIDTH,VISITOR_14_WIDTH,VISITOR_14_WIDTH,VISITOR_14_WIDTH};

  int tMonthT = 0;
  if(tMonth >9){
    numOnes++;
    tMonthT = 1;
    tMonth -= 10;
    spacing[0] = VISITOR_14_1WIDTH;
  }

  if(tMonth == 1){
    numOnes++;
    spacing[1] = VISITOR_14_1WIDTH;
  }

  if(tDayT==1){
    numOnes++;
    spacing[2] = VISITOR_14_1WIDTH;
  }

  if(tDay==1){
    numOnes++;
    spacing[3] = VISITOR_14_1WIDTH;
  }
  
  if(tYearT==1){
    numOnes++;
    spacing[4] = VISITOR_14_1WIDTH;
  }

  if(tYear==1){
    numOnes++;
    spacing[5] = VISITOR_14_1WIDTH;
  }

  int tDateWidth = ((5-numOnes)*VISITOR_14_WIDTH) + (numOnes * VISITOR_14_1WIDTH) + (2*VISITOR_14_DOT_WIDTH) + (7*VISITOR_14_SPACING);
  int tDatePos = (tDateWidth/2);

  glcd.fillrect(0,DATE_Y_POS,128,VISITOR_14_HEIGHT,WHITE);

  glcd.drawbitmap(tDatePos, DATE_Y_POS, getV14_BMP(tMonthT), spacing[0], VISITOR_14_HEIGHT, BLACK);
  tDatePos += spacing[0] + VISITOR_14_SPACING;

  glcd.drawbitmap(tDatePos, DATE_Y_POS, getV14_BMP(tMonth), spacing[1], VISITOR_14_HEIGHT, BLACK);
  tDatePos += spacing[1] + VISITOR_14_SPACING;

  glcd.drawbitmap(tDatePos, DATE_Y_POS, v14_dot, VISITOR_14_DOT_WIDTH, VISITOR_14_HEIGHT, BLACK);
  tDatePos += VISITOR_14_DOT_WIDTH + VISITOR_14_SPACING;

  glcd.drawbitmap(tDatePos, DATE_Y_POS, getV14_BMP(tDayT), spacing[2], VISITOR_14_HEIGHT, BLACK);
  tDatePos += spacing[2] + VISITOR_14_SPACING;
  
  glcd.drawbitmap(tDatePos, DATE_Y_POS, getV14_BMP(tDay), spacing[3], VISITOR_14_HEIGHT, BLACK);
  tDatePos += spacing[3] + VISITOR_14_SPACING;

  glcd.drawbitmap(tDatePos, DATE_Y_POS, v14_dot, VISITOR_14_DOT_WIDTH, VISITOR_14_HEIGHT, BLACK);
  tDatePos += VISITOR_14_DOT_WIDTH + VISITOR_14_SPACING;

  glcd.drawbitmap(tDatePos, DATE_Y_POS, getV14_BMP(tYearT), spacing[4], VISITOR_14_HEIGHT, BLACK);
  tDatePos += spacing[4] + VISITOR_14_SPACING;

  glcd.drawbitmap(tDatePos, DATE_Y_POS, getV14_BMP(tYear), spacing[5], VISITOR_14_HEIGHT, BLACK);
  tDatePos += spacing[5] + VISITOR_14_SPACING;

  glcd.display();  
    
}

void initNTP(){
  static WiFiEventHandler e1, e2;

  NTP.setTimeZone(-7);

  NTP.onNTPSyncEvent([](NTPSyncEvent_t ntpEvent) {
    if (ntpEvent) {
      Serial.print("Time Sync error: ");
      if (ntpEvent == noResponse)
        Serial.println("NTP server not reachable");
      else if (ntpEvent == invalidAddress)
        Serial.println("Invalid NTP server address");
    }
    else {
      Serial.print("Got NTP time: ");
      Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
    }
  });
  WiFi.onEvent([](WiFiEvent_t e) {
    Serial.printf("Event wifi -----> %d\n", e);
  });
  e1 = WiFi.onStationModeGotIP(onSTAGotIP);// As soon WiFi is connected, start NTP Client
  e2 = WiFi.onStationModeDisconnected(onSTADisconnected);
}

void syncTime(){
    RtcDateTime newTime = NTP.getTime();
    if(newTime>0)
      Rtc.SetDateTime(newTime-946684800);    
}

void initRTC(){
  //--------RTC SETUP ------------
  Rtc.Begin();
  //Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL
  // if you are using ESP-01 then uncomment the line below to reset the pins to
  // the available pins for SDA, SCL
  

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  //Serial.println();

  if (!Rtc.IsDateTimeValid()) 
  {
      // Common Cuases:
      //    1) first time you ran and the device wasn't running yet
      //    2) the battery on the device is low or even missing

      // following line sets the RTC to the date & time this sketch was compiled
      // it will also reset the valid flag internally unless the Rtc device is
      // having an issue

      Rtc.SetDateTime(compiled);
  }

  if (!Rtc.GetIsRunning())
  {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled) 
  {
      Serial.println("RTC is older than compile time!  (Updating DateTime)");
      Rtc.SetDateTime(compiled);
  }
  else if (now > compiled) 
  {
      Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled) 
  {
      //Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
}

