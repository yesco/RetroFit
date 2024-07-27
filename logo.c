#include "jio.c"
#include "graphics.c"
#include <math.h>

// TODO: use gx and gy?

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

float angle(float d) {
  return rad= 2*pi*d/360;
}

float left(float d) {
  return rad-= 2*pi*d/360;
}

float right(float d) {
  return rad+= 2*pi*d/360;
}

int xc, yc;

void center() {
  x= xc= gsizex/2;
  y= yc= gsizey/2;
  rad= 0;
  c= white;
}

// plot a Function in x=[xa,xb] with STeP, on y=[ya,yb] using Color
void mygraph(float xa, float xb, float stp, float ya, float yb, double f(double), int c) {
  for(float x= xa; x<=xb; x+= stp) {
    float y= f(x);
    gset(x/(xb-xa)*2*xc+xc, y/(yb-ya)*2*yc+yc, c);
    gupdate();
  }
}

void myclock() {
  int h= 1, m= 45, s= 0;

  int w= gsizex/2;
  if (gsizey/2 < w) w= gsizey/2;
  w-= 3;
  
  wclear();
  
  while(peekey()<0) {
    //usleep(1000*1000);
    usleep(1000*1000/10);

    s++;
    if (s>=60) m++,s=0;
    if (m>=60) h++,m=0;
    
    gclear();
    
    center(); left(90);
    right(h*360/12);
    c= red;
    forward(w/2);
      
    center(); left(90);
    right(m*360/60);
    c= cyan;
    forward(w*0.8);
	    
    center(); left(90);
    right(s*360/60);
    c= white;
    forward(w);

    gupdate();
  }
}

int main(void) {
  jio();
  cursoroff();
  
  pixels_per_col= 2;
  //pixels_per_col= 1; // TODO: 1
  pixels_per_row= 2;
  gcanvas();
  
  forward(10);

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

  // graphs w axis
  wclear();
  center();

  // draw axis
  center(); angle(  0); forward(xc); {
    gx= x-8; gy=y; gputc('x');
  }
  center(); angle( 90); forward(xc); {
    gx= x-8; gy=0; gputc('y');
  }    
  center(); angle(180); forward(xc);
  center(); angle(270); forward(xc);
  
  mygraph(-10,10, 0.01, -1.2,1.2, sin, red);
  mygraph(-10,10, 0.1, -1.2,1.2, cos, green);
  mygraph(-10,10, 0.01, -1.2,1.2, tan, cyan);

  gupdate();
  key();

  // draw a ticking clock
  myclock();

  cursoron();
  exit(0);
}
