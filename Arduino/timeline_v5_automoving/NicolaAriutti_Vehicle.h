/*
*  The "Vehicle" class
* Created by Nicola Ariutti, March 06, 2018
* CC0
*
* Inspired by
* The Nature of Code
* Daniel Shiffman
* http://natureofcode.com
*
* for reference see: https://natureofcode.com/book/chapter-6-autonomous-agents/
*
*/
#ifndef _NICOLAARIUTTI_VEHICLE
#define _NICOLAARIUTTI_VEHICLE
#include "Arduino.h"

class Vehicle
{
public:
	Vehicle() {};

  // Initial position, maxspeed, maxforce, damp distance.
  void init( float _x, float _ms, float _mf, float _damp );
  void changePhysics(float _ms, float _mf, float _damp);
  void update();
  void setTarget(float _target);
  float getPosition();

private:
  float position;
  float velocity;
  float acceleration;
  float target;

  float maxforce;    // Maximum steering force
  float maxspeed;    // Maximum speed
  float DAMPING_DISTANCE;
};
#endif
