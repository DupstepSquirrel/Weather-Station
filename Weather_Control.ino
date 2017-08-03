#include <DS3231.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>             //Wire library

#include <Sodaq_DS3231.h>     //RTC
#include "SPI.h"

#include "Adafruit_GFX.h"     //TFT (Adafruit) LCD library
#include "Adafruit_ILI9341.h"

#include "DHT.h"              //DHT library for sensors

#include "RF24.h"             //RF24 library for communication

#define DHTPIN 8              //DHT define
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

#define TFT_DC 9              //TFT (Adafruit) define
#define TFT_CS 10

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

float remoteHumidity = 0.0;             //floats for storing for display on the LCD
float remoteTemperature = 0.0;
float indoorHumidity = 0;
float indoorTemperature = 0;

String dateString;                      //date and time values for display on the LCD
String hours;
int minuteNow=0;
int minutePrevious=0;

float previousIndoorHumidity = 0;       //floats for storing the previous weather values
float previousIndoorTemperature = 10;
float previousRemoteHumidity = 0.1;
float previousRemoteTemperature = 0.1;

extern uint8_t SmallFont[];

//Serial Communication
const char starter = '<';               //opening and closing varibles
const char finisher = '>';
int preTemperature;                     //value for holding the non-converted weather values
int preHumidity;
String weatherString = String();

void setup() {                          //setup

  Serial.begin(9600);
  Serial1.begin(9600);

  tft.begin();                         //start LCD and set rotation
  tft.setRotation(3);

  rtc.begin();                         //start RTC (Real Time Clock)
  dht.begin();                         //start DHT (weather sensor library)
  
  delay(2000);                         //RTC
  setRTCTime();

  tft.fillScreen(TFT_BLACK);           //UI
  printUI();

  

}

void loop() {                          //loop for looping code

   wireConnection();                   //begin the processing of incoming serial values
   getAndPrintTime();                  //get and print time
   
   printIndoorTemperature();           //display weather values from the base station on the LCD
   printIndoorHumidity();

   printRemoteTemperature();           //display weather values from the remote device on the LCD
   printRemoteHumidity();

   remoteConversion();                 //convert the values from the incoming serial port to strings used for displaying the information
    
   
}

void wireConnection()                 //Serial Connection
{
  
  while (Serial1.available ())
    processInput ();  
}

void processNumber (const long n)    //process numbers
  {
  Serial.println (n);
  weatherString = n;
  }                                 
  
void processInput ()                //receiving numbers and processing
  {
  static long receivedNumber = 0;
  static boolean negative = false;
  
  byte c = Serial1.read ();
  
  switch (c)
    {
      
    case finisher:  
      if (negative) 
        processNumber (- receivedNumber); 
      else
        processNumber (receivedNumber); 

                                      // fall through to start a new number
    case starter: 
      receivedNumber = 0; 
      negative = false;
      break;
      
    case '0' ... '9': 
      receivedNumber *= 10;
      receivedNumber += c - '0';
      break;
      
    case '-':
      negative = true;
      break;
      
    }                                 // end of switch 


  }

void remoteConversion()               //conversion from weather string to floats for display
{

  preTemperature = ((int)(weatherString[0]-'0'))*1000+((int)(weatherString[1]-'0'))*100+((int)(weatherString[2]-'0'))*10+((int)(weatherString[3]-'0'));

  preHumidity = ((int)(weatherString[4]-'0'))*1000+((int)(weatherString[5]-'0'))*100+((int)(weatherString[6]-'0'))*10+((int)(weatherString[7]-'0'));

  remoteTemperature = Celcius2Fahrenheit(preTemperature / 100);
  remoteHumidity = preHumidity / 100;
}




                                            // 320 x 240 size of LCD

void printUI()
{
  
tft.drawRect(5,5,310,71,TFT_WHITE);         //Header Box
tft.drawRect(6,6,310,71,TFT_WHITE);

tft.drawRect(5,90,150,145,TFT_WHITE);       //Left Box
tft.drawRect(6,91,150,145,TFT_WHITE);

tft.drawRect(165,90,150,145,TFT_WHITE);     //Right Box
tft.drawRect(166,91,150,145,TFT_WHITE);


tft.fillRect(31,90,100,30, TFT_GREEN);      //Left Title Box

tft.fillRect(190,90,100,30, TFT_CYAN);      //Right Title Box

tft.setCursor(47,97);                       //Left Title
tft.setTextColor(TFT_BLACK);
tft.setTextSize(2);
tft.print("REMOTE");

tft.setCursor(205,97);                     //Right Title
tft.setTextColor(TFT_BLACK);
tft.setTextSize(2);
tft.print("INDOOR");

tft.setCursor(130,130);                    //Left %
tft.setTextColor(TFT_GREEN);
tft.setTextSize(3);
tft.print("%");

tft.setCursor(290,130);                    //Right %
tft.setTextColor(TFT_CYAN);
tft.setTextSize(3);
tft.print("%");

tft.setCursor(133,180);                    //Left "F"
tft.setTextColor(TFT_GREEN);
tft.setTextSize(3);
tft.print("F");

tft.setCursor(293,180);                    //Right "F"
tft.setTextColor(TFT_CYAN);
tft.setTextSize(3);
tft.print("F");

tft.setCursor(125,177);                    //Left "o"
tft.setTextColor(TFT_GREEN);
tft.setTextSize(1);
tft.print("o");

tft.setCursor(285,177);                    //Right "o"
tft.setTextColor(TFT_CYAN);
tft.setTextSize(1);
tft.print("o");

}

void getAndPrintTime()                    //RTC script
{
  
   delay(100);
   DateTime now = rtc.now();              //get the current date-time
   minuteNow = now.minute();
   if(minuteNow!=minutePrevious)
   {
      readSensor();
      dateString = getDayOfWeek(now.dayOfWeek())+" ";
      dateString = dateString+String(now.date())+"/"+String(now.month());
      dateString= dateString+"/"+ String(now.year()); 
      minutePrevious = minuteNow;
      hours = String(now.hour());
    if(now.minute()<10)
    {
      hours = hours+":0"+String(now.minute());
    }else
    {
      hours = hours+":"+String(now.minute());
    }
    printTime();
   }
}

void printTime()                           //printing of date and time
{
  String dateAndTime = dateString+" "+hours;
  
  tft.setTextSize(2);
  char charBuf[25];
  dateAndTime.toCharArray(charBuf, 25);

  tft.setCursor(35,35);                    //Date and Time location
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println(charBuf);
  //Serial.println(charBuf);
}

void setRTCTime()
{
  DateTime dt(2017, 8, 2, 11, 16, 20, 3);   // Year, Month, Day, Hour, Minutes, Seconds, Day of Week
  rtc.setDateTime(dt);                      //Adjust date-time as defined 'dt' above 
}

String getDayOfWeek(int i)
{
  switch(i)
  {
    case 1: return "Monday";break;
    case 2: return "Tuesday";break;
    case 3: return "Wednesday";break;
    case 4: return "Thursday";break;
    case 5: return "Friday";break;
    case 6: return "Saturday";break;
    case 7: return "Sunday";break;
    default: return "Monday";break;
  }
}

void readSensor()                   //base station weather values from sensor
{
  previousIndoorTemperature = indoorTemperature;
  previousIndoorHumidity = indoorHumidity;
  
  indoorHumidity = dht.readHumidity();
  indoorTemperature = Celcius2Fahrenheit(dht.readTemperature());
}

void printIndoorTemperature()       //printing local temperature
{
  String temperature;
  if(indoorTemperature != previousIndoorTemperature)
  {

    if(indoorTemperature>=100)
    {
      temperature = String(indoorTemperature,0);
    }else
    {
      temperature = String(indoorTemperature,1);
    }

    tft.fillRect(190,170,90,40, TFT_BLACK);
  
    tft.setCursor(210,180);
    tft.setTextColor(TFT_CYAN);
    tft.setTextSize(2);
    tft.print(temperature);

    previousIndoorTemperature = indoorTemperature;
  }
}

void printRemoteHumidity()            //printing remote humidity
{
  String humidity;
  if(remoteHumidity != previousRemoteHumidity)
  {
    if(remoteHumidity == 0) //We just booted up
    {
      
      humidity = "---";
      
    }else
    {
          humidity = String(remoteHumidity,1);
    }

    tft.fillRect(31,127,90,40,TFT_BLACK);
  
    tft.setCursor(61,137);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    tft.print(humidity);

    previousRemoteHumidity = remoteHumidity;
  }
}

void printRemoteTemperature()           //printing remote temperature
{
  String temperature;
  if(remoteTemperature != previousRemoteTemperature)
  {
    if(remoteTemperature == 0) //We just booted up
    {
      temperature = "---";
    }else if(remoteTemperature>=100)
    {
      temperature = String(remoteTemperature,0);
    }else
    {
        temperature = String(remoteTemperature,1);
    }
    
    tft.fillRect(31,170,90,40,TFT_BLACK);
  
    tft.setCursor(61,180);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    tft.print(temperature);

    previousRemoteTemperature = remoteTemperature;
  }
}

void printIndoorHumidity()              //printing local humidity
{
   if(indoorHumidity != previousIndoorHumidity)
  {

    String humidity = String(indoorHumidity,1);
   
    tft.fillRect(190,127,90,40,TFT_BLACK);
  
    tft.setCursor(210,137);
    tft.setTextColor(TFT_CYAN);
    tft.setTextSize(2);
    tft.print(humidity);

    previousIndoorHumidity = indoorHumidity; 
  }
}

float Celcius2Fahrenheit(float celsius)  //for conversion of celcius to fahrenheit
{
  return 1.8 * celsius + 32;
}



