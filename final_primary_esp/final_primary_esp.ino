#include <Encoder.h>
#include <Servo.h>
#include <ESP8266WiFi.h>   // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> //Connect and publish to the MQTT broker
#include <ESP8266WiFiMulti.h>
#include <SoftwareSerial.h>


#define MAXLINELENGTH 255
char txt[MAXLINELENGTH];
#define SERIAL_RX 14      //GPIO14 = D5 pin for SoftwareSerial RX
#define SERIAL_TX 12      //GPIO12 = D6 pin for SoftwareSerial TX
SoftwareSerial mySerial;  //(SERIAL_RX, SERIAL_TX, false, MAXLINELENGTH); // (RX, TX, inverted, buffer)


//WiFi
const char* ssid = "IIIPA";  // my network's SSID
const char* wifi_password = "ME310Printer";   // the password of my network


//MQTT
const char* mqtt_server = "192.168.30.8";
const char* mqtt_username = "avatar"; 
const char* mqtt_password = "avatar";
const char* clientID = "microcontroller_client";

//defined the MQTT topic to which I need to subscribe and then later publish as well
const char* mqtt_voice_command_topic = "V";
const char* mqtt_UI_command_topic = "UI";


// Initialize the WiFi and MQTT Client objects
WiFiClient wifiClient;
// 1883 is the listener port for the broker
PubSubClient client(mqtt_server, 1883, wifiClient);


//custom function to connect to the MQTT broker via WiFi
void connect_MQTT(){
  Serial.println("Connecting to");
  Serial.println(ssid);

  // Connect to the WiFi
  WiFi.begin(ssid, wifi_password);

  // Wait until the connection has been confimed before continuing
  while (WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // Debugging - Output the IP Address of the ESP8266
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Connect to MQTT Broker
  // client.connect returns a Boolean value to let us know if the connection was successful
  if (client.connect(clientID, mqtt_username, mqtt_password)){
    Serial.print(clientID);
    Serial.println(" Connected to MQTT Broker");
  }
  else{
    Serial.println("Connection to MQTT Broker failed...");
  }
}


// DC Motor Configuations
int ENB = D7;   // D7 (13) // 2
int IN3 = D4;  // D4 (2) // 50
int IN4 = D2;  // D2 (4) // 51
int encoderPinA = 16;   // Yellow  D0 // 4 (Arduino Mega)
int encoderPinB = 0;   // Green    D3 // 5 (Arduino Mega)

long encoder_value = 0;
long encoder_counts_per_rev = 2443;
int wheel_rev = 2;
int wheel_rev_turn = 1;
int encoder_subtraction_value = 750;     // as i just want to move the walker by 50 cms

Encoder encoder(encoderPinA, encoderPinB);
long encoderValue;
unsigned long abs_encoderValue;


// ULTRASONIC SENSORS
const int triggerPin_sensor_1 = 10;  // SD3
const int echoPin_sensor_1 = D1;
// const int triggerPin_sensor_2 = 
// const int echoPin_sensor_2 = 

// to define the variables
long duration_sensor_1;
int distance_sensor_1;
// long duration_sensor_2;
// int distance_sensor_2;


//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701


// SERVO MOTOR
Servo servo_motor;  // Create a servo object

// FLAGS
bool recvd_mqtt_voice_cmd = false;
bool recvd_mqtt_UI_cmd = false;


// GLOBAL VARIABLES FOR DC AND SERVO MOTORS
#define DC_LEFT 0
#define DC_RIGHT 1
#define DC_STRAIGHT 2
#define DC_REVERSE 3
#define DC_STOP 4
#define SERVO_LEFT 5
#define SERVO_RIGHT 6
#define SERVO_RESET 7


// DEFINE INTERRUPT FUNCTION
// void ICACHE_RAM_ATTR DCMotorISR(){
//   encoderValue = encoder.read();
//   abs_encoderValue = abs(encoderValue);      // taking the absolute value as I am not concerned about the direction of rotation and I am using this function for forward and reverse motion
// }


void setup() {
  Serial.begin(115200);
  // Serial.println("Started robot!");

  // CONNECT TO MQTT
  connect_MQTT();   // connect to the MQTT broker
  Serial.setTimeout(2000);
  client.setCallback(callback);

  // DC MOTOR
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  // ULTRASONIC SENSORS
  pinMode(triggerPin_sensor_1, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin_sensor_1, INPUT); // Sets the echoPin as an Input
  // pinMode(triggerPin_sensor_2, OUTPUT); // Sets the trigPin as an Output
  // pinMode(echoPin_sensor_2, INPUT); // Sets the echoPin as an Input

  // SERVO MOTOR
  servo_motor.attach(D8);

  // ATTACHING INTERRUPT
  //attachInterrupt(encoderPinA, DCMotorISR, CHANGE);  // to enable interrupt on encoder PinA

  // SUBSCRIBE TO TOPICS
  if (client.connected()){
    client.subscribe(mqtt_voice_command_topic);
    client.subscribe(mqtt_UI_command_topic);
    Serial.println("Have subscribed to the topics!!");
    delay(100);
  }

  // SERIAL COMMUNICATION
  while (!Serial) {};
  //delay(2000);
  mySerial.begin(9600, SWSERIAL_8N1, SERIAL_RX, SERIAL_TX, false, MAXLINELENGTH);
  Serial.println("\nBegin 9600");
}


void stop_motor(){
  // Serial.println("Inside stop motor function...");
  analogWrite(ENB, 0);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  delay(1000);
}



// RETURN BOOLEAN VALUE BASED ON DISTANCES RETURNED BY THE SENSORS - 60 CMS IS THE THRESHOLD
bool check_ultrasonic_sensors(){

  // FOR ULTRASONIC SENSOR 1
  digitalWrite(triggerPin_sensor_1, LOW); //set trigger signal low for 2us
  delayMicroseconds(2);
  
  /*send 10 microsecond pulse to trigger pin of HC-SR04 */
  digitalWrite(triggerPin_sensor_1, HIGH);  // make trigger pin active high
  delayMicroseconds(10);            // wait for 10 microseconds
  digitalWrite(triggerPin_sensor_1, LOW);   // make trigger pin active low
  
  /*Measure the Echo output signal duration or pulss width */
  duration_sensor_1 = pulseIn(echoPin_sensor_1, HIGH); // save time duration value in "duration variable
  distance_sensor_1= duration_sensor_1*0.034/2; //Convert pulse duration into distance

  // Serial.print("Distance from ultrasonic sensor 1 is: ");
  // Serial.println(distance_sensor_1);

  
  // // FOR ULTRASONIC SENSOR 2
  // digitalWrite(triggerPin_sensor_2, LOW); 
  // delayMicroseconds(2);
  
  // /*send 10 microsecond pulse to trigger pin of HC-SR04 */
  // digitalWrite(triggerPin_sensor_2, HIGH);  
  // delayMicroseconds(10);            
  // digitalWrite(triggerPin_sensor_2, LOW);   
  
  // /*Measure the Echo output signal duration or pulss width */
  // duration_sensor_2 = pulseIn(echoPin_sensor_2, HIGH); 
  // distance_sensor_2 = duration_sensor_2*0.034/2; 

  // Serial.print("Distance from ultrasonic sensor 2 is: ");
  // Serial.println(distance_sensor_2);

  // RETURN a bool value of True if the distances returned by the sensors is greater than 50 cms
  if (distance_sensor_1 > 60){
    return true;
  }
  else{
    return false;
  }
}



// MOVING THE SERVO MOTOR
void move_servo_motor(int SERVO_motion_type){
  if (SERVO_motion_type == 5){
    servo_motor.write(180);       // LEFT
  }
  if (SERVO_motion_type == 6){    // RIGHT
    servo_motor.write(20);
  }
  if (SERVO_motion_type == 7){    // RESET
    servo_motor.write(115);
  }
}



// STRAIGHT FUNCTION
void move_straight(){

  Serial.println("Inside the move forward function!!");
  analogWrite(ENB, 225);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  move_servo_motor(SERVO_RESET);

  //long target_encoder_counts = wheel_rev * encoder_counts_per_rev;    // 4886 = 2 * 2443; (75 encoder steps = walker moving 1cm forward)
  //target_encoder_counts = target_encoder_counts - encoder_subtraction_value;   // 4136 = 4886 - 75
  long target_encoder_counts = 200000;
  encoder.write(0);     // to reset the encoder

  int counter = 0; 
  long next_target = 75;

  // using interrupts to monitor the encoder steps and exit the while loop once the encoder value exceeds the target encoder value
  while(encoder.read() < target_encoder_counts){

    analogWrite(ENB, 225);
    counter++;
    // Serial.print("Encoder Value: ");
    // Serial.println(encoder.read());

    if (counter >= 5){        // to scan the ultrasonic sensors after every 75 encoder steps i.e. after every 1 cm that the walker has moved
      counter = 0;             // reset the counter value as I have to scan the sensors again after 75 encoder steps i.e. after 1 cm
      if (!check_ultrasonic_sensors()){
        stop_motor();         // stop the motors if the ultrasonic sensor def return False
      }
      else{  // else continue moving forward
      }
    }
  }
  stop_motor();
  delay(500);
  move_servo_motor(SERVO_RESET);
}


// REVERSE FUNCTION
void move_reverse(){

  Serial.println("Inside the move forward function!!");
  analogWrite(ENB, 250);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  move_servo_motor(SERVO_RESET);

  //long target_encoder_counts = wheel_rev * encoder_counts_per_rev;    // 4886 = 2 * 2443; (75 encoder steps = walker moving 1cm forward)
  //target_encoder_counts = target_encoder_counts - encoder_subtraction_value;   // 4136 = 4886 - 75
  long target_encoder_counts = 200000;
  encoder.write(0);     // to reset the encoder

  int counter = 0; 
  long next_target = 75;

  // using interrupts to monitor the encoder steps and exit the while loop once the encoder value exceeds the target encoder value
  while(encoder.read() < target_encoder_counts){

    analogWrite(ENB, 250);
    counter++;
    // Serial.print("Encoder Value: ");
    // Serial.println(encoder.read());
  }
  stop_motor();
  delay(500);
  move_servo_motor(SERVO_RESET);
}


// LEFT FUNCTION
void move_left(){

  Serial.println("Inside the move left function!");
  
  move_servo_motor(SERVO_RESET);
  delay(300);
  move_servo_motor(SERVO_LEFT);
  delay(1000);

  // Setting the directions for the DC motor
  analogWrite(ENB, 225);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  //long target_encoder_counts = wheel_rev_turn * encoder_counts_per_rev;      // 2443 = 1 * 2443  // to just move ahead by 30 cms
  //long target_encoder_counts = 50000;
  long target_encoder_counts = 2443;
  encoder.write(0);     // to reset the encoders

  int counter = 0;

  // using interrupts to monitor the encoder steps and exit the while loop once the encoder value exceeds the target encoder value
  while (encoder.read() < target_encoder_counts){     
    analogWrite(ENB, 225);
    // Serial.print("Encoder Value: ");
    // Serial.println(encoder.read());
    counter++;

    if (counter >= 5){        // to scan the ultrasonic sensors after every 75 encoder steps i.e. after every 1 cm that the walker has moved
      counter = 0;             // reset the counter value as I have to scan the sensors again after 75 encoder steps i.e. after 1 cm
      if (!check_ultrasonic_sensors()){
        stop_motor();         // stop the motors if the ultrasonic sensor def return False
      }
      else{  // else continue moving forward
      }
    }
  }
  stop_motor();
  delay(500);
  move_servo_motor(SERVO_RESET);
  // abs_encoderValue = 0;
}


// RIGHT FUNCTION
void move_right(){

  Serial.println("Inside the move left function!");
  
  move_servo_motor(SERVO_RESET);
  delay(300);
  move_servo_motor(SERVO_RIGHT);
  delay(1000);

  // Setting the directions for the DC motor
  analogWrite(ENB, 225);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  //long target_encoder_counts = wheel_rev_turn * encoder_counts_per_rev;      // 2443 = 1 * 2443  // to just move ahead by 30 cms
  //long target_encoder_counts = 50000;
  long target_encoder_counts = 2443;
  encoder.write(0);     // to reset the encoders

  int counter = 0;

  // using interrupts to monitor the encoder steps and exit the while loop once the encoder value exceeds the target encoder value
  while (encoder.read() < target_encoder_counts){     
    analogWrite(ENB, 225);
    Serial.print("Encoder Value: ");
    Serial.println(encoder.read());
    counter++;

    if (counter >= 5){        // to scan the ultrasonic sensors after every 75 encoder steps i.e. after every 1 cm that the walker has moved
      counter = 0;             // reset the counter value as I have to scan the sensors again after 75 encoder steps i.e. after 1 cm
      if (!check_ultrasonic_sensors()){
        stop_motor();         // stop the motors if the ultrasonic sensor def return False
      }
      else{  // else continue moving forward
      }
    }
  }
  stop_motor();
  delay(500);
  move_servo_motor(SERVO_RESET);
  // abs_encoderValue = 0;
}



// CD FUNCTION THE LAST PATCH (50 CMS) WITHOUT TRIGGERING THE ULTRASONIC SENSORS
void move_straight_CD(){

  Serial.println("Inside the move forward function!!");
  analogWrite(ENB, 225);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  move_servo_motor(SERVO_RESET);

  long target_encoder_counts = 4886;
  encoder.write(0);     // to reset the encoder

  int counter = 0; 
  long next_target = 75;

  // using interrupts to monitor the encoder steps and exit the while loop once the encoder value exceeds the target encoder value
  while(encoder.read() < target_encoder_counts){

    analogWrite(ENB, 225);
    counter++;

  }
  stop_motor();
  delay(500);
  move_servo_motor(SERVO_RESET);

}






// FUNCTION WHICH LISTENS TO MQTT MESSAGES
void callback(char* topic, byte* payload, unsigned int length) {
  
  // to ensure that the user is not sending voice and UI commands simulataneously and just execute the one that is received first
  String topicString = String(topic);

  if (topicString == mqtt_voice_command_topic){
    recvd_mqtt_voice_cmd = true;
    recvd_mqtt_UI_cmd = false;
    Serial.println("Message received via VOICE COMMAND!!");
  }
  if (topicString == mqtt_UI_command_topic){
    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = true;
    Serial.println("Message received via UI COMMAND");
  }
  
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);

  // Convert payload to a string
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Payload: ");
  Serial.println(message);


  // GO Straight
  if ((strcmp(message.c_str(), "straight") == 0 && recvd_mqtt_voice_cmd) || (strcmp(message.c_str(), "straight") == 0 && recvd_mqtt_UI_cmd)){     // to start turning the wheels to the left if any other MQTT or UI commands are not being executed

    Serial.println("Going Straight!");
    //delay(500);

    // moving the motors according to the specified command
    move_straight();       // assuming that clockwise motion moves the walker forward
    client.publish(mqtt_voice_command_topic, "Have executed the Straight command!");

    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }


  // GO LEFT
  if ((strcmp(message.c_str(), "left") == 0 && recvd_mqtt_voice_cmd) || (strcmp(message.c_str(), "left") == 0 && recvd_mqtt_UI_cmd)){     // to start turning the wheels to the left if any other MQTT or UI commands are not being executed

    Serial.println("Going Left!");
    //delay(500);

    // moving the motors according to the specified command
    move_left();       // assuming that clockwise motion moves the walker forward
    client.publish(mqtt_voice_command_topic, "Have executed the Left command!");

    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }


  // GO RIGHT
  if ((strcmp(message.c_str(), "right") == 0 && recvd_mqtt_voice_cmd) || (strcmp(message.c_str(), "right") == 0 && recvd_mqtt_UI_cmd)){     // to start turning the wheels to the left if any other MQTT or UI commands are not being executed

    Serial.println("Going Right!");
    //delay(500);

    // moving the motors according to the specified command
    move_right();       // assuming that clockwise motion moves the walker forward
    client.publish(mqtt_voice_command_topic, "Have executed the Right command!");

    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }


  // GO REVERSE
  if ((strcmp(message.c_str(), "reverse") == 0 && recvd_mqtt_voice_cmd) || (strcmp(message.c_str(), "reverse") == 0 && recvd_mqtt_UI_cmd)){     // to start turning the wheels to the left if any other MQTT or UI commands are not being executed

    Serial.println("Going Reverse!");
    //delay(500);

    // moving the motors according to the specified command
    move_reverse();       // assuming that clockwise motion moves the walker forward
    client.publish(mqtt_voice_command_topic, "Have executed the Reverse command!");

    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }


  // START AUTONOMOUS MODE
  if ((strcmp(message.c_str(), "startA") == 0 && recvd_mqtt_voice_cmd) || (strcmp(message.c_str(), "startA") == 0 && recvd_mqtt_UI_cmd)){
    Serial.println("Starting the autonomous system mode!!");

    delay(500);
    mySerial.write("b");

    while (mySerial.available()){
      char c = mySerial.read();
      Serial.println("Processed finished!");
      yield();
    }

    client.publish(mqtt_voice_command_topic, "Have executed the START autonomous system command!");

    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }


  // STOP AUTONOMOUS MODE
  if ((strcmp(message.c_str(), "stopA") == 0 && recvd_mqtt_voice_cmd) || (strcmp(message.c_str(), "stopA") == 0 && recvd_mqtt_UI_cmd)){
    Serial.println("Starting the autonomous system mode!!");

    delay(500);
    mySerial.write("d");

    while (mySerial.available()){
      char c = mySerial.read();
      Serial.println("Processed finished!");
      yield();
    }

    client.publish(mqtt_voice_command_topic, "Have executed the STOP autonomous system command!");

    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }



  // GO CHARGING DOCK // Last minute hard coding the values
  if ((strcmp(message.c_str(), "CD") == 0 && recvd_mqtt_voice_cmd) || (strcmp(message.c_str(), "CD") == 0 && recvd_mqtt_UI_cmd)){     // to start turning the wheels to the left if any other MQTT or UI commands are not being executed

    Serial.println("Going to CD!");
    //delay(500);

    // to poll the IR sensor
    mySerial.write("i");

    while (mySerial.available()){
      char receivedChar = mySerial.read();

      if (receivedChar == 'N'){
        move_straight();
      }

      if (receivedChar = 'S'){
        move_reverse();
      }

      if (receivedChar = 'W'){
        move_left();
      }

      if (receivedChar = 'E'){
        move_right();
      }

      Serial.println("Processed finished!");
      yield();
    }


    // move_left();
    // delay(100);
    // move_straight();
    // delay(100);
    // move_straight();
    // delay(100);
    // move_straight_CD();

    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }




}





void loop() {
  client.loop();
  stop_motor();
}

