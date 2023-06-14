//Code for the ESP8266
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>   // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> //Connect and publish to the MQTT broker
#include <Servo.h>
#include <Encoder.h>
#include <ESP8266WiFiMulti.h>


//WiFi
const char* ssid = "IIIPA";  // my network's SSID
const char* wifi_password = "ME310Printer";   // the password of my network


//MQTT
const char* mqtt_server = "192.168.30.3";
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



// GLOBAL VARIABLES FOR DC AND SERVO MOTORS
#define DC_LEFT 0
#define DC_RIGHT 1
#define DC_STRAIGHT 2
#define DC_REVERSE 3
#define DC_STOP 4
#define SERVO_LEFT 5
#define SERVO_RIGHT 6
#define SERVO_RESET 7
#define DC_CD 8


// ULTRASONIC SENSORS
#define triggerPin_sensor_1 12   // D6
#define echoPin_sensor_1 14      // D5
#define triggerPin_sensor_2 4   // D2
#define echoPin_sensor_2 5   // D1

//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

// to define the variables
long duration_sensor_1;
int distance_sensor_1;
long duration_sensor_2;
int distance_sensor_2;


// DC MOTOR
const int ENA = D7;
const int IN1 = D1;
const int IN2 = D2;
const int encoderPinA = D5;
const int encoderPinB = D6;

long encoder_value = 0;
long encoder_counts_per_rev = 2443;
const int wheel_rev = 2;
const int wheel_rev_turn = 1;
const int CD_wheel_rev = 5;
const int encoder_subtraction_value = 760;     // as i just want to move the walker by 50 cms
const int IR_straight_motion = 4136;
const int IR_turn_motion = 2443;

Encoder encoder(encoderPinA, encoderPinB);
long encoderValue;
unsigned long abs_encoderValue = 0;


// SERVO MOTOR
Servo servo_motor;     // creating a servo motor object
int servo_pin = 2;    // D4


// SETTING FLAGS
bool recvd_mqtt_voice_cmd = false;     // flag: when true indicates that a received voice cmd is being executed
bool recvd_mqtt_UI_cmd = false;        // flag: when true indicates that a received UI cmd is being executed


// Interrupt Service Function for the DC Motor
void ICACHE_RAM_ATTR DCMotorISR(){
  encoderValue = encoder.read();
  abs_encoderValue = abs(encoderValue);      // taking the absolute value as I am not concerned about the direction of rotation and I am using this function for forward and reverse motion
}


// FOR SOFTWARE SERIAL COMMUNICATION
#define MAXLINELENGTH 255
char txt[MAXLINELENGTH];
#define SERIAL_RX 14      //GPIO14 = D5 pin for SoftwareSerial RX
#define SERIAL_TX 12      //GPIO12 = D6 pin for SoftwareSerial TX
SoftwareSerial mySerial;  //(SERIAL_RX, SERIAL_TX, false, MAXLINELENGTH); // (RX, TX, inverted, buffer)



// TO setup all the sensors
void setup() {
  Serial.begin(115200); // Starts the serial communication
  Serial.println("In the void setup block");
  
  connect_MQTT();   // connect to the MQTT broker
  
  Serial.setTimeout(2000);
  client.setCallback(callback);

  //  To begin Serial Communication
  while (!Serial) {};
    //delay(2000);
    mySerial.begin(9600, SWSERIAL_8N1, SERIAL_RX, SERIAL_TX, false, MAXLINELENGTH);
    Serial.println("\nBegin 9600");
  
  // ULTRASONIC SENSORS
  pinMode(triggerPin_sensor_1, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin_sensor_1, INPUT); // Sets the echoPin as an Input
  pinMode(triggerPin_sensor_2, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin_sensor_2, INPUT); // Sets the echoPin as an Input


  // SERVO MOTOR
  servo_motor.attach(servo_pin);      // attaching servo motor to D4

  // DC MOTOR
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT); 
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  attachInterrupt(encoderPinA, DCMotorISR, CHANGE);  // to enable interrupt on encoder PinA

  // SUBSCRIBE TO TOPICS
  if (client.connected()){
    client.subscribe(mqtt_voice_command_topic);
    client.subscribe(mqtt_UI_command_topic);
    Serial.println("Have subscribed to the topics!!");
    delay(100);
  }
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

  Serial.print("Distance from ultrasonic sensor 1 is: ");
  Serial.println(distance_sensor_1);

  
  // FOR ULTRASONIC SENSOR 2
  digitalWrite(triggerPin_sensor_2, LOW); 
  delayMicroseconds(2);
  
  /*send 10 microsecond pulse to trigger pin of HC-SR04 */
  digitalWrite(triggerPin_sensor_2, HIGH);  
  delayMicroseconds(10);            
  digitalWrite(triggerPin_sensor_2, LOW);   
  
  /*Measure the Echo output signal duration or pulss width */
  duration_sensor_2 = pulseIn(echoPin_sensor_2, HIGH); 
  distance_sensor_2 = duration_sensor_2*0.034/2; 

  Serial.print("Distance from ultrasonic sensor 2 is: ");
  Serial.println(distance_sensor_2);

  // RETURN a bool value of True if the distances returned by the sensors is greater than 50 cms
  if (distance_sensor_1 > 60 && distance_sensor_2 > 60){
    return true;
  }
  else{
    return false;
  }
}



// STOPPING THE DC MOTORS
void stop_DC_motor(){
  analogWrite(ENA, 0);       // setting the PWM pin to 0% Duty cycle    
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  delay(1);
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



// give the straight command to move 50 cms in the clockwise direction
void STRAIGHT_MOTION(){
  
  analogWrite(ENA, 100);   // to rotate the DC motor at 65 rpm
  digitalWrite(IN1, HIGH);  // this is the clockwise direction
  digitalWrite(IN2, LOW);

  int current_encoder_value = encoder.read();

  if (current_encoder_value == 0){
    long target_encoder_counts = wheel_rev * encoder_counts_per_rev;    // 4886 = 2 * 2443; (75 encoder steps = walker moving 1cm forward)
    target_encoder_counts = target_encoder_counts - encoder_subtraction_value;                  // to scan the ultrasonic distances for 60 cms but just moving forward for 50 cms
    long next_target = 75;    // to scan ultrasonic sensors after 75 encoder steps i.e. after every 1 cm
  }
  else (){
    long target_encoder_counts = wheel_rev * encoder_counts_per_rev;    
    target_encoder_counts = target_encoder_counts - encoder_subtraction_value + abs_encoderValue;                  
    long next_target = 75 + abs_encoderValue;    // to add the current encoderValue
  }



  // using interrupts to monitor the encoder steps and exit the while loop once the encoder value exceeds the target encoder value
  while (abs_encoderValue < target_encoder_counts){     
    analogWrite(ENA, 100);

    if (abs_encoderValue >= next_target){      // to use encoder values i.e. to scan ultrasonic sensors on the amount of distance moved instead of time
      next_target += 75;
      if (!check_ultrasonic_sensors()){
        stop_DC_motor();
      }
      else{
      }
    }
  }
  stop_DC_motor();
  move_servo_motor(SERVO_RESET);
}



// move the servo motors first and then the DC for 30 cms
void LEFT_MOTION(){

  move_servo_motor(SERVO_LEFT);

  // actuating the DC motor
  analogWrite(ENA, 100);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  long target_encoder_counts = wheel_rev_turn * encoder_counts_per_rev;      // 2443 = 1 * 2443  // to just move ahead by 30 cms
  int counter = 0;

  // using interrupts to monitor the encoder steps and exit the while loop once the encoder value exceeds the target encoder value
  while (abs_encoderValue < target_encoder_counts){     
    analogWrite(ENA, 100);
    counter++;

    if (counter >= 75){        // to scan the ultrasonic sensors after every 75 encoder steps i.e. after every 1 cm that the walker has moved
      counter = 0;             // reset the counter value as I have to scan the sensors again after 75 encoder steps i.e. after 1 cm
      if (!check_ultrasonic_sensors()){
        stop_DC_motor();         // stop the motors if the ultrasonic sensor def return False
      }
      else{  // else continue moving forward
      }
    }
  }
  stop_DC_motor();
  move_servo_motor(SERVO_RESET);
}



// move the sevro motor first and then the DC for 30 cms
void RIGHT_MOTION(){

  move_servo_motor(SERVO_RIGHT);

  // actuating the DC motor
  analogWrite(ENA, 100);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  long target_encoder_counts = wheel_rev_turn * encoder_counts_per_rev;      // 2443 = 1 * 2443
  int counter = 0;

  // using interrupts to monitor the encoder steps and exit the while loop once the encoder value exceeds the target encoder value
  while (abs_encoderValue < target_encoder_counts){     
    analogWrite(ENA, 100);
    counter++;

    if (counter >= 75){        // to scan the ultrasonic sensors after every 75 encoder steps i.e. after every 1 cm that the walker has moved
      counter = 0;             // reset the counter value as I have to scan the sensors again after 75 encoder steps i.e. after 1 cm
      if (!check_ultrasonic_sensors()){
        stop_DC_motor();         // stop the motors if the ultrasonic sensor def return False
      }
      else{  // else continue moving forward
      }
    }
  }
  stop_DC_motor();
  move_servo_motor(SERVO_RESET);  
}



// give the reverse command to move 50 cms in the anticlockwise direction
void REVERSE_MOTION(){

  analogWrite(ENA, 100);    // rotate the DC motor at 65 rpm
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  long target_encoder_counts = wheel_rev * encoder_counts_per_rev;    // 4886 = 2 * 2443
  target_encoder_counts = target_encoder_counts - encoder_subtraction_value;

  while (abs_encoderValue < target_encoder_counts){
    analogWrite(ENA, 100);
  }
  stop_DC_motor();
  move_servo_motor(SERVO_RESET);
}




// MOVING THE DC MOTOR
void move_DC_motor(int DC_motion_type){

  if (DC_motion_type == DC_STRAIGHT){   // STRAIGHT MOTION
    encoder.write(0);
    STRAIGHT_MOTION();
  }

  if (DC_motion_type == DC_REVERSE){   // REVERSE MOTION
    encoder.write(0);
    REVERSE_MOTION();
  }

  if (DC_motion_type == DC_LEFT){     // LEFT MOTION
    encoder.write(0);
    LEFT_MOTION();
  }

  if (DC_motion_type == DC_RIGHT){    // RIGHT MOTION
    encoder.write(0);
    RIGHT_MOTION();
  }

  if (DC_motion_type == DC_STOP){    // STOPPING THE DC MOTOR
    encoder.write(0);
    stop_DC_motor();
  }
  

  if (DC_motion_type == DC_CD){     // CHARGING DOCK

    long target_encoder_counts = CD_wheel_rev * encoder_counts_per_rev;    // 12,215 = 5 * 2443; (75 encoder steps = walker moving 1cm forward)
    encoder.write(0);     // to reset the encoder
    
    while (abs_encoderValue < target_encoder_counts){
      
      mySerial.write('i');     // to check the output from the IR sensor

      while (true){
        if (mySerial.availble()){
          char receivedChar = mySerial.read();    // taking the feedback from the IR sensor

          if (receivedChar == 'N'){
            STRAIGHT_MOTION();
          }
          if (receivedChar == 'S'){
            // that means I need to go reverse
          }
          if (receivedChar == 'E'){
            // move right
          }
          if (receivedChar == 'W'){
            // move left
          }
          if (receivedChar == 'Z'){
            // did not get an input from the IR sensor
          }
        }
      } 
    }
  }
}



// FUNCTION WHICH LISTENS TO MQTT MESSAGES
void callback(char* topic, byte* payload, unsigned int length) {
  
  // to ensure that the user is not sending voice and UI commands simulataneously and just execute the one that is received first
  String topicString = String(topic);

  if (topicString == mqtt_voice_command_topic){
    recvd_mqtt_voice_cmd = true;
    recvd_mqtt_UI_cmd = false;
  }
  if (topicString == mqtt_voice_command_topic){
    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = true;
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
  if ((strcmp(message.c_str(), "straight") == 0 && recvd_mqtt_voice_cmd && check_ultrasonic_sensors()) || (strcmp(message.c_str(), "straight") == 0 && recvd_mqtt_UI_cmd && check_ultrasonic_sensors())){     // to start turning the wheels to the left if any other MQTT or UI commands are not being executed

    Serial.println("Going Straight!");

    // moving the motors according to the specified command
    move_DC_motor(DC_STRAIGHT);       // assuming that clockwise motion moves the walker forward
    client.publish(mqtt_voice_command_topic, "Have executed the Left command!");

    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }
  else {
    client.publish(mqtt_voice_command_topic, "Waiting for 3 seconds to check if the object in front of me moves away!");
    delay(3000);

    if (check_ultrasonic_sensors()){

      Serial.println("Going Straight!");

      // moving the motors according to the specified command
      move_DC_motor(DC_STRAIGHT);       // assuming that clockwise motion moves the walker forward
      client.publish(mqtt_voice_command_topic, "Have executed the Straight command!");

      recvd_mqtt_voice_cmd = false;
      recvd_mqtt_UI_cmd = false;
    }
    else{
      client.publish(mqtt_voice_command_topic, "Object is still in front of me and so I cannot go straight! Please try something else!");
    }
  }


  // START THE AUTONOMOUS SYSTEM MODE
  if ((strcmp(message.c_str(), "StartA") == 0 && recvd_mqtt_voice_cmd && check_ultrasonic_sensors()) || (strcmp(message.c_str(), "StartA") == 0 && recvd_mqtt_UI_cmd && check_ultrasonic_sensors())){       // to execute this command only if any other MQTT or UI commands are not being executed

    Serial.println("Starting the autonomous system mode!");

    mySerial.write('b');
    while (true){
      if (mySerial.availble()){
        char receivedChar = mySerial.read();
        if (receivedChar == 'f'){
          break;
        }
      }
    }

    client.publish(mqtt_voice_command_topic, "Have activated the autonomous system mode!");
    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }


  // STOP THE AUTONOMOUS SYSTEM MODE  
  if ((strcmp(message.c_str(), "StopA") == 0 && recvd_mqtt_voice_cmd && check_ultrasonic_sensors()) || (strcmp(message.c_str(), "StopA") == 0 && recvd_mqtt_UI_cmd && check_ultrasonic_sensors())){       // to execute this command only if any other MQTT or UI commands are not being executed

    Serial.println("Stopping the autonomous system mode!");

    mySerial.write('p');
    while (true){
      if (mySerial.availble()){
        char receivedChar = mySerial.read();
        if (receivedChar == 'd'){
          break;
        }
      }
    }

    client.publish(mqtt_voice_command_topic, "Have deactivated the autonomous system mode!");
    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }


  // STOP THE WALKER FROM MOVING
  if ((strcmp(message.c_str(), "SM") == 0 && recvd_mqtt_voice_cmd && check_ultrasonic_sensors()) || (strcmp(message.c_str(), "SM") == 0 && recvd_mqtt_UI_cmd && check_ultrasonic_sensors())){     // to execute this command only if any other MQTT or UI commands are not being executed

    Serial.println("Stopping now!");
    move_DC_motor(DC_STOP);
    client.publish(mqtt_voice_command_topic, "Have stopped moving!");

    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }


  // GO LEFT
  if ((strcmp(message.c_str(), "left") == 0 && recvd_mqtt_voice_cmd && check_ultrasonic_sensors()) || (strcmp(message.c_str(), "left") == 0 && recvd_mqtt_UI_cmd && check_ultrasonic_sensors())){     // to start turning the wheels to the left if any other MQTT or UI commands are not being executed

    Serial.println("Going Left!");
    move_DC_motor(DC_LEFT);
    client.publish(mqtt_voice_command_topic, "Have executed the Left command!");

    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }
  else {
    client.publish(mqtt_voice_command_topic, "Waiting for 3 seconds to check if the object in front of me moves away!");
    delay(3000);

    if (check_ultrasonic_sensors()){

      Serial.println("Going Left!");
      move_DC_motor(DC_LEFT);
      client.publish(mqtt_voice_command_topic, "Have executed the Left command!");

      recvd_mqtt_voice_cmd = false;
      recvd_mqtt_UI_cmd = false;
    }
    else{
      client.publish(mqtt_voice_command_topic, "Object is still in front of me and so I cannot turn left!");
    }
  }


  // GO RIGHT
  if ((strcmp(message.c_str(), "right") == 0 && recvd_mqtt_voice_cmd && check_ultrasonic_sensors()) || (strcmp(message.c_str(), "right") == 0 && recvd_mqtt_UI_cmd && check_ultrasonic_sensors())){     // to start turning the wheels to the left if any other MQTT or UI commands are not being executed

    Serial.println("Going Right!");
    move_DC_motor(DC_RIGHT);
    client.publish(mqtt_voice_command_topic, "Have executed the Right command!");

    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }
  else {
    client.publish(mqtt_voice_command_topic, "Waiting for 3 seconds to check if the object in front of me moves away!");
    delay(3000);

    if (check_ultrasonic_sensors()){

      Serial.println("Going Right!");
      move_DC_motor(DC_RIGHT);
      client.publish(mqtt_voice_command_topic, "Have executed the Left command!");

      recvd_mqtt_voice_cmd = false;
      recvd_mqtt_UI_cmd = false;
    }
    else{
      client.publish(mqtt_voice_command_topic, "Object is still in front of me and so I cannot turn Right!");
    }
  }



  // GO REVERSE
  if ((strcmp(message.c_str(), "reverse") == 0 && recvd_mqtt_voice_cmd) || (strcmp(message.c_str(), "reverse") == 0 && recvd_mqtt_UI_cmd)){     // to start turning the wheels to the left if any other MQTT or UI commands are not being executed

    Serial.println("Going Reverse!");
    move_DC_motor(DC_REVERSE);
    client.publish(mqtt_voice_command_topic, "Have executed the Reverse command!");

    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;
  }


  
  // GO TO THE CHARGING DOCK
  if ((strcmp(message.c_str(), "CD") == 0 && recvd_mqtt_voice_cmd && check_ultrasonic_sensors()) || (strcmp(message.c_str(), "CD") == 0 && recvd_mqtt_UI_cmd && check_ultrasonic_sensors())){
    Serial.println("Going to the charging dock!");

    move_DC_motor(DC_CD);
    client.publish(mqtt_voice_command_topic, "Have reached the charging dock!");

    recvd_mqtt_voice_cmd = false;
    recvd_mqtt_UI_cmd = false;

  }
  else {
    client.publish(mqtt_voice_command_topic, "Waiting for 3 seconds to check if the object in front of me moves away!");
    delay(3000);

    if (check_ultrasonic_sensors()){

      Serial.println("Going to charging dock!");
      move_DC_motor(DC_CD);
      client.publish(mqtt_voice_command_topic, ""Have reached the charging dock!");

      recvd_mqtt_voice_cmd = false;
      recvd_mqtt_UI_cmd = false;
    }
    else{
      client.publish(mqtt_voice_command_topic, "Object is still in front of me and so I cannot Reverse!");
    }

}








