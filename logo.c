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

  c=blue;
  forward(10);

  //gline(x,y,0,10,c);
  //gline(x,y,+10,0,green);
  //gline(x,y,0,10,c);
  //gline(x,y,+10,0,green);
  //x+=10; y+=0;
  
  gupdate();

  cursoron();
  exit(0);
}
