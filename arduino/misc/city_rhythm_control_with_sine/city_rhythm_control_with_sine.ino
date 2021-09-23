#include <Ramp.h>

// adafruit shield:   https://learn.adafruit.com/adafruit-16-channel-pwm-slash-servo-shield/using-the-adafruit-library
// tinytronics servo: https://www.tinytronics.nl/shop/nl/mechanica-en-actuatoren/motoren/servomotoren/mg996r-servo

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver servoShield = Adafruit_PWMServoDriver();

const int PWM_MIN = 75;    // (0 - 4096)
const int PWM_MAX = 550; // (0 - 4096)

// specs tinytronics:
const int USMIN = 500; // microseconds (us)
const int USMAX = 2400;

int angle_min = 0;
int angle_max = 180;

struct _servo {
  int angle;    // (0 - 180)

  float freq;   // Hz
  float phase;  // (0 - 2pi)
};

const int numServo = 1;

_servo servo[numServo];

void setup() {
  Serial.begin(115200);

  servoShield.begin();
  servoShield.setPWMFreq(50);

  for (int i = 0; i < numServo; i++) {
    servo[i].angle = angle_min;
    servo[i].phase = 0;
    servo[i].freq = 0.5;
  }
}

void loop() {
  float t = millis() * 0.001;

  for (int i = 0; i < numServo; i++) {
    float cos_val = cos(TWO_PI * t * servo[i].freq);

    servo[i].angle = int(floatMap(cos_val, -1.0, 1.0, angle_min, angle_max));

    int pwm = map(servo[i].angle, 0, 180, PWM_MIN, PWM_MAX);

    servoShield.setPWM(0, 0, pwm);
  }
}


void parseMsgFromMax() {
  // first byte is servo ID
  int ID = Serial.read();

  ID = constrain(ID, 0, numServo);

  // second two bytes are servo frequency
  int hundreds = Serial.read();
  int tenths = Serial.read();

  float frequency = hundreds + (tenths / 100.0);
  servo[ID].freq = frequency;

  Serial.print(ID);
  Serial.print(", ");
  Serial.println(frequency);
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

float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
