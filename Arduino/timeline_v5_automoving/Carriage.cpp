#include "Carriage.h"

void Carriage::init(Adafruit_NeoPixel *_strip, int _nPositions)
{
  strip = _strip;
  nPositions = _nPositions;

  // fill the lookup table
  for(int i=0; i<SIDELOBE; i++) {
    int x = i+1;
    lookup[i] = 0.5 * (cos ( (PI*x)/SIDELOBE ) + 1 );
    lookup[i] = lookup[i]*lookup[i];
  }

  nLeds = strip->numPixels();

  // LED STUFF *********************************************************************/
  strip->begin(); // Initialize pins for output

  // set every pixel to sleep
  for(int i=0; i<nLeds; i++) {
    strip->setPixelColor(i, 0x00, 0x00, 0x00);
  }
  strip->show();  // Turn all LEDs off ASAP

  // ANIMATION / VEHICLE ***********************************************************/
  // We must calculate this parameter with extreme precision
  // also taking into account the number of leds in the strip
  // and the DELAY_TIME used for each loop cycle.
  vehicle = new Vehicle();

  ledPos = 0.5*nLeds;
  // Initial position, maxspeed, maxforce, damp distance.
  vehicle->init( ledPos, 0.5, 0.1, 1.5);
  //int initialPos = (((7 * 1.0)/nPositions) * (nLeds) );

  arsine = new Animator_Sine();
  arsine->init(1, 0.0);

	// 2020-07-12: does it make sense to do this?
	prevTime = millis();
}


/*
void Carriage::update()
{
	//STATE MACHINE Management
	if( state == AUTO )
	{
		// here we have a routine to automatically move the carriage
		// every TIME_TO_REACH milliseconds
		if( (millis()-prevTime > TIME_TO_REACH)) {
			setNewAutoPos();
		}

    arsine->update();
    float y = arsine->getY();
    y = y * y;

    brightness = MAX_BRIGHTNESS * y;

		// VEHICLE STUFF
		// calculate the scaled centroid
		vehicle->update();
		ledPos = vehicle->getPosition();

		//brightness = MID_BRIGHTNESS;
  }
  else if(state == ACTIVE)
	{
		if( (millis() - prevTime) > TIME_TO_STANDBY) {
	    state = AUTO;
	    prevTime = millis();
      arsine->reset();
      // maxspeed, maxforce, damp distance
      //vehicle->changePhysics( 0.5, 0.05, 10 );
      setNewAutoPos();
		}
		else
		{
			// if we are here it means we are in an active state and
			// we should update hte carriage position an retrive the
			// led position from it according to its movements

	    // VEHICLE STUFF
	    // calculate the scaled centroid
	    vehicle->update();
	    ledPos = vehicle->getPosition();
	    //Serial.println(pos);

			brightness = MAX_BRIGHTNESS;
		}
	}

	// LED STUFF
	// The logic below is created in order to properly light-up
	// leds around the centroid according to the
	// squared raised-cosine lookup table.
	byte color;
	for(int i=0; i<nLeds; i++) {
		int distance = abs(i-ledPos);
		if(distance == 0) {
			color = 255.0 * brightness;
		}
		else if( distance <= SIDELOBE ) {
			color = lookup[distance-1] * 255.0 * brightness;
		}
		else {
			color = 0.0 * brightness;
		}
		strip->setPixelColor(i, color, color, color);
	}
	strip->show();
}
*/


 void Carriage::update()
{
 //STATE MACHINE Management
  if( state == AUTO )
  {
    // here we have a routine to automatically move the carriage
    // every TIME_TO_REACH milliseconds
    if( (millis()-prevTime > TIME_TO_REACH)) {
      setNewAutoPos();
    }

    // LED STUFF
    arsine->update();
    float y = arsine->getY();
    y = y * y;

    // VEHICLE STUFF
    // calculate the scaled centroid
    vehicle->update();
    ledPos = vehicle->getPosition();

    brightness = MID_BRIGHTNESS * y;
    //brightness = MID_BRIGHTNESS;
  }
  else if( state == STANDBY )
  {
    if( (millis() - prevTime) > TIME_TO_AUTO ) {
      // if we are here it means we have spent enough time
      // in the STANBY mode, we should pass to the AUTO mode
      state = AUTO;
      prevTime = millis(); // superfluous
      // maxspeed, maxforce, damp distance
      //vehicle->changePhysics( 0.5, 0.05, 10 );
      setNewAutoPos();
      return;
    }
    else
    {
      // if we are here it means we are in STANBY mode and
      // TIME_TO_AUTO isn't passed yet so we can keep
      // animate the LEDs with the "brightness breathing" fx.
      arsine->update();
      float y = arsine->getY();
      y = y * y;

      brightness = MAX_BRIGHTNESS * y;
    }
  }
  else if(state == ACTIVE)
  {
    if( (millis() - prevTime) > TIME_TO_STANDBY) {
      // if we are here it means there has been touch inactivity
      // for a while (TIME_TO_STANDBY), so we pass to the next state
      // which is STANDBY
      state = STANDBY;
      prevTime = millis();
      arsine->reset();
      return;
    }
    else
    {
      // if we are here it means we are in an active state and
      // we should update hte carriage position an retrive the
      // led position from it according to its movements

      // VEHICLE STUFF
      // calculate the scaled centroid
      vehicle->update();
      ledPos = vehicle->getPosition();
      //Serial.println(pos);

      brightness = MAX_BRIGHTNESS;
    }
  }

  // LED STUFF
  // The logic below is created in order to properly light-up
  // leds around the centroid according to the
  // squared raised-cosine lookup table.
  byte color;
  for(int i=0; i<nLeds; i++) {
    int distance = abs(i-ledPos);
    if(distance == 0) {
      color = 255.0 * brightness;
    }
    else if( distance <= SIDELOBE ) {
      color = lookup[distance-1] * 255.0 * brightness;
    }
    else {
      color = 0.0 * brightness;
    }
    strip->setPixelColor(i, color, color, color);
  }
  strip->show();
}

/*
void Carriage::update()
{
 //STATE MACHINE Management
  if( state == AUTO )
  {
    // here we have a routine to automatically move the carriage
    // every TIME_TO_REACH milliseconds
    if( (millis()-prevTime > TIME_TO_REACH)) {
      setNewAutoPos();
    }

    // VEHICLE STUFF
    // calculate the scaled centroid
    vehicle->update();
    ledPos = vehicle->getPosition();

    brightness = MID_BRIGHTNESS;
  }
  else if( state == STANDBY )
  {
    if( (millis() - prevTime) > TIME_TO_AUTO ) {
      // if we are here it means we have spent enough time
      // in the STANBY mode, we should pass to the AUTO mode
      state = AUTO;
      prevTime = millis(); // superfluous
      // maxspeed, maxforce, damp distance
      vehicle->changePhysics( 0.5, 0.05, 10 );
    }
    else
    {
      // if we are here it means we are in STANBY mode and
      // TIME_TO_AUTO isn't passed yet so we can keep
      // animate the LEDs with the "brightness breathing" fx.
      arsine->update();
      float y = arsine->getY();
      y = y * y;

      brightness = MAX_BRIGHTNESS * y;
    }
  }
  else if(state == ACTIVE)
  {
    if( (millis() - prevTime) > TIME_TO_STANDBY) {
      // if we are here it means there has been touch inactivity
      // for a while (TIME_TO_STANDBY), so we pass to the next state
      // which is STANDBY
      state = STANDBY;
      prevTime = millis();
      arsine->reset();
    }
    else
    {
      // if we are here it means we are in an active state and
      // we should update hte carriage position an retrive the
      // led position from it according to its movements

      // VEHICLE STUFF
      // calculate the scaled centroid
      vehicle->update();
      ledPos = vehicle->getPosition();
      //Serial.println(pos);

      brightness = MAX_BRIGHTNESS;
    }
  }

  // LED STUFF
  // The logic below is created in order to properly light-up
  // leds around the centroid according to the
  // squared raised-cosine lookup table.
  byte color;
  for(int i=0; i<nLeds; i++) {
    int distance = abs(i-ledPos);
    if(distance == 0) {
      color = 255.0 * brightness;
    }
    else if( distance <= SIDELOBE ) {
      color = lookup[distance-1] * 255.0 * brightness;
    }
    else {
      color = 0.0 * brightness;
    }
    strip->setPixelColor(i, color, color, color);
  }
  strip->show();
}
 */



void Carriage::setNewPos( int _newPos )
{
  // time
  state = ACTIVE;
  prevTime = millis();

  pos = _newPos;

  // don't be afraid by the following formula. It is only a way to prevent
  // one of the sidelobes from disappearing under the console cover when
  // the carriage is at its extreme position on its "Pads scale".
  // We are compressing the Led scale a bit to prevent this behaviour.
  //ledPos = (((1.0 * pos)/nPositions) * (nLeds - 2*SIDELOBE +1))+SIDELOBE;

  // if you prefer you can always use the classical style:
  ledPos = ((1.0 * pos)/nPositions) * nLeds;

  //2020-07-14
  // update the physics of vehicle movement here
  // maxspeed, maxforce, damp distance
  //vehicle->changePhysics( 0.5, 0.1, 1.5 );
  vehicle->setTarget( ledPos );
}


void Carriage::setNewAutoPos( ) {
	float A = random(0, nPositions);
	prevTime = millis();

	// don't be afraid by the following formula. It is only a way to prevent
  // one of the sidelobes from disappearing under the console cover when
  // the carriage is at its extreme position on its "Pads scale".
  // We are compressing the Led scale a bit to prevent this behaviour.
  //ledPos = (((1.0 * pos)/nPositions) * (nLeds - 2*SIDELOBE +1))+SIDELOBE;

  // if you prefer you can always use the classical style:
  int B = int( (A/nPositions) * nLeds);

  vehicle->setTarget( B );
}


void Carriage::debug()
{
  Serial.print("STATUS: ");
  if(state == 0)
    Serial.print("AUTO");
  else if(state == 1)
    Serial.print("ACTIVE");
  else if(state == 2)
   Serial.print("STANDBY");
  Serial.print(", centroids: POS ");
  Serial.print(pos);
  Serial.print(", LEDs ");
  Serial.print(ledPos);
  Serial.println();
}
