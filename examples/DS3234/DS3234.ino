#include <Time.h>
#include <SPI.h>
#include <Wire.h>
#include <DS3234RTC.h>

void setup () {
  Serial.begin(9600);
  RTC.begin(8); //Start the RTC using the chip select pin it's connected to
  //Optional: Check whether the RTC has been correctly connected
  if (RTC.isRunning()) {
    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    if(timeStatus()!= timeSet) 
      Serial.println("Unable to sync with the RTC");
    else
      Serial.println("RTC has set the system time");  
  }  
  else
    Serial.println("ERROR: RTC is NOT connected!");     
}

void loop () {
  if(Serial.available())
  {
    time_t t = processSyncMessage();
    if(t >0)
    {
      RTC.set(t);   // set the RTC and the system time to the received value
      setTime(t);   
      setSyncProvider(RTC.get);   // the function to get the time
      Serial.println("RTC has set the system time");            
    } 
  }

  //Optional: Update system time from RTC
  //Every day at 23:00:00
  if ((hour(now()) % 23 == 0) && (minute(now()) % 0 == 0) && (second(now()) % 0 == 0)) {
    setTime(RTC.get());
    if(timeStatus()!= timeSet) 
      Serial.println("Unable to sync with the RTC");
    else
      Serial.println("RTC has set the system time");    
  }

  digitalClockDisplay();  
  delay(1000);
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(year());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*  code to process time sync messages from the serial port   */
#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by unix time_t as ten ascii digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message

time_t processSyncMessage() {
  // return the time if a valid sync message is received on the serial port.
  while(Serial.available() >=  TIME_MSG_LEN ){  // time message consists of a header and ten ascii digits
    char c = Serial.read() ; 
    Serial.print(c);  
    if( c == TIME_HEADER ) {       
      time_t pctime = 0;
      for(int i=0; i < TIME_MSG_LEN -1; i++){   
        c = Serial.read();          
        if( c >= '0' && c <= '9'){   
          pctime = (10 * pctime) + (c - '0') ; // convert digits to a number    
        }
      }   
      return pctime; 
    }  
  }
  return 0;
}
