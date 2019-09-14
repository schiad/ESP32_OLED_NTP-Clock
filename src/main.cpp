/**
* The MIT License (MIT)
*
* Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* ThingPulse invests considerable time and money to develop these open source libraries.
* Please support us by buying our products (and not the clones) from
* https://thingpulse.com
*
*/

#include <TimeLib.h>

// Include the correct display library
// For a connection via I2C using Wire include
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#include <Adafruit_BMP085.h>
#include <WiFi.h>
#include <NTPClient.h>
// or #include "SH1106Wire.h", legacy include: `#include "SH1106.h"`
// For a connection via I2C using brzo_i2c (must be installed) include
// #include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Brzo.h"
// #include "SH1106Brzo.h"
// For a connection via SPI include
// #include <SPI.h> // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Spi.h"
// #include "SH1106SPi.h"

// Include the UI lib
#include "OLEDDisplayUi.h"

// Include custom images
//#include "images.h"

// Use the corresponding display class:

// Initialize the OLED display using SPI
// D5 -> CLK
// D7 -> MOSI (DOUT)
// D0 -> RES
// D2 -> DC
// D8 -> CS
// SSD1306Spi        display(D0, D2, D8);
// or
// SH1106Spi         display(D0, D2);

// Initialize the OLED display using brzo_i2c
// D3 -> SDA
// D5 -> SCL
// SSD1306Brzo display(0x3c, D3, D5);
// or
// SH1106Brzo  display(0x3c, D3, D5);

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, 4, 15);
Adafruit_BMP085 bmp;
// SH1106 display(0x3c, D3, D5);

OLEDDisplayUi ui(&display);
char wifistate[50];
bool BMP085state = 0;
bool WiFistate = 0;

const char *ssid      = "ssid";
const char *password  = "password";

int screenW = 128;
int screenH = 64;
int clockCenterX = (screenW / 2) + 4;
//int clockCenterY = ((screenH-16)/2)+16;   // top yellow part is 16 px height
int clockCenterY = (screenH / 2);
int clockRadius = 23;
uint8_t sc[2];

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 0);

// utility function for digital clock display: prints leading 0
String twoDigits(int digits){
  if(digits < 10) {
    String i = '0'+String(digits);
    return i;
  }
  else {
    return String(digits);
  }
}

void clockOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {

}

void analogClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  //  ui.disableIndicator();

  // Draw the clock face
  //  display->drawCircle(clockCenterX + x, clockCenterY + y, clockRadius);
  display->drawCircle(clockCenterX + x, clockCenterY + y, 2);
  //
  //hour ticks
  for( int z=0; z < 360;z= z + 30 ){
    //Begin at 0° and stop at 360°
    float angle = z ;
    angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
    int x2 = ( clockCenterX + ( sin(angle) * clockRadius ) );
    int y2 = ( clockCenterY - ( cos(angle) * clockRadius ) );
    int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    display->drawLine( x2 + x , y2 + y , x3 + x , y3 + y);
  }

  // display second hand
  float angle = second() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display minute hand
  angle = minute() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display hour hand
  angle = hour() * 30 + int( ( minute() / 12 ) * 6 )   ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  String timenow = String(hour())+":"+twoDigits(minute())+":"+twoDigits(second());
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(clockCenterX , clockCenterY, timenow );
}

void digitalClockDateFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  static char wkday[5];
  //static uint8_t we;
  //we = weekday();

  switch (weekday())
  {
    case (1):
    //weekday = "Lun.";
    strcpy(wkday, "Dim.");
    break;
    case (2):
    //weekday = "Mar.";
    strcpy(wkday, "Lun.");
    break;
    case (3):
    //weekday = "Mer.";
    strcpy(wkday, "Mar.");
    break;
    case (4):
    //weekday = "Jeu.";
    strcpy(wkday, "Mer.");
    break;
    case (5):
    //weekday = "Ven.";
    strcpy(wkday, "Jeu.");
    break;
    case (6):
    //weekday = "Sam.";
    strcpy(wkday, "Ven.");
    break;
    case (7):
    //weekday = "Dim.";
    strcpy(wkday, "Sam.");
    break;
  }

  if (bmp.begin()) {
    //strcpy(wifistate, "BMP085 ko");
    wifistate[7] = 'o';
    wifistate[8] = 'k';
    BMP085state = 1;
  } else {
    //strcpy(wifistate, "BMP085 ok");
    wifistate[7] = 'k';
    wifistate[8] = 'o';
    BMP085state = 0;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifistate[16] = 'o';
    wifistate[17] = 'k';
    WiFistate = 1;
    timeClient.begin();
  } else {
    wifistate[16] = 'k';
    wifistate[17] = 'o';
    WiFistate = 0;
  }

  String timenow = String(hour())+":"+twoDigits(minute())+":"+twoDigits(second());
  String datenow = wkday+twoDigits(day())+"/"+twoDigits(month())+"/"+String(year());
  String pressure;

  if (BMP085state)
  pressure = String(bmp.readPressure())+" Pa, "+String(bmp.readTemperature())+" °C";
  else
  pressure = "CHECK WIRING BMP280";
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(clockCenterX + x , clockCenterY + y + 16, timenow );
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(clockCenterX + x , clockCenterY + y + 2, datenow );
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(clockCenterX + x , clockCenterY + y - 7, pressure );
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(clockCenterX + x , clockCenterY + y - 30, wifistate );
  //display->setTextAlignment(TEXT_ALIGN_CENTER);
  //display->setFont(ArialMT_Plain_10);
  //display->drawString(clockCenterX + x , clockCenterY + y - 21, strip );
}

// This array keeps function pointers to all frames
// frames are the single views that slide in
//FrameCallback frames[] = { digitalClockFrame, digitalClockDateFrame};
FrameCallback frames[] = {digitalClockDateFrame};

// how many frames are there?
int frameCount = 1; // 2

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { clockOverlay };
int overlaysCount = 1;

void displayhelp() {
  Serial.println("To set time use send F+YYYY+MM+DD+HH+MM+SS");
  Serial.println("For exemple : for 13:32:45 on 1st of July 2019 send");
  Serial.println("T20190701133245");
}

void setup() {
  Serial.begin(115200);
  pinMode(25, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.println("Starting\n");

  displayhelp();

  pinMode(16, OUTPUT);
  digitalWrite(16, 1);
  delay(100);
  digitalWrite(16, 0);
  delay(100);
  digitalWrite(16, 1);


  // The ESP is capable of rendering 60fps in 80Mhz mode
  // but that won't give you much time for anything else
  // run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(60);

  // Customize the active and inactive symbol
  //ui.setActiveSymbol(activeSymbol);
  //ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(LEFT);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_UP);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();

  display.flipScreenVertically();

  // unsigned long secsSinceStart = millis();
  // // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  // const unsigned long seventyYears = 2208988800UL;
  // // subtract seventy years:
  // unsigned long epoch = secsSinceStart - seventyYears * SECS_PER_HOUR;
  setTime(558100800);

  if (!bmp.begin()) {
    strcpy(wifistate, "BMP085 ko, WIFI ko");
    BMP085state = 0,
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  } else {
    strcpy(wifistate, "BMP085 ok, WIFI ko");
    BMP085state = 1;
  }
}

uint8_t my_atoi(uint8_t len) {
  static char buff[5];
  static uint8_t i;
  i = 0;
  while (i < len){
    buff[i] = Serial.read();
    i++;
  }
  buff[i] = '\0';

  return atoi(buff);
}

void loop() {
  digitalWrite(25, LOW);
  static int remainingTimeBudget;
  remainingTimeBudget = ui.update();
  static int _hr, _min, _sec, _day, _month, _yr;
  static char key;

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }
  if (Serial.available()) {
    key = Serial.read();
  }

  if (key == 'h') {
    displayhelp();
    key = '\0';
  }

  if (key == 'T') {
    Serial.println("ok set");
    _yr = my_atoi(4);
    _month = my_atoi(2);
    _day = my_atoi(2);
    _hr = my_atoi(2);
    _min = my_atoi(2);
    _sec = my_atoi(2);
    key = 0;
    setTime(_hr, _min, _sec, _day, _month, _yr);
  }
  sc[1] = sc[0];
  sc[0] = (millis()/1000) % 10;
  //Serial.print(sc[0]);
  //Serial.println(sc[1]);
  if (sc[0] == 0 && sc[1] !=  0 && WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    // Serial.println(timeClient.getFormattedTime());
    // Serial.println(timeClient.getDay());
    //Serial.println(timeClient.
    // _yr = year();
    // _month = month();
    // _day = day();
    // _hr = timeClient.getHours();
    // _min = timeClient.getMinutes();
    // _sec = timeClient.getSeconds();
    // setTime(_hr, _min, _sec, _day, _month, _yr);
    setTime(timeClient.getEpochTime());
    digitalWrite(25, HIGH);
  }
}
