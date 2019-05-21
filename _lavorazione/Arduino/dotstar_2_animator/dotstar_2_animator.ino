#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#define NUMPIXELS 46 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    4
#define CLOCKPIN   5
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

#include <Limulo_Animator.h>
Animator_ASR myAsr[2];
int amplitude[2];

uint8_t r, g, b;
const int PPP = 5; //PIXELS PER PAD
const int NUMPAD = 9;
uint8_t pad = 1;
uint8_t head, tail, pivot;
int in; // the index of the first pixel of the raised cosine animation

// fill a lookup table of raised cosine values
// sampled at specific interval
float raisedCos[PPP*2 + 1];

uint32_t color;

// SETUP //////////////////////////////////////////////////////////////////////////////
void setup() 
{
  Serial.begin(9600);
  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  for(int i=0; i<(PPP*2 + 1); i++)
  {
    raisedCos[i] = 0.5 * (1- cos( (i*2*PI)/(PPP*2) ));
    Serial.print(raisedCos[i]); Serial.print(", ");
  }

  myAsr[0].init(2000, 1000);
  myAsr[1].init(2000, 1000);
  
}


// LOOP ///////////////////////////////////////////////////////////////////////////////
void loop() 
{
  myAsr[0].update();
  myAsr[1].update();
  amplitude[0] = (int) (255*myAsr[0].getY());
  amplitude[1] = (int) (255*myAsr[1].getY());

  // calculate position of the rised cosine
  head = PPP*(pad+1)-1;
  tail = head - PPP +1;
  pivot = tail + (PPP/2);
  in = tail - (PPP/2 + 1);
  
  for(int i=0; i<(PPP*2 + 1); i++)
  {
    color = strip.getPixelColor(in+i);
    b = (color)     & 0xFF;
    g = (color>>8)  & 0xFF;
    r = (color>>16) & 0xFF;
    // TODO: here we have a quite complicated task to solve.
    // we must mix 3 different contributions by consecutive pads.
    //strip.setPixelColor(in+i, amplitude[0]*raisedCos[i] + , 0x00, 0x00);
  }
  strip.show();
  
  delay(10);
}

void serialEvent()
{
  byte b = Serial.read();
  if( b == '1')
  {
    bSecondPad = !bSecondPad
    if(bSecondPad)
      myAsr[0].triggerAttack();
    else
      myAsr[0].triggerRelease();
  }
  else if (b == '2')
  {
    bSecondPad = !bSecondPad
    if(bSecondPad)
      myAsr[1].triggerAttack();
    else
      myAsr[1].triggerRelease();
  
  }
}

