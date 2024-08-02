#include "jio.c"
#include "graphics.c"
#include <math.h>

// TODO: use gx and gy?

int c= white, pen= 1, turtle= 1, size= 1;
float rad= 0;

int forward(int n) {
  int dx= n*cos(rad);
  int dy= n*sin(rad);
  gline(gx, gy, dx, dy, c);
  gx+= dx;
  gy+= dy;
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
  gx= xc= gsizex/2;
  gy= yc= gsizey/2;
  rad= 0;
  c= white;
}

// plot a Function in x=[xa,xb] with STeP, on y=[ya,yb] using Color
void graph(float xa, float xb, float stp, float ya, float yb, double f(double), int c) {
  for(float x= xa; x<=xb; x+= stp) {
    float y= f(x);
    gset(x/(xb-xa)*2*xc+xc, y/(yb-ya)*2*yc+yc, c);
    gupdate();
  }
}

// graphs w axis and label
void mygraph() {
  wclear();
  center();

  // draw axis
  //center(); angle(  0); forward(xc); {
  center(); forward(xc); {
    gx-= 8; gputc('x');
  }
  center(); left(90); forward(xc); {
    gx-= 8; gy=0; gputc('y');
  }    

  // TODO: these two doesn't work???
  center(); angle(180); forward(xc);
  center(); angle(270); forward(xc);

  center(); right(180); forward(xc);
  center(); left(90); forward(xc);
  
  graph(-10,10, 0.01, -1.2,1.2, sin, red);
  graph(-10,10, 0.1, -1.2,1.2, cos, green);
  graph(-10,10, 0.01, -1.2,1.2, tan, cyan);

  gupdate();
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

int argdef(char** p, int def) {
  if (!p || !*p) return 0;
  char* s = *p;
  int r= 0;
  while(isdigit(*s)) {
    r= 10*r + *s++-'0';
  }
  if (s== *p && *s) {
    if (islower(*s)) { // variable...
      assert(!"TODO");
    } else if (*s=='$') { // arg
      assert(!"TODO");
    } else if (*s=='*') { // pop stack?
      assert(!"TODO");
    }
  }
  if (s==*p) r= def;
  *p= s;
  return r;
}

int arg(char** p) {
  return argdef(p, 0);
}

// LOGO single char commands
// used: J K N Q (T) V (X) (Y)
//   Forward
//   Up (Pen)
//   Down (Pen)
//   Show (turtle)
//   Hide (turtle)
//   Left (default 90)
//   Right (default 90)
//   Angle (default 90/up)
//  cOlor (default white)
// siZe (default 1)
//   #repeat N [...]
//   Wait (default 1000ms)
//   Erase
//   I - cewnter lol

// TODO: how to say filled?
//   Circle
//   Box
//   Graphics (bitblt/"sprite")
//   Move x y
//   X45 Y27
//   Pattern
//   Text

void logo(char* p) {
  while(*p) {
    //printf(">%s\n", p);
    switch(*p++) {
    case ' ': case '\t': case '\n': break;
    case '[': break;
    case ']': case 0: return;
    case 'F': forward(arg(&p)); break;
    case 'E': gclear(); break;
    case 'U': pen= 0; assert(!"TODO"); break;
    case 'D': pen= 1; assert(!"TODO"); break;
    case 'H': turtle= 0; assert(!"TODO"); break;
    case 'S': turtle= 1; assert(!"TODO"); break;
    case 'L': left(argdef(&p, 90)); break;
    case 'R': right(argdef(&p, 90)); break;
    case 'A': angle(argdef(&p, 90)); break;
    case 'O': c= argdef(&p, white); break; // TODO: cOlor names
    case 'Z': size= argdef(&p, 1); break;
    case '#': { int n= arg(&p); for(int i=1; i<=n; i++) logo(p); }
    case 'W': usleep(argdef(&p, 1000)*1000); break;
    case 'I': center(); break;
      //case 'C': { int x=arg(&p), y=arg(&p), r=arg(&p); circle(x, y, r); break; }
    default: printf("\n%%ERROR.logo: %s\n", p);
    }
  }
}

int main(void) {
  jio();
  cursoroff();
  
  pixels_per_col= 2;
  //pixels_per_col= 1; // TODO: 1
  pixels_per_row= 2;
  gcanvas();
  
  switch(4) {
  case 1: {
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
    break; }

  case 2: {
    mygraph();
    key();
    break;
  }

  case 3: {
    myclock();
    break;
  }

  case 4: {
    logo("E");
    logo("I#4[F30L]");
    logo("I#90[F2L4]"); // "circle"
    gupdate();
    break;
  }
  }
  
  cursoron();
  exit(0);
}
