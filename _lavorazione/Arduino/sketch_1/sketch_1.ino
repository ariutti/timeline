/*
* This sketch is meant to work with the
* "capacitive_plotter" sketch for Processing.
*/

#include <Wire.h>
#include "Limulo_MPR121.h"

#define BOARD 13
long actualTime, previousTime;
boolean bBoardLed = false;


#define LED_VERDE  52
#define LED_GIALLO 50
#define LED_ROSSO  48
boolean bLedState[3] = {false, false, false};

#define PUSH 53

#define TOUCH_WHEEL 0
#define TOUCH_GEAR 1

// You can have up to 4 on one i2c bus but one is enough for testing!
Limulo_MPR121 cap = Limulo_MPR121();

// Keeps track of the last pins touched so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

boolean bToPlotter;

int delayTime = 50;


// SETUP ////////////////////////////////////////////////////////////////////////////
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
  
  //cap.setRising( 1, 8, 1, 0);
  //cap.setFalling( 1, 1, 2, 1);
  //cap.setTouched( 0, 0, 0);
  //cap.setThresholds( 150, 50 );
  //cap.setDebounces(1, 1); 
  
  bToPlotter=false;
  
  pinMode( PUSH, INPUT );
  pinMode( LED_VERDE,  OUTPUT );
  pinMode( LED_GIALLO, OUTPUT );
  pinMode( LED_ROSSO,  OUTPUT );
  pinMode( BOARD,  OUTPUT );
  digitalWrite( BOARD, LOW );
  
  previousTime = millis();
  
}


// LOOP /////////////////////////////////////////////////////////////////////////////
void loop()
{
  actualTime = millis();
  if( actualTime - previousTime > 1000) {
    bBoardLed = !bBoardLed;
    previousTime = actualTime;
  }
  digitalWrite( BOARD, bBoardLed );
  
  
  currtouched = cap.touched(); // Get the currently touched pads
  
  // GEAR TOUCH DETECTION: Is there someone who is touching the gear?
  if ((currtouched & _BV(TOUCH_GEAR)) && !(lasttouched & _BV(TOUCH_GEAR)) ) {
    //Serial.println("GEAR:\ttouched!");
    bLedState[ TOUCH_GEAR ] = true;
  }
  if (!(currtouched & _BV(TOUCH_GEAR)) && (lasttouched & _BV(TOUCH_GEAR)) ) {
    //Serial.println("GEAR:\treleased!");
    bLedState[ TOUCH_GEAR ] = false;
  }
  
  // STEERING WHEEL TOUCH DETECTION: Is there someone who is touching the steering wheel?
  if ((currtouched & _BV(TOUCH_WHEEL)) && !(lasttouched & _BV(TOUCH_WHEEL)) ) {
    //Serial.println("WHEEL:\ttouched!");
    bLedState[ TOUCH_WHEEL ] = true;
  }
  if (!(currtouched & _BV(TOUCH_WHEEL)) && (lasttouched & _BV(TOUCH_WHEEL)) ) {
    //Serial.println("WHEEL:\treleased!");
    bLedState[ TOUCH_WHEEL ] = false;
  }

  // reset our state
  lasttouched = currtouched;
  
  if( digitalRead( PUSH ) )
  {
    //Serial.println("PUSH");
    bLedState[2] = false;
  }
  else
  {
    //Serial.println("DE-PUSH");
    bLedState[2] = true;
  }
  
  digitalWrite( LED_VERDE,  bLedState[ TOUCH_GEAR ] );
  digitalWrite( LED_GIALLO, bLedState[ TOUCH_WHEEL ] );
  digitalWrite( LED_ROSSO,  bLedState[2] );
  
  // Send Serial data ////////////////////////////////////////////////////////////
  int filt0 = cap.filteredData(TOUCH_WHEEL);
  int base0 = cap.baselineData(TOUCH_WHEEL);
  int filt1 = cap.filteredData(TOUCH_GEAR);
  int base1 = cap.baselineData(TOUCH_GEAR);
  //int delta= base - filt;
  
  if(bToPlotter)
  {
    // first electrode
    Serial.write(7 + 128);
    Serial.write(base0 / 8);
    Serial.write(8 + 128);
    Serial.write(filt0 / 8);
    Serial.write(9 + 128);
    Serial.write( bLedState[0] );
    
    // second electrode
    Serial.write(10 + 128);
    Serial.write(base1 / 8);
    Serial.write(11 + 128);
    Serial.write(filt1 / 8);
    Serial.write(12 + 128);
    Serial.write( bLedState[1] );
    /*
    Serial.print(base0); Serial.print(",");
    Serial.print(filt0); Serial.print(",");
    Serial.print(base1); Serial.print(",");
    Serial.print(filt1); Serial.println();
    */
  }
      
//  Serial.print("Filt: ");
//  for (uint8_t i=0; i<12; i++) {
//    Serial.print(cap.filteredData(i)); Serial.print("\t");
//  }
//  Serial.println();
//  
//  Serial.print("Base: ");
//  for (uint8_t i=0; i<12; i++) {
//    Serial.print(cap.baselineData(i)); Serial.print("\t");
//  }
//  Serial.println();
  
  
  if( bLedState[ TOUCH_WHEEL ] )
  {
    tone(8, 440 );
    delay(delayTime);
    noTone(8);
  }
  else
  {
    delay(delayTime); // put a delay so it isn't overwhelming
  }
}

// SERIAL EVENT /////////////////////////////////////////////////////////////////////
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
