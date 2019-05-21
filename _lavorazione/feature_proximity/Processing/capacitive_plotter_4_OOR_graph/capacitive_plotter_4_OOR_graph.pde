/*
* This sketch is meant to work with the Arduino
* 'sketch_6_proximity_test'.
*/

// serial stuff
import processing.serial.*;
Serial s_port;
boolean bSerialListen = false;
boolean touched = false;

int NGRAPHS = 1;
PadGraph padGraphs[];

int mprAddr = -1;
int padAddr = -1;
int datacount = -1;

PFont font;

color base  = color(0, 168, 255);
color filt  = color(240, 0, 0);
color touch = color(255, 0, 0);
color lines = color(220, 220, 220, 120);
color bckgr = color(30, 30, 30);

OOR oor;
Touch t;


// SETUP ////////////////////////////////////////////////////////////////////////////
void setup()
{
  size(1000, 720);
  frameRate(30);
  smooth();

  // initialize the graphs
  padGraphs = new PadGraph[NGRAPHS];
  for(int i=0; i<NGRAPHS; i++) {
     padGraphs[i] = new PadGraph(0, i*height / NGRAPHS,height / NGRAPHS);
  }

  oor = new OOR(width-300, 20);
  t = new Touch(width-300, 40);
  
  // serial stuff
  println(Serial.list());
  s_port = new Serial(this, Serial.list()[0], 9600);
  //s_port.buffer( 2 );

  // graphics stuff
  font = loadFont("Monospaced-32.vlw");
  textFont(font, 14);
}

// DRAW /////////////////////////////////////////////////////////////////////////////
void draw()
{
  background(bckgr);

  if( bSerialListen )
  {
    for(int i=0; i<NGRAPHS;i++) {
      padGraphs[i].display();
    }
    
    oor.update();
    oor.display();
    
    t.update();
    t.display();
  }
  draw_other_info();
}

// SERIAL EVENT /////////////////////////////////////////////////////////////////////
// We are reaading pairs of 'ids' and 'values'
// id is always between 128 and 255.
void serialEvent(Serial s)
{
  int b = s.read();
  //println(b);
  
  if(b>127)
  {
    // this is a 'Status Byte' and so it contains
    // the mpr address.
    mprAddr = b & 0x03; // discard 6 MSB
    datacount = 0;
  }
  else
  {
    // These are data bytes (value<=127)
    // datacount values from 4 to 28
    // are information related to pad 0 through 11
    // The number or datacount is double than the number of pads
    // because we are transmitting both the filtered and the 
    // baseline values.
    switch(datacount)
    {
      case 0:
        t.pt1 = b;
        datacount++;
        break;
      case 1:
        t.pt2 = b;
        datacount++;
        break;
        
      case 2:
        oor.pt1 = b;
        datacount++;
        break;
      case 3:
        oor.pt2 = b;
        datacount++;
        break;
        
      case 4:
        padAddr = 0;
        padGraphs[ mprAddr*12 + padAddr ].setBase(b<<3);
        datacount++;
        break;
      case 5:
        padAddr = 0;
        padGraphs[ mprAddr*12 + padAddr ].setFilt(b<<3);
        padGraphs[ mprAddr*12 + padAddr ].update();
        datacount++;
        break;
        
      case 6:
        padAddr = 1;
        padGraphs[ mprAddr*12 + padAddr ].setBase(b<<3);
        datacount++;
        break;
      case 7:
        padAddr = 1;
        padGraphs[ mprAddr*12 + padAddr ].setFilt(b<<3);
        padGraphs[ mprAddr*12 + padAddr ].update();
        datacount++;
        break;
        
      case 8:
        padAddr = 2;
        padGraphs[ mprAddr*12 + padAddr ].setBase(b<<3);
        datacount++;
        break;
      case 9:
        padAddr = 2;
        padGraphs[ mprAddr*12 + padAddr ].setFilt(b<<3);
        padGraphs[ mprAddr*12 + padAddr ].update();
        datacount++;
        break;
        
      case 10:
        padAddr = 3;
        padGraphs[ mprAddr*12 + padAddr ].setBase(b<<3);
        datacount++;
        break;
      case 11:
        padAddr = 3;
        padGraphs[ mprAddr*12 + padAddr ].setFilt(b<<3);
        padGraphs[ mprAddr*12 + padAddr ].update();
        datacount++;
        break;
        
      case 12:
        padAddr = 4;
        padGraphs[ mprAddr*12 + padAddr ].setBase(b<<3);
        datacount++;
        break;
      case 13:
        padAddr = 4;
        padGraphs[ mprAddr*12 + padAddr ].setFilt(b<<3);
        padGraphs[ mprAddr*12 + padAddr ].update();
        datacount++;
        break;
        
      case 14:
        padAddr = 5;
        padGraphs[ mprAddr*12 + padAddr ].setBase(b<<3);
        datacount++;
        break;
      case 15:
        padAddr = 5;
        padGraphs[ mprAddr*12 + padAddr ].setFilt(b<<3);
        padGraphs[ mprAddr*12 + padAddr ].update();
        datacount++;
        break;
        
      case 16:
        padAddr = 6;
        padGraphs[ mprAddr*12 + padAddr ].setBase(b<<3);
        datacount++;
        break;
      case 17:
        padAddr = 6;
        padGraphs[ mprAddr*12 + padAddr ].setFilt(b<<3);
        padGraphs[ mprAddr*12 + padAddr ].update();
        datacount++;
        break;
        
      case 18:
        padAddr = 7;
        padGraphs[ mprAddr*12 + padAddr ].setBase(b<<3);
        datacount++;
        break;
      case 19:
        padAddr = 7;
        padGraphs[ mprAddr*12 + padAddr ].setFilt(b<<3);
        padGraphs[ mprAddr*12 + padAddr ].update();
        datacount++;
        break;
        
      case 20:
        padAddr = 8;
        padGraphs[ mprAddr*12 + padAddr ].setBase(b<<3);
        datacount++;
        break;
      case 21:
        padAddr = 8;
        padGraphs[ mprAddr*12 + padAddr ].setFilt(b<<3);
        padGraphs[ mprAddr*12 + padAddr ].update();
        datacount++;
        break;
        
      //TODO: add more cases in accordance to the number of pads we are using.
        
      default:
        // do nothing
        break;
    }
  }
}

// KEY PRESSED //////////////////////////////////////////////////////////////////////
void keyPressed()
{
  if (key == 'o' || key == 'O')
  {
    s_port.write('o');
    bSerialListen = true;
  }
  else if( key == 'c' || key == 'C')
  {
    s_port.write('c');
    bSerialListen = false;
  }
  else if (key == 'r' || key == 'R')
  {
    s_port.write('r');
    for(int i=0; i<NGRAPHS; i++)
    {
      padGraphs[i].reset();
    }
  }
}

// CUSTOM ///////////////////////////////////////////////////////////////////////////
void draw_other_info()
{
  pushStyle();
  textAlign(CENTER);
  if( bSerialListen )
    text("Serial: ON", width/2, 20);
  else
    text("Serial: OFF", width/2, 20);
  popStyle();
}
