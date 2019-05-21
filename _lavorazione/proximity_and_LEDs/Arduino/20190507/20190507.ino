/*
 * 2018/03/28 - Nicola Ariutti
 * This sketch uses a MPR121 with 9 capacitive sensors.
 * At the same time it drives a 46 DotStar LED strip.
 * 
 * The sketch shows an animation of a light spot on the strip
 * according to which capacitive pad the hand is on.
 */


/* CAPACITIVE STUFF ****************************************************************/
#include <Wire.h>
#include "Limulo_MPR121.h"

#define NMPR 1
#define NPADS 12
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


/* LED STUFF ***********************************************************************/
// this is the order of the colors GREEN, RED, BLUE
//#include <Adafruit_DotStar.h>

#include "Adafruit_NeoPixel.h"
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif


// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    4

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 45 // 60

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)


/*
#include <SPI.h>         // see Adafruit DotStar library example to better understand

#define NLEDS 46 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    4
#define CLOCKPIN   5
Adafruit_DotStar strip = Adafruit_DotStar(NLEDS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
*/

/* ANIMATION / VEHICLE *************************************************************/
#include <NicolaAriutti_Vehicle.h>
NicolaAriutti_Vehicle vehicle;

// must define sidelobe dimension first
const int SIDELOBS = 4;
// the we declare an array that will work as a lookup table
// for the squared-raised-cosine figure.
float lookup [SIDELOBS];

int centroidPads = 0;  // an integer from 0 to NPADS
int centroidLeds = 0;  // an integer from 0 to NLEDS
int blue = 0;


/* UTILITY STUFF *******************************************************************/
int i=0, j=0;
const int DELAY_TIME = 10;


/************************************************************************************
 * SETUP
 ***********************************************************************************/
void setup()
{
  pinMode(13, OUTPUT); digitalWrite(13, LOW); // turn off the annoying LED
  
  // open the serial communication
  Serial.begin(115200, SERIAL_8N1);

  
  // CAPACITIVE STUFF **************************************************************/
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
    
    // First Filter Iteration
    // Second Filter Iteration
    // Electrode Sample Interval
    // NOTE: the system seems to behave better if
    // these value are more than 0 
    mpr[i].cap.setFFI_SFI_ESI(1, 1, 1);  // See AN3890


    // MHD, NHD, NCL, FDL
    mpr[i].cap.setFalling( 1, 1, 1, 1 );
    mpr[i].cap.setRising( 1, 1, 1, 1 );
    mpr[i].cap.setTouched( 0, 0, 0 );
    mpr[i].cap.setThresholds( 18, 9);
    mpr[i].cap.setDebounces(2, 2);
  }

  bToPlotter=false;


  // LED STUFF *********************************************************************/
  strip.begin(); // Initialize pins for output

  // set every pixel to sleep
  for(i=0; i<NLEDS; i++) 
  {
    strip.setPixelColor(i, 0x00, 0x00, 0x00, 0x00);
  }
  strip.show();  // Turn all LEDs off ASAP 

  // fill the lookup table
  for(i=0; i<SIDELOBS; i++)
  {
    int x = i+1;
    lookup[i] = 0.5 * (cos ( (PI*x)/SIDELOBS ) + 1 );
    lookup[i] = lookup[i]*lookup[i];
  } 


  // ANIMATION / VEHICLE ***********************************************************/
  // We must calculate this parameter with extreme precision
  // also taking into account the number of leds in the strip
  // and the DELAY_TIME used for each loop cylce.
  // Initial position, maxspeed, maxforce, damp distance.
  vehicle.init(0, 0.5, 0.1, 1.5);
}

/************************************************************************************
 * LOOP
 ***********************************************************************************/
void loop()
{
  // TODO: Add here serial communication section in order
  // to discard the serial event callback function


  // CAPACITIVE STUFF **************************************************************/
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
        //Serial.print("Pad:"); Serial.print(j); Serial.println("touched!"); 
        //blocks[j].asr.triggerAttack();
        centroidPads = j; // TODO: maybe the correct formula is 12*i + j
        centroidLeds = ((1.0 * centroidPads)/NPADS) * NLEDS;
        vehicle.setTarget(centroidLeds);
      }
      if (!(mpr[i].currtouched & _BV(j)) && (mpr[i].lasttouched & _BV(j)) )
      {
        // pad 'i' has been released
        //Serial.println("Pad 0 :\treleased!");
        //blocks[j].asr.triggerRelease();
      }
    }
    // reset our state
    mpr[i].lasttouched = mpr[i].currtouched;

    mpr[i].oor = mpr[i].cap.getOOR();

    // ### Send Serial data ### //
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


  // VEHICLE STUFF *****************************************************************/
  // calculate the scaled centroid
  
  vehicle.update();
  int pos = vehicle.getPosition();
  //Serial.println(pos);

  
  // LED STUFF *********************************************************************/
  // The logic below is created in order to properly light-up
  // leds around the centroid according to the 
  // squared raised-cosine lookup table.
  strip.clear(); // Set all pixel colors to 'off'
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
