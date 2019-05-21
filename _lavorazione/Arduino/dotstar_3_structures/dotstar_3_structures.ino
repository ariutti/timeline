/*
 * this sketch is meant to be used with an Adafruit DotStarLED strip.
 */


#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#define NUMPIXELS 46 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    4
#define CLOCKPIN   5
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);


// GREEN, RED, BLUE

#include <Limulo_Animator.h>

const int PPP = 5; //PIXELS PER PAD
const int NPADS = 9;
const int FADEIN_TIME = 500;
const int FADEOUT_TIME = 500;
// the following is a factor to correct the overall brightness
const float AMPLITUDE = 0.5;

struct ledblock {
  Animator_ASR asr;
  float rawAmp;
  uint8_t intAmp;
  uint8_t head, tail; // indexes of first/last pixel of the block
  uint8_t in, out; // indexes of the first/last pixel of the raised cosine animation
  uint8_t pivot;
};
ledblock blocks[NPADS];


// utility variables 
int i=0, j=0;

// SETUP //////////////////////////////////////////////////////////////////////////////
void setup() 
{
  Serial.begin(9600);

  for(i=0; i<NPADS; i++)
  {
    blocks[i].asr.init(FADEIN_TIME, FADEOUT_TIME);
    blocks[i].tail = i*PPP;
    blocks[i].head = blocks[i].tail + PPP - 1;   
    blocks[i].rawAmp = 0.0;
    blocks[i].intAmp = 0;
  }
  
  strip.begin(); // Initialize pins for output

  // set every pixel to sleep
  for(i=0; i<NPADS; i++) 
  {
    for(j=0; j<PPP; j++)
    {
      strip.setPixelColor(blocks[i].tail+j, 0x00, 0x00, 0x00);
    }
  }
  strip.show();  // Turn all LEDs off ASAP
}


// LOOP ///////////////////////////////////////////////////////////////////////////////
void loop() 
{
  Serial.println();
  for(i=0; i<NPADS; i++)
  {
    blocks[i].asr.update();
    blocks[i].rawAmp = blocks[i].asr.getY();
    // better to sqare the 0.0 - 1.0 value in order to get a smoother transition
    blocks[i].rawAmp = blocks[i].rawAmp * blocks[i].rawAmp;
    blocks[i].intAmp = 255.0*blocks[i].rawAmp * AMPLITUDE;
    Serial.print( blocks[i].rawAmp );Serial.print(" ");
    Serial.print( blocks[i].intAmp );Serial.print(" ");
    Serial.print( blocks[i].asr.getState() );Serial.print("\t");
    for(j=0; j<PPP; j++)
    {
      strip.setPixelColor(blocks[i].tail+j, 0x00, 0x00, blocks[i].intAmp);
    }
  }
  Serial.println();
  strip.show();
  
  delay(10);
}

void serialEvent()
{
  byte b = Serial.read();
  b = b - 48;
  //Serial.println( b );
  if(b >= NPADS)
    return;

  releaseAll();
  blocks[b].asr.triggerAttack();
}


void releaseAll()
{
  for(int i=0; i<NPADS; i++)
  {
    blocks[i].asr.triggerRelease();
  }  
}

