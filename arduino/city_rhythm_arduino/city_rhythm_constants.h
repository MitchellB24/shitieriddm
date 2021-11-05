#ifndef _CITY_RHYTHM_CONSTANTS_
#define _CITY_RHYTHM_CONSTANTS_

/*
   Ramp library:      https://github.com/siteswapjuggler/RAMP
   Adafruit shield:   https://learn.adafruit.com/adafruit-16-channel-pwm-slash-servo-shield/using-the-adafruit-library
   tinytronics servo: https://www.tinytronics.nl/shop/nl/mechanica-en-actuatoren/motoren/servomotoren/mg996r-servo
*/

//-------------------------------------------//

const long BAUDRATE = 230400;

const int numServo = 1;
const int PWM_VAL[numServo][2] = {  {100, 500} // servo: minPWM, maxPWM
                                 };
const int ANGLE_MIN = 29;
const int ANGLE_MAX = 117;

const int SERVO_FREQ = 50;       // To do: set this to frequency specific for servo?

// Set to true to test every servo's PWM values
boolean testMode = false;

//-------------------------------------------//

struct CityRhythmServo {
  float freq;     // Hz
  boolean Direction = false;
  boolean Stop = false;
  ramp posRamp;
};

enum messageType { MSG_STOP = 253, MSG_SYNC, MSG_SERVO };
boolean syncServos = false;
float syncFreq;
boolean stopServos = false;

#endif
