/*
  This software bases on the example "Reading CO2, humidity and temperature from the SCD30" by Nathan Seidle, SparkFun Electronics
  By: Dr. Andreas Beck
  Gymnasium Kirn
  Date: 14.Feb.2021
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.
  Hardware Connections:
  Attach Arduino Nano V3.x to computer using a USB cable.
  Open Serial Monitor at 115200 baud, when 
*/

/* n.b. With activated OLED-Display the Arduino hangs up. Only a hard reset by pulling the power cable will help. 
        Without an attached OLED-Display the CO2-Raumluftwaechter works mostly without any flaws and crashes over days.  */

#include <Arduino.h>
#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30 
#include <U8x8lib.h>
#include <RTC.h>
#include <SPI.h>
#include <SD.h>
#include <FastLED.h>


// change this to match your SD shield or module;
const int chipSelect = 10;

// Options remove comment for different behavior of the CO2-Raumluftwaechter
 
//#define SOP       // serial output (SerialOutPut, if != 0 the ON else OFF
//#define RTCE      // RealTimeClock enable
//#define OLED      // OLED enable
//#define OLEDR     // OLED refresh: only required for SSD1606/7  
//#define SDCARD    // SDCARD enable
//#define DEBUG     // enables debug mode by using extensive serial monitor
//#define NOC       // no calibration / autocalibration is disabled

#define CO2VERSION  "CO2-Raumluftwaecher V"
#define MAINVERSION 0
#define SUBVERSION  64    // version number for displaying over LED ring
#define CO2OFFSET   -110      // if necessary you can add here an offset for meassured co2 values
#define PTRS        3     // number of single meassured CO2 values for averaging 
#define LED_PIN     3     // Arduino pin no. for LED
#define NUM_LEDS    16    // number of the used LEDs in the design

SCD30 airSensor;          // declare air sensor SCD30

CRGB leds[NUM_LEDS];      // declare array of LEDs

#ifdef OLED   // OLED Display supported so far
//U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);         
//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
#endif


const int lum = 1;      // declare const brightness (luminosity) level 1 (dark) to 8 (bright); 1 is for the most cases suitable
int aval = 0;           // average CO2 value
int oldco2level = 0, co2level = 0;    //CO2 levels actual and old one
#if (defined RTCE) || (defined OLED)
int cday, cmonth, cyear;              // declare day, month and year for RTC
static DS3231 RTC;                    // declare RTC
#endif
String fname = "";                    // declare filename
int timer, heartbeat = 0;             // declare variables vor timer (time counter between two meassured CO2 values) und heartbeat counter


/***********************************************************************
 *                          setup                                      *  
 ***********************************************************************/
void setup( void)
{
int i, j;

  #if ( defined SOP) || ( defined DEBUG)  // set up serial connection 115200 Baud
  Serial.begin(115200);
  Serial.print(F(CO2VERSION));
  Serial.print(MAINVERSION);
  Serial.print(F("."));
  Serial.println(SUBVERSION);
  Serial.print(F("CO2-Offset: "));
  Serial.println(CO2OFFSET);

//  #if ( defined SOP) || ( defined DEBUG)  // set up serial connection 115200 Baud
 #endif

  #ifdef SDCARD // see if the card is present and can be initialized:
  #if ( defined  SOP) || ( defined DEBUG)
  Serial.print(F("Initializing SD card..."));
  #endif // SOP || DEBUG 

  #ifdef DEBUG
  Serial.println(F("SD.begin"));
  #endif
  
  if (!SD.begin(chipSelect)) {
    #if ( defined SOP) || ( defined DEBUG)
    Serial.println(F("Card failed, or not present"));
    #endif
    
    // don't do anything more:
    while (1)
      ;
  }
  
  #ifdef SOP  
  Serial.println(F("card initialized."));
  #endif
  #endif  //SDCARD

  #ifdef RTCE
  RTC.begin();      // initialize RTC

  DateTime A1, A2;  //

  #ifdef SOP
  Serial.println();
  Serial.println(F("*** RTC 3231 ***"));
  Serial.print(F("Is Clock Running : "));
  #endif

  if ( RTC.isRunning()) {
    #ifdef SOP
    Serial.println(F("Yes"));
    #endif
  } else {
    #ifdef SOP    
    Serial.println(F("No. Time may be Inaccurate"));
    #endif
  
    RTC.startClock();
  }
  #endif  //RTCE

  #if (defined SOP) && (defined RTCE)  
  Serial.print("Hour Mode :");
  if (RTC.getHourMode() == CLOCK_H12)
    Serial.println("12 Hours");
  else
    Serial.println("24 Hours");
  #endif
  
  #ifdef RTCE
  cday = RTC.getDay();
  cmonth = RTC.getMonth();
  cyear = RTC.getYear();
  cyear -= 2000;
  #elif ( defined OLED)
  cday = 6;
  cmonth = 9;
  cyear = 19;
  #endif 

  #if ( defined SDCARD) && ( defined RTCE)
  fname += String(cyear);
  fname += String(cmonth);
  fname += String(cday);
  #else
  fname += String(20);
  fname += String(10);
  fname += String(01);
  #endif

  // init all LEDs
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  // show on all LEDs white
  for( i = 0; i < 16; i++) {
    leds[i] = CRGB(lum, lum, lum);  
    //leds[trans[i]] = CRGB(0, 0, 0);  
  }
  FastLED.show();

  delay(2000);      // wait 2 seconds before version number and correctin value is shown

  // main version number is shown on leds[0] = D1: blue(b) = 0; rg = 1; rb = 2; gb = 3; else: black
  switch ( MAINVERSION) { 
    case 0: leds[0] = CRGB( 0, 0, lum); break;    // main version number: 0 blue (b)
    case 1: leds[0] = CRGB( lum, lum, 0); break;  // main version number: 0 red green: yellow (rg)
    case 2: leds[0] = CRGB( lum, 0, lum); break;    // main version number: 0 red blue: violet (rb)
    case 3: leds[0] = CRGB( 0, lum, lum); break;    // main version number: 0 green blue: cyan blue (gb)
    default: leds[0] = CRGB( 0, 0, 0); break;   // main version number: unknown - black
  }

  
  // show sub version number leds[1 to 7] in white D2-7
  j = SUBVERSION;
  for( i = 1; i < 8; i++) {
    if( j & 1) {
      leds[i] = CRGB(lum, lum, lum);  
    } else {
      leds[i] = CRGB(0, 0, 0);
    }
    j = j >> 1;
  }
//  for( i = 1; i < 8; i++) {
//    if( vc[i] > 0 ) {
//      leds[i] = CRGB(lum, lum, lum);  
//    } else {
//      leds[i] = CRGB(0, 0, 0);
//    }
//  }

  // show correction value on leds[8 to 15] negative in red and positive in green D9-D16 starting with Bit 1 and not 0
  j = abs( CO2OFFSET);
  for( i = 8; i < 16; i++) {
     j = j >> 1;
    if( j & 1 ) {
      if( CO2OFFSET > 0) {
        leds[i] = CRGB(0, lum, 0);
      } else {  
        leds[i] = CRGB(lum, 0, 0); 
      }
    } else {
      leds[i] = CRGB(0, 0, 0);
    }
  }
  
//  for( i = 8; i < 16; i++) {
//    if( CO2OFFSET > 0) {
//      if( vc[i] > 0 ) {
//        leds[i] = CRGB(0, lum, 0);  
//      } else {
//        leds[i] = CRGB(0, 0, 0);
//      }
//    } else {
//      if( vc[i] > 0 ) {
//        leds[i] = CRGB(lum, 0, 0);  
//      } else {
//        leds[i] = CRGB(0, 0, 0);
//      }
//    }
//  }
  FastLED.show();

  
  delay(4000);      // wait 2 seconds before init of CO2 sensor 

  // clear all LEDs
   for( i = 0; i < 16; i++) {
    leds[i] = CRGB(0, 0, 0);  
    //leds[trans[i]] = CRGB(0, 0, 0);  
  }
  FastLED.show();

  Serial.println(F("Sensor Init"));

  Wire.begin();
  Wire.setClock(100000L);
  timer = 0;

  do{
     if (airSensor.begin() == false) {
//     if (airSensor.begin( Wire, false) == false) {
      #ifdef SOP
      Serial.println(F("Air sensor not detected. Please check wiring."));
      #endif
      
      if( timer >= 3) {
        for( i = 1; i < 16; i+=2) {
          leds[i] = CRGB(32*lum, 0, 0);
          leds[i+1] = CRGB(32*lum, 0, 0);
        }   
        FastLED.show();
        #ifdef SOP
        Serial.println(F(" Freezing..."));
        #endif
        while (1)
          ;
      }
    }
    timer++;
    delay(2000);
  } while ( timer < 4);

  timer = 0;

  leds[0] = CRGB(0, 12*lum, 0);     //CRGB(0, 6*lum, 0);
  for( i = 1; i < 16; i++) {
    leds[i] = CRGB(0, 0, 0);  
  }
  FastLED.show();

  oldco2level = 0;
  co2level = 0;   // 0 to 8


  //The SCD30 has data ready every two seconds
  airSensor.setMeasurementInterval(4); //Change number of seconds between measurements: 2 to 1800 (30 minutes)

  //My desk is ~1600m above sealevel
  airSensor.setAltitudeCompensation(104); //Set altitude of the sensor in m

  //Pressure in Boulder, CO is 24.65inHg or 834.74mBar
  //airSensor.setAmbientPressure(1020); //Current ambient pressure in mBar: 700 to 1200

  
  #ifdef SOP  
  float offset = airSensor.getTemperatureOffset();
  Serial.print(F("Current temp offset: "));
  Serial.print(offset, 2);
  Serial.println(F("C"));
  #endif
  
  airSensor.setTemperatureOffset( 3.2); //Optionally we can set temperature offset to 5°C

//  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  #ifdef OLED
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  u8x8.setCursor( 0, 0);
  u8x8.print(F("CO2 [ppm]: "));
  u8x8.setCursor( 0, 2);
  u8x8.print(F("feucht.[%]: "));
  u8x8.setCursor( 0, 4);
  u8x8.print(F("temp [C]: "));
  #endif //OLED
}


/***********************************************************************
 *                          setlevel                                   *  
 ***********************************************************************/
int setlevel(int co2val) {
static int vals[3] = { 0, 0, 0}, lastgood = 0, meanval = 0, valptr = 0, i;

  co2val += CO2OFFSET;  // add offset value to the measured CO2 value

  // for the first start last known good value is zero: 
  //
  if( lastgood == 0) {  //set up array and last known good value 
    lastgood = co2val;  // set the actual CO2 value to the last known good
    for( i = 1; i < PTRS; i++) { // fill array with the first measured value
      vals[i] = co2val; 
    }
  }

  // If the CO2 concentration in the air is unreasonble:
  //
  if ( (co2val > 4000) || ( co2val < 0)) {   // set the definded use lastgood value
    vals[valptr] = lastgood;
  } else {
     vals[valptr] = co2val;
  }
  valptr++;

  if( valptr == PTRS) {
    valptr = 0;
  }

  meanval = 0;
  for( i = 0; i < PTRS; i++) {
    meanval += vals[i];
  }
  meanval = meanval / PTRS;  

  if( meanval < 400) {
   return ( int) 0;
  } else if( co2val < 500) {
   return ( int) 1; 
  } else if( co2val < 600) {
   return ( int) 2;
  } else if( co2val < 700) {
   return ( int) 3;
  } else if( co2val < 800) {
   return ( int) 4;
  } else if( co2val < 900) {
   return ( int) 5;
  } else if( co2val < 1000) {
   return ( int) 6;
  } else if( co2val < 1100) {
   return ( int) 7;
  } else return ( int) 8;
}


/***********************************************************************
 *                          visualize                                  *  
 ***********************************************************************/
void visualize( int level) {
int g, o, r, i;                    // variables for green, orange, red and i as a loop counter
static int oldlevel = 0, led0 = 0; // the old CO2 level and heart beat of LED 0 should variables should be kept for the next function call

  if( level != oldlevel) {         // if CO2 level has been change then determine which LED should light up and colour
    switch( level) {
      case 0:  g = 0;  o = 0;   r = -1;  break;
      case 1:  g = 2;  o = 0;   r = -1;  break;
      case 2:  g = 4;  o = 0;   r = -1;  break;
      case 3:  g = 6;  o = 0;   r = -1;  break;
      case 4:  g = 8;  o = 0;   r = -1;  break;
      case 5:  g = 8;  o = 2;   r = -1;  break;
      case 6:  g = 8;  o = 4;   r = -1;  break;
      case 7:  g = -1; o = 12;  r = 2;  break;
      default: g = -1; o = -1;  r = 15; break;
    }
  
    if( g != -1) {
      led0 = 0;
      for( i = 0; i <= g; i++) {
        if( i == 0) {
          if( heartbeat != 0) {
            leds[i] = CRGB( 0, 2*lum, 0);  //CRGB( 0, 12*lum, 0);
          } else {
            leds[i] = CRGB( 0, 12*lum, 0);  //CRGB( 0, 12*lum, 0)
          }
        } else {
          leds[i] = CRGB( 0, 12*lum, 0);  //CRGB( 0, 12*lum, 0); 
        }
      }
      for(  i = (g + o + 1); i <= 15; i ++) {
        leds[i] = CRGB( 0, 0, 0); 
      }
    }
        
    if( o != -1) 
      if( g == -1) {
        led0 = 1;
      }
      for( i = (g + 1); i <= (g + o); i++) {
        if( i == 0) {
          if( heartbeat == 0) {
            leds[i] = CRGB( 10*lum, 4*lum, 0); //CRGB( 32*lum, 16*lum, 0); 
          } else {
            leds[i] = CRGB( 32*lum, 16*lum, 0); //CRGB( 32*lum, 16*lum, 0); 
          }
        } else {
          leds[i] = CRGB( 32*lum, 16*lum, 0); //CRGB( 32*lum, 16*lum, 0); 
        }
      }
      
    if( r != -1) 
      led0 = 2;
      for( i = (o + 1); i <= (o + r + 1); i++) {
        if( i == 0) {
          if( heartbeat == 0) {
            leds[i] = CRGB( 10*lum, 0, 0); //CRGB( 32*lum, 16*lum, 0); 
          } else {
            leds[i] = CRGB( 32*lum, 0, 0); //CRGB( 32*lum, 16*lum, 0); 
          }
        } else {
          leds[i] = CRGB( 32*lum, 0, 0); //CRGB( 32*lum, 16*lum, 0); 
        }
      }
      
    oldlevel = level;
    
  } else if ( led0 == 0) {
    if( heartbeat != 0) {
      leds[0] = CRGB( 0, 2*lum, 0);  //CRGB( 0, 12*lum, 0);
    } else {
      leds[0] = CRGB( 0, 12*lum, 0);  //CRGB( 0, 12*lum, 0)
    }
  } else if ( led0 == 1) {
    if( heartbeat == 0) {
      leds[0] = CRGB( 10*lum, 4*lum, 0); //CRGB( 32*lum, 16*lum, 0); 
    } else {
      leds[0] = CRGB( 32*lum, 16*lum, 0); //CRGB( 32*lum, 16*lum, 0); 
    }
  } else if ( led0 == 2) {
    if( heartbeat == 0) {
      leds[i] = CRGB( 10*lum, 0, 0); //CRGB( 32*lum, 16*lum, 0); 
    } else {
      leds[i] = CRGB( 32*lum, 0, 0); //CRGB( 32*lum, 16*lum, 0); 
    }
  }

  FastLED.show();
}


/***********************************************************************
 *                          loop                                       *  
 ***********************************************************************/
void loop( void)
{
String dataString = "";  
int co2, cut;
#ifdef RTCE
int newday, chours, cmin, csec;
#endif //RTCE
float hum, temp;

  #ifdef DEBUG
  Serial.print(F("Loop: HB:"));
  Serial.print(heartbeat);
  Serial.print(F(" Start, "));
  #endif //DEBUG


  #ifdef RTCE
  #ifdef DEBUG
  Serial.print(F("RTC, "));
  #endif //DEBUG
  
  newday = RTC.getDay();
  
  if( newday != cday) {
    cday = newday;
    cmonth = RTC.getMonth();
    cyear = RTC.getYear();
    cyear -= 2000;
    fname = String(cyear);
    fname += String(cmonth);
    fname += String(cday);
  }
  #else //RTC
  fname += String(20);
  fname += String(0);
  fname += String(9);
  fname += String(31);
  #endif

  #ifdef DEBUG
  Serial.print(F("CO2S, "));
  #endif

  if( airSensor.dataAvailable()) {
    if ( timer < 5) {
      //The SCD30 should have data ready every four seconds. If the time interval is not correct between two messurements then set the measurement interval back to 4 seconds.
      airSensor.setMeasurementInterval(4); //Change number of seconds between measurements: 2 to 1800 (30 minutes)
    }
    
    #ifdef DEBUG
    Serial.print(F("NCO2, "));
    #endif //DEBUG
    
    co2 = airSensor.getCO2();

    #ifdef DEBUG
    Serial.print(F("GTem, "));
    #endif //DEBUG
    
    temp = airSensor.getTemperature();
    temp *=10;
    cut = temp;
    temp = cut;
    temp /= 10;
    if( ( temp < 0.0) || ( temp > 42.0)) {
      temp = 41.9;
    }

    #ifdef DEBUG
    Serial.print(F("GHum, "));
    #endif //DEBUG

    hum = airSensor.getHumidity();
    hum *= 10;
    cut = hum;
    hum = cut;
    hum /= 10;
    if( ( hum < 0.0) || ( hum > 100.0)) {
      hum = 99.9;
    }

    #ifdef SOP
    Serial.print(F("co2(ppm):"));
    Serial.print(co2);

    Serial.print(F(" temp(C):"));
    Serial.print(temp, 1);

    Serial.print(F(" humidity(%):"));
    Serial.print(hum, 1);
    Serial.println();
    #endif SOP

    #ifdef DEBUG
    Serial.print(F("SLev, "));
    #endif //DEBUG

    co2level = setlevel( co2);


    #ifdef DEBUG
    Serial.print(F("Visu, "));
    #endif //DEBUG

    visualize( co2level);


//    if( co2level != oldco2level) {
//      oldco2level = co2level;
//      visualize( co2level);
//    }


    #ifdef OLED

    #ifdef DEBUG
    Serial.print(F("OLED0"));
    #endif //DEBUG

//    u8x8.setCursor( 0, 0);
//    u8x8.print(F("CO2 [ppm]: "));
//    u8x8.setCursor( 0, 2);
//    u8x8.print(F("feucht.[%]: "));
//    u8x8.setCursor( 0, 4);
//    u8x8.print(F("temp [C]: "));
       
    #ifdef DEBUG
    Serial.print(F(",1"));
    #endif //DEBUG
    u8x8.setCursor( 11, 0);

    if( co2 < 1000) {
      #ifdef DEBUG
      Serial.print(F("a"));
      #endif //DEBUG
      u8x8.print(F(" "));
    }
    
    #ifdef DEBUG
    Serial.print(F("2"));
    #endif //DEBUG
    u8x8.print(co2);


//    #ifdef DEBUG
//    Serial.print(F("3"));
//    #endif //DEBUG
//    u8x8.setCursor( 0, 2);
//    
//    #ifdef DEBUG
//    Serial.print(F("4"));
//    #endif //DEBUG
//    u8x8.print(F("feucht.[%]: "));

    #ifdef DEBUG
    Serial.print(F("3"));
    #endif //DEBUG
    u8x8.setCursor( 12, 2);
    
    #ifdef DEBUG
    Serial.print(F("4"));
    #endif //DEBUG
    u8x8.print( hum, 1);

    #ifdef DEBUG
    Serial.print(F("5"));
    #endif //DEBUG
    u8x8.setCursor( 10, 4);
    
    #ifdef DEBUG
    Serial.print(F("6, "));
    #endif //DEBUG
    u8x8.print( temp, 1);

    #ifdef DEBUG
    Serial.print(F("ODT, "));   //OLED Date Time
    #endif //DEBUG
    u8x8.setCursor(0,6);
    u8x8.print(F("Date: "));
    u8x8.print(cday);
    u8x8.print(F("."));
    u8x8.print(cmonth);
    u8x8.print(F("."));
    u8x8.print( cyear);

    u8x8.setCursor(0,7);
    u8x8.print(F("Time: ")); 
    u8x8.print(F("        "));
    u8x8.setCursor(6,7);
    #endif //OLED
    
    #ifdef RTCE
    #ifdef DEBUG
    Serial.print(F("GRTC, "));
    #endif //DEBUG

    chours = RTC.getHours();
    #ifdef OLED
    u8x8.print(chours);
    u8x8.print(F(":"));
    #endif //OLED
    cmin = RTC.getMinutes();
    #ifdef OLED
    u8x8.print(cmin);
    u8x8.print(F(":"));
    #endif //OLED
    csec = RTC.getSeconds();
    #ifdef OLED
    u8x8.print(csec);
    #endif //OLED
    #elif (defined OLED)  //endif RTC else OLED
    u8x8.print(10); //hours
    u8x8.print(F(":"));
    u8x8.print(11); //minutes
    u8x8.print(F(":"));
    u8x8.print(12); //seconds
    #endif //OLED

    #ifdef SDCARD
    dataString = String(chours) + ":" + String(cmin) + ":" + String(csec)+ ", " + String(co2) + ", " + String(temp) + ", " + String(hum); 
    
    File dataFile = SD.open(fname, FILE_WRITE);
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      #ifdef SOP      
      Serial.println(dataString);
      #endif //SOP
    }
    // if the file isn't open, pop up an error:
    else {
      #ifdef SOP
      Serial.println(F("error opening datalog.txt"));
      #endif //SOP
    }
    #endif //SDCARD

    timer = 0;

    #ifdef DEBUG
    Serial.print(F("OLHB, "));
    #endif //DEBUG

    #ifdef OLED
    u8x8.setCursor(15,7);
    #endif //OLED

    if( heartbeat == 0) {
      #ifdef OLED
      u8x8.setInverseFont(0);
      #endif //OLED
      heartbeat++;
    } else {
      #ifdef OLED
      u8x8.setInverseFont(1);
      #endif //OLED
      heartbeat = 0;
    }

    #ifdef OLED
    u8x8.print(F(" "));
    u8x8.setInverseFont(0);
    #endif //OLED
    
  } else {
    #ifdef DEBUG
    Serial.print(F("Timer, "));
    #endif //DEBUG

    timer++;
    if(timer >= 8) {
      #ifdef SOP
      Serial.println(F("Waiting over 4 seconds for new data"));
      #endif //SOP
      timer = 0;
    }
  }

  #ifdef OLEDR
  #ifdef DEBUG
  Serial.print(F("OLEDR, "));
  #endif //DEBUG

  u8x8.refreshDisplay();    // only required for SSD1606/7  
  #endif //OLEDR

  delay(500);

  #ifdef DEBUG
  Serial.println(F("."));
  #endif //DEBUG

}
