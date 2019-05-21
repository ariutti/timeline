#include "Limulo_Animator.h"


// Set attack and release time in millisecond
// default are attack time (t_1) = 10ms and
// release time (t_2) = 500ms
void Animator_ASR::init( int t_1, int t_2 ) {
	t1 = t_1>0.0?t_1:10;
	t2 = t_2>0.0?t_2:500;
	state = QUIET;
}

void Animator_ASR::update()
{
	if(state == ATTACK)
	{
		dt = millis() - st;
		y =  (1.0*dt)/t1;
		if(y > 1.0)
		{
			y = 1.0;
			state = SUSTAIN;
		}
	}
	else if( state == RELEASE )
	{
		dt = millis() - st;
		y =  1.0 - ( (1.0*dt)/t2 );
		if(y < 0.0)
		{
			y = 0.0;
			state = QUIET;
		}
	}
}

void Animator_ASR::triggerAttack()
{
	if(state == SUSTAIN)
	{
		return;
	}

	state = ATTACK;
	st = millis();
}

void Animator_ASR::triggerRelease()
{
	if(state == QUIET)
	{
		return;
	}
	else if(state == SUSTAIN)
	{
		st = millis();
	}
	else if (state == ATTACK)
	{
		st = int( millis()*(1+((1.0*t2)/t1)) - t2*(1 + ((1.0*st)/t1)) );
	}
	state = RELEASE;
}

float Animator_ASR::getY()
{
	return y;
}

int Animator_ASR::getState()
{
	return state;
}
