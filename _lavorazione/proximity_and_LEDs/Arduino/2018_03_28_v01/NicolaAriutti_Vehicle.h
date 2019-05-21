/*
*  The "Vehicle" class
* Created by Nicola Ariutti, March 06, 2018
* TODO: license
*
* Inspired by
* The Nature of Code
* Daniel Shiffman
* http://natureofcode.com
*/
#ifndef _NICOLAARIUTTI_VEHICLE
#define _NICOLAARIUTTI_VEHICLE
#include "Arduino.h"

class NicolaAriutti_Vehicle
{
public:
	NicolaAriutti_Vehicle() {};

	void init( float _x, float _ms, float _mf, int _damp );
  void update();
	void setTarget(float _target);
  float getPosition();

private:
	float position;
  float velocity;
  float acceleration;

	float maxforce;    // Maximum steering force
  float maxspeed;    // Maximum speed

  float target;
  int DAMPING_DISTANCE;
};
#endif
