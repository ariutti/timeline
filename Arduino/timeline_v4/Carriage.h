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

  void init(Adafruit_NeoPixel *strip, int nPositions);
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

	// The carriage has two different (but logically coincident) positions:
	// 1. A position based on the maximum number of "pads" that the user can touch

	// the number of total positions the carraige might visit
  int nPositions;
	// the position the carriage should move to (via steering behaviour - see vehicle class)
  int pos;

  // 2. A position based on the total number of LEDs we are using
  // the number of leds the carriage is free to use lighting them up while is moving
  int nLeds;
  // the position of the carriage LEDS relative (between 0 and nLeds)
  int ledPos;

  // must define sidelobe dimension first (this is a monolateral dimension)
  const int SIDELOBE = 4;
  // then we declare an array that will work as a lookup table
  // for the squared-raised-cosine figure.
  float* lookup = new float[SIDELOBE];

  int blue = 0;
};
#endif
