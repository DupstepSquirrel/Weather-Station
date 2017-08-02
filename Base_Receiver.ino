
/*  Receiver Code
    Code to receive data from RF24L01+ */


#include <SPI.h>
#include <RF24.h>
#include <SoftwareSerial.h>



//Radio Configuration

RF24 myradio(7, 8);
byte addresses[][6] = {"Test1", "Test2"};  //transmitter adresses

bool radioNumber = 0;
bool role = 0;  //Control transmit/receive


struct package
{
  float temperature ;
  float humidity ;
} data;

//Serial Communication

SoftwareSerial mySerial(3, 4);

int temperature;
int humidity;
const char starter = '<';
const char finisher = '>';


void setup() {
  Serial.begin(9600);
  while (!Serial);
  mySerial.begin(9600);
  
  myradio.begin();                              // Initializes radio
  myradio.setPALevel(RF24_PA_LOW);              // Sets power output to low.
  myradio.openWritingPipe(addresses[0]);        //opens writing pipeline
  myradio.openReadingPipe(1, addresses[1]);     //opens reading pipeline
  myradio.setPayloadSize(32);                   //sets payload size, but this is default
  myradio.startListening();


}
void loop()
{
  //delay(150);                                 //increase for debuggy, decrease to decrease jitter
  
  if (mySerial.available()) {
    Serial.println(mySerial.read());
  }


  if (myradio.available()) {
    myradio.read(&data, sizeof(data));
    //Serial.println(data.humidity);            //prints values from struct
    //Serial.println(data.temperature);         //reads value of go

  } else {
  }

  temperature = data.temperature * 100;
  humidity = data.humidity * 100;
  mySerial.print(starter);
  mySerial.print(temperature);
  mySerial.print(humidity);
  mySerial.print(finisher);
  mySerial.println();

  //delay(100);

}


