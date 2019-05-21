class Touch {
  
  PVector pos;
  int L = 10;
  int N=13;
  boolean status[];
  int pt1;
  int pt2;
  
  // CONSTRUCTOR /////////////////////////////////////////
  Touch(int _x, int _y)
  {
    status = new boolean[N];
    for(int i=0; i<N;i++)
    {
      status[i] = false;
    }
    pos = new PVector(_x, _y);
  }
  
  
  void display()
  {
    pushStyle();
    pushMatrix();
    
    stroke(0);
    translate(pos.x, pos.y);

    for(int i=0; i<N; i++)
    {
      if(status[i])
      {
        fill(255, 0, 0);
      }
      else
      {
        noFill();
      }
      pushMatrix();
      translate(i*L, 0);
      rect(0, 0, L, L);
      popMatrix();
    }
   
    textAlign(LEFT);
    fill(255);
    text("TOUCH", 13*L+10, L);
   
    popMatrix();
    popStyle();
  }
  
  void update()
  {
    updatePt1( pt1 );
    updatePt2( pt2 );
  }
  
  void updatePt1(int _b)
  {
    // the data byte contains only 7 bit of information
    int c = 0;
    for(int i=0; i<7; i++)
    {
      c = (1<<i);
      c = _b & c;
      c = (c>>i);
      status[i] = (c==1);
    }
  }
  
  void updatePt2(int _b)
  {
    // the data byte contains only 7 bit of information
    int c = 0;
    for(int i=0; i<6; i++)
    {
      c = (1<<i);
      c = _b & c;
      c = (c>>i);
      status[i+7] = (c==1);
    }
  }
  
}