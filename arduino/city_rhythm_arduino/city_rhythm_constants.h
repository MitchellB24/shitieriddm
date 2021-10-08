#ifndef _CITY_RHYTHM_CONSTANTS_
#define _CITY_RHYTHM_CONSTANTS_

const long BAUDRATE = 230400;

const int numServo = 2;
const int PWM_VAL[numServo][2] = {  {75, 550}, // servo: minPWM, maxPWM
                                    {100, 450}
                                 };

const int ANGLE_MIN = 29;
const int ANGLE_MAX = 117;

const int SERVO_FREQ = 50;       // To do: set this to frequency specific for servo?

struct CityRhythmServo {
  float freq;     // Hz
  boolean Direction = false;
  boolean Stop = false;
  ramp posRamp;
};

CityRhythmServo servo[numServo];

enum messageType { MSG_STOP = 253, MSG_SYNC, MSG_SERVO };
boolean syncServos = false;
float syncFreq;
boolean stopServos = false;

// set to true to test every servo's pwm values
boolean testMode = false;

#endif
