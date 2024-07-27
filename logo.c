#include "jio.c"
#include "graphics.c"
#include <math.h>

int x= 0, y= 0, c= white;
float rad= 0;

int forward(int n) {
  int dx= n*cos(rad);
  int dy= n*sin(rad);
  gline(x, y, dx, dy, c);
  x+= dx;
  y+= dy;
  return n;
}

float pi= 3.14; // TODO: fix

float left(float d) {
  return rad-= 2*pi*d/360;
}

float right(float d) {
  return rad+= 2*pi*d/360;
}

void myclock() {
  int h= 1, m= 45, s= 0;

  int w= gsizex/2;
  if (gsizey/2 < w) w= gsizey/2;
  w-= 3;
  
  while(peekey()<0) {
    s++;
    if (s>=60) m++,s=0;
    if (m>=60) h++,m=0;
    
    wclear();
    
    x= gsizex/2; y= gsizey/2;
    rad= 0; left(90);
    right(h*360/12);
    c= red;
    forward(w/2);
      
    x= gsizex/2; y= gsizey/2;
    rad= 0; left(90);
    right(m*360/60);
    c= cyan;
    forward(w*0.8);
	    
    x= gsizex/2; y= gsizey/2;
    rad= 0; left(90);
    right(s*360/60);
    c= white;
    forward(w);

    gupdate();

    //usleep(1000*1000);
    usleep(1000*1000/10);
  }
}

int main(void) {
  jio();
  cursoroff();
  
  pixels_per_col= 2;
  //pixels_per_col= 1; // TODO: 1
  pixels_per_row= 2;
  
  gcanvas();
  
  // TODO: 1
  x= gsizex/2;
  y= gsizey/2;

  forward(10);
  //left(90);
  //c= green;
  //forward(10);
  //gline(x,y,0,+30,c);
  //gline(x,y,10,-10,c);
  //gline(x,y,0,-10,c);
  //gline(x,y,0,10,c);
  //left(90);

  c=green;
  forward(10);

  left(45);

  c=red;
  forward(10);

  right(45+90);
  c=cyan;
  forward(9);

  //gline(x,y,0,10,c);
  //gline(x,y,+10,0,green);
  //gline(x,y,0,10,c);
  //gline(x,y,+10,0,green);
  //x+=10; y+=0;
  
  gupdate();

  myclock();

  cursoron();
  exit(0);
}
