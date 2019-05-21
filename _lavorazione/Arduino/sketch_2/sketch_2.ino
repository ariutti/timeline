/*
* This sketch is meant to work with the
* "capacitive_plotter" sketch for Processing.
*/

#include <Wire.h>
#include <Limulo_MPR121.h>

#define NPADS 1
boolean bPadState[1];

// You can have up to 4 on one i2c bus but one is enough for testing!
Limulo_MPR121 cap = Limulo_MPR121();

// Keeps track of the last pins touched so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

boolean bToPlotter;
int delayTime = 50;

/************************************************************************************
 * SETUP
 ***********************************************************************************/
void setup()
{
  Serial.begin(9600, SERIAL_8N1);
  //Serial.println("Adafruit MPR121 Capacitive Touch sensor test");

  if (!cap.begin(0x5A))
  {
    //Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  //Serial.println("MPR121 found!");

  // MPR121 initial settings
  //cap.setRising( 1, 8, 1, 0);
  //cap.setFalling( 1, 1, 2, 1);
  //cap.setTouched( 0, 0, 0);
  //cap.setThresholds( 150, 50 );
  //cap.setDebounces(1, 1);

  bToPlotter=false;

  // initialize the array of booleans
	for(int i=0; i<NPADS; i++)
	{
		bPadState[i] = false;
	}
}

/************************************************************************************
 * LOOP
 ***********************************************************************************/
void loop()
{
  // Add here serial communication section in order 
  // to discard the serial event callback function

  
  currtouched = cap.touched(); // Get the currently touched pads

  for(int i=0; i<NPADS; i++)
  {
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) 
    {
      // pad 'i' has been touched
      //Serial.println("Pad 0:\ttouched!");
      bPadState[i] = true;
    }
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) 
    {
      // pad 'i' has been released
      //Serial.println("Pad 0 :\treleased!");
      bPadState[i] = false;
    }
  }
  // reset our state
  lasttouched = currtouched;

  // Send Serial data ////////////////////////////////////////////////////////////

  //int delta= base - filt;

  if(bToPlotter)
  {
    // Send data via serial:
		// 1. First send an address. Address byte have a 128 <= value < 255.
    //    Each address identifies a specific upcoming value;
    // 2. then send that value. Consider the value is a 10 bit but we must
    //    send it inside a 7bit packet. So made 3 right bit shift (/8). 
    //    This way we will loose some precision but this is not important.

    // First electrode:
    for(int i=0; i<NPADS; i++) 
    {
      int filt = cap.filteredData(i);
      int base = cap.baselineData(i);
      int ADDR_BASE = i*3; 
      int ADDR_FILT = i*3 + 1;
      int ADDR_TOUCH= i*3 + 2;
      Serial.write(ADDR_BASE  + 128);
      Serial.write(base / 8);
      Serial.write(ADDR_FILT  + 128);
      Serial.write(filt / 8);
      Serial.write(ADDR_TOUCH + 128);
      Serial.write(bPadState[i]);
    }
  }

	delay(delayTime); // put a delay so it isn't overwhelming
}

/************************************************************************************
 * SERIAL EVENT
 ***********************************************************************************/
void serialEvent()
{
  byte b = Serial.read();
  if (b == 'o')
    bToPlotter = true;
  else if (b == 'c')
    bToPlotter = false;
  else if (b == 'r')
    cap.reset();
}
