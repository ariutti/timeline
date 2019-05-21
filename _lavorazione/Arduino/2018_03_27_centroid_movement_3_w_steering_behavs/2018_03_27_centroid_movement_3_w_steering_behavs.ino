/*
 * 2018/03/28
 * This sketch is meant to be used with the following
 * PureData sketch (using the [comport] object:

#N canvas 746 274 450 300 10;
#X msg 92 79 close;
#X msg 125 122 ports;
#X obj 92 149 comport 1 9600;
#X obj 92 50 loadbang;
#X obj 29 20 hsl 128 15 0 99 0 0 empty empty empty -2 -8 0 10 -262144
-1 -1 0 1;
#X floatatom 26 40 5 0 0 0 - - -, f 5;
#X obj 187 199 print;
#X obj 187 177 route ports;
#X msg 113 100 open 3;
#X obj 265 87 hradio 15 1 0 9 empty empty empty 0 -8 0 10 -262144 -1
-1 2;
#X floatatom 265 107 5 0 0 0 - - -, f 5;
#X connect 0 0 2 0;
#X connect 1 0 2 0;
#X connect 2 1 7 0;
#X connect 3 0 0 0;
#X connect 4 0 5 0;
#X connect 5 0 2 0;
#X connect 7 0 6 0;
#X connect 8 0 2 0;
#X connect 9 0 10 0;
#X connect 10 0 2 0;

 */

#define NPADS 9

// LED stuff
#include <Adafruit_DotStar.h>
#include <SPI.h>     // see Adafruit DotStar library example to better understand

#define NLEDS 46 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    4
#define CLOCKPIN   5
Adafruit_DotStar strip = Adafruit_DotStar(NLEDS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

const int DELAY_TIME = 10;
int centroid = 0;     // an integer from 0 to 100
int centroidLed = 0;  // expresses value scaled to the number of pixels 
int blue = 0;

// utility variables 
int i=0, j=0;


// must define sidelobe dimension first
const int SIDELOBS = 4;
// the we declare an array that will work as a lookup table
// for the squared-raised-cosine figure.
float lookup [SIDELOBS];

#include <NicolaAriutti_Vehicle.h>
NicolaAriutti_Vehicle vehicle;


/************************************************************************************
 * SETUP
 ***********************************************************************************/
void setup()
{
  pinMode(13, OUTPUT); digitalWrite(13, LOW); // turn off the annoying LED
  
  // open the serial communication
  Serial.begin(9600, SERIAL_8N1);

  strip.begin(); // Initialize pins for output

  // set every pixel to sleep
  for(i=0; i<NLEDS; i++) 
  {
    strip.setPixelColor(i, 0x00, 0x00, 0x00);
  }
  strip.show();  // Turn all LEDs off ASAP 

  // fill the lookup table
  for(i=0; i<SIDELOBS; i++)
  {
    int x = i+1;
    lookup[i] = 0.5 * (cos ( (PI*x)/SIDELOBS ) + 1 );
    lookup[i] = lookup[i]*lookup[i];
  } 


  // We must calculate this parameter with extreme precision
  // also taking into account the number of leds in the strip
  // and the DELAY_TIME used for each loop cylce.
  // Initial position, maxspeed, maxforce, damp distance.
  vehicle.init(0, 0.5, 0.1, 2);
}

/************************************************************************************
 * LOOP
 ***********************************************************************************/
void loop()
{
  
  // TODO: Add here serial communication section in order
  // to discard the serial event callback function

  // VEHICLE STUFF //////////////////////////////////////////////////////
  vehicle.update();
  int pos = vehicle.getPosition();
  //Serial.println(pos);

  // LED stuff //////////////////////////////////////////////////////////
  // The logic below is created in order to properly light-up
  // leds around the centroid according to the 
  // squared raised-cosine lookup table.
  for(i=0; i<NLEDS; i++)
  {
    int distance = abs(i-pos);
    if(distance == 0)
    {
      blue = 255;
    }
    
    else if( distance <= SIDELOBS )
    {
      blue = lookup[distance-1]*255.0;
    }
    else
    {
      blue = 0; 
    }
    strip.setPixelColor(i, 0x00, 0x00, blue);
  }
  
  strip.show();
  
  delay(DELAY_TIME); // put a delay so it isn't overwhelming
}

/************************************************************************************
 * SERIAL EVENT
 ***********************************************************************************/
void serialEvent()
{
  byte c = Serial.read();
  //centroidLed = ((1.0 * c)/NPADS) * NLEDS;
  vehicle.setTarget(c);
  //Serial.println(centroidLed);
}
