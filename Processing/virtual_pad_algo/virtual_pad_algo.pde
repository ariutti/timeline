final int NPADS = 25;
Pad pads[];

final int NVIRTUALPADS = (NPADS*2)-1;
VirtualPad vPads[];

// serial stuff
import processing.serial.*;
Serial myPort;
float MARGIN = 5.0;

PFont font;

String myString = null;

Touch touchObj;

// SETUP //////////////////////////////////////////////////////////////
void setup() 
{
  size(1400, 200);
  smooth();
  
  font = createFont("Monofonto-48", 18);
  textFont(font);
  
  touchObj = new Touch();
  
  /*
  printArray(Serial.list());
  // Open the port you are using at the rate you want:
  myPort = new Serial(this, Serial.list()[0], 9600);
  myPort.clear();
  */
  
  float padW = (width*1.0 - ((NPADS+1)*MARGIN)) / (NPADS*1.0);
  float padH = height*1.0 - (2*MARGIN);
  pads = new Pad[NPADS];
  for(int i=0; i<NPADS; i++)
  {
    pads[i] = new Pad(i, MARGIN + (i*(padW+MARGIN)), MARGIN, padW, padH);
  }
}


// DRAW ///////////////////////////////////////////////////////////////
void draw() 
{
  background(0);
  for(int i=0; i<NPADS; i++) {
    pads[i].display();
  }
}

// SERIAL FUNC ////////////////////////////////////////////////////////
void serialFunc()
{
  while (myPort.available() > 0) {
    myString = myPort.readStringUntil('\n');
    
    if (myString != null) {
      //println(myString);
      //println(myString.length());
      for(int i=0; i<myString.length()-3; i++) {
        //print( int(myString.charAt(i) )-48 );
        pads[i].update( int(myString.charAt(i) )-48 );
      }
    }
  }
}

void keyPressed()
{
  if(key>=48 && key<58) 
  {
    int index = int(key)-48;
    
    if( !pads[ index ].getStatus()) {
      pads[ index ].update(1);
  
      touchObj.addIndex( index );
      int virtualPadIndex = touchObj.getInterpolatedIndex();
      println( virtualPadIndex );
    }
  
    //print("pressed: ");
    //println( index ); 
  }
}

void keyReleased()
{ 
  if(key>=48 && key<58) 
  {
    int index = int(key)-48;
    if( pads[ index ].getStatus()) {
      pads[ index ].update(0);
      touchObj.removeIndex( index );
      int virtualPadIndex = touchObj.getInterpolatedIndex();
      println( virtualPadIndex );
    }
    
    //print("released: ");
    //println( index );
  }
}
