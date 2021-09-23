// adafruit shield:   https://learn.adafruit.com/adafruit-16-channel-pwm-slash-servo-shield/using-the-adafruit-library
// tinytronics servo: https://www.tinytronics.nl/shop/nl/mechanica-en-actuatoren/motoren/servomotoren/mg996r-servo

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver servoShield = Adafruit_PWMServoDriver();

const int PWM_MIN = 75;
const int PWM_MAX = 550;

// specs tinytronics:
const int USMIN = 500; // microseconds (us)
const int USMAX = 2400;

int angle_min = 0;
int angle_max = 180;

const int numServo = 10;

byte serialData[numServo];
byte serialCount = 0;

void setup() {
  Serial.begin(115200);

  servoShield.begin();
  servoShield.setPWMFreq(50);

  for (int i = 0; i < numServo; i++) {
    serialData[i] = 0;
  }
  yield();
}



void loop() {
  if (Serial.available() > 0) {
    byte incomingByte = Serial.read();

    if (incomingByte == 255) {
      serialCount = 0;
    }

    else {
      int pwm = map(incomingByte, 0, 180, PWM_MIN, PWM_MAX);

      servoShield.setPWM(serialCount, 0, pwm);

      serialData[serialCount] = incomingByte;
      serialCount++;
    }
  }

  for (int i = 0; i < numServo; i++) {
    Serial.print(serialData[i]);
    Serial.print(",");
  }
  Serial.println();
}


void test_run() {
  int delay_time = 1000;
  boolean control_PWM = true; // pwm or us

  for (int s = 0; s < numServo; s++) {

    if (control_PWM) {
      for (int i = PWM_MIN; i < PWM_MAX; i++) {
        servoShield.setPWM(s, 0, i);
      }
      delay(delay_time);

      for (int i = PWM_MAX; i >= PWM_MIN; i--) {
        servoShield.setPWM(s, 0, i);
      }
      delay(delay_time);
    }

    else {
      for (int i = USMIN; i < USMAX; i++) {
        servoShield.writeMicroseconds(s, i);
      }
      delay(delay_time);

      for (int i = USMAX; i >= USMIN; i--) {
        servoShield.writeMicroseconds(s, i);
      }
      delay(delay_time);
    }
  }
}
