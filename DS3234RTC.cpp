/*
 * DS3234RTC.h - library for DS3234 RTC
 * Assembled by KristoZ (http://kristoz.com)

  This library is intended to be used with Arduino Time.h library functions

  The library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
 */

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include <avr/pgmspace.h>
#include <SPI.h>
#include "DS3234RTC.h"

const int CONTROL_R = 0x0e;
const int CONTROL_W = 0x8e;
const int CONTROL_STATUS_R = 0x0f;
const int CONTROL_STATUS_W = 0x8f;
const int SECONDS_R = 0x00;
const int SECONDS_W = 0x80;
const int MODE = 0x60; //60= disable Osciallator and Battery SQ wave @1hz, temp compensation, Alarms disabled
const int EOSC = 7;
const int OSF = 7;
int cs_pin;

DS3234RTC::DS3234RTC() {
  SPI.begin();
}
void DS3234RTC::begin(int _cs_pin)
{
  cs_pin = _cs_pin;
  pinMode(cs_pin,OUTPUT);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE1); // both mode 1 & 3 should work 
  cs(LOW);
  SPI.transfer(CONTROL_W);
  SPI.transfer(MODE); 
  cs(HIGH);
  delay(10);
}

void DS3234RTC::cs(int _value)
{
  digitalWrite(cs_pin,_value);
}

bool DS3234RTC::isRunning()
{
  cs(LOW);
  SPI.transfer(CONTROL_R);
  uint8_t ss = SPI.transfer(-1);
  cs(HIGH);
  return !(ss & _BV(OSF));
}

time_t DS3234RTC::get()   // Aquire data from buffer and convert to time_t
{
  tmElements_t tm;
  read(tm);
  time_t rt = makeTime(tm);
  return(rt);
}

void  DS3234RTC::set(time_t t)
{
  tmElements_t tm;
  breakTime(t, tm);
  int TimeDate [7]={
    second(t),minute(t),hour(t),weekday(t),day(t),month(t),year(t)-2000  };
  for(int i=0; i<=6;i++){
    if(i==3)
      i++;
    int b= TimeDate[i]/10;
    int a= TimeDate[i]-b*10;
    if(i==2){
      if (b==2)
        b=B00000010;
      else if (b==1)
        b=B00000001;
    }	
    TimeDate[i]= a+(b<<4);
    cs(LOW);
    SPI.transfer(i+SECONDS_W); 
    SPI.transfer(TimeDate[i]);        
    cs(HIGH);
  }
}

// Aquire data from the RTC chip in BCD format
void DS3234RTC::read( tmElements_t &tm)
{
  cs(LOW);
  SPI.transfer(SECONDS_R);
  tm.Second = bcd2dec(SPI.transfer(-1) & 0x7F); 
  tm.Minute = bcd2dec(SPI.transfer(-1));
  tm.Hour = bcd2dec(SPI.transfer(-1));  // mask assumes 24hr clock
  tm.Wday = bcd2dec(SPI.transfer(-1));
  tm.Day = bcd2dec(SPI.transfer(-1));
  tm.Month = bcd2dec(SPI.transfer(-1));
  tm.Year = y2kYearToTm((bcd2dec(SPI.transfer(-1))));
  cs(HIGH);
}

// PRIVATE FUNCTIONS

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t DS3234RTC::dec2bcd(uint8_t num)
{
  return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t DS3234RTC::bcd2dec(uint8_t num)
{
  return ((num/16 * 10) + (num % 16));
}

DS3234RTC RTC = DS3234RTC(); // create an instance for the user
