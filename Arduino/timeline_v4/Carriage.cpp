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
}

void Carriage::update()
{
  if( state == STANDBY )
  {
    arsine->update();
    float y = arsine->getY();
    y = y * y;
    byte color;
    for(int i=0; i<nLeds; i++) {
      int distance = abs(i-ledPos);
      if(distance == 0) {
        color = y * 255.0;
      }
      else if( distance <= SIDELOBE ) {
        color = lookup[distance-1]*255.0 *y;
      }
      else {
        blue = 0;
      }
      strip->setPixelColor(i, color, color, color);
    }
    //strip->show();
    strip->show();
  }
  else if(state == ACTIVE && (millis() - prevTime) > TIMETOWAIT) {
    state = STANDBY;
    prevTime = millis();
  }
  else
  {
    // VEHICLE STUFF
    // calculate the scaled centroid

    vehicle->update();
    ledPos = vehicle->getPosition();
    //Serial.println(pos);

    // LED STUFF
    // The logic below is created in order to properly light-up
    // leds around the centroid according to the
    // squared raised-cosine lookup table.
    for(int i=0; i<nLeds; i++) {
      int distance = abs(i-ledPos);
      if(distance == 0) {
        blue = 255;
      }
      else if( distance <= SIDELOBE ) {
        blue = lookup[distance-1]*255.0;
      }
      else {
        blue = 0;
      }
      strip->setPixelColor(i, blue, blue, blue);
    }
    strip->show();
  }
}

void Carriage::setNewPos(int _newPos )
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

  vehicle->setTarget( ledPos );
}

void Carriage::debug()
{
  Serial.print("STATUS: ");
  if(state == 0)
    Serial.print("STANDBY");
  else if(state == 1)
    Serial.print("ACTIVE");
  Serial.print(", centroids: POS ");
  Serial.print(pos);
  Serial.print(", LEDs ");
  Serial.print(ledPos);
  Serial.println();
}
