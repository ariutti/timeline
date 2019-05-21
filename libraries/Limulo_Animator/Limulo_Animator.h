/*
* Animator.h - a library to create smooth linear variation
* on a value according to a customizable ASR envelope.
* Created by Nicola Ariutti, March 06, 2018
* TODO: license
*/
#ifndef _LIMULO_ANIMATOR
#define _LIMULO_ANIMATOR
#include "Arduino.h"

class Animator_ASR
{
public:
	Animator_ASR() {};

	void init( int t_1, int t_2 );
  void update();
  void triggerAttack();
  void triggerRelease();
  float getY();
	int getState();

private:

	int t1, t2;  // t attack, t release
	int dt;      // delta time
	int st;      // start time

  // states
  const int QUIET = -1;
  const int ATTACK  = 0;
  const int SUSTAIN  = 1;
  const int RELEASE = 2;
  int state;

  float y;
};
#endif
