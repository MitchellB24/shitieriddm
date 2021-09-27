// adafruit shield:   https://learn.adafruit.com/adafruit-16-channel-pwm-slash-servo-shield/using-the-adafruit-library
// tinytronics servo: https://www.tinytronics.nl/shop/nl/mechanica-en-actuatoren/motoren/servomotoren/mg996r-servo

#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver servoShield = Adafruit_PWMServoDriver();

const int numServo = 16;
const int PWM_VAL[numServo][2] = {  {75, 550}, // servo {minPWM, maxPWM}
                                    {100, 450}
};
const int ANGLE_MIN = 0;
const int ANGLE_MAX = 180;

const unsigned long BAUDRATE = 230400;

byte angle[numServo];

enum SerialModes {
  SEPERATE, LIST
};

int serialMode = LIST;

void setup() {
  Serial.begin(BAUDRATE);

  servoShield.begin();
  servoShield.setPWMFreq(50);

  for (int i = 0; i < numServo; i++) {
    angle[i] = 0;
  }
}


void loop() {
  if (serialMode == SEPERATE) {
    receiveIndivualServo();
  }
  else if (serialMode == LIST) {
    receiveServoList();
  }

  for (int i = 0; i < numServo; i++) {
    Serial.print(angle[i]);
    Serial.print(",");
  }
  Serial.println();
}

void receiveIndivualServo() {
  if (Serial.available() >= 2) {
    int ID = Serial.read();

    if (ID >= 0 && ID < numServo) {
      angle[ID] = Serial.read();

      int pwm = map(angle[ID], 0, 180, PWM_VAL[ID][0], PWM_VAL[ID][1]);
      servoShield.setPWM(ID, 0, pwm);
    }
  }
}

void receiveServoList() {
  if (Serial.available() >= numServo) {
    Serial.readBytes(angle, numServo);

    for (int i = 0; i < numServo; i++) {
      int pwm = map(angle[i], 0, 180, PWM_VAL[i][0], PWM_VAL[i][1]);
      servoShield.setPWM(i, 0, pwm);
    }
  }
}
