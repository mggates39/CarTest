
/*
Code Name: Arduino Bluetooth Control Car
Code URI: https://circuitbest.com/category/arduino-projects/
Before uploading the code you have to install the "Adafruit Motor Shield" library
Open Arduino IDE >> Go to sketch >> Include Libray >> Manage Librays...  >> Search "Adafruit Motor Shield" >> Install the Library
AFMotor Library: https://learn.adafruit.com/adafruit-motor-shield/library-install
Author: Make DIY
Author URI: https://circuitbest.com/author/admin/
Description: This program is used to control a robot using a appthat communicates with Arduino through a HC-05 Bluetooth Module.
App URI: https://bit.ly/2BlMAea
Version: 1.0
License: Remixing or Changing this Thing is allowed. Commercial use is not allowed.
*/

#include <AFMotor.h>
#include <Servo.h> 
#include <HCSR04.h>
#include "MyDelay.h"

//initial motors pin and control speed
AF_DCMotor motor1(1, MOTOR12_1KHZ);
AF_DCMotor motor2(2, MOTOR12_1KHZ);
AF_DCMotor motor3(3, MOTOR34_1KHZ);
AF_DCMotor motor4(4, MOTOR34_1KHZ);

#define MAX_MOTOR_SPEED 255
#define SLOW_MOTOR_SPEED 127
#define MIN_MOTOR_SPEED 0

// Servo Pins - pre-defined because of the L293D Motor Shield
#define SERVO_1_PIN 9
#define SERVO_2_PIN 10

// DC hobby servo
Servo servo1;

// Sonar Pins
#define TRIGGER_PIN A4
#define ECHO_PIN A5

// Sonar Sensor
HCSR04 hc(TRIGGER_PIN, ECHO_PIN);


#define FAILOVER_TIME 300000

myDelay blueToothFailTimer(FAILOVER_TIME);

char command;

int current_motor_speed;

int servo_position;

// Assume 180 degree servo with 90 front and center
#define SERVO_FORWARD_POSITION 90

// pre-define the command processor function
void processCommand(char);

void setup()
{
  Serial.begin(9600);  //Set the baud rate to your Bluetooth module.
  
  blueToothFailTimer.stop();  // so we don't start with the sonar
  
  // turn on servo
  servo1.attach(SERVO_1_PIN);

  current_motor_speed = MAX_MOTOR_SPEED;
   
}

void loop()
{
  if (blueToothFailTimer.update()) {
    // no command in the time out period. 
    // stop the vehical and go to sonar mode
    Stop(); //initialize with motors stoped
    blueToothFailTimer.stop();
  }
  
  if(Serial.available() > 0){
    // Get the command and process it
    command = Serial.read();
    processCommand(command);
    
    // Restart the timer since we have a command
    blueToothFailTimer.start();
  }

  faceForward();
  float distance = hc.dist(); // distance in centimeters
  if (distance < 25) {
    current_motor_speed = SLOW_MOTOR_SPEED;
  } else if (distance < 5) {
    Stop();
  } else {
    current_motor_speed = MAX_MOTOR_SPEED;
  }
}

void processCommand(char newCommand) {

  Stop(); //initialize with motors stoped
  
  switch(newCommand){
    case 'F':
      forward();
      break;
    case 'B':
       back();
      break;
    case 'L':
      left();
      break;
    case 'R':
      right();
      break;
    }
}

void faceForward()
{
  if (servo_position != SERVO_FORWARD_POSITION) {
    servo1.write(SERVO_FORWARD_POSITION);
    delay(15);
  }
}

/**
 * Run the four motors in a forward direction
 * using the current motor speed
 */
void forward()
{
  motor1.setSpeed(current_motor_speed); 
  motor1.run(FORWARD); //rotate the motor clockwise
  motor2.setSpeed(current_motor_speed); 
  motor2.run(FORWARD); //rotate the motor clockwise
  motor3.setSpeed(current_motor_speed);
  motor3.run(FORWARD); //rotate the motor clockwise
  motor4.setSpeed(current_motor_speed);
  motor4.run(FORWARD); //rotate the motor clockwise
}

/**
 * Run the four motors in a backward direction
 * using the current motor speed
 */
void back()
{
  motor1.setSpeed(current_motor_speed);
  motor1.run(BACKWARD); //rotate the motor anti-clockwise
  motor2.setSpeed(current_motor_speed);
  motor2.run(BACKWARD); //rotate the motor anti-clockwise
  motor3.setSpeed(current_motor_speed);
  motor3.run(BACKWARD); //rotate the motor anti-clockwise
  motor4.setSpeed(current_motor_speed);
  motor4.run(BACKWARD); //rotate the motor anti-clockwise
}

/**
 * Run the left two motors backward
 * and the right two motors forward
 * This pivots the vehical around the center to the left
 * using the current motor speed
 */
void left()
{
  motor1.setSpeed(current_motor_speed); //Define maximum velocity
  motor1.run(BACKWARD); //rotate the motor anti-clockwise
  motor2.setSpeed(current_motor_speed); //Define maximum velocity
  motor2.run(BACKWARD); //rotate the motor anti-clockwise
  motor3.setSpeed(current_motor_speed); //Define maximum velocity
  motor3.run(FORWARD);  //rotate the motor clockwise
  motor4.setSpeed(current_motor_speed); //Define maximum velocity
  motor4.run(FORWARD);  //rotate the motor clockwise
}

/**
 * Run the left two motors forward
 * and the right two motors backward
 * This pivots the vehical around the center to the right
 * using the current motor speed
 */
void right()
{
  motor1.setSpeed(current_motor_speed); 
  motor1.run(FORWARD); //rotate the motor clockwise
  motor2.setSpeed(current_motor_speed); 
  motor2.run(FORWARD); //rotate the motor clockwise
  motor3.setSpeed(current_motor_speed); 
  motor3.run(BACKWARD); //rotate the motor anti-clockwise
  motor4.setSpeed(current_motor_speed); 
  motor4.run(BACKWARD); //rotate the motor anti-clockwise
}

/**
 * Stop all motor motion by setting the speed to the 
 * minimum value and releasing the motor run control
 */
void Stop()
{
  motor1.setSpeed(MIN_MOTOR_SPEED);
  motor1.run(RELEASE); //stop the motor when release the button
  motor2.setSpeed(MIN_MOTOR_SPEED);
  motor2.run(RELEASE); //rotate the motor clockwise
  motor3.setSpeed(MIN_MOTOR_SPEED);
  motor3.run(RELEASE); //stop the motor when release the button
  motor4.setSpeed(MIN_MOTOR_SPEED); 
  motor4.run(RELEASE); //stop the motor when release the button
}
