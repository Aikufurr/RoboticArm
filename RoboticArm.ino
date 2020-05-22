#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN          150    // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX          600    // This is the 'maximum' pulse length count (out of 4096)
#define USMIN             600    // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX             2400   // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ        50     // Analog servos run at ~50 Hz updates
#define SERVOMID          0      // Min degree of servo
#define SERVOMAD          290    // Max degree of servo
#define JOYSTICK_MIN      0      // Min input of servo
#define JOYSTICK_MAX      1021   // Max input of servo
#define SERVO_MOVE_SPEED  10     // Delay for interpolation
#define GRIPPER_CLOSED    60     // Degree of closed gripper
#define GRIPPER_OPEN      30     // Degree of open gripper

const int RightJoystickY = A0;   // Arduino UNO pin for Right Joystick Y
const int RightJoystickX = A1;   // Arduino UNO pin for Right Joystick X
const int LeftJoystickY = A2;    // Arduino UNO pin for Left Joystick Y
const int LeftJoystickX = A3;    // Arduino UNO pin for Left Joystick X

const int LeftJoystickButton = 13;  // Arduino UNO pin for Left Joystick Button
const int RightJoystickButton = 12; // Arduino UNO pin for Right Joystick Button

const int MagnetPin = 11; // Arduino UNO pin for the magnet
const boolean MagnetUsed = false;


// =-=-=-=-=-=-=-=-=
// Default positions 
// for servos
// =-=-=-=-=-=-=-=-=

int RightJoystickYAngle = 90;
int RightJoystickXAngle = 90;
int LeftJoystickYAngle = 90;
int LeftJoystickXAngle = 90;

int RightJoystickYAngle_old = 90;
int RightJoystickXAngle_old = 90;
int LeftJoystickYAngle_old = 90;
int LeftJoystickXAngle_old = 90;

// End of default positions for servos

// Servo # counter
const int servoBase = 0;
const int servoShoulder = 1;
const int servoElbow = 2;
const int servoWrist = 3;
const int servoGripper = 4;

boolean gripperActive = false;

void setup() {
  pinMode(MagnetPin, OUTPUT);
  pwm.begin();                           // Start the PWM
  pwm.setOscillatorFrequency(27000000);  // The int.osc. is closer to 27MHz  
  pwm.setPWMFreq(SERVO_FREQ);            // Analog servos run at ~50 Hz updates
  delay(500);
}


// =-=-=-=-=-=-=-=-=
// Custom coded interpolation for
// setting the servo's position
//
//  @param  servoNumber    Index on PWM board of what servo to move
//  @param  degree_target  The new degree that the user wants the servo to move to
//  @param  degree_old     The old (current) position that the servo is currently in
//  @param  moveSpeed      The amount of time is used for the delay function to either -
//                           slow down or speed up the servo's movement
//  @return void           Returns nothing
// =-=-=-=-=-=-=-=-=
void setAngle(int servoNumber, int degree_target, int degree_old, int moveSpeed) {
  // If the servo needs to move to a higher degree else for a lower degree,
  if (degree_target > degree_old){
    // For loop is used to move the servo to each degree inbetween it's current old position to the new one
    for (int newDegree = degree_old; newDegree < degree_target; newDegree++) {
      // Set the indexed servo to the loop'ed index degree
      pwm.setPWM(servoNumber, 0, map(newDegree, SERVOMID, SERVOMAD, SERVOMIN, SERVOMAX));
      // Delay is used to slow down the speed of the for loop, therefor slowing the movement speed of the servos
      delay(moveSpeed);
    }  
  } else {
    for (int newDegree = degree_old; newDegree > degree_target; newDegree--) {
      pwm.setPWM(servoNumber, 0, map(newDegree, SERVOMID, SERVOMAD, SERVOMIN, SERVOMAX));
      delay(moveSpeed);
    }
  }
}

// Used for the Joystick buttons to avoid multiple presses on a single attempt
int LeftButtonLast = 1;
int RightButtonLast = 1;
long time = 0;
long debounce = 200;

void loop() {
  // FUNCTION Toggle gripper state
  // If the Left Joystick Button is being presses down (LOW) 
  if (digitalRead(LeftJoystickButton) == 0 && LeftButtonLast == 1 && millis() - time > debounce) {
    gripperActive = !gripperActive;  // Toggle the state of the gripper
    time = millis(); 
    LeftButtonLast = 0;
  } 
  
  LeftButtonLast = digitalRead(LeftJoystickButton);

  // FUNCTION Return-to-home
  // If the Right Joystick Button is being presses down (LOW) 
  if (digitalRead(RightJoystickButton) == 0 && LeftButtonLast == 1 && millis() - time > debounce) {
    // Set the servos to 90 degrees
    setAngle(servoBase, 90, LeftJoystickXAngle, 25);
    setAngle(servoShoulder, 90, LeftJoystickYAngle, 25);
    setAngle(servoElbow, 90, RightJoystickYAngle, 25);
    setAngle(servoWrist, 90, RightJoystickXAngle, 25);
    // Reset the known positions of the servos to 90 to avoid jumps
    RightJoystickYAngle = 90;
    RightJoystickXAngle = 90;
    LeftJoystickYAngle = 90;
    LeftJoystickXAngle = 90;
    RightJoystickYAngle_old = RightJoystickYAngle;
    RightJoystickXAngle_old = RightJoystickXAngle;
    LeftJoystickYAngle_old = LeftJoystickYAngle;
    LeftJoystickXAngle_old = LeftJoystickXAngle;
    time = millis(); 
  } 
  
  RightButtonLast = digitalRead(RightJoystickButton);

  // Open / Close the gripper depending on the state
  if (gripperActive) {
    if (MagnetUsed) {
      digitalWrite(MagnetPin, HIGH);
    } else {
      pwm.setPWM(servoGripper, 0, map(GRIPPER_OPEN, SERVOMID, SERVOMAD, SERVOMIN, SERVOMAX));
    }
  } else {
    if (MagnetUsed){
      digitalWrite(MagnetPin, LOW);
    } else {
      pwm.setPWM(servoGripper, 0, map(GRIPPER_CLOSED, SERVOMID, SERVOMAD, SERVOMIN, SERVOMAX));
    }
  }

  // =-=-=-=-=-=-=-=-=
  // Read the positons of the Joysticks and add/sub to their servo's target
  //
  // FUNC  analogRead   Read the pin of the L/R Joystick X/Y
  // FUNC  map          Map the input from it's possible min/man to an range of min/max
  // FUNC  contrain     Constrains a number to be within a range.
  // =-=-=-=-=-=-=-=-=
  if ((analogRead(RightJoystickY) > 510) || (analogRead(RightJoystickY) < 500)) {
    RightJoystickYAngle += map(analogRead(RightJoystickY), JOYSTICK_MIN, JOYSTICK_MAX, -1, 1);
  }
  RightJoystickYAngle = constrain(RightJoystickYAngle, 0, 180);

  if ((analogRead(RightJoystickX) > 510) || (analogRead(RightJoystickX) < 500)) {
    RightJoystickXAngle += map(analogRead(RightJoystickX), JOYSTICK_MIN, JOYSTICK_MAX, -1, 1);
  }
  RightJoystickXAngle = constrain(RightJoystickXAngle, 0, 180);

  if ((analogRead(LeftJoystickY) > 520) || (analogRead(LeftJoystickY) < 490)) {
    LeftJoystickYAngle += map(analogRead(LeftJoystickY), JOYSTICK_MIN, JOYSTICK_MAX, -1, 1);
  } 
  LeftJoystickYAngle = constrain(LeftJoystickYAngle, 0, 180);
  
  if ((analogRead(LeftJoystickX) > 510) || (analogRead(LeftJoystickX) < 500)) {
    LeftJoystickXAngle += map(analogRead(LeftJoystickX), JOYSTICK_MIN, JOYSTICK_MAX, -1, 1);
  }
  LeftJoystickXAngle = constrain(LeftJoystickXAngle, 0, 180);

  // =-=-=-=-=-=-=-=-=
  // FUNC setAngle  Set's the positon of the servos from one degree to another, uses interpolation
  // =-=-=-=-=-=-=-=-=

  setAngle(servoBase, LeftJoystickXAngle, LeftJoystickXAngle_old, SERVO_MOVE_SPEED);
  setAngle(servoShoulder, LeftJoystickYAngle, LeftJoystickYAngle_old, SERVO_MOVE_SPEED);
  setAngle(servoElbow, RightJoystickYAngle, RightJoystickYAngle_old, SERVO_MOVE_SPEED);
  setAngle(servoWrist, RightJoystickXAngle, RightJoystickXAngle_old, SERVO_MOVE_SPEED);

  // Update the servo's old position with it's new one  
  RightJoystickYAngle_old = RightJoystickYAngle;
  RightJoystickXAngle_old = RightJoystickXAngle;
  LeftJoystickYAngle_old = LeftJoystickYAngle;
  LeftJoystickXAngle_old = LeftJoystickXAngle;
}
