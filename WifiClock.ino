#include <Arduino.h>
#include "ST7565.h"
//#include <avr/pgmspace.h>
#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDS1307.h>


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <String.h>
#include <WiFiUdp.h>
#include <TimeLib.h>





/*#include "./resources/bttf_glcd.h"
#include "./resources/willow_glcd.h"
#include "./resources/dimitri_glcd.h"
#include "./resources/aldo_glcd.h"
#include "./resources/atlanta_glcd.h"
#include "./resources/hl2_glcd.h"
#include "./resources/cone_glcd.h"
#include "./resources/force_glcd.h"
#include "./resources/micro1_glcd.h"
#include "./resources/force_sample_glcd.h"*/

#include "./resources/force_nums.h"
#include "./resources/visitor_14.h"

//******** TIME DRAWING CONSTANTS/VARS *******//
#define TIME_SPACE 6   //not to be confused with the space time continuum
#define TIME_HEIGHT 36
#define TIME_WIDTH 22
#define SCREEN_WIDTH 128
#define TIME_Y_POS 3

#define DATE_Y_POS 50

RtcDS1307 Rtc;

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

/*#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 */

int test_img = 0;
long next_test = 0;

// a bitmap of a 16x16 fruit icon
/*const static unsigned char __attribute__ ((progmem)) logo16_glcd_bmp[]={
0x30, 0xf0, 0xf0, 0xf0, 0xf0, 0x30, 0xf8, 0xbe, 0x9f, 0xff, 0xf8, 0xc0, 0xc0, 0xc0, 0x80, 0x00, 
0x20, 0x3c, 0x3f, 0x3f, 0x1f, 0x19, 0x1f, 0x7b, 0xfb, 0xfe, 0xfe, 0x07, 0x07, 0x07, 0x03, 0x00, };*/

// The setup() method runs once, when the sketch starts
void setup()   {                
  //Serial.begin(9600);

    //Serial.print("compiled: ");
    //Serial.print(__DATE__);
    //Serial.println(__TIME__);

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

        Serial.println("RTC lost confidence in the DateTime!");

        // following line sets the RTC to the date & time this sketch was compiled
        // it will also reset the valid flag internally unless the Rtc device is
        // having an issue

        Rtc.SetDateTime(compiled);
    }

    if (!Rtc.GetIsRunning())
    {
        //Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        //Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        //Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        //Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.SetSquareWavePin(DS1307SquareWaveOut_Low); 

#ifdef __AVR__
  Serial.println(freeRam());
#endif

  // turn on backlight
  pinMode(BACKLIGHT_LED, OUTPUT);
  
  pinMode(D8, OUTPUT);
  pinMode(D9, OUTPUT);
  /*
   * digitalWrite(BACKLIGHT_LED, LOW);
   * digitalWrite(11, LOW);
   * digitalWrite(10, LOW);
   */
  
  analogWrite(BACKLIGHT_LED,0);
  analogWrite(D8,255);
  analogWrite(D9,255);

  // initialize and set the contrast to 0x18
  //glcd.begin(0x18);
  glcd.begin(0x1B);
  glcd.clear();

/*
  glcd.display(); // show splashscreen
  delay(2000);
  glcd.clear();

  // draw a single pixel
  glcd.setpixel(10, 10, BLACK);
  glcd.display();        // show the changes to the buffer
  delay(2000);
  glcd.clear();

  // draw many lines
  testdrawline();
  glcd.display();       // show the lines
  delay(2000);
  glcd.clear();

  // draw rectangles
  testdrawrect();
  glcd.display();
  delay(2000);
  glcd.clear();

  // draw multiple rectangles
  testfillrect();
  glcd.display();
  delay(2000);
  glcd.clear();

  // draw mulitple circles
  testdrawcircle();
  glcd.display();
  delay(2000);
  glcd.clear();

  // draw a black circle, 10 pixel radius, at location (32,32)
  glcd.fillcircle(32, 32, 10, BLACK);
  glcd.display();
  delay(2000);
  glcd.clear();

  // draw the first ~120 characters in the font
  testdrawchar();
  glcd.display();
  delay(2000);
  glcd.clear();

  // draw a string at location (0,0)
  glcd.drawstring(0, 0, "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation");
  glcd.display();
  delay(2000);
  glcd.clear();

  // draw a bitmap icon and 'animate' movement
  testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);*/
}


void loop()                     
{
  RtcDateTime now = Rtc.GetDateTime();
  drawTime(now); 
  drawDate(now);
  //Serial.println(freeRam());
  delay(1000);
  /*if(millis() > next_test){
    test();
  }*/
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

  int tDateWidth = ((6-numOnes)*VISITOR_14_WIDTH) + (numOnes * VISITOR_14_1WIDTH) + (2*VISITOR_14_DOT_WIDTH) + (7*VISITOR_14_SPACING);
  int tDatePos = (tDateWidth/2) - 2;

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

/*
void test(){
  //glcd.clear();
  glcd.fillrect(0,0,128,48,WHITE);
  switch(test_img){
    case 0:
     //glcd.drawstring(0,0,"BTTF");
     glcd.drawbitmap(0,0,force_sample_glcd_bmp,128, 64,BLACK);
     break;
    case 1:
     glcd.drawstring(0,0,"WILLOW");
     glcd.drawbitmap(0,0,willow_glcd_bmp,128, 48,BLACK);
     break;
    case 2:
     glcd.drawstring(0,0,"DIMITRI");
     glcd.drawbitmap(0,0,dimitri_glcd_bmp,128, 48,BLACK);
     break;
    case 3:
     glcd.drawstring(0,0,"ALDO");
     glcd.drawbitmap(0,0,aldo_glcd_bmp,128, 48,BLACK);
     break;
    case 4:
     glcd.drawstring(0,0,"ATLANTA");
     glcd.drawbitmap(0,0,atlanta_glcd_bmp,128, 48,BLACK);
     break;
    case 5:
     glcd.drawstring(0,0,"HL2");
     glcd.drawbitmap(0,0,hl2_glcd_bmp,128, 48,BLACK);
     break;
    case 6:
     glcd.drawstring(0,0,"CONE");
     glcd.drawbitmap(0,0,cone_glcd_bmp,128, 48,BLACK);
     break;
    case 7:
     glcd.drawstring(0,0,"FORCE");
     glcd.drawbitmap(0,0,force_glcd_bmp,128, 48,BLACK);
     break;   
    case 8:
     glcd.drawstring(0,0,"MICRO1");
     glcd.drawbitmap(0,0,micro1_glcd_bmp,128, 48,BLACK);
     break;
  }
  glcd.display();
  test_img++;
  if(test_img>8)
    test_img=0;

  next_test = millis() + 5000;
  
}
*/

#ifdef __AVR__
// this handy function will return the number of bytes currently free in RAM, great for debugging!   
int freeRam(void)
{
  extern int  __bss_end; 
  extern int  *__brkval; 
  int free_memory; 
  if((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__bss_end); 
  }
  else {
    free_memory = ((int)&free_memory) - ((int)__brkval); 
  }
  return free_memory; 
} 
#endif

