/*
 * Ramp library:      https://github.com/siteswapjuggler/RAMP
 * Adafruit shield:   https://learn.adafruit.com/adafruit-16-channel-pwm-slash-servo-shield/using-the-adafruit-library
 * tinytronics servo: https://www.tinytronics.nl/shop/nl/mechanica-en-actuatoren/motoren/servomotoren/mg996r-servo
*/


#include <Ramp.h>
#include <Servo.h>

const int PWM_MIN = 75;
const int PWM_MAX = 550;

int ANGLE_MIN = 20;
int ANGLE_MAX = 90;

const int numServo = 1;

struct _Servo {
  float freq;     // Hz
  boolean posToggle;

  ramp posRamp;
};

_Servo servo[numServo];

Servo myservo;

void setup() {
  Serial.begin(115200);
  
  myservo.attach(9,500,2500);

  for (int i = 0; i < numServo; i++) {
    servo[i].freq = 1.0;
    servo[i].posRamp.go(ANGLE_MIN);
    servo[i].posRamp.pause();
  }
}


void loop() {
  
  if (Serial.available() >= 3) {
    parseMsgFromMax();
  }

  for (int i = 0; i < numServo; i++) {
    // isPaused() seems to be reversed?
    if (servo[i].posRamp.isPaused()) {
      servo[i].posRamp.update();
  
      if (servo[i].posRamp.isFinished()) {
        int angle;
        servo[i].posToggle ? angle = ANGLE_MAX : angle = ANGLE_MIN;
  
        int ramp_duration = int(1000.0 / servo[i].freq);
  
        servo[i].posRamp.go(angle, ramp_duration, LINEAR);
        servo[i].posToggle = !servo[i].posToggle;
      }

      myservo.write(int(servo[i].posRamp.getValue()));
    }
  }
}


void parseMsgFromMax() {
  // first char is servo ID
  int ID = Serial.read();

  if (ID >= 0 && ID < numServo) {
    // second two chars are servo frequency
    int hundreds = Serial.read();
    int tenths = Serial.read();

    float frequency = hundreds + (tenths / 100.0);

    if (frequency > 0.0) {
      servo[ID].freq = frequency;

      // percentage of ramp still to go
      float ramp_percentage = fabs(servo[ID].posRamp.getValue() - servo[ID].posRamp.getTarget()) / float(ANGLE_MAX-ANGLE_MIN);

      int ramp_duration = int(1000.0 / servo[ID].freq * ramp_percentage);

      // continue the ramp with new frequency
      servo[ID].posRamp.go(servo[ID].posRamp.getTarget(), ramp_duration, LINEAR);
    }
    else {
      servo[ID].posRamp.pause();
    }
  }
}
