/*
*  The "Carriage" class
* Created by Nicola Ariutti, May 17, 2019
* This class contains all the logic to light up LEDS and
* carriage movement (using the Vehicle support class).
* CC0
*/
#ifndef _NA_ABOCA_TIMELINE_CARRIAGE_
#define _NA_ABOCA_TIMELINE_CARRIAGE_
#include "Arduino.h"
#include "Adafruit_NeoPixel.h"
#include "NicolaAriutti_Vehicle.h"
#include "Animator_Sine.h"

class Carriage
{
public:
  Carriage() {};

  void init(Adafruit_NeoPixel *strip, int nVirtPads);
  void update();
  void setNewPos(int _newPos);
  void debug();

private:
  Adafruit_NeoPixel *strip;
  Vehicle *vehicle;
  Animator_Sine *arsine;

  enum {
    STANDBY = 0,
    ACTIVE 
  } state = STANDBY;
  long TIMETOWAIT = 5000; // time to wait before moving from ACTIVE to STANDBY
  long prevTime = 0;
  
  int nLeds;
  int nVirtPads;
  int selectedVirtualPad;

  // the position of the carriage LEDS relative (between 0 and nLeds)
  int centroidPos;
  // must define sidelobe dimension first
  // this is a monolateral dimension
  const int SIDELOBE = 4;
  // then we declare an array that will work as a lookup table
  // for the squared-raised-cosine figure.
  float* lookup = new float[SIDELOBE];

  //int centroidLeds = 0;  // an integer from 0 to NLEDS
  int blue = 0;
};
#endif
