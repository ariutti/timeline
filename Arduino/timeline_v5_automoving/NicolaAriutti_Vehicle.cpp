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
void Vehicle::update() {
	// A float (vector) pointing from the current pos to the target pos
	float desired = target - position;

	if( abs(desired) == 0.0 )
	{
		// we are already there
		position = target;
		return;
	}
	// Scale with arbitrary damping within DAMPING_DISTANCE "pixels"
	// This way we are controlling the vehicle slowdown.
	else if ( abs(desired) < DAMPING_DISTANCE)
	{
		float m = map( abs(desired), DAMPING_DISTANCE, 0, maxspeed, 0 );
		// We do this instead of simply
		// desired = m
		// in order to maintain the sign
		desired = (desired * m)/abs(desired);

	}
	else
	{
		desired = (desired * maxspeed)/abs(desired);
	}

	// Calculating a steer (steering force) towards the target
	// STEER = DESIRED - VELOCITY
	float steer = desired - velocity;
	if(abs(steer) >= maxforce) {
		// We do this instead of simply
		// steer = maxforce
		// in order to maintain the sign
		steer = (steer*maxforce)/abs(steer); // Limit to maximum steering force
	}

	// We could add mass here if we want A = F / M
	acceleration += steer;
	// Update velocity
	velocity += acceleration;

	// Limit speed
	if(abs(velocity) >= maxspeed) {
		// We do this instead of simply
		// velocity = maxspeed
		// in order to maintain the sign
		velocity = (velocity*maxspeed)/abs(velocity);
	}
	// Update position
	position += velocity;

	// Reset acceleration to 0 each cycle
	acceleration = 0.0;
}

void Vehicle::setTarget(float _target) {
	target = _target;
}

float Vehicle::getPosition() {
	return position;
}
