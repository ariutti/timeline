/*
 * 2019/10/02 - Nicola Ariutti
 * Using 4 MPR121 to detect proximity on 25 Pads
 * Also Using a NeoPixel LED strip.
 */

/* CAPACITIVE STUFF ****************************************************************/
#include <Wire.h>
#include "Limulo_MPR121.h"

#define FIRST_MPR_ADDR 0x5A
const int  NMPR = 4;
const int  NPADS[] = {7, 6, 6, 6};
// virtual pads number will be calculated inside setup function
int NVIRTUALPADS = 0;
int padIndex, virtualPadIndex;

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

/* TOUCH LOGIC STUFF ***************************************************************/
#include "Touch.h"
Touch touchObj;
int oldIdx = -1;
int newIdx = -1;
// wait some time before sending data to VVVV
// in order stabilize the touch.
unsigned long waitToSend = 10;
unsigned long startWaiting;
boolean bTouchStabilized = false;


/* LED STUFF ***********************************************************************/
// TODO: change here to adapt the code for NeoPixel LED Strip
// this is the order of the colors GREEN, RED, BLUE
#include "Adafruit_NeoPixel.h"

#define NLEDS 68 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define LEDPIN    4
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NLEDS, LEDPIN, NEO_RGBW + NEO_KHZ800);

#include "Carriage.h"
Carriage carriage;

/* DEBUG ***************************************************************************/
boolean bToPlotter = false; // this boolean is set by Processing Plotter
boolean b2VVVV     = false;
boolean b2SC       = false; // when you want to have a SuperCollider sound feedback
boolean DEBUG      = false;
boolean bUseProcessing = false;
boolean bUseLEDs   = true;

/* UTILITY STUFF *******************************************************************/
int i=0, j=0;
const int DELAY_TIME = 10;

// SETUP ////////////////////////////////////////////////////////////////////////////

void setup()
{
  pinMode(13, OUTPUT); digitalWrite(13, LOW); // turn off the annoying LED
  
  // open the serial communication
  Serial.begin(9600, SERIAL_8N1);
  while(!Serial) {}

  if( DEBUG ) Serial.println("Starting!");

  int TOTALNUMBERPADS = 0;
  for(int i=0; i<NMPR; i++) {
    TOTALNUMBERPADS += NPADS[i];
  }
  NVIRTUALPADS = (TOTALNUMBERPADS * 2) - 1;

  if(DEBUG) {
    Serial.print("NMPR: ");
    Serial.print( NMPR );
    Serial.print(", NPADS tot: ");
    Serial.print( TOTALNUMBERPADS );
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

    // TODO: 2019-10-02 
    // use some trick from FLOS capacitive wall here

    // MHD, NHD, NCL, FDL
    mpr[i].cap.setFalling( 4, 4, 2, 1 ); // 1, 1, 1, 1
    mpr[i].cap.setRising( 1, 1, 1, 1 );
    // if touch timing is too short there will be a moment in which 
    // an automatic release will be interpreted as a user release 
    // so a change in the pad index :(
    mpr[i].cap.setTouched( 1, 1, 32 );
    mpr[i].cap.setThresholds( 25, 5 ); // originariamente: 18, 9
    mpr[i].cap.setDebounces(2, 2);

    // initial reset of MPR struct fields
  }

  // LED STUFF *********************************************************************/
  //if(bUseLEDs) carriage.init(&strip, NVIRTUALPADS);
  if(bUseLEDs) carriage.init(&strip, 25);
}


// LOOP /////////////////////////////////////////////////////////////////////////////

void loop()
{
  // get data from serial port
  getSerialData();

  // CAPACITIVE STUFF **************************************************************/
  for(int i=0; i<NMPR; i++)  // cycle through all the MPR
  {
    // Get the currently touched pads
    mpr[i].currtouched = mpr[i].cap.touched();
   
    if( mpr[i].currtouched != mpr[i].lasttouched ) 
    {
      // do the following only if something has changed
      //Serial.println(mpr[i].currtouched, BIN);
      
      // interpolated Pad is the virtual indexes we obtain from the 'Touch obj'.
      virtualPadIndex = -1;
      padIndex = -1;
      for(int j=0; j<NPADS[i]; j++) // cycle through all the electrodes
      {
        if (( mpr[i].currtouched & _BV(j)) && !(mpr[i].lasttouched & _BV(j)) )
        {
          // pad 'j' has been touched
          mpr[i].bPadState[j] = true;

          padIndex = composeIndex(i, j);
          touchObj.addIndex( padIndex );
          virtualPadIndex = touchObj.getInterpolatedIndex();
          
          newIdx = virtualPadIndex;
          //newIdx = padIndex;
          bTouchStabilized = false;

          if( bUseProcessing )
            printAllSensors();

          if( DEBUG ) {
            Serial.print("P - "); // P for pressed
            printAllSensors();
            Serial.print("REAL: ");
            Serial.print( padIndex );
            Serial.print(", VIRTUAL: ");
            Serial.print( virtualPadIndex );
            Serial.println();
          }

          if( b2VVVV ) {
            sentToVVVV( virtualPadIndex );
          }

          if( b2SC && virtualPadIndex!=-1) {
            Serial.print( virtualPadIndex );
            Serial.print('a');
            //Serial.println();
          }

          if(bUseLEDs) {
            //carriage.setNewPos( virtualPadIndex );
            carriage.setNewPos( newIdx );
            if( DEBUG ) {
              carriage.debug();
            }
          }
        }
        else if (!(mpr[i].currtouched & _BV(j)) && (mpr[i].lasttouched & _BV(j)) )
        {
          // pad 'i' has been released
          mpr[i].bPadState[j] = false;

          padIndex = composeIndex(i, j);
          
          touchObj.removeIndex( padIndex );
          virtualPadIndex = touchObj.getInterpolatedIndex();

          newIdx = virtualPadIndex;
          //newIdx = padIndex;
          bTouchStabilized = false;

          if( bUseProcessing )
            printAllSensors();

          if( DEBUG ) {
            Serial.print("R - "); // R for released
            printAllSensors();
            Serial.print("REAL: ");
            Serial.print( padIndex );
            Serial.print(", VIRTUAL: ");
            Serial.print( virtualPadIndex );
            Serial.println();
          }

          if( b2VVVV ) {
            sentToVVVV( virtualPadIndex );
          }

          if( b2SC && virtualPadIndex!=-1) {
            Serial.print( virtualPadIndex );
            Serial.print('a');
            //Serial.println();
          }

          if( bUseLEDs ) {
            //carriage.setNewPos( virtualPadIndex );
            carriage.setNewPos( newIdx );
            if( DEBUG ) {
              carriage.debug();
            }
          }
        }
      }
      // reset our state
      mpr[i].lasttouched = mpr[i].currtouched;
    }
    
    mpr[i].oor = mpr[i].cap.getOOR();
    
    // SEND DATA TO PROCESSING *****************************************************/
    if( bToPlotter ) {
      sendDataToProcessingPlotter(i);
    }
    //mpr[i].cap.printOOR(); // added for debug purposes
  }

  if( bUseLEDs ) carriage.update();

  

  // put here a function to control the timing 
  // to send send messages to VVVV
  if(newIdx != -1) {
    
    if(newIdx != oldIdx ) {
      oldIdx = newIdx;
      startWaiting = millis();    
    }

    if(millis() - startWaiting > waitToSend && !bTouchStabilized)
    {
      bTouchStabilized = true;
      //Serial.print("touch stabilized - ");
      Serial.write( newIdx );
      //Serial.println();
    }
  }

  delay(DELAY_TIME); // put a delay so it isn't overwhelming
}

/************************************************************************************
 * SERIAL UTILITIES
 ***********************************************************************************/
void getSerialData()
{
  if(Serial.available()) 
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
}

/*
// SERIAL EVENT /////////////////////////////////////////////////////////////////////
// This function cannot be used on Arduino micro
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
*/

/************************************************************************************
 * COMPOSE INDEX
 ***********************************************************************************/
int composeIndex(int mprIndex, int padIndex) {
  int acc = 0;
  for(int i=0; i<mprIndex; i++) {
    acc += NPADS[i];
  }
  return acc + padIndex;  
}


/************************************************************************************
 * PRINT ALL SENSORS
 ***********************************************************************************/
void printAllSensors()
{
  // cycle through all the mpr
  for(int i=0; i<NMPR; i++)
  {
    // cycle through all the electrodes
    for(int j=0; j<NPADS[i]; j++)
    {
      int state = (mpr[i].currtouched & _BV(j)) >> j;
      Serial.print( state );
    }
  }
  Serial.println(";");
}

/************************************************************************************
 * SEND TO VVVV
 ***********************************************************************************/
void sentToVVVV(int activeVirtualPad) {
  /*
  for( int i=0; i<NVIRTUALPADS; i++) {
    if(i == activeVirtualPad)
      Serial.print(1);
    else
      Serial.print(0);
  }
  Serial.println();
  */
  Serial.println( activeVirtualPad );
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
  for(int j=0; j<NPADS[mprIndex]; j++)
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
