class Pad
{
  PVector pos;
  PVector size;
  boolean status;
  int index;
  
  Pad(int _index, float _x, float _y, float _w, float _h)
  {
    index = _index;
    pos = new PVector(_x, _y);
    size= new PVector(_w, _h);
    
  }
  
  void update(int _status)
  {
    if(_status == 1)
      status = true;
    else
      status = false;
  }
  
  void display()
  {
    pushMatrix();
   
    translate(pos.x, pos.y);
    stroke(255, 0, 0);
    if( status )
       fill(255, 0, 0);
    else
      noFill();
    rect(0, 0, size.x, size.y);
    
    fill(255);
    text(index, size.x*0.5, size.y*0.5); 
    
    popMatrix();
  }
  
  boolean getStatus() {
    return status;
  }
}
