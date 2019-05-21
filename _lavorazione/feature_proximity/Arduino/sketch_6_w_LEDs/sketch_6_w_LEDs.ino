/*
* This sketch is meant to work with the
* "capacitive_plotter_4" sketch for Processing.
* Here we are testing a new protocol to send data 
* to Processing.
*/

#include <Wire.h>
#include <Limulo_MPR121.h>

#define NMPR 1
#define NPADS 9
#define FIRST_MPR_ADDR 0x5A

struct mpr121
{
  Limulo_MPR121 cap;
  uint8_t addr;
  // Save an history of the pads that have been touched/released
  uint16_t lasttouched = 0;
  uint16_t currtouched = 0;
  uint16_t oor=0;
};


// an array of mpr121! You can have up to 4 on the same i2c bus
mpr121 mpr[NMPR];

// utility variables to save values readed from MPR
boolean bToPlotter;
uint16_t filt;
uint16_t base;
byte b;
int oor;
int delayTime = 50;


// LED stuff
#include <Adafruit_DotStar.h>
#include <SPI.h>         // see Adafruit DotStar library example to better understand

#define NUMPIXELS 46 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    4
#define CLOCKPIN   5
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

// this is the order of the colors GREEN, RED, BLUE

#include <Limulo_Animator.h>

const int PPP = 5; //PIXELS PER PAD
const int  FADEIN_TIME = 500;
const int  FADEOUT_TIME = FADEIN_TIME;
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



/************************************************************************************
 * SETUP
 ***********************************************************************************/
void setup()
{
  // open the serial communication
  Serial.begin(115200, SERIAL_8N1);

  // turn off the annoying LED
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

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
    //Serial.println("MPR found!");
    
    // possibly initialize the mpr with some initial settings
    mpr[i].cap.setUSL(201);
    mpr[i].cap.setTL(180);
    mpr[i].cap.setLSL(130);
    mpr[i].cap.setFFI_SFI_ESI(0, 0, 0); // See AN3890

    // MHD, NHD, NCL, FDL
    mpr[i].cap.setFalling( 1, 1, 1, 1);
    mpr[i].cap.setRising( 1, 1, 1, 1);
    mpr[i].cap.setTouched( 0, 0, 0);
    mpr[i].cap.setThresholds( 8, 6 );
    mpr[i].cap.setDebounces(1, 0);
  }

  bToPlotter=false;

  // LED stuff
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
    // Get the currently touched pads
    mpr[i].currtouched = mpr[i].cap.touched(); 
    
    // cycle through all the electrodes
    for(int j=0; j<NPADS; j++)
    {
      if (( mpr[i].currtouched & _BV(j)) && !(mpr[i].lasttouched & _BV(j)) )
      {
        // pad 'i' has been touched
        //Serial.println("Pad 0:\ttouched!");
        blocks[j].asr.triggerAttack();
      }
      if (!(mpr[i].currtouched & _BV(j)) && (mpr[i].lasttouched & _BV(j)) )
      {
        // pad 'i' has been released
        //Serial.println("Pad 0 :\treleased!");
        blocks[j].asr.triggerRelease();
      }
    }
    // reset our state
    mpr[i].lasttouched = mpr[i].currtouched;

    mpr[i].oor = mpr[i].cap.getOOR();

    // Send Serial data ////////////////////////////////////////////////////////////
    if(bToPlotter)
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
    //mpr[i].cap.printOOR(); // added for debug purposes
  }

  // LED stuff //////////////////////////////////////////////////////////
  //Serial.println();
  for(i=0; i<NPADS; i++)
  {
    blocks[i].asr.update();
    //Serial.print( blocks[i].asr.getY() );Serial.print(" ");
    blocks[i].rawAmp = blocks[i].asr.getY();
    // better to sqare the 0.0 - 1.0 value in order to get a smoother transition
    blocks[i].rawAmp = blocks[i].rawAmp * blocks[i].rawAmp;
    blocks[i].intAmp = 255.0*blocks[i].rawAmp * AMPLITUDE;
    //Serial.print( blocks[i].amplitude );Serial.print(" ");
    //Serial.print( blocks[i].asr.getState() );Serial.print("\t");
    for(j=0; j<PPP; j++)
    {
      strip.setPixelColor(blocks[i].tail+j, 0x00, 0x00, blocks[i].intAmp);
    }
  }
  //Serial.println();
  strip.show();

  
  delay(delayTime); // put a delay so it isn't overwhelming
}

/************************************************************************************
 * SERIAL EVENT
 ***********************************************************************************/
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
