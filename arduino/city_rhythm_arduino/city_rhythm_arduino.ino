/*
   Ramp library:      https://github.com/siteswapjuggler/RAMP
   Adafruit shield:   https://learn.adafruit.com/adafruit-16-channel-pwm-slash-servo-shield/using-the-adafruit-library
   tinytronics servo: https://www.tinytronics.nl/shop/nl/mechanica-en-actuatoren/motoren/servomotoren/mg996r-servo
*/


#include <Ramp.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver servoShield = Adafruit_PWMServoDriver();

const int numServo = 2;
const int PWM_VAL[numServo][2] = {  {75, 550}, // servo {minPWM, maxPWM}
                                    {100, 450} 
                                 };
const int SERVO_FREQ = 50;       // To do: set this to frequency specific for servo?
const int ANGLE_MIN = 0;
const int ANGLE_MAX = 180;


struct CityRhythmServo {
  float freq;     // Hz
  boolean Direction;
  ramp posRamp;
};

CityRhythmServo servo[numServo];

enum messageType { MSG_STOP = 253, MSG_SYNC, MSG_SERVO };
boolean syncServos = false;
boolean stopServos = false;


//-------------------------------------------//

void setup() {
  Serial.begin(115200);

  servoShield.begin();
  servoShield.setPWMFreq(SERVO_FREQ);

  for (int i = 0; i < numServo; i++) {
    servo[i].freq = 1.0;
    servo[i].posRamp.go(ANGLE_MIN);
    servo[i].posRamp.pause();
  }
//  testRun();
}


void loop() {
  // Parse serial messages from Max
  // Every message starts with 1 byte specifying the type
  if (Serial.available() > 0) {
    int msgType = Serial.read();

    if (msgType == MSG_SERVO) {
      parseServoMsg();
    }
    else if (msgType == MSG_SYNC) {
      Serial.println("sync message");
      syncServos = true;
    }
    else if (msgType == MSG_STOP) {
      Serial.println("stop message");
      stopServos = true;
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

        // 2. If sync or stop, wait for servo to arrive at min angle
        if ((syncServos || stopServos) && angle == ANGLE_MAX) {
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
      int PWM = map(servo[i].posRamp.getValue(), 0, 180, PWM_VAL[i][0], PWM_VAL[i][1]);
      servoShield.setPWM(i, 0, PWM);
    }
  }

  if (allPaused) {
    if (stopServos) {
      stopServos = false;
    }
    else if (syncServos) {
      syncServos = false;

      for (int i = 0; i < numServo; i++) {
        servo[i].posRamp.resume();
//        Serial.println(servo[i].Direction);
      }
    }
  }
}

void parseServoMsg() {
  /*
    Servo message: 3 bytes
     - first byte is servo ID
     - second two bytes together is servo frequency
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
      Serial.println("Set all servos");
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
    float ramp_percentage = fabs(servo[ID].posRamp.getValue() - servo[ID].posRamp.getTarget()) / float(ANGLE_MAX - ANGLE_MIN);

    // 2. Get new duration from frequency
    int ramp_duration = int(1000.0 / servo[ID].freq * ramp_percentage);

    // 3. Continue the ramp with new duration
    servo[ID].posRamp.go(servo[ID].posRamp.getTarget(), ramp_duration, LINEAR);
  }
  // Frequency <= 0 means stop
  else {
    servo[ID].posRamp.pause();
  }
}

//----------------------------------------------------//


void testRun() {
  int delay_time = 1000;
  boolean control_PWM = true; // pwm or us

  // specs tinytronics:
  const int USMIN = 500; // microseconds
  const int USMAX = 2400;

  while (true) {
    Serial.println("HI");
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
}
