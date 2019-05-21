#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#define NUMPIXELS 46 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    4
#define CLOCKPIN   5
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
// The last parameter is optional -- this is the color data order of the
// DotStar strip, which has changed over time in different production runs.
// Your code just uses R,G,B colors, the library then reassigns as needed.
// Default is DOTSTAR_BRG, so change this if you have an earlier strip.

// Hardware SPI is a little faster, but must be wired to specific pins
// (Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
//Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);


// SETUP //////////////////////////////////////////////////////////////////////////////
void setup() 
{
  Serial.begin(9600);
  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP
}

uint8_t r, g, b;

const int PIXELPERPAD = 5;
const int NUMPAD = 9;
uint8_t pad = 0;
uint8_t head, tail;

// LOOP ///////////////////////////////////////////////////////////////////////////////
void loop() 
{

  if(++pad>=NUMPAD)
    pad = 0;

  head = PIXELPERPAD*(pad+1)-1;
  tail = head - PIXELPERPAD +1;
  Serial.print( pad ); Serial.print( ": head: " );
  Serial.print( head ); Serial.print( ", tail: " );
  Serial.print( tail ); Serial.println();
  
  for(int i=0; i<PIXELPERPAD; i++)
  {
    strip.setPixelColor(head-i, 0x22, 0x00, 0x00);
  }
  strip.show();
  
  delay(500);
  strip.clear();
  delay(20);
}
