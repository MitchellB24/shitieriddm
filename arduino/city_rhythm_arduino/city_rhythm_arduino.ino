/*
   Ramp library:      https://github.com/siteswapjuggler/RAMP
   Adafruit shield:   https://learn.adafruit.com/adafruit-16-channel-pwm-slash-servo-shield/using-the-adafruit-library
   tinytronics servo: https://www.tinytronics.nl/shop/nl/mechanica-en-actuatoren/motoren/servomotoren/mg996r-servo
*/


#include <Ramp.h>
#include <Adafruit_PWMServoDriver.h>
#include "city_rhythm_constants.h"

Adafruit_PWMServoDriver servoShield = Adafruit_PWMServoDriver();

//-------------------------------------------//

void setup() {
  Serial.begin(BAUDRATE);

  servoShield.begin();
  servoShield.setPWMFreq(SERVO_FREQ);

  // Start servo's in horizontal position
  int startAngle = (ANGLE_MIN + ANGLE_MAX) / 2;

  for (int i = 0; i < numServo; i++) {
    servo[i].freq = 1.0;
    servo[i].posRamp.go(startAngle);
    servo[i].posRamp.pause();

    int PWM = map(startAngle, 0, 180, PWM_VAL[i][0], PWM_VAL[i][1]);
    servoShield.setPWM(i, 0, PWM);
  }

  //  delay(250); // wait for a bit here?

  if (testMode) {
    while (true) {
      testRun();
    }
  }
}


void loop() {
  // Parse serial messages from Max
  // Every message starts with 1 byte specifying its type
  if (Serial.available() > 0) {
    int msgType = Serial.read();

    if (msgType == MSG_SERVO) {
      parseServoMsg();
    }
    else if (msgType == MSG_SYNC) {
      parseSyncMsg();
    }
    else if (msgType == MSG_STOP) {
      stopServos = true;
      //      Serial.println("stop message");

      for (int i = 0; i < numServo; i++) {
        servo[i].Stop = true;
      }
    }
  }

  boolean allPaused = true;

  for (int i = 0; i < numServo; i++) {
    // isPaused() seems to be reversed?
    boolean servoPaused = !servo[i].posRamp.isPaused();

    if (!servoPaused) {
      allPaused = false;
      servo[i].posRamp.update();

      // 1. Is the ramp finished? (the servo arrived at min or max angle)
      if (servo[i].posRamp.isFinished()) {
        int angle;
        servo[i].Direction ? angle = ANGLE_MAX : angle = ANGLE_MIN;

        // 2. If sync or stop, and servo is at min angle: stop moving
        if ((syncServos || servo[i].Stop) && angle == ANGLE_MAX) {
          if (!servoPaused) {
            servo[i].posRamp.pause();
          }
        }

        // 3. Else, continue moving and start a new ramp in opposite direction
        else {
          int ramp_duration = int(1000.0 / servo[i].freq);

          servo[i].posRamp.go(angle, ramp_duration, LINEAR);
          servo[i].Direction = !servo[i].Direction;
        }
      }

      // 4. Map angle to PWM value
      Serial.print(servo[i].posRamp.getValue());
      Serial.print(",");

      int PWM = map(servo[i].posRamp.getValue(), 0, 180, PWM_VAL[i][0], PWM_VAL[i][1]);
      servoShield.setPWM(i, 0, PWM);
    }
  }
  Serial.println();

  if (allPaused) {
    if (stopServos) {
      stopServos = false;

      for (int i = 0; i < numServo; i++) {
        servo[i].Stop = false;
      }
    }
    else if (syncServos) {
      syncServos = false;

      for (int i = 0; i < numServo; i++) {
        servo[i].freq = syncFreq;
        servo[i].posRamp.resume();
      }
    }
  }
}


void parseServoMsg() {
  /*
    Servo message: 3 bytes
     - first byte is servo ID
     - second two bytes are servo frequency
  */
  char bytes[3];
  Serial.readBytes(bytes, 3);

  int ID = bytes[0];

  if (ID >= 0 && ID <= numServo) {
    int hundreds = bytes[1];
    int tenths = bytes[2];
    float frequency = hundreds + (tenths / 100.0);

    // Set servo[ID] to freq and start
    if (ID < numServo) {
      setServo(ID, frequency);
    }
    // if ID = numServo, set all servos
    else if (ID == numServo) {
      //      Serial.println("Set all servos");
      for (int i = 0; i < numServo; i++) {
        setServo(i, frequency);
      }
    }
  }
}

void setServo(int ID, float frequency) {
  //  Serial.print("Servo ");
  //  Serial.print(ID);
  //  Serial.print(", ");
  //  Serial.println(frequency);

  if (frequency > 0.0) {
    servo[ID].freq = frequency;

    // 1. Percentage of ramp still to go
    float ramp_percentage = abs(servo[ID].posRamp.getValue() - servo[ID].posRamp.getTarget()) / float(ANGLE_MAX - ANGLE_MIN); // abs() or fabs()? <---------

    // 2. Get new duration from frequency
    int ramp_duration = int(1000.0 / servo[ID].freq * ramp_percentage);

    // 3. Continue the ramp with new duration
    servo[ID].posRamp.go(servo[ID].posRamp.getTarget(), ramp_duration, LINEAR);
  }
  // Frequency <= 0 means stop
  else {
    servo[ID].Stop = true;
  }
}

void parseSyncMsg() {
  char bytes[2];

  Serial.readBytes(bytes, 2);
  syncServos = true;

  int hundreds = bytes[0];
  int tenths = bytes[1];
  syncFreq = hundreds + (tenths / 100.0);

  Serial.println("sync message");
  Serial.print("sync frequency: ");
  Serial.println(syncFreq);
}

//----------------------------------------------------//


void testRun() {
  // Function to test min and max PWM values for every servo
  // It cycles through each servo, toggling between min and max PWM, on a loop

  int delay_time = 1000;
  boolean control_PWM = true; // pwm or us

  // specs Tinytronics:
  const int USMIN = 500; // microseconds
  const int USMAX = 2400;

  for (int i = 0; i < numServo; i++) {
    if (control_PWM) {
      for (int j = PWM_VAL[i][0]; j < PWM_VAL[i][1]; j++) {
        servoShield.setPWM(i, 0, j);
      }
      delay(delay_time);

      for (int j = PWM_VAL[i][1]; j >= PWM_VAL[i][0]; j--) {
        servoShield.setPWM(i, 0, j);
      }
      delay(delay_time);
    }

    else {
      for (int j = USMIN; j < USMAX; j++) {
        servoShield.writeMicroseconds(i, j);
      }
      delay(delay_time);

      for (int j = USMAX; j >= USMIN; j--) {
        servoShield.writeMicroseconds(i, j);
      }
      delay(delay_time);
    }
  }
}
