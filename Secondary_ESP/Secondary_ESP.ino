#include <SoftwareSerial.h>
//Above should select ESPsoftwareserial when selecting ESP8266 boards
#include <ESP8266WiFi.h>  //I need wifi, despite I have removed these pieces from the code sample below
#include <ESP8266WiFiMulti.h>
#include <AccelStepper.h>


const int DIR = 9;    // SD2
const int STEP = 10;  // SD3

#define motorInterfaceType 1
AccelStepper stepper_motor(motorInterfaceType, STEP, DIR);


#define MAXLINELENGTH 255
char txt[MAXLINELENGTH];
#define SERIAL_RX 14      //GPIO14 = D5 pin for SoftwareSerial RX
#define SERIAL_TX 12      //GPIO12 = D6 pin for SoftwareSerial TX
SoftwareSerial mySerial;  //(SERIAL_RX, SERIAL_TX, false, MAXLINELENGTH); // (RX, TX, inverted, buffer)

// SYMBOLS FOR STARTING AND STOPPING AUTONOMOUS MODE
char startAChar = 'b';
char stopAChar = 'd';
char IRChar = 'i';


// IR SENSORS
int northPin = 0;   // D3 - Dark blue wire
int southPin = 5;   // D1 - GReen wire
int eastPin = 4;    // D2 - Red wire
int westPin = 16;    // D0 - Black Wire

// Direction Pin Values:
boolean northValue;
boolean southValue;
boolean eastValue;
boolean westValue;


// Direction Sensors Variables:
int north;
int south;
int east;
int west;



void setup() {
  Serial.begin(115200);

  stepper_motor.setMaxSpeed(1000); // Set maximum speed to 100 steps per second
  stepper_motor.setAcceleration(500); // Set acceleration to 50 steps per second per second
  stepper_motor.setSpeed(500); // Set current speed to 50 steps per second


  while (!Serial) {};
  //delay(2000);
  mySerial.begin(9600, SWSERIAL_8N1, SERIAL_RX, SERIAL_TX, false, MAXLINELENGTH);
  Serial.println("\nBegin 9600");


}


// Read Data from Chip:
void senseDirection(int sensorPin1, int sensorPin2, int sensorPin3, int sensorPin4) {
 northValue = digitalRead(sensorPin1);
 southValue = digitalRead(sensorPin2);
 eastValue = digitalRead(sensorPin3);
 westValue = digitalRead(sensorPin4);
}


// Detect Chip Strongest Direction:
int chooseDirection(boolean north, boolean south, boolean east, boolean west) {
 if (north == 0) {
  return 'N';
 }
 if (south == 0) {
  return 'S';
 }
 if (east == 0) {
  return 'E';
 }
 if (west == 0) {
   return 'W';
 } 
 else {
   return 'Z';
 }
}



void loop() {

  Serial.println("Reading the values now!");

  if (mySerial.available()){
    char output = mySerial.read();
    Serial.print("Output: ");
    Serial.println(output);

    // TO START THE AUTONOMOUS SYSTEM MODE
    if (output == startAChar){
      Serial.println("Moving the motor downwards!");
      delay(50);
      stepper_motor.move(- 360 / 1.8 * 4);
      while (stepper_motor.distanceToGo() != 0){
        stepper_motor.run();
      }
      mySerial.write("f");
    }


    // TO STOP THE AUTONOMOUS SYSTEM MODE
    if (output == stopAChar){
      char output = mySerial.read();
      Serial.println("Moving the motor upwards");
      delay(50);
      stepper_motor.move(360 / 1.8 * 4);
      while (stepper_motor.distanceToGo() != 0){
        stepper_motor.run();
      }
      mySerial.write("f");
    }

    // TO RETURN THE DIRECTION FROM THE IR SENSORS
    if (output == IRChar){
      senseDirection(northPin, southPin, eastPin, westPin);
      strongestSignal = chooseDirection(northValue, southValue, eastValue,westValue);
    }

    yield();

  }

}

