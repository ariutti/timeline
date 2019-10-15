#include "Carriage.h"

void Carriage::init(Adafruit_NeoPixel *_strip, int _nVirtPads)
{
  strip = _strip;
  nVirtPads = _nVirtPads;

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
  // Initial position, maxspeed, maxforce, damp distance.
  centroidPos = 0.5*nLeds;
  vehicle->init( centroidPos, 0.5, 0.1, 1.5);
  //int initialPos = (((7 * 1.0)/nVirtPads) * (nLeds) );
  
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
      int distance = abs(i-centroidPos);
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
    centroidPos = vehicle->getPosition();
    //Serial.println(pos);
    
    // LED STUFF 
    // The logic below is created in order to properly light-up
    // leds around the centroid according to the 
    // squared raised-cosine lookup table.
    for(int i=0; i<nLeds; i++) {
      int distance = abs(i-centroidPos);
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

void Carriage::setNewPos(int _selectedVirtualPad ) 
{
  // time
  state = ACTIVE;
  prevTime = millis();
  
  // if the touch object is not EMPTY
  if( _selectedVirtualPad != -1) {
    selectedVirtualPad = _selectedVirtualPad;
    centroidPos = (((1.0 *  selectedVirtualPad)/nVirtPads) * (nLeds - 2*SIDELOBE +1))+SIDELOBE;
    vehicle->setTarget( centroidPos );
  }
}

void Carriage::debug() {
  Serial.print("STATUS: ");
  if(state == 0)
    Serial.print("STANDBY");
  else if(state == 1)
    Serial.print("ACTIVE");
  Serial.print(", centroids: PAD ");
  Serial.print(selectedVirtualPad);
  Serial.print(", LEDs ");
  Serial.print(centroidPos);
  Serial.println();
}
