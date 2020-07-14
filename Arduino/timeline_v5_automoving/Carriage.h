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

  void init(Adafruit_NeoPixel *strip, int _nPositions);
  void update();
  void setNewPos(int _newPos);
  void debug();

private:
	void setNewAutoPos( );
  Adafruit_NeoPixel *strip;
  Vehicle *vehicle;
  Animator_Sine *arsine;

  enum {
    AUTO = 0,	// a state where the carriage moves around automatically
    ACTIVE,		// the user is touching
		STANDBY		// we enter here when time is passed from the last "active" touch
  } state = AUTO;
  
	// time to wait before moving from ACTIVE to STANDBY
  long TIME_TO_STANDBY = 5000; // ms
	// time to wait before moving from STANDBY to AUTO
	long TIME_TO_AUTO = 10; // ms
	// this time variable is used for in-between movements while in AUTO mode
	long TIME_TO_REACH = 2000; // ms
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

	// a support variable to be used to light up the LEDs
	float brightness = 0.0;
	// this value of brightness is used during the AUTO mode
	float MID_BRIGHTNESS = 0.5;
	// this value of brightness is used during the STANBY and ACTIVE mode
	float MAX_BRIGHTNESS = 1.0;
};
#endif
