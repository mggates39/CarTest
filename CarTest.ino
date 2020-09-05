
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

// Allow for 3 seconds (3000 milliseconds)
// before taking over with sonar control
#define FAILOVER_TIME 3000

myDelay blueToothFailTimer(FAILOVER_TIME);

char command = ' ';
char previous_command = ' ';

int current_motor_speed;
bool is_moving = false;

int servo_position = -1;

bool sonar_mode = false;

// Assume 180 degree servo with 90 front and center
#define SERVO_MIN_POSITION 0
#define SERVO_FORWARD_POSITION 90
#define SERVO_MAX_POSITION 180

#define MAX_SCAN_STEPS  5
#define FORWARD_SCAN_STEP 2
int search_positions[MAX_SCAN_STEPS] = {SERVO_MIN_POSITION, // Far Left
                                        SERVO_MIN_POSITION + (SERVO_FORWARD_POSITION / 2), // Mid Left
                                        SERVO_FORWARD_POSITION, // Straight ahead
                                        SERVO_FORWARD_POSITION + (SERVO_FORWARD_POSITION / 2), // Mid Right
                                        SERVO_MAX_POSITION
                                       }; // Far Right
float search_range[MAX_SCAN_STEPS] = {0.0, 0.0, 0.0, 0.0, 0.0};
int scan_step = 0;
int scan_increment = 1;


// pre-define several functions
bool processCommand(char);
void setServoPosition(int);
void avoidCollision(float);

void setup()
{
  Serial.begin(9600);  //Set the baud rate to your Bluetooth module.

  // so we don't start with the sonar
  blueToothFailTimer.stop();
  sonar_mode = false;

  //initialize with motors stopped
  Stop();

  // turn on servo
  servo1.attach(SERVO_1_PIN);

  current_motor_speed = SLOW_MOTOR_SPEED;

  // set unknown servo position so the first
  // positioning attempt will always fire
  servo_position = -1;
  setServoPosition(SERVO_MIN_POSITION);
  setServoPosition(SERVO_MAX_POSITION);
  faceForward();

}

void loop()
{
  if (blueToothFailTimer.update()) {
    // no command in the time out period.
    // stop the vehical and go to sonar mode
    Stop();
    blueToothFailTimer.stop();
    sonar_mode = true;
  }

  if (Serial.available() > 0) {
    // Get the command and process it
    command = Serial.read();

    if (processCommand(command)) {
      // Restart the timer since we have a
      // command that is causing the car to move
      blueToothFailTimer.start();
    } else {
      // The command stopped the vehicle
      // do not allow sonar to take over
      blueToothFailTimer.stop();
      sonar_mode = false;
    }
  }

  if (sonar_mode == false) {
    testForCollisionOnBlueTooth();
  } else {
    scanAhead();
  }

}
/**
   This is the way we know we are using bluetooth
   the sonar will just point straight ahead
*/
void testForCollisionOnBlueTooth()
{
  faceForward();
  float distance = hc.dist(); // distance in centimeters

  avoidCollision(distance);
}

/**
   If we are far away from something, go full speed
   else If we start getting close to something, slow down
   else we getting Real close, backup turn to the side
   and start moving forward again
*/
void avoidCollision(float distance)
{
  if (distance >= 25.0) {
    current_motor_speed = MAX_MOTOR_SPEED;
  } else if (distance >= 5.0) {
    current_motor_speed = SLOW_MOTOR_SPEED;
  } else {
    processCommand('B');
    delay(100);
    processCommand('R');
    delay(300);
    processCommand('F');
  }

}

/**
   This is called every time through the main loop.
   Each time it scans in the next direction
   and records the distance in an array of ranges
   when it gets to the end of the array, it calls the
   the function to plan the next move
*/
void scanAhead()
{
  setServoPosition(search_positions[scan_step]);
  search_range[scan_step] = hc.dist(); // distance in centimeters
  scan_step += scan_increment;
  if (scan_step == MAX_SCAN_STEPS) {
    scan_increment = -1;
    scan_step += scan_increment;
    planNextMove();
  } else if (scan_step == 0) {
    scan_increment = 1;
    planNextMove();
  }
}

/**
   Go through the array of ranges and find the largest one
   note which element it is so we know which way to turn
*/
void planNextMove()
{
  // Find the largest distance
  float largest_range = 0.0;
  int largest_step = -1;

  for (int i = 0; i < MAX_SCAN_STEPS; i++) {
    if (search_range[i] > largest_range) {
      largest_step = i;
      largest_range = search_range[i];
    }
  }

  if ( largest_step > -1) {
    // We have a winner, let's point that way and go forward
    switch (largest_step)
    {
      case 0:
        // Turn hard left
        processCommand('L');
        delay(600);
        processCommand('F');
        break;
      case 1:
        // Turn slight left
        processCommand('L');
        delay(300);
        processCommand('F');
        break;
      case 2:
        // it is the way we are going,
        // so just do collision avoidance
        avoidCollision(largest_range);
        break;
      case 3:
        // Turn slight right
        processCommand('R');
        delay(300);
        processCommand('F');
        break;
      case 4:
        // Turn hard right
        processCommand('R');
        delay(600);
        processCommand('F');
        break;
    }
  } else {
    // We don't see anything,
    // pretend we are about to hit a wall
    // step back, turn and start moving again
    avoidCollision(10.0);
    avoidCollision(1.0);
  }
}

bool processCommand(char newCommand)
{
  /**
     Commands from the Bluetooth app

     Forward----------------F
     Back-------------------B
     Left-------------------L
     Right------------------R
     Forward Left-----------G
     Forward Right----------I
     Back Left--------------H
     Back Right-------------J
     Stop-------------------S
     Front Lights On--------W
     Front Lights Off-------w (lower case)
     Back Lights On---------U
     Back Lights Off--------u (lower case)
     Speed 0----------------0
     Speed 10---------------1
     Speed 20---------------2
     Speed 30---------------3
     Speed 40---------------4
     Speed 50---------------5
     Speed 60---------------6
     Speed 70---------------7
     Speed 80---------------8
     Speed 90---------------9
     Speed 100--------------q
     Everything OFF---------D
  */


  if (newCommand != previous_command)
  {
    previous_command = newCommand;

    Stop(); //initialize with motors stopped

    switch (newCommand)
    {
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
  return is_moving;
}

void setServoPosition(int position) {
  if (servo_position != position) {
    servo1.write(position);
    servo_position = position;
    delay(15);
  }
}

/**
   Make the servo point forward
*/
void faceForward()
{
  setServoPosition(SERVO_FORWARD_POSITION);
}

/**
   Run the four motors in a forward direction
   using the current motor speed
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
  is_moving = true;
}

/**
   Run the four motors in a backward direction
   using the current motor speed
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
  is_moving = true;
}

/**
   Run the left two motors backward
   and the right two motors forward
   This pivots the vehical around the center to the left
   using the current motor speed
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
  is_moving = true;
}

/**
   Run the left two motors forward
   and the right two motors backward
   This pivots the vehical around the center to the right
   using the current motor speed
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
  is_moving = true;
}

/**
   Stop all motor motion by setting the speed to the
   minimum value and releasing the motor run control
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
  is_moving = false;
}
