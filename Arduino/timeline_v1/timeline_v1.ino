/*
 * 2019/05/21 - Nicola Ariutti
 * Using 7 capacitive pads via MPR121
 * and a DotStar LED strip.
 */

#include "Carriage.h"
Carriage carriage;


/* CAPACITIVE STUFF ****************************************************************/
#include <Wire.h>
#include "Limulo_MPR121.h"

#define NMPR 1
#define NPADS 7
#define FIRST_MPR_ADDR 0x5A
const int NVIRTUALPADS = (NPADS*2)-1;

struct mpr121
{
  Limulo_MPR121 cap;
  uint8_t addr;
  // Save an history of the pads that have been touched/released
  uint16_t lasttouched = 0;
  uint16_t currtouched = 0;
  boolean bPadState[12];
  uint16_t oor=0;
};

// an array of mpr121! You can have up to 4 on the same i2c bus
mpr121 mpr[NMPR];

// utility variables to save values readed from MPR
uint16_t filt;
uint16_t base;
byte b;
int oor;

#include "Touch.h"
Touch touchObj;

/* DEBUG ***************************************************************************/
boolean bToPlotter = false;
boolean b2VVVV     = false;
boolean DEBUG      = true;

/* LED STUFF ***********************************************************************/
// this is the order of the colors GREEN, RED, BLUE
#include "Adafruit_DotStar.h"
#include <SPI.h>         // see Adafruit DotStar library example to better understand

#define NLEDS 36 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    2
#define CLOCKPIN   3
Adafruit_DotStar strip = Adafruit_DotStar(NLEDS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);


/* UTILITY STUFF *******************************************************************/
int i=0, j=0;
const int DELAY_TIME = 10;

// SETUP ////////////////////////////////////////////////////////////////////////////

void setup()
{
  pinMode(13, OUTPUT); digitalWrite(13, LOW); // turn off the annoying LED
  
  // open the serial communication
  Serial.begin(9600, SERIAL_8N1);

  if(DEBUG) {
    Serial.print("NPADS: ");
    Serial.print(NPADS);
    Serial.print(", NVIRTUALPADS: ");
    Serial.print(NVIRTUALPADS);
    Serial.println(";");    
  }

  // CAPACITIVE STUFF **************************************************************/
  // cycle through all the MPR
  for(int i=0; i<NMPR; i++)
  {
    mpr[i].cap = Limulo_MPR121();
    // mpr address can be one of these: 0x5A, 0x5B, 0x5C o 0x5D
    mpr[i].addr = FIRST_MPR_ADDR + i;

    // Look for the MPR121 chip on the I2C bus
    if(DEBUG) Serial.println("Looking for MPRs!");
    if ( !mpr[i].cap.begin( mpr[i].addr ) )
    {
      if(DEBUG) Serial.println("MPR121 not found, check wiring?");
      while(1);
    }
    if(DEBUG) Serial.println("MPR found!");
    
    // initialize the array of booleans
    for(int j=0; j<12; j++) {
      mpr[i].bPadState[j] = false;
    }

    // possibly initialize the mpr with some initial settings
    mpr[i].cap.setUSL(201);
    mpr[i].cap.setTL(180);
    mpr[i].cap.setLSL(130);
    
    // First Filter Iteration
    // Second Filter Iteration
    // Electrode Sample Interval
    // NOTE: the system seems to behave better if
    // these value are more than 0 
    mpr[i].cap.setFFI_SFI_ESI(1, 1, 1);  // See AN3890


    // MHD, NHD, NCL, FDL
    mpr[i].cap.setFalling( 1, 1, 1, 1 );
    mpr[i].cap.setRising( 1, 1, 1, 1 );
    // if touch timing is too short there will be a moment in which 
    // an automatic release will be interpreted as a user release 
    // so a change in the pad index :(
    mpr[i].cap.setTouched( 1, 1, 32 );
    mpr[i].cap.setThresholds( 18, 9);
    mpr[i].cap.setDebounces(2, 2);

    // initial reset of MPR struct fields
  }

  carriage.init(&strip, NVIRTUALPADS);
}


// LOOP /////////////////////////////////////////////////////////////////////////////

void loop()
{
  // TODO: Add here serial communication section in order
  // to discard the serial event callback function


  // CAPACITIVE STUFF **************************************************************/
  for(int i=0; i<NMPR; i++)  // cycle through all the MPR
  {
    // Get the currently touched pads
    mpr[i].currtouched = mpr[i].cap.touched();
   
    if( mpr[i].currtouched != mpr[i].lasttouched ) 
    {
      // do the following only if something has changed
      //Serial.println(mpr[i].currtouched, BIN);
      
      // interpolated Pad is the virtual indexes we obtain from the touch obj.
      int interpolatedPad = -1;
      for(int j=0; j<NPADS; j++) // cycle through all the electrodes
      {
        if (( mpr[i].currtouched & _BV(j)) && !(mpr[i].lasttouched & _BV(j)) )
        {
          // pad 'j' has been touched
          mpr[i].bPadState[j] = true;

          touchObj.addIndex(j);

          interpolatedPad = touchObj.getInterpolatedIndex();
          // if the touch object is not EMPTY
          carriage.setNewPos( interpolatedPad );
          if( DEBUG ) {
            carriage.debug();
          }
        }
        else if (!(mpr[i].currtouched & _BV(j)) && (mpr[i].lasttouched & _BV(j)) )
        {
          // pad 'i' has been released
          mpr[i].bPadState[j] = false;
          
          touchObj.removeIndex(j);
          
          interpolatedPad = touchObj.getInterpolatedIndex();

          carriage.setNewPos( interpolatedPad );
          if( DEBUG ) {
            carriage.debug();
          }
        }
      }
      // reset our state
      mpr[i].lasttouched = mpr[i].currtouched;
    }
    
    mpr[i].oor = mpr[i].cap.getOOR();
    
    // SEND DATA TO PROCESSING *****************************************************/
    if(bToPlotter) {
      sendDataToProcessingPlotter(i);
    }
    //mpr[i].cap.printOOR(); // added for debug purposes
  }

  carriage.update();

  delay(DELAY_TIME); // put a delay so it isn't overwhelming
}

// SERIAL EVENT /////////////////////////////////////////////////////////////////////

void serialEvent()
{
  byte c = Serial.read();
  if (c == 'o')
    bToPlotter = true;
  else if (c == 'c')
    bToPlotter = false;
  else if (c == 'r')
  {
    // reset all the MPR
    for(int i=0; i<NMPR; i++)
      mpr[i].cap.reset();
  }
}

// SEND DATA TO PROCESSING PLOTTER //////////////////////////////////////////////////
void sendDataToProcessingPlotter( int mprIndex )
{
  
  // Send data via serial:
  // 1. First send a byte containing the address of the mpr + the address of the pad +
  //    the 'touched' status of the pad; This byte has a value greater than 127 by convention;
  // 2. Then send two more bytes for 'baseline' and 'filtered' respectively. 
  //    Because these values are 10bit values and we must send them
  //    inside a 7bit packet, we must made a 3 times bit shift to the right (/8).
  //    This way we will loose some precision but this is not important.
  //    This two other bytes have values lesser than 127 by convention.

  // cycle all the electrodes
  for(int j=0; j<NPADS; j++)
  {
    filt = mpr[ mprIndex ].cap.filteredData(j);
    base = mpr[ mprIndex ].cap.baselineData(j);
    b = (1<<7) | (i<<5) | (j<<1) | mpr[ mprIndex ].bPadState[j];
    Serial.write(b); // send address & touched
    Serial.write(base / 8); // send base value
    Serial.write(filt / 8); // send filtered value
  }
}


/*
void sendDataToProcessingPlotter()
{

// ### NEW COMMUNICATION PROTOCOL (19-02-2018) ###
  //
  // Send data via serial:
  // 1. 'Status Byte': first we send a byte containing the address of the mpr.
  //    The most significant bit of the byte is 1 (value of the byte is > 127).
  //    This is a convention in order for the receiver program to be able to recognize it;
  // 2. Then we send 'Data Bytes'. The most significant bit of these bytes is 
  //    always 0 in order to differenciate them from the status byte.
  //    We can send as many data bytes as we want. The only thing to keep in mind
  //    is that we must be coherent the receiver side in order not to create confusion
  //    in receiving the data.
  //
  //    For instance we can send pais of byte containing the 'baseline' and 'filtered'
  //    data for each mpr pad. 
  //
  //    We can also use data bytes for sending information as:
  //    * 'touched' register;
  //    * 'oor' register;
  
  
  // 1. write the status byte containing the mpr addr
  b = (1<<7) | i;
  Serial.write(b);
  // 2. write 'touched' register
  b = mpr[i].currtouched & 0x7F; 
  Serial.write(b); //touch status: pad 0 - 6
  b = (mpr[i].currtouched>>7) & 0x7F;
  Serial.write(b); //touch status: pad 7 - 12 (eleprox)
  // 3. write 'oor' register
  b = mpr[i].oor & 0x7F; 
  Serial.write(b); //oor status: pad 0 - 6
  b = (mpr[i].oor>>7) & 0x7F;
  Serial.write(b); //oor status: pad 7 - 12 (eleprox)
  
  // Cycle all the electrodes and send pairs of
  // 'baseline' and 'filtered' data. Mind the bit shifting!
  for(int j=0; j<NPADS; j++)
  {
    base = mpr[i].cap.baselineData(j); 
    filt = mpr[i].cap.filteredData(j);
    Serial.write(base>>3); // baseline is a 10 bit value
    Serial.write(filt>>3); // sfiltered is a 10 bit value
  }
}
*/
