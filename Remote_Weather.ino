#include "DHT.h"
#include <SPI.h>  
#include "RF24.h"

#define DHTPIN 4  
#define DHTTYPE DHT22 

RF24 myRadio (7, 8);
byte addresses[][6] = {"Test1","Test2"};
const int led_pin = 13;

struct package
{
  float temperature ;
  float humidity ;
} data;

bool radioNumber=1;
bool role = 1;  //Control transmit 1/receive 0

DHT dht(DHTPIN, DHTTYPE);



void setup()
{
    Serial.begin(9600);
    pinMode(led_pin, OUTPUT);
    dht.begin();
    myRadio.begin();  
    //myRadio.setChannel(115); 
    myRadio.setPALevel(RF24_PA_LOW);
    //myRadio.setDataRate( RF24_250KBPS ) ; 
    myRadio.openWritingPipe(addresses[1]);
    myRadio.openReadingPipe(1, addresses[0]);
    delay(1000);
}



void loop()
{
  digitalWrite(led_pin, HIGH); // Flash a light to show transmitting
  readSensor();
  //Serial.println(data.humidity);
  //Serial.println(data.temperature);

  myRadio.stopListening();

  if(
    myRadio.write(&data, sizeof(data)),
    Serial.println(data.humidity) &
    Serial.println(data.temperature));

  
    
  digitalWrite(led_pin, LOW);
  delay(100);

  myRadio.startListening();
}

void readSensor()
{
 data.humidity = dht.readHumidity();
 data.temperature = dht.readTemperature();
}


