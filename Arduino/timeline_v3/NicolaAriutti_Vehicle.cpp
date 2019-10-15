#include "NicolaAriutti_Vehicle.h"


void Vehicle::init(float _x, float _ms, float _mf, int _damp)
{
	acceleration = 0.0;
	velocity = 0.0;
	position = _x;
	target = position;

	maxspeed = _ms;
	maxforce = _mf;
	DAMPING_DISTANCE = _damp;
}

// Method to update position
void Vehicle::update()
{
	float desired = target - position;  // A float pointing from the position to the target
	float d = abs(desired);

	if( d == 0.0 )
	{
		// we are already there
		position = target;
		return;
	}
	// Scale with arbitrary damping within 100 pixels
	else if (d < DAMPING_DISTANCE)
	{
		float m = map(d,0,DAMPING_DISTANCE,0,maxspeed);
		desired = (desired * m)/d;
	}
	else
	{
		desired = (desired * maxspeed)/d;
	}

	// Steering = Desired minus Velocity
	float steer = desired - velocity;
	if(abs(steer) >= maxforce)
		steer = (steer*maxforce)/abs(steer); // Limit to maximum steering force

	// We could add mass here if we want A = F / M
	acceleration += steer;

	// Update velocity
	velocity +=acceleration;
	// Limit speed
	if(abs(velocity) >= maxspeed)
		velocity = (velocity*maxspeed)/abs(velocity);
	// Update position
	position += velocity;
	// Reset accelerationelertion to 0 each cycle
	acceleration = 0.0;
}


// A method that calculates a steering force towards a target
// STEER = DESIRED MINUS VELOCITY
void Vehicle::setTarget(float _target)
{
	target = _target;
}


float Vehicle::getPosition()
{
	return position;
}
