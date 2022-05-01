
//THIS SKETCH USES A MODIFIED MD_Parola.h FILE TO FREE MEMORY
//BY REMOVING GRAPHIC FUNCTIONALITY AS INDICATED IN FILE
//ARDUINO R3, MAX7219 LED MATRIX, DS1307 RTC, DS18B20 TEMP SENSOR, 4x4 MATRIX ARRAY KEYPAD
//REQUIRES USB POWER FOR AMP REQUIREMENTS
//PINOUT
//
//PD1- KEYPAD   PIN 8 (CLOSEST TO 'D')
//PD2- DS1307   DS
//PD3- MAX7219 CS
//PD4- KEYPAD   PIN 7
//PD5- KEYPAD   PIN 6
//PD6- KEYPAD   PIN 5
//PD7- KEYPAD   PIN 4
//PB0- KEYPAD   PIN 3
//PB1- KEYPAD   PIN 2
//PB2- KEYPAD   PIN 1 (CLOSEST TO '*')
//PB3 COPI- MAX7219 DIN
//PB5 SCK- MAX7219 CLK
//PC5 A5- DS1307 SCL
//PC4 A4- DS1307 SDA
//5V- DS1307 VCC, MAX7219 VCC
//GND- DS1307 GND, MAX7219 GND
//
//TOOLING FOR CALLING TEMPERATURE, CURRENTLY UNUSED
//
//#include <OneWire.h>
//#include <DallasTemperature.h>
//#define ONE_WIRE_BUS 2
//OneWire oneWire(ONE_WIRE_BUS);
//DallasTemperature sensors(&oneWire);

#include <Keypad.h>
#include <Wire.h>

const int DS1307 = 0x68;  
byte second = 0;
byte minute = 0;
byte hour = 0;
byte weekday = 1;
byte monthday = 1;
byte month = 1;
byte year = 21;
int ledbrt = 1;
byte ledval = 1;

unsigned long vart;
unsigned long vart1;
unsigned long vart2;
unsigned long gap;

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {10, 9, 8, 7};
byte colPins[COLS] = {6, 5, 4, 1};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
char key;
byte keyAsANumber;
byte var;
unsigned int total = 0;

int hrlen = 0; 
int secjust = 0;
int runjust = 0;
int ctr2 = 0;
byte sett = 0;
byte prevsec = 0;

#include <MD_Parola.h>//MODIFIED FILE
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "Font_Data2.h"//stylized, else use "Font_Data.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 3
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void setup() {

  //sensors.begin();
  Wire.begin();
  delay(2000); 

  myDisplay.begin();
  myDisplay.setIntensity(1);
  myDisplay.displayClear();
  myDisplay.setFont(numeric7Seg);
  myDisplay.setTextAlignment(PA_CENTER);
  
   myDisplay.print("OK");
   delay(1000);
   myDisplay.print("SET");
   delay(1000);
    myDisplay.print("BRIGHT");
   delay(1000);
   myDisplay.print("LEVEL");
   delay(1000);
   myDisplay.print("1-5");
   delay(100);
   ledval = readByte();
   ledbrt = ledval;
   if (ledval <= 5 && ledval >= 1) {
    myDisplay.setIntensity(ledval);
    }
  myDisplay.print("OK");
   delay(1000);
   myDisplay.print("SET");
   delay(1000);
   myDisplay.print("TIME?");
   delay(1000);
   myDisplay.print("YES-1");
   delay(1000);
   myDisplay.print("NO-0");
   delay(100);
   sett = readByte();
   if (sett == 1) {
    clksend();
    setTime();
   }
}

void loop() {

vart = millis(); 
vart1 = vart + 1001;

ctr2 += 1;
if (ctr2 >= 30000) {//31320 8.7 hrs, 28800 8 hrs
  ctr2 = 0;         //THIS COMPENSATES FOR EXCESSIVE SPEED
  if (second >= 1) {
  second -= 1;
  clksend();
  } else {
    second = 59;
    clksend();
  }
  delay(100);
}

printTime();

//sensors.requestTemperatures();

vart2 = millis();
if (vart2 < vart1) {//TIMES THE DISPLAY INCREMENTS
   gap = vart1 - vart2;
   if (gap < 5000) { 
    delay(gap);
    } else {
    delay(500);}
  }
}



byte decToBcd(byte val) {//CONVERTS DATA FOR RTC RX-TX
  return ((val/10*16) + (val%10));
}
byte bcdToDec(byte val) {
  return ((val/16*10) + (val%16));
}

void (* resetFunc) (void) = 0;//RESETS ARDUINO

void setTime() {
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print("ENTER");
  delay(1000);
  myDisplay.print("HOUR");
  delay(1000);
  myDisplay.print("0-23");
  delay(500);
  hour = readByte();
  if (hour <= 23) {
  myDisplay.print("OK");
  delay(500);
  } else {
    myDisplay.print("INVALID");
  delay(500);
  resetFunc();
  }
  String hrdisp = String(hour);
  myDisplay.print(hrdisp);
  delay(1700);
  
  myDisplay.print("ENTER");
  delay(1000);
  myDisplay.print("MINUTE");
  delay(1000);
  myDisplay.print("0-59");
  delay(500);
  minute = readByte();
  if (minute <= 59) {
    second = 1;
  } else {
    myDisplay.print("INVALID");
  delay(500);
  resetFunc();
  }
  myDisplay.print("OK");
  delay(400);
  clksend();
}

 void clksend() {//RTC TX
  Wire.beginTransmission(DS1307);
  Wire.write(byte(0));
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(weekday));
  Wire.write(decToBcd(monthday));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.write(byte(0));
  Wire.endTransmission();
 }

byte readByte() {//KEYPAD FN, SPECIFIC FOR THIS INSTANCE
  myDisplay.setTextAlignment(PA_CENTER);
  total = 0;
  var = 0;
  String btdisp;
  key = keypad.waitForKey();
  if (key != '#') 
  {
  if (key >= '0' && key <= '9')
  {
    keyAsANumber = key - 48;
    total = total * 10;
    total = total + keyAsANumber;
    String btdisp = String(key);
    myDisplay.print(btdisp);
    delay(250);
    }
  } 
  else if (key == '#'){
    var = total;
    myDisplay.displayClear();
    delay(100);
    return var; 
    }
  key = keypad.waitForKey();
  if (key != '#') 
  {
  if (key >= '0' && key <= '9')
  {
    keyAsANumber = key - 48;
    total = total * 10;
    total = total + keyAsANumber;
    btdisp = key;
    myDisplay.displayClear();
    delay(100);
    myDisplay.print(btdisp);
    delay(550);
    var = total;
    return var;
    }
  } 
  else if (key == '#'){
    myDisplay.displayClear();
    delay(100);
    var = total;
    return var; 
    }
}

void printTime() {//ADJUSTS VALUES DOUBLE-DIGIT FOR DISPLAY RENDER
  readTime();
 if (hour >= 10 && hour <= 23) {
  if (minute >= 0 && minute <= 9) {
    if (second >= 0 && second <= 9) {
  String minsec = String(hour) + "0" + String(minute) + "0" + String(second);
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(minsec);
  
      }
    }
  }
if (hour >= 0 && hour <= 9) {
 if (minute >= 0 && minute <= 9) {
   if (second >= 0 && second <= 9) {
  String minsec = "0" + String(hour) + "0" + String(minute) + "0" + String(second);
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(minsec);
  
      }
    }
  }
if (hour >= 0 && hour <= 9) {
  if (minute >= 0 && minute <= 9) {
    if (second >= 10 && second <= 59) {
  String minsec = "0" + String(hour) + "0" + String(minute) + String(second);
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(minsec);
      }
    }
  }
if (hour >= 0 && hour <= 9) {
  if (minute >= 10 && minute <= 59) {
    if (second >= 10 && second <= 59) {
  String minsec = "0" + String(hour) + String(minute) + String(second);
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(minsec);
      }
    }
  }
if (hour >= 0 && hour <= 9) {
  if (minute >= 10 && minute <= 59) {
    if (second >= 0 && second <= 9) {
  String minsec = "0" + String(hour) + String(minute) + "0" +  String(second);
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(minsec);
      }
    }
  }
if (hour >= 10 && hour <= 23) {
  if (minute >= 10 && minute <= 59) {
    if (second >= 0 && second <= 9) {
  String minsec = String(hour) + String(minute) + "0" +  String(second);
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(minsec);
      }
    }
  }
if (hour >=10 && hour <= 23) {
  if (minute >= 0 && minute <= 9) {
    if (second >= 10 && second <= 59) {
  String minsec = String(hour) + "0" + String(minute) + String(second);
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(minsec);
      }
    }
  }    
if (hour >= 10 && hour <= 23) {
  if (minute >= 10 && minute <= 59) {
    if (second >= 10 && second <= 59) {
  String minsec = String(hour) + String(minute) + String(second);
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(minsec);
      }
    }
  }  
}

void readTime() {//RTC RX
  Wire.beginTransmission(DS1307);
  Wire.write(byte(0));
  Wire.endTransmission();
  Wire.requestFrom(DS1307, 7);
  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read());
  weekday = bcdToDec(Wire.read());
  monthday = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());
}
