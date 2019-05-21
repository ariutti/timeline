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
}

/************************************************************************************
 * LOOP
 ***********************************************************************************/
void loop()
{
  // TODO: Add here serial communication section in order
  // to discard the serial event callback function

  // LED stuff //////////////////////////////////////////////////////////
  //Serial.println();
  for(i=0; i<NLEDS; i++)
  {
    if(i == centroidLed)
      blue = 255;
    else 
      blue = 0;
    strip.setPixelColor(i, 0x00, 0x00, blue);
  }
  //Serial.println();
  strip.show();
  
  delay(DELAY_TIME); // put a delay so it isn't overwhelming
}

/************************************************************************************
 * SERIAL EVENT
 ***********************************************************************************/
void serialEvent()
{
  byte c = Serial.read();
  centroid = c;
  centroidLed = ((1.0 * centroid)/100.0) * NLEDS;
}
