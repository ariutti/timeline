/*
* This sketch is meant to work with the
* "capacitive_plotter" sketch for Processing.
* This sketch uses more than one MPR121 on the same 
* I2C bus.
*/

#include <Wire.h>
#include <Limulo_MPR121.h>


#define NMPR 1
#define NPADS 4
#define FIRST_MPR_ADDR 0x5A


struct mpr121 
{
  Limulo_MPR121 cap;
  uint8_t addr;
  // Save an history of the pads that have been touched/released
  uint16_t lasttouched = 0;
  uint16_t currtouched = 0;
  boolean bPadState[12];
};


// an array of mpr121! You can have up to 4 on the same i2c bus
mpr121 mpr[NMPR];


boolean bToPlotter;
int delayTime = 50;


/************************************************************************************
 * SETUP
 ***********************************************************************************/
void setup()
{
  // open the I2C communication
  Serial.begin(115200, SERIAL_8N1);

  // cycle through all the MPR
  for(int i=0; i<NMPR; i++)
  {
    mpr[i].cap = Limulo_MPR121();
    // mpr address can be one of these: 0x5A, 0x5B, 0x5C o 0x5D 
    mpr[i].addr = FIRST_MPR_ADDR + i;
    
    // Look for the MPR121 chip on the I2C bus
    if ( !mpr[i].cap.begin( mpr[i].addr ) )
    {
      //Serial.println("MPR121 not found, check wiring?");
      while (1);
    }

    // initialize the array of booleans
    for(int j=0; j<12; j++)
    {
      mpr[i].bPadState[j] = false;
    }
    
    // possibly initialize the mpr with some initial settings
    //mpr.setRising( 1, 8, 1, 0);
    //mpr.setFalling( 1, 1, 2, 1);
    //mpr.setTouched( 0, 0, 0);
    //mpr.setThresholds( 150, 50 );
    //mpr.setDebounces(1, 1);
  }

  bToPlotter=false;
}

/************************************************************************************
 * LOOP
 ***********************************************************************************/
void loop()
{
  // TODO: Add here serial communication section in order 
  // to discard the serial event callback function

  // cycle through all the MPR
  for(int i=0; i<NMPR; i++)
  {
    mpr[i].currtouched = mpr[i].cap.touched(); // Get the currently touched pads
    // cycle through all the electrodes
    for(int j=0; j<NPADS; j++)
    {
      if (( mpr[i].currtouched & _BV(j)) && !(mpr[i].lasttouched & _BV(j)) ) 
      {
        // pad 'i' has been touched
        //Serial.println("Pad 0:\ttouched!");
        mpr[i].bPadState[j] = true;
      }
      if (!(mpr[i].currtouched & _BV(j)) && (mpr[i].lasttouched & _BV(j)) ) 
      {
        // pad 'i' has been released
        //Serial.println("Pad 0 :\treleased!");
        mpr[i].bPadState[j] = false;
      }
    }
    // reset our state
    mpr[i].lasttouched = mpr[i].currtouched;
  
    // Send Serial data ////////////////////////////////////////////////////////////
    if(bToPlotter)
    {
      // Send data via serial:
      // 1. First send an address. Address byte have a 128 <= value < 255.
      //    Each address identifies a specific upcoming value;
      // 2. then send that value. Consider the value is a 10 bit but we must
      //    send it inside a 7bit packet. So made 3 right bit shift (/8). 
      //    This way we will loose some precision but this is not important.
  
      // cycle all the electrodes
      for(int j=0; j<NPADS; j++) 
      {
        int filt = mpr[i].cap.filteredData(j);
        int base = mpr[i].cap.baselineData(j);
        byte b = (1<<7) | (i<<5) | (j<<1) | mpr[i].bPadState[j];
        Serial.write(b); // send address & touched
        Serial.write(base / 8); // send base value
        Serial.write(filt / 8); // send filtered value
      }
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
  {
    // reset all the MPR
    for(int i=0; i<NMPR; i++)
      mpr[i].cap.reset();
  }
}
